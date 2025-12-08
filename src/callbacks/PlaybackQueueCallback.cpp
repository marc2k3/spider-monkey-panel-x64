#include <stdafx.h>

#include <events/event_dispatcher.h>
#include <events/event_js_callback.h>

namespace smp
{
	class PlaybackQueueCallback : public playback_queue_callback
	{
	public:
		void on_changed(t_change_origin origin) noexcept final
		{
			EventDispatcher::Get().PutEventToAll(GenerateEvent_JsCallback(EventId::kFbPlaybackQueueChanged, static_cast<uint32_t>(origin)));
		}
	};

	FB2K_SERVICE_FACTORY(PlaybackQueueCallback);
}
