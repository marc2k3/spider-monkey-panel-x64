#include "PCH.hpp"
#include "API.hpp"

namespace fb2k::api
{
	album_art_manager_v2::ptr aam;
	playback_control_v3::ptr pc;
	playlist_manager_v5::ptr pm;
	bool is_2_26{};

	void before_ui_init() noexcept
	{
		is_2_26 = core_version_info_v2::get()->test_version(2u, 26u, 0u, 0u);
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
