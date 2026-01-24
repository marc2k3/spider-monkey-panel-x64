#pragma once

class ProcessLocationsNotify : public process_locations_notify
{
public:
	ProcessLocationsNotify(const GUID& guid, size_t base, bool to_select);

	static void init(const pfc::string_list_impl& list, process_locations_notify::ptr ptr) noexcept;

	void on_aborted() noexcept final {}
	void on_completion(metadb_handle_list_cref handles) noexcept final;

private:
	GUID m_guid;
	bool m_to_select{};
	size_t m_base{};
};
