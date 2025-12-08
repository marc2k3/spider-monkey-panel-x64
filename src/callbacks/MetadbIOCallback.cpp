#include <stdafx.h>

#include <events/event_dispatcher.h>
#include <events/event_js_callback.h>

namespace smp
{
	class MetadbIOCallback : public metadb_io_callback
	{
	public:
		void on_changed_sorted(metadb_handle_list_cref handles, bool fromhook) noexcept final
		{
			EventDispatcher::Get().PutEventToAll(GenerateEvent_JsCallback(EventId::kFbMetadbChanged, std::make_shared<metadb_handle_list>(handles), fromhook));
		}
	};

	FB2K_SERVICE_FACTORY(MetadbIOCallback);
}
