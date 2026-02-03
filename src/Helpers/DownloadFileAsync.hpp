#pragma once

class DownloadFileAsync : public fb2k::threadEntry
{
public:
	DownloadFileAsync(HWND wnd, std::string_view url, std::wstring_view path);

	void run() noexcept final;

private:
	HWND m_wnd;
	std::string m_url;
	std::wstring m_path;
};
