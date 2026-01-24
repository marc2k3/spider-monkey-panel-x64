#include "PCH.hpp"

#include <events/event_dispatcher.h>
#include <events/event_js_callback.h>

namespace smp
{
	class ConfigObjectNotify : public config_object_notify
	{
	public:
		GUID get_watched_object(size_t index) noexcept final
		{
			return *watched_objects[index].first;
		}

		size_t get_watched_object_count() noexcept final
		{
			return watched_objects.size();
		}

		void on_watched_object_changed(const config_object::ptr& object) noexcept final
		{
			const auto it = std::ranges::find_if(watched_objects, [g = object->get_guid()](const auto& item)
				{
					return g == *item.first;
				});

			if (it != watched_objects.end())
			{
				const auto b = object->get_data_bool_simple(false);
				EventDispatcher::Get().PutEventToAll(GenerateEvent_JsCallback(it->second, b));
			}
		}

	private:
		static constexpr std::array watched_objects =
		{
			std::make_pair(&standard_config_objects::bool_playlist_stop_after_current, EventId::kFbPlaylistStopAfterCurrentChanged),
			std::make_pair(&standard_config_objects::bool_cursor_follows_playback, EventId::kFbCursorFollowPlaybackChanged),
			std::make_pair(&standard_config_objects::bool_playback_follows_cursor, EventId::kFbPlaybackFollowCursorChanged),
			std::make_pair(&standard_config_objects::bool_ui_always_on_top, EventId::kFbAlwaysOnTopChanged)
		};
	};

	FB2K_SERVICE_FACTORY(ConfigObjectNotify);
}
