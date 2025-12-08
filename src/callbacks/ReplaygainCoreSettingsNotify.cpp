#include <stdafx.h>

#include <events/event_dispatcher.h>
#include <events/event_js_callback.h>

namespace smp
{
	class ReplaygainCoreSettingsNotify : public initquit, public replaygain_core_settings_notify
	{
	public:
		void on_changed(const t_replaygain_config& cfg) noexcept final
		{
			EventDispatcher::Get().PutEventToAll(GenerateEvent_JsCallback(EventId::kFbReplaygainModeChanged, static_cast<uint32_t>(cfg.m_source_mode)));
		}

		void on_init() noexcept final
		{
			replaygain_manager_v2::get()->add_notify(this);
		}

		void on_quit() noexcept final
		{
			replaygain_manager_v2::get()->remove_notify(this);
		}
	};

	FB2K_SERVICE_FACTORY(ReplaygainCoreSettingsNotify);
}
