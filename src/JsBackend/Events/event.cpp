#include "PCH.hpp"
#include "event.h"

#include <Panel/js_panel_window.h>

namespace smp
{
	PanelTarget::PanelTarget(js_panel_window& panel) : pPanel_(&panel), hWnd_(panel.GetHWND()) {}

	HWND PanelTarget::GetHwnd()
	{
		return hWnd_;
	}

	js_panel_window* PanelTarget::GetPanel()
	{
		return pPanel_;
	}

	void PanelTarget::UnlinkPanel()
	{
		pPanel_ = nullptr;
		hWnd_ = nullptr;
	}

	EventBase::EventBase(EventId id) : id_(id) {}

	std::unique_ptr<EventBase> EventBase::Clone()
	{
		return nullptr;
	}

	void EventBase::SetTarget(std::shared_ptr<PanelTarget> pTarget)
	{
		pTarget_ = pTarget;
	}

	EventId EventBase::GetId() const
	{
		return id_;
	}

	Event_Mouse* EventBase::AsMouseEvent()
	{
		return nullptr;
	}

	Event_Drag* EventBase::AsDragEvent()
	{
		return nullptr;
	}

	Event_Basic::Event_Basic(EventId id) : EventBase(id) {}

	void Event_Basic::Run()
	{
		if (pTarget_)
		{
			auto pPanel = pTarget_->GetPanel();

			if (pPanel)
			{
				pPanel->ExecuteEvent_Basic(id_);
			}
		}
	}

	Event_JsExecutor::Event_JsExecutor(EventId id) : EventBase(id) {}

	void Event_JsExecutor::Run()
	{
		if (pTarget_)
		{
			auto pPanel = pTarget_->GetPanel();

			if (pPanel)
			{
				pPanel->ExecuteEvent_JsTask(id_, *this);
			}
		}
	}

	Event_Timer::Event_Timer(std::shared_ptr<ITimer> pTimer, uint64_t generation)
		: EventBase(EventId::kTimer)
		, pTimer_(pTimer)
		, generation_(generation) {}

	void Event_Timer::Run()
	{
		pTimer_->Fire(generation_);
	}

	Event_JsTask::Event_JsTask(EventId id, std::shared_ptr<mozjs::JsAsyncTask> pTask) : Event_JsExecutor(id), pTask_(pTask) {}

	std::optional<bool> Event_JsTask::JsExecute(mozjs::JsContainer& jsContainer)
	{
		jsContainer.InvokeJsAsyncTask(*pTask_);
		return std::nullopt;
	}
}
