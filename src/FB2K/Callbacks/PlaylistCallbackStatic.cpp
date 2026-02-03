#include "PCH.hpp"

#include <events/event_dispatcher.h>
#include <events/event_js_callback.h>

namespace smp
{
	class PlaylistCallbackStatic : public playlist_callback_static
	{
	public:
		void on_default_format_changed() noexcept final {}
		void on_items_modified(size_t, const pfc::bit_array&) noexcept final {}
		void on_items_modified_fromplayback(size_t, const pfc::bit_array&, playback_control::t_display_level) noexcept final {}
		void on_items_removing(size_t, const pfc::bit_array&, size_t, size_t) noexcept final {}
		void on_items_replaced(size_t, const pfc::bit_array&, const pfc::list_base_const_t<t_on_items_replaced_entry>&) noexcept final {}
		void on_playlists_removing(const pfc::bit_array&, size_t, size_t) noexcept final {}

		uint32_t get_flags() noexcept final
		{
			return flag_on_item_ensure_visible | flag_on_item_focus_change | flag_on_items_added |
				flag_on_items_removed | flag_on_items_reordered | flag_on_items_selection_change |
				flag_on_playback_order_changed | flag_on_playlist_activate | flag_on_playlist_created | flag_on_playlist_locked |
				flag_on_playlists_removed | flag_on_playlist_renamed | flag_on_playlists_reorder;
		}

		void on_item_ensure_visible(size_t playlistIndex, size_t playlistItemIndex) noexcept final
		{
			EventDispatcher::Get().PutEventToAll(GenerateEvent_JsCallback(EventId::kFbPlaylistItemEnsureVisible, playlistIndex, playlistItemIndex));
		}

		void on_item_focus_change(size_t playlistIndex, size_t from, size_t to) noexcept final
		{
			EventDispatcher::Get().PutEventToAll(GenerateEvent_JsCallback(EventId::kFbItemFocusChange, playlistIndex, from, to));
		}

		void on_items_added(size_t playlistIndex, size_t, metadb_handle_list_cref, const pfc::bit_array&) noexcept final
		{
			EventDispatcher::Get().PutEventToAll(GenerateEvent_JsCallback(EventId::kFbPlaylistItemsAdded, playlistIndex));
		}

		void on_items_removed(size_t playlistIndex, const pfc::bit_array&, size_t, size_t count) noexcept final
		{
			EventDispatcher::Get().PutEventToAll(GenerateEvent_JsCallback(EventId::kFbPlaylistItemsRemoved, playlistIndex, count));
		}

		void on_items_reordered(size_t playlistIndex, const size_t*, size_t) noexcept final
		{
			EventDispatcher::Get().PutEventToAll(GenerateEvent_JsCallback(EventId::kFbPlaylistItemsReordered, playlistIndex));
		}

		void on_items_selection_change(size_t, const pfc::bit_array&, const pfc::bit_array&) noexcept final
		{
			EventDispatcher::Get().PutEventToAll(GenerateEvent_JsCallback(EventId::kFbPlaylistItemsSelectionChange));
		}

		void on_playback_order_changed(size_t new_index) noexcept final
		{
			EventDispatcher::Get().PutEventToAll(GenerateEvent_JsCallback(EventId::kFbPlaybackOrderChanged, new_index));
		}

		void on_playlist_activate(size_t old_index, size_t new_index) noexcept final
		{
			if (old_index != new_index)
			{
				EventDispatcher::Get().PutEventToAll(GenerateEvent_JsCallback(EventId::kFbPlaylistSwitch));
			}
		}

		void on_playlist_created(size_t, const char*, size_t) noexcept final
		{
			on_playlists_changed();
		}

		void on_playlist_locked(size_t, bool) noexcept final
		{
			on_playlists_changed();
		}

		void on_playlist_renamed(size_t, const char*, size_t) noexcept final
		{
			on_playlists_changed();
		}

		void on_playlists_removed(const pfc::bit_array&, size_t, size_t) noexcept final
		{
			on_playlists_changed();
		}

		void on_playlists_reorder(const size_t*, size_t) noexcept final
		{
			on_playlists_changed();
		}

	private:
		void on_playlists_changed() noexcept
		{
			EventDispatcher::Get().PutEventToAll(GenerateEvent_JsCallback(EventId::kFbPlaylistsChanged));
		}
	};

	FB2K_SERVICE_FACTORY(PlaylistCallbackStatic);
}
