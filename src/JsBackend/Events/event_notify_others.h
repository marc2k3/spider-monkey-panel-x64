#pragma once
#include "event.h"

namespace smp
{
	class Event_NotifyOthers final : public Event_JsExecutor
	{
	public:
		Event_NotifyOthers(JSContext* ctx, const std::wstring& name, JS::HandleValue info);
		~Event_NotifyOthers() override;

		std::unique_ptr<EventBase> Clone() override;

		std::optional<bool> JsExecute(mozjs::JsContainer& jsContainer) override;

	private:
		JSContext* m_ctx{};
		mozjs::HeapHelper m_heap_helper;
		const std::wstring m_name;
		const uint32_t m_info_id;
	};
}
