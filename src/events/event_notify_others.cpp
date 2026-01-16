#include "PCH.hpp"
#include "event_notify_others.h"

namespace smp
{
	Event_NotifyOthers::Event_NotifyOthers(JSContext* ctx, const std::wstring& name, JS::HandleValue info)
		: Event_JsExecutor(EventId::kNotifyOthers)
		, m_ctx(ctx)
		, m_heap_helper(ctx)
		, m_name(name)
		, m_info_id(m_heap_helper.Store(info)) {}

	Event_NotifyOthers::~Event_NotifyOthers()
	{
		m_heap_helper.Finalize();
	}

	std::unique_ptr<EventBase> Event_NotifyOthers::Clone()
	{
		// Note: this clone operation might result in JS object being used in another panel/global
		JS::RootedValue jsValue(m_ctx, m_heap_helper.Get(m_info_id));
		return std::make_unique<Event_NotifyOthers>(m_ctx, m_name, jsValue);
	}

	std::optional<bool> Event_NotifyOthers::JsExecute(mozjs::JsContainer& jsContainer)
	{
		if (!m_heap_helper.IsJsAvailable())
		{ // This might happen if the original panel/global died
			return std::nullopt;
		}

		JS::RootedValue jsValue(m_ctx, m_heap_helper.Get(m_info_id));
		jsContainer.InvokeOnNotify(m_name, jsValue);

		return std::nullopt;
	}
}
