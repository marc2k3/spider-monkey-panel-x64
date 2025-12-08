#pragma once

class TextFile
{
public:
	TextFile(std::wstring_view path);

	bool write(std::string_view content, bool write_bom = false) noexcept;
	std::string read() noexcept;
	uint32_t guess_codepage() noexcept;
	void read_wide(uint32_t codepage, std::wstring& content) noexcept;

private:
	static uint32_t guess_codepage(std::string_view content) noexcept;

	static constexpr std::string_view UTF_16_LE_BOM = "\xFF\xFE";
	static constexpr std::string_view UTF_8_BOM = "\xEF\xBB\xBF";

	std::filesystem::path m_path;
};
