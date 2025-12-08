#include <stdafx.h>

#include <events/event_dispatcher.h>
#include <events/event_js_callback.h>

namespace smp
{
	class UISelectionCallback : public initquit, public ui_selection_callback
	{
	public:
		void on_init() noexcept final
		{
			ui_selection_manager_v2::get()->register_callback(this, 0);
		}

		void on_quit() noexcept final
		{
			ui_selection_manager_v2::get()->unregister_callback(this);
		}

		void on_selection_changed(metadb_handle_list_cref) noexcept final
		{
			EventDispatcher::Get().PutEventToAll(GenerateEvent_JsCallback(EventId::kFbSelectionChanged));
		}
	};

	FB2K_SERVICE_FACTORY(UISelectionCallback);
}
