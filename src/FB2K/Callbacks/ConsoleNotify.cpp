#include "PCH.hpp"

#include <events/event_dispatcher.h>
#include <events/event_js_callback.h>

namespace smp
{
	class ConsoleNotify : public initquit, public fb2k::console_notify
	{
	public:
		void on_init() noexcept final
		{
			fb2k::console_manager::get()->addNotify(this);
		}

		void on_quit() noexcept final
		{
			fb2k::console_manager::get()->removeNotify(this);
		}

		void onConsoleRefresh() noexcept final
		{
			EventDispatcher::Get().PutEventToAll(GenerateEvent_JsCallback(EventId::kFbConsoleRefresh));
		}
	};

	FB2K_SERVICE_FACTORY(ConsoleNotify);
}
