#include "PCH.hpp"
#include "ProcessLocationsNotify.hpp"

ProcessLocationsNotify::ProcessLocationsNotify(const GUID& guid, size_t base, bool to_select) : m_guid(guid), m_base(base), m_to_select(to_select) {}

#pragma region static
void ProcessLocationsNotify::init(const pfc::string_list_impl& list, process_locations_notify::ptr ptr) noexcept
{
	static constexpr uint32_t flags = playlist_incoming_item_filter_v2::op_flag_no_filter | playlist_incoming_item_filter_v2::op_flag_delay_ui;
	playlist_incoming_item_filter_v2::get()->process_locations_async(list, flags, nullptr, nullptr, nullptr, ptr);
}
#pragma endregion

void ProcessLocationsNotify::on_completion(metadb_handle_list_cref handles) noexcept
{
	const auto playlistIndex = fb2k::api::pm->find_playlist_by_guid(m_guid);

	if (playlistIndex == SIZE_MAX)
		return;

	const auto mask = fb2k::api::pm->playlist_lock_get_filter_mask(playlistIndex);

	if (WI_IsFlagSet(mask, playlist_lock::filter_add))
		return;

	if (m_to_select)
	{
		fb2k::api::pm->playlist_insert_items(playlistIndex, m_base, handles, pfc::bit_array_true());
		fb2k::api::pm->set_active_playlist(playlistIndex);
		fb2k::api::pm->playlist_set_focus_item(playlistIndex, m_base);
	}
	else
	{
		fb2k::api::pm->playlist_insert_items(playlistIndex, m_base, handles, pfc::bit_array_false());
	}
}
