#pragma once

class FileHelper
{
public:
	FileHelper(std::wstring_view path);

	static bool rename(std::wstring_view from, std::wstring_view to) noexcept;
	static std::filesystem::copy_options create_options(bool overwrite, bool recur = false) noexcept;
	static std::unique_ptr<Gdiplus::Bitmap> stream_to_bitmap(IStream* stream) noexcept;
	static uint32_t get_stream_size(IStream* stream) noexcept;

	HRESULT read(wil::com_ptr<IStream>& stream) noexcept;
	bool copy_file(std::wstring_view to, bool overwrite) noexcept;
	bool copy_folder(std::wstring_view to, bool overwrite, bool recur) noexcept;
	bool create_folder() noexcept;
	bool exists() noexcept;
	bool is_file() noexcept;
	bool is_folder() noexcept;
	bool remove() noexcept;
	bool write(const void* data, size_t size) noexcept;
	std::unique_ptr<Gdiplus::Bitmap> load_image() noexcept;
	uint64_t file_size() noexcept;
	uint64_t last_modified() noexcept;

	static constexpr uint32_t kMaxStreamSize = 64 * 1024 * 1024;

private:
	std::filesystem::path m_path;
};
