#include "PCH.hpp"
#include "FileHelper.hpp"

#include <utils/image_helpers.h>

namespace fs = std::filesystem;

FileHelper::FileHelper(std::wstring_view path) : m_path(path) {}

#pragma region static
bool FileHelper::rename(std::wstring_view from, std::wstring_view to) noexcept
{
	std::error_code ec;
	fs::rename(from, to, ec);
	return ec.value() == 0;
}

fs::copy_options FileHelper::create_options(bool overwrite, bool recur) noexcept
{
	fs::copy_options options{};

	if (overwrite)
	{
		options |= fs::copy_options::overwrite_existing;
	}

	if (recur)
	{
		options |= fs::copy_options::recursive;
	}

	return options;
}

std::unique_ptr<Gdiplus::Bitmap> FileHelper::stream_to_bitmap(IStream* stream) noexcept
{
	auto bitmap = std::make_unique<Gdiplus::Bitmap>(stream, TRUE);

	if (bitmap && Gdiplus::Ok == bitmap->GetLastStatus())
		return bitmap;

	return smp::LoadWithWIC(stream);
}

uint32_t FileHelper::get_stream_size(IStream* stream) noexcept
{
	STATSTG stats{};

	if FAILED(stream->Stat(&stats, STATFLAG_DEFAULT))
		return UINT_MAX;

	return stats.cbSize.LowPart;
}
#pragma endregion

HRESULT FileHelper::read(wil::com_ptr<IStream>& stream) noexcept
{
	RETURN_IF_FAILED(SHCreateStreamOnFileEx(m_path.c_str(), STGM_READ | STGM_SHARE_DENY_WRITE, GENERIC_READ, FALSE, nullptr, &stream));
	RETURN_HR_IF(E_INVALIDARG, get_stream_size(stream.get()) > kMaxStreamSize);
	return S_OK;
}

bool FileHelper::copy_file(std::wstring_view to, bool overwrite) noexcept
{
	if (!is_file())
		return false;

	const auto options = create_options(overwrite);
	std::error_code ec;
	return fs::copy_file(m_path, to, options, ec);
}

bool FileHelper::copy_folder(std::wstring_view to, bool overwrite, bool recur) noexcept
{
	if (!is_folder())
		return false;

	const auto options = create_options(overwrite, recur);
	std::error_code ec;
	fs::copy(m_path, to, options, ec);
	return ec.value() == 0;
}

bool FileHelper::create_folder() noexcept
{
	std::error_code ec;

	if (fs::is_directory(m_path, ec))
		return true;

	return fs::create_directories(m_path, ec);
}

bool FileHelper::exists() noexcept
{
	std::error_code ec;
	return fs::exists(m_path, ec);
}

bool FileHelper::is_file() noexcept
{
	std::error_code ec;
	return fs::is_regular_file(m_path, ec);
}

bool FileHelper::is_folder() noexcept
{
	std::error_code ec;
	return fs::is_directory(m_path, ec);
}

bool FileHelper::write(const void* data, size_t size) noexcept
{
	auto f = std::ofstream(m_path, std::ios::binary);

	if (!f.is_open())
		return false;

	return f.write((char*)data, size).good();
}

int32_t FileHelper::remove_all() noexcept
{
	std::error_code ec;
	return to_int(fs::remove_all(m_path, ec));
}

std::unique_ptr<Gdiplus::Bitmap> FileHelper::load_image() noexcept
{
	wil::com_ptr<IStream> stream;
	if FAILED(read(stream))
		return nullptr;

	return stream_to_bitmap(stream.get());
}

uint64_t FileHelper::file_size() noexcept
{
	std::error_code ec;

	if (!fs::is_regular_file(m_path, ec))
		return {};

	return fs::file_size(m_path, ec);
}

uint64_t FileHelper::last_modified() noexcept
{
	std::error_code ec;
	const auto last = fs::last_write_time(m_path, ec);

	if (ec.value() != 0)
		return {};

	const auto windows_time = static_cast<uint64_t>(last.time_since_epoch().count());
	return pfc::fileTimeWtoU(windows_time);
}
