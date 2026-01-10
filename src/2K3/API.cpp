#include "PCH.hpp"
#include "API.hpp"

namespace fb2k::api
{
	album_art_manager_v2::ptr aam;
	playback_control_v3::ptr pc;
	playlist_manager_v5::ptr pm;

	void before_ui_init() noexcept
	{
		aam = album_art_manager_v2::get();
		pc = playback_control_v3::get();
		pm = playlist_manager_v5::get();
	}

	void reset() noexcept
	{
		aam.release();
		pc.release();
		pm.release();
	}
}
