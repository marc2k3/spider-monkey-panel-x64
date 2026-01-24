#include "PCH.hpp"

#include <events/event_dispatcher.h>
#include <events/event_js_callback.h>

namespace smp
{
	class OutputConfigChangeCallback : public initquit, public output_config_change_callback
	{
	public:
		void on_init() noexcept final
		{
			output_manager_v2::get()->addCallback(this);
		}

		void on_quit() noexcept final
		{
			output_manager_v2::get()->removeCallback(this);
		}

		void outputConfigChanged() noexcept final
		{
			EventDispatcher::Get().PutEventToAll(GenerateEvent_JsCallback(EventId::kFbOutputDeviceChanged));
		}
	};

	FB2K_SERVICE_FACTORY(OutputConfigChangeCallback);
}
