#pragma once
#include "AlbumArtStatic.hpp"

class Attach : public threaded_process_callback
{
public:
	enum class Action
	{
		Attach,
		Remove,
		RemoveAll
	};

	Attach(Action action, metadb_handle_list_cref handles, const AlbumArtStatic::Type& type = {}, const album_art_data_ptr& data = {});

	static void from_path(metadb_handle_list_cref handles, size_t type_id, std::wstring_view path) noexcept;
	static void remove_id(metadb_handle_list_cref handles, size_t type_id) noexcept;
	static void remove_all(metadb_handle_list_cref handles) noexcept;

	void run(threaded_process_status& status, abort_callback& abort) noexcept final;

private:
	static void init(threaded_process_callback::ptr callback) noexcept;

	Action m_action{};
	AlbumArtStatic::Type m_type;
	album_art_data_ptr m_data;
	file_list_helper::file_list_from_metadb_handle_list m_list;
};
