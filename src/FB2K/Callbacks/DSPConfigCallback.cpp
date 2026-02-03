#include "PCH.hpp"

#include <events/event_dispatcher.h>
#include <events/event_js_callback.h>

namespace smp
{
	class DSPConfigCallback : public dsp_config_callback
	{
	public:
		void on_core_settings_change(const dsp_chain_config&) noexcept final
		{
			EventDispatcher::Get().PutEventToAll(GenerateEvent_JsCallback(EventId::kFbDspPresetChanged));
		}
	};

	FB2K_SERVICE_FACTORY(DSPConfigCallback);
}
