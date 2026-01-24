#pragma once

namespace smp
{
	enum class EventId
	{
		// fb
		kFbAlwaysOnTopChanged,
		kFbCursorFollowPlaybackChanged,
		kFbConsoleRefresh,
		kFbDspPresetChanged,
		kFbItemFocusChange,
		kFbItemPlayed,
		kFbLibraryItemsAdded,
		kFbLibraryItemsChanged,
		kFbLibraryItemsRemoved,
		kFbMetadbChanged,
		kFbOutputDeviceChanged,
		kFbPlaybackDynamicInfo,
		kFbPlaybackDynamicInfoTrack,
		kFbPlaybackEdited,
		kFbPlaybackFollowCursorChanged,
		kFbPlaybackNewTrack,
		kFbPlaybackOrderChanged,
		kFbPlaybackPause,
		kFbPlaybackQueueChanged,
		kFbPlaybackSeek,
		kFbPlaybackStarting,
		kFbPlaybackStop,
		kFbPlaybackTime,
		kFbPlaylistItemEnsureVisible,
		kFbPlaylistItemsAdded,
		kFbPlaylistItemsRemoved,
		kFbPlaylistItemsReordered,
		kFbPlaylistItemsSelectionChange,
		kFbPlaylistStopAfterCurrentChanged,
		kFbPlaylistSwitch,
		kFbPlaylistsChanged,
		kFbReplaygainModeChanged,
		kFbSelectionChanged,
		kFbVolumeChange,
		// input
		/// control
		kInputBlur,
		kInputFocus,
		/// keyboard
		kKeyboardChar,
		kKeyboardKeyDown,
		kKeyboardKeyUp,
		/// mouse
		//// buttons
		kMouseLeftButtonDoubleClick,
		kMouseLeftButtonDown,
		kMouseLeftButtonUp,
		kMouseMiddleButtonDoubleClick,
		kMouseMiddleButtonDown,
		kMouseMiddleButtonUp,
		kMouseRightButtonDoubleClick,
		kMouseRightButtonDown,
		kMouseRightButtonUp,
		//// wheel
		kMouseHorizontalWheel,
		kMouseVerticalWheel,
		//// move
		kMouseLeave,
		kMouseMove,
		//// context
		kMouseContextMenu,
		//// drag-n-drop
		kMouseDragDrop,
		kMouseDragEnter,
		kMouseDragLeave,
		kMouseDragOver,
		/// main menu
		kStaticMainMenu,
		kDynamicMainMenu,
		// internal
		kInternalDownloadFileDone,
		kInternalGetAlbumArtDone,
		kInternalGetAlbumArtPromiseDone,
		kInternalHttpRequestDone,
		kInternalLoadImageDone,
		kInternalLoadImagePromiseDone,
		kInternalLocationsAdded,
		// ui
		kUiColoursChanged,
		kUiFontChanged,
		// window
		kWndPaint,
		kWndResize,
		// script
		kScriptEdit,
		kScriptReload,
		kScriptShowConfigure,
		kScriptShowConfigureLegacy,
		kScriptShowProperties,
		// custom
		kNotifyOthers,
		kTimer
	};

	const std::unordered_map<EventId, std::string> kCallbackIdToName = {
		{ EventId::kFbAlwaysOnTopChanged, "on_always_on_top_changed" },
		{ EventId::kFbConsoleRefresh , "on_console_refresh" },
		{ EventId::kFbCursorFollowPlaybackChanged, "on_cursor_follow_playback_changed" },
		{ EventId::kFbDspPresetChanged, "on_dsp_preset_changed" },
		{ EventId::kFbItemFocusChange, "on_item_focus_change" },
		{ EventId::kFbItemPlayed, "on_item_played" },
		{ EventId::kFbLibraryItemsAdded, "on_library_items_added" },
		{ EventId::kFbLibraryItemsChanged, "on_library_items_changed" },
		{ EventId::kFbLibraryItemsRemoved, "on_library_items_removed" },
		{ EventId::kFbMetadbChanged, "on_metadb_changed" },
		{ EventId::kFbOutputDeviceChanged, "on_output_device_changed" },
		{ EventId::kFbPlaybackDynamicInfo, "on_playback_dynamic_info" },
		{ EventId::kFbPlaybackDynamicInfoTrack, "on_playback_dynamic_info_track" },
		{ EventId::kFbPlaybackEdited, "on_playback_edited" },
		{ EventId::kFbPlaybackFollowCursorChanged, "on_playback_follow_cursor_changed" },
		{ EventId::kFbPlaybackNewTrack, "on_playback_new_track" },
		{ EventId::kFbPlaybackOrderChanged, "on_playback_order_changed" },
		{ EventId::kFbPlaybackPause, "on_playback_pause" },
		{ EventId::kFbPlaybackQueueChanged, "on_playback_queue_changed" },
		{ EventId::kFbPlaybackSeek, "on_playback_seek" },
		{ EventId::kFbPlaybackStarting, "on_playback_starting" },
		{ EventId::kFbPlaybackStop, "on_playback_stop" },
		{ EventId::kFbPlaybackTime, "on_playback_time" },
		{ EventId::kFbPlaylistItemEnsureVisible, "on_playlist_item_ensure_visible" },
		{ EventId::kFbPlaylistItemsAdded, "on_playlist_items_added" },
		{ EventId::kFbPlaylistItemsRemoved, "on_playlist_items_removed" },
		{ EventId::kFbPlaylistItemsReordered, "on_playlist_items_reordered" },
		{ EventId::kFbPlaylistItemsSelectionChange, "on_playlist_items_selection_change" },
		{ EventId::kFbPlaylistStopAfterCurrentChanged, "on_playlist_stop_after_current_changed" },
		{ EventId::kFbPlaylistSwitch, "on_playlist_switch" },
		{ EventId::kFbPlaylistsChanged, "on_playlists_changed" },
		{ EventId::kFbReplaygainModeChanged, "on_replaygain_mode_changed" },
		{ EventId::kFbSelectionChanged, "on_selection_changed" },
		{ EventId::kFbVolumeChange, "on_volume_change" },
		// input
		/// control
		{ EventId::kInputBlur, "on_focus" },
		{ EventId::kInputFocus, "on_focus" },
		/// keyboard
		{ EventId::kKeyboardChar, "on_char" },
		{ EventId::kKeyboardKeyDown, "on_key_down" },
		{ EventId::kKeyboardKeyUp, "on_key_up" },
		/// mouse
		//// buttons
		{ EventId::kMouseLeftButtonDoubleClick, "on_mouse_lbtn_dblclk" },
		{ EventId::kMouseLeftButtonDown, "on_mouse_lbtn_down" },
		{ EventId::kMouseLeftButtonUp, "on_mouse_lbtn_up" },
		{ EventId::kMouseMiddleButtonDoubleClick, "on_mouse_mbtn_dblclk" },
		{ EventId::kMouseMiddleButtonDown, "on_mouse_mbtn_down" },
		{ EventId::kMouseMiddleButtonUp, "on_mouse_mbtn_up" },
		{ EventId::kMouseRightButtonDoubleClick, "on_mouse_rbtn_dblclk" },
		{ EventId::kMouseRightButtonDown, "on_mouse_rbtn_down" },
		{ EventId::kMouseRightButtonUp, "on_mouse_rbtn_up" },
		//// wheel
		{ EventId::kMouseHorizontalWheel, "on_mouse_wheel_h" },
		{ EventId::kMouseVerticalWheel, "on_mouse_wheel" },
		//// move
		{ EventId::kMouseLeave, "on_mouse_leave" },
		{ EventId::kMouseMove, "on_mouse_move" },
		//// context
		{ EventId::kMouseContextMenu, "UNUSED" },
		//// drag-n-drop
		{ EventId::kMouseDragDrop, "UNUSED" },
		{ EventId::kMouseDragEnter, "UNUSED" },
		{ EventId::kMouseDragLeave, "UNUSED" },
		{ EventId::kMouseDragOver, "UNUSED" },
		/// main menu
		{ EventId::kStaticMainMenu, "on_main_menu" },
		{ EventId::kDynamicMainMenu, "on_main_menu_dynamic" },
		// internal
		{ EventId::kInternalDownloadFileDone, "on_download_file_done" },
		{ EventId::kInternalGetAlbumArtDone, "on_get_album_art_done" },
		{ EventId::kInternalHttpRequestDone, "on_http_request_done" },
		{ EventId::kInternalLoadImageDone, "on_load_image_done" },
		{ EventId::kInternalLocationsAdded, "on_locations_added" },
		// ui
		{ EventId::kUiColoursChanged, "on_colours_changed" },
		{ EventId::kUiFontChanged, "on_font_changed" },
		// custom
		{ EventId::kNotifyOthers, "on_notify_data" },
	};
}
