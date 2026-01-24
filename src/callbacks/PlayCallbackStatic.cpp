#include "PCH.hpp"

#include <events/event_dispatcher.h>
#include <events/event_js_callback.h>

namespace smp
{
	class PlayCallbackStatic : public play_callback_static
	{
	public:
		uint32_t get_flags() noexcept final
		{
			return flag_on_playback_all | flag_on_volume_change;
		}

		void on_playback_dynamic_info(const file_info&) noexcept final
		{
			EventDispatcher::Get().PutEventToAll(GenerateEvent_JsCallback(EventId::kFbPlaybackDynamicInfo));
		}

		void on_playback_dynamic_info_track(const file_info&) noexcept final
		{
			EventDispatcher::Get().PutEventToAll(GenerateEvent_JsCallback(EventId::kFbPlaybackDynamicInfoTrack));
		}

		void on_playback_edited(metadb_handle_ptr handle) noexcept final
		{
			EventDispatcher::Get().PutEventToAll(GenerateEvent_JsCallback(EventId::kFbPlaybackEdited, handle));
		}

		void on_playback_new_track(metadb_handle_ptr handle) noexcept final
		{
			EventDispatcher::Get().PutEventToAll(GenerateEvent_JsCallback(EventId::kFbPlaybackNewTrack, handle));
		}

		void on_playback_pause(bool state) noexcept final
		{
			EventDispatcher::Get().PutEventToAll(GenerateEvent_JsCallback(EventId::kFbPlaybackPause, state));
		}

		void on_playback_seek(double time) noexcept final
		{
			EventDispatcher::Get().PutEventToAll(GenerateEvent_JsCallback(EventId::kFbPlaybackSeek, time));
		}

		void on_playback_starting(playback_control::t_track_command cmd, bool paused) noexcept final
		{
			EventDispatcher::Get().PutEventToAll(GenerateEvent_JsCallback(EventId::kFbPlaybackStarting, static_cast<uint32_t>(cmd), paused));
		}

		void on_playback_stop(playback_control::t_stop_reason reason) noexcept final
		{
			EventDispatcher::Get().PutEventToAll(GenerateEvent_JsCallback(EventId::kFbPlaybackStop, static_cast<uint32_t>(reason)));
		}

		void on_playback_time(double time) noexcept final
		{
			EventDispatcher::Get().PutEventToAll(GenerateEvent_JsCallback(EventId::kFbPlaybackTime, time));
		}

		void on_volume_change(float volume) noexcept final
		{
			EventDispatcher::Get().PutEventToAll(GenerateEvent_JsCallback(EventId::kFbVolumeChange, volume));
		}
	};

	FB2K_SERVICE_FACTORY(PlayCallbackStatic);
}
