#include "PCH.hpp"
#include "PlaylistLock.hpp"
#include <utils/guid_helpers.h>

PlaylistLock::PlaylistLock(uint32_t mask) : m_mask(mask) {}

#pragma region static
bool PlaylistLock::add(size_t playlistIndex, uint32_t mask) noexcept
{
	auto lock = fb2k::service_new<PlaylistLock>(mask);

	if (!fb2k::api::pm->playlist_lock_install(playlistIndex, lock))
		return false;

	fb2k::api::pm->playlist_set_property_int(playlistIndex, smp::guid::playlist_attribute_lock_state, mask);
	const auto g = fb2k::api::pm->playlist_get_guid(playlistIndex);
	s_map.set(g, lock);
	return true;
}

bool PlaylistLock::is_my_lock(size_t playlistIndex) noexcept
{
	const auto g = fb2k::api::pm->playlist_get_guid(playlistIndex);
	return s_map.contains(g);
}

bool PlaylistLock::remove(size_t playlistIndex) noexcept
{
	const auto g = fb2k::api::pm->playlist_get_guid(playlistIndex);
	const auto it = s_map.find(g);

	if (it.is_empty())
		return false;

	const auto ret = fb2k::api::pm->playlist_lock_uninstall(playlistIndex, it->m_value);
	fb2k::api::pm->playlist_remove_property(playlistIndex, smp::guid::playlist_attribute_lock_state);
	s_map.remove(g);
	return ret;
}

void PlaylistLock::before_ui_init() noexcept
{
	const auto count = fb2k::api::pm->get_playlist_count();

	for (const auto playlistIndex : indices(count))
	{
		uint32_t mask{};

		if (fb2k::api::pm->playlist_get_property_int(playlistIndex, smp::guid::playlist_attribute_lock_state, mask))
		{
			add(playlistIndex, mask);
		}
	}
}
#pragma endregion

bool PlaylistLock::execute_default_action(size_t) noexcept
{
	return WI_IsFlagSet(m_mask, filter_default_action);
}

bool PlaylistLock::query_items_add(size_t, const pfc::list_base_const_t<metadb_handle_ptr>&, const bit_array&) noexcept
{
	return !WI_IsFlagSet(m_mask, filter_add);
}

bool PlaylistLock::query_items_remove(const bit_array&, bool) noexcept
{
	return !WI_IsFlagSet(m_mask, filter_remove);
}

bool PlaylistLock::query_items_reorder(const size_t*, size_t) noexcept
{
	return !WI_IsFlagSet(m_mask, filter_reorder);
}

bool PlaylistLock::query_item_replace(size_t, const metadb_handle_ptr&, const metadb_handle_ptr&) noexcept
{
	return !WI_IsFlagSet(m_mask, filter_replace);
}

bool PlaylistLock::query_playlist_remove() noexcept
{
	return !WI_IsFlagSet(m_mask, filter_remove_playlist);
}

bool PlaylistLock::query_playlist_rename(const char*, size_t) noexcept
{
	return !WI_IsFlagSet(m_mask, filter_rename);
}

uint32_t PlaylistLock::get_filter_mask() noexcept
{
	return m_mask;
}

void PlaylistLock::get_lock_name(pfc::string_base& out) noexcept
{
	out = Component::underscore_name.data();
}
