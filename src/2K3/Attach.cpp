#include <stdafx.h>
#include "Attach.hpp"

Attach::Attach(Action action, metadb_handle_list_cref handles, const AlbumArtStatic::Type& type, const album_art_data_ptr& data)
	: m_action(action)
	, m_type(type)
	, m_data(data)
{
	m_list.init_from_list(handles);
}

#pragma region static
void Attach::from_path(metadb_handle_list_cref handles, size_t type_id, std::wstring_view path) noexcept
{
	const auto type = AlbumArtStatic::get_type(type_id);
	auto data = AlbumArtStatic::to_data(path);

	if (data.is_valid())
	{
		auto callback = fb2k::service_new<Attach>(Action::Attach, handles, type, data);
		init(callback);
	}
}

void Attach::init(threaded_process_callback::ptr callback) noexcept
{
	threaded_process::get()->run_modeless(callback, threaded_process::flag_silent, core_api::get_main_window(), "LOL");
}

void Attach::remove_id(metadb_handle_list_cref handles, size_t type_id) noexcept
{
	const auto type = AlbumArtStatic::get_type(type_id);
	auto callback = fb2k::service_new<Attach>(Action::Remove, handles, type);
	init(callback);
}

void Attach::remove_all(metadb_handle_list_cref handles) noexcept
{
	auto callback = fb2k::service_new<Attach>(Action::RemoveAll, handles);
	init(callback);
}
#pragma endregion

void Attach::run(threaded_process_status&, abort_callback& abort) noexcept
{
	auto api = file_lock_manager::get();
	album_art_editor::ptr ptr;

	for (auto&& path : m_list)
	{
		if (!album_art_editor::g_get_interface(ptr, path))
			continue;

		try
		{
			auto lock = api->acquire_write(path, abort);
			auto instance_ptr = ptr->open(nullptr, path, abort);

			switch (m_action)
			{
			case Action::Attach:
				instance_ptr->set(m_type, m_data, abort);
				break;
			case Action::Remove:
				instance_ptr->remove(m_type);
				break;
			case Action::RemoveAll:
				instance_ptr->remove_all_();
				break;
			}

			instance_ptr->commit(abort);
		}
		catch (...) {}
	}
}
