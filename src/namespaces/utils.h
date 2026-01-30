#pragma once
#include <ui/ui_html.h>

namespace mozjs
{
	class JsFbMetadbHandle;

	class Utils : public JsObjectBase<Utils>
	{
	public:
		DEFINE_JS_NAMESPACE_VARS

		static std::unique_ptr<Utils> CreateNative(JSContext* ctx);
		uint32_t GetInternalSize();

	public:
		bool CheckComponent(const std::string& name, bool is_dll = true) const;
		bool CheckComponentWithOpt(size_t optArgCount, const std::string& name, bool is_dll) const;
		bool CheckFont(const std::wstring& name) const;
		uint32_t ColourPicker(uint32_t hWnd, uint32_t default_colour);
		std::string ConvertToAscii(const std::string& str);
		bool CopyFile(const std::wstring& from, const std::wstring& to, bool overwrite = true) const;
		bool CopyFileWithOpt(size_t optArgCount, const std::wstring& from, const std::wstring& to, bool overwrite) const;
		bool CopyFolder(const std::wstring& from, const std::wstring& to, bool overwrite = true, bool recur = true) const;
		bool CopyFolderWithOpt(size_t optArgCount, const std::wstring& from, const std::wstring& to, bool overwrite, bool recur) const;
		bool CreateFolder(const std::wstring& path) const;
		uint32_t DetectCharset(const std::wstring& path) const;
		void DownloadFileAsync(const std::string& url, const std::wstring& path);
		void EditTextFile(const std::wstring& path);
		bool FileExists(const std::wstring& path) const;
		// TODO v2: remove
		JS::Value FileTest(const std::wstring& path, const std::wstring& mode);
		std::string FormatDuration(double p) const;
		std::string FormatFileSize(uint64_t p) const;
		void GetAlbumArtAsync(uint32_t hWnd, JsFbMetadbHandle* handle, uint32_t art_id = 0, bool need_stub = true, bool only_embed = false, bool no_load = false);
		void GetAlbumArtAsyncWithOpt(size_t optArgCount, uint32_t hWnd, JsFbMetadbHandle* handle, uint32_t art_id, bool need_stub, bool only_embed, bool no_load);
		JSObject* GetAlbumArtAsyncV2(uint32_t hWnd, JsFbMetadbHandle* handle, uint32_t art_id = 0, bool need_stub = true, bool only_embed = false, bool no_load = false);
		JSObject* GetAlbumArtAsyncV2WithOpt(size_t optArgCount, uint32_t hWnd, JsFbMetadbHandle* handle, uint32_t art_id, bool need_stub, bool only_embed, bool no_load);
		JSObject* GetAlbumArtEmbedded(const std::string& rawpath, uint32_t art_id = 0);
		JSObject* GetAlbumArtEmbeddedWithOpt(size_t optArgCount, const std::string& rawpath, uint32_t art_id);
		JSObject* GetAlbumArtV2(JsFbMetadbHandle* handle, uint32_t art_id = 0, bool need_stub = true);
		JSObject* GetAlbumArtV2WithOpt(size_t optArgCount, JsFbMetadbHandle* handle, uint32_t art_id, bool need_stub);
		std::string GetClipboardText() const;
		uint64_t GetFileSize(const std::wstring& path) const;
		uint64_t GetLastModified(const std::wstring& path) const;
		JSObject* GetPackageInfo(const std::string& packageId) const;
		// TODO: remove in the next version (not necessarily v2)
		std::wstring GetPackagePath(const std::string& packageId) const;
		uint32_t GetSysColour(uint32_t index) const;
		uint32_t GetSystemMetrics(uint32_t index) const;
		JS::Value Glob(const std::wstring& pattern, uint32_t exc_mask = FILE_ATTRIBUTE_DIRECTORY, uint32_t inc_mask = 0xFFFFFFFF);
		JS::Value GlobWithOpt(size_t optArgCount, const std::wstring& pattern, uint32_t exc_mask, uint32_t inc_mask);
		uint32_t HTTPRequestAsync(uint32_t type, const std::string& url, const std::string& user_agent_or_headers = "", const std::string& body = "");
		uint32_t HTTPRequestAsyncWithOpt(size_t optArgCount, uint32_t type, const std::string& url, const std::string& user_agent_or_headers, const std::string& body);
		std::string InputBox(uint32_t hWnd, const std::string& prompt, const std::string& caption, const std::string& def = "", bool error_on_cancel = false);
		std::string InputBoxWithOpt(size_t optArgCount, uint32_t hWnd, const std::string& prompt, const std::string& caption, const std::string& def, bool error_on_cancel);
		bool IsDirectory(const std::wstring& path) const;
		bool IsFile(const std::wstring& path) const;
		bool IsKeyPressed(uint32_t vkey) const;
		JS::Value ListFonts();
		std::wstring MapString(const std::wstring& str, uint32_t lcid, uint32_t flags);
		uint32_t Now();
		bool PathWildcardMatch(const std::wstring& pattern, const std::wstring& str);
		std::wstring ReadINI(const std::wstring& filename, const std::wstring& section, const std::wstring& key, const std::wstring& defaultval = L"");
		std::wstring ReadINIWithOpt(size_t optArgCount, const std::wstring& filename, const std::wstring& section, const std::wstring& key, const std::wstring& defaultval);
		std::wstring ReadTextFile(const std::wstring& filePath, uint32_t codepage = CP_UTF8);
		std::wstring ReadTextFileWithOpt(size_t optArgCount, const std::wstring& filePath, uint32_t codepage);
		std::string ReadUTF8(const std::wstring& path);
		int32_t RemovePath(const std::wstring& path) const;
		bool RenamePath(const std::wstring& from, const std::wstring& to) const;
		std::string ReplaceIllegalChars(const std::string& str, bool strip_trailing_periods = false);
		std::string ReplaceIllegalCharsWithOpt(size_t optArgCount, const std::string& str, bool strip_trailing_periods );
		void SetClipboardText(const std::string& text);
		void ShowHtmlDialog(uint32_t hWnd, const std::wstring& code_or_path, JS::HandleValue options = JS::UndefinedHandleValue);
		void ShowHtmlDialogWithOpt(size_t optArgCount, uint32_t hWnd, const std::wstring& code_or_path, JS::HandleValue options);
		JS::Value SplitFilePath(const std::wstring& path);
		bool WriteINI(const std::wstring& filename, const std::wstring& section, const std::wstring& key, const std::wstring& val);
		bool WriteTextFile(const std::wstring& filename, const std::string& content, bool write_bom = true);
		bool WriteTextFileWithOpt(size_t optArgCount, const std::wstring& filename, const std::string& content, bool write_bom);

	public:
		std::string get_Version() const;

	private:
		Utils(JSContext* ctx);

		CDialogHtml::Options ParseHTMLOptions(JS::HandleValue options, wil::com_ptr<CDialogHtml::HostExternal>& host_external);

	private:
		JSContext* m_ctx{};
	};
}
