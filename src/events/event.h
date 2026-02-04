#pragma once
#include "event_ids.h"

#include <timeout/timer_interface_fwd.h>

namespace smp
{
	class Event_Mouse;
	class Event_Drag;

	enum class EventPriority
	{
		kNormal,
		kInput,
		kRedraw,
		kResize,
		kControl,
	};

	class PanelTarget final
	{
	public:
		PanelTarget(js_panel_window& panel);

		[[nodiscard]] HWND GetHwnd();

		[[nodiscard]] js_panel_window* GetPanel();
		void UnlinkPanel();

	private:
		js_panel_window* pPanel_ = nullptr;
		HWND hWnd_ = nullptr;
	};

	class Runnable
	{
	public:
		virtual ~Runnable() = default;
		virtual void Run() = 0;
	};

	class EventBase : public Runnable
	{
	public:
		EventBase(EventId id);
		virtual ~EventBase() = default;

		[[nodiscard]] virtual std::unique_ptr<EventBase> Clone();

		void SetTarget(std::shared_ptr<PanelTarget> pTarget);
		[[nodiscard]] EventId GetId() const;

		[[nodiscard]] virtual Event_Mouse* AsMouseEvent();
		[[nodiscard]] virtual Event_Drag* AsDragEvent();

	protected:
		const EventId id_;
		std::shared_ptr<PanelTarget> pTarget_;
	};

	class Event_Basic : public EventBase
	{
	public:
		Event_Basic(EventId id);

		void Run() override;
	};

	class Event_JsExecutor : public EventBase
	{
	public:
		Event_JsExecutor(EventId id);
		~Event_JsExecutor() override = default;

		void Run() final;
		virtual std::optional<bool> JsExecute(mozjs::JsContainer& jsContainer) = 0;
	};

	class Event_Timer : public EventBase
	{
	public:
		Event_Timer(std::shared_ptr<ITimer> pTimer, uint64_t generation);

		void Run() override;

	private:
		std::shared_ptr<ITimer> pTimer_;
		uint64_t generation_;
	};

	class Event_JsTask : public Event_JsExecutor
	{
	public:
		Event_JsTask(EventId id, std::shared_ptr<mozjs::JsAsyncTask> pTask);

		std::optional<bool> JsExecute(mozjs::JsContainer& jsContainer) override;

	private:
		std::shared_ptr<mozjs::JsAsyncTask> pTask_;
	};
}
