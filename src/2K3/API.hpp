#pragma once

namespace fb2k::api
{
	void before_ui_init() noexcept;
	void reset() noexcept;

	extern album_art_manager_v2::ptr aam;
	extern playback_control_v3::ptr pc;
	extern playlist_manager_v5::ptr pm;
	extern bool is_2_26;
}
