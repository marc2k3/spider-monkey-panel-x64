#include "PCH.hpp"
#include "AlbumArtStatic.hpp"

#include <utils/image_helpers.h>

HRESULT AlbumArtStatic::to_istream(const album_art_data_ptr& data, wil::com_ptr<IStream>& stream) noexcept
{
	RETURN_HR_IF_EXPECTED(E_FAIL, data.is_empty());

	auto ptr = static_cast<const uint8_t*>(data->data());
	const auto size = static_cast<uint32_t>(data->size());

	auto tmp = SHCreateMemStream(ptr, size);
	RETURN_HR_IF_NULL(E_FAIL, tmp);

	stream.attach(tmp);
	return S_OK;
}

AlbumArtStatic::Type AlbumArtStatic::get_type(size_t type_id) noexcept
{
	return *s_types[type_id];
}

album_art_data_ptr AlbumArtStatic::get(const metadb_handle_ptr& handle, size_t type_id, bool want_stub, bool only_embed, std::string& path) noexcept
{
	if (only_embed)
	{
		const auto rawpath = handle->get_path();
		path = filesystem::g_get_native_path(rawpath).get_ptr();
		return get_embedded(rawpath, type_id);
	}

	const auto type = get_type(type_id);
	album_art_data_ptr data;
	album_art_path_list::ptr paths;

	const auto handles = pfc_list(handle);
	const auto types = pfc_list(type);

	try
	{
		auto instance = fb2k::api::aam->open(handles, types, fb2k::noAbort);
		data = instance->query(type, fb2k::noAbort);
		paths = instance->query_paths(type, fb2k::noAbort);
	}
	catch (...)
	{
		if (want_stub)
		{
			try
			{
				auto instance = fb2k::api::aam->open_stub(fb2k::noAbort);
				data = instance->query(type, fb2k::noAbort);
				paths = instance->query_paths(type, fb2k::noAbort);
			}
			catch (...) {}
		}
	}

	if (paths.is_valid() && paths->get_count() > 0)
	{
		path = filesystem::g_get_native_path(paths->get_path(0)).get_ptr();
	}

	return data;
}

album_art_data_ptr AlbumArtStatic::get_embedded(std::string_view path, size_t type_id) noexcept
{
	const auto type = get_type(type_id);
	album_art_extractor::ptr ptr;

	if (album_art_extractor::g_get_interface(ptr, path.data()))
	{
		try
		{
			auto instance = ptr->open(nullptr, path.data(), fb2k::noAbort);
			return instance->query(type, fb2k::noAbort);
		}
		catch (...) {}
	}

	return {};
}

album_art_data_ptr AlbumArtStatic::to_data(IStream* stream) noexcept
{
	const auto size = FileHelper::get_stream_size(stream);

	if (size <= FileHelper::kMaxStreamSize)
	{
		auto data = fb2k::service_new<album_art_data_impl>();
		data->set_size(size);
		ULONG bytes_read{};

		if SUCCEEDED(stream->Read(data->get_ptr(), size, &bytes_read))
			return data;
	}

	return {};
}

album_art_data_ptr AlbumArtStatic::to_data(std::wstring_view path) noexcept
{
	wil::com_ptr<IStream> stream;

	if SUCCEEDED(FileHelper(path).read(stream))
	{
		return to_data(stream.get());
	}

	return {};
}

bool AlbumArtStatic::check_type_id(size_t type_id) noexcept
{
	return type_id < s_types.size();
}

std::unique_ptr<Gdiplus::Bitmap> AlbumArtStatic::to_bitmap(const album_art_data_ptr& data) noexcept
{
	wil::com_ptr<IStream> stream;
	if FAILED(to_istream(data, stream))
		return nullptr;

	return FileHelper::stream_to_bitmap(stream.get());
}

void AlbumArtStatic::show_viewer(const album_art_data_ptr& data) noexcept
{
	if (data.is_valid())
	{
		fb2k::imageViewer::get()->show(core_api::get_main_window(), data);
	}
}
