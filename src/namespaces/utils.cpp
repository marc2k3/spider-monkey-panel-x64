#include "PCH.hpp"
#include "utils.h"

#include <FB2K/AlbumArtStatic.hpp>
#include <Helpers/CustomSort.hpp>
#include <Helpers/DownloadFileAsync.hpp>
#include <Helpers/FontHelper.hpp>
#include <Helpers/GetAlbumArtAsync.hpp>
#include <Helpers/HTTPRequestAsync.hpp>
#include <config/package_utils.h>
#include <interfaces/fb_metadb_handle.h>
#include <interfaces/gdi_bitmap.h>
#include <js_backend/com_convert.h>
#include <js_backend/js_art_helpers.h>
#include <ui/ui_input_box.h>
#include <utils/colour_helpers.h>
#include <utils/edit_text.h>

namespace
{
	using namespace mozjs;

	DEFINE_JS_CLASS_OPS(Utils::FinalizeJsObject)

	DEFINE_JS_CLASS("Utils")

	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(CheckComponent, Utils::CheckComponent, Utils::CheckComponentWithOpt, 1)
	MJS_DEFINE_JS_FN_FROM_NATIVE(CheckFont, Utils::CheckFont)
	MJS_DEFINE_JS_FN_FROM_NATIVE(ColourPicker, Utils::ColourPicker)
	MJS_DEFINE_JS_FN_FROM_NATIVE(ConvertToAscii, Utils::ConvertToAscii)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(CopyFile, Utils::CopyFile, Utils::CopyFileWithOpt, 1)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(CopyFolder, Utils::CopyFolder, Utils::CopyFolderWithOpt, 2)
	MJS_DEFINE_JS_FN_FROM_NATIVE(CreateFolder, Utils::CreateFolder)
	MJS_DEFINE_JS_FN_FROM_NATIVE(DetectCharset, Utils::DetectCharset)
	MJS_DEFINE_JS_FN_FROM_NATIVE(DownloadFileAsync, Utils::DownloadFileAsync)
	MJS_DEFINE_JS_FN_FROM_NATIVE(EditTextFile, Utils::EditTextFile)
	MJS_DEFINE_JS_FN_FROM_NATIVE(FileExists, Utils::FileExists)
	MJS_DEFINE_JS_FN_FROM_NATIVE(FileTest, Utils::FileTest)
	MJS_DEFINE_JS_FN_FROM_NATIVE(FormatDuration, Utils::FormatDuration)
	MJS_DEFINE_JS_FN_FROM_NATIVE(FormatFileSize, Utils::FormatFileSize)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(GetAlbumArtAsync, Utils::GetAlbumArtAsync, Utils::GetAlbumArtAsyncWithOpt, 4)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(GetAlbumArtAsyncV2, Utils::GetAlbumArtAsyncV2, Utils::GetAlbumArtAsyncV2WithOpt, 4)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(GetAlbumArtEmbedded, Utils::GetAlbumArtEmbedded, Utils::GetAlbumArtEmbeddedWithOpt, 1)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(GetAlbumArtV2, Utils::GetAlbumArtV2, Utils::GetAlbumArtV2WithOpt, 2);
	MJS_DEFINE_JS_FN_FROM_NATIVE(GetClipboardText, Utils::GetClipboardText)
	MJS_DEFINE_JS_FN_FROM_NATIVE(GetFileSize, Utils::GetFileSize)
	MJS_DEFINE_JS_FN_FROM_NATIVE(GetLastModified, Utils::GetLastModified)
	MJS_DEFINE_JS_FN_FROM_NATIVE(GetPackageInfo, Utils::GetPackageInfo)
	MJS_DEFINE_JS_FN_FROM_NATIVE(GetPackagePath, Utils::GetPackagePath)
	MJS_DEFINE_JS_FN_FROM_NATIVE(GetSysColour, Utils::GetSysColour)
	MJS_DEFINE_JS_FN_FROM_NATIVE(GetSystemMetrics, Utils::GetSystemMetrics);
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(Glob, Utils::Glob, Utils::GlobWithOpt, 2);
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(HTTPRequestAsync, Utils::HTTPRequestAsync, Utils::HTTPRequestAsyncWithOpt, 2)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(InputBox, Utils::InputBox, Utils::InputBoxWithOpt, 2)
	MJS_DEFINE_JS_FN_FROM_NATIVE(IsDirectory, Utils::IsDirectory)
	MJS_DEFINE_JS_FN_FROM_NATIVE(IsFile, Utils::IsFile)
	MJS_DEFINE_JS_FN_FROM_NATIVE(IsKeyPressed, Utils::IsKeyPressed)
	MJS_DEFINE_JS_FN_FROM_NATIVE(ListFonts, Utils::ListFonts)
	MJS_DEFINE_JS_FN_FROM_NATIVE(MapString, Utils::MapString)
	MJS_DEFINE_JS_FN_FROM_NATIVE(Now, Utils::Now)
	MJS_DEFINE_JS_FN_FROM_NATIVE(PathWildcardMatch, Utils::PathWildcardMatch)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(ReadINI, Utils::ReadINI, Utils::ReadINIWithOpt, 1)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(ReadTextFile, Utils::ReadTextFile, Utils::ReadTextFileWithOpt, 1)
	MJS_DEFINE_JS_FN_FROM_NATIVE(ReadUTF8, Utils::ReadUTF8)
	MJS_DEFINE_JS_FN_FROM_NATIVE(RemovePath, Utils::RemovePath)
	MJS_DEFINE_JS_FN_FROM_NATIVE(RenamePath, Utils::RenamePath)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(ReplaceIllegalChars, Utils::ReplaceIllegalChars, Utils::ReplaceIllegalCharsWithOpt, 1)
	MJS_DEFINE_JS_FN_FROM_NATIVE(SetClipboardText, Utils::SetClipboardText)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(ShowHtmlDialog, Utils::ShowHtmlDialog, Utils::ShowHtmlDialogWithOpt, 1)
	MJS_DEFINE_JS_FN_FROM_NATIVE(SplitFilePath, Utils::SplitFilePath)
	MJS_DEFINE_JS_FN_FROM_NATIVE(WriteINI, Utils::WriteINI)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(WriteTextFile, Utils::WriteTextFile, Utils::WriteTextFileWithOpt, 1)

	constexpr auto jsFunctions = std::to_array<JSFunctionSpec>(
		{
			JS_FN("CheckComponent", CheckComponent, 1, kDefaultPropsFlags),
			JS_FN("CheckFont", CheckFont, 1, kDefaultPropsFlags),
			JS_FN("ColourPicker", ColourPicker, 2, kDefaultPropsFlags),
			JS_FN("ConvertToAscii", ConvertToAscii, 1, kDefaultPropsFlags),
			JS_FN("CopyFile", CopyFile, 2, kDefaultPropsFlags),
			JS_FN("CopyFolder", CopyFolder, 2, kDefaultPropsFlags),
			JS_FN("CreateFolder", CreateFolder, 1, kDefaultPropsFlags),
			JS_FN("DetectCharset", DetectCharset, 1, kDefaultPropsFlags),
			JS_FN("DownloadFileAsync", DownloadFileAsync, 2, kDefaultPropsFlags),
			JS_FN("EditTextFile", ::EditTextFile, 2, kDefaultPropsFlags),
			JS_FN("FileExists", FileExists, 1, kDefaultPropsFlags),
			JS_FN("FileTest", FileTest, 2, kDefaultPropsFlags),
			JS_FN("FormatDuration", FormatDuration, 1, kDefaultPropsFlags),
			JS_FN("FormatFileSize", FormatFileSize, 1, kDefaultPropsFlags),
			JS_FN("GetAlbumArtAsync", GetAlbumArtAsync, 2, kDefaultPropsFlags),
			JS_FN("GetAlbumArtAsyncV2", GetAlbumArtAsyncV2, 2, kDefaultPropsFlags),
			JS_FN("GetAlbumArtEmbedded", GetAlbumArtEmbedded, 1, kDefaultPropsFlags),
			JS_FN("GetAlbumArtV2", GetAlbumArtV2, 1, kDefaultPropsFlags),
			JS_FN("GetClipboardText", GetClipboardText, 0, kDefaultPropsFlags),
			JS_FN("GetFileSize", GetFileSize, 1, kDefaultPropsFlags),
			JS_FN("GetLastModified", GetLastModified, 1, kDefaultPropsFlags),
			JS_FN("GetPackageInfo", GetPackageInfo, 1, kDefaultPropsFlags),
			JS_FN("GetPackagePath", GetPackagePath, 1, kDefaultPropsFlags),
			JS_FN("GetSysColour", GetSysColour, 1, kDefaultPropsFlags),
			JS_FN("GetSystemMetrics", GetSystemMetrics, 1, kDefaultPropsFlags),
			JS_FN("Glob", Glob, 1, kDefaultPropsFlags),
			JS_FN("HTTPRequestAsync", HTTPRequestAsync, 2, kDefaultPropsFlags),
			JS_FN("InputBox", InputBox, 3, kDefaultPropsFlags),
			JS_FN("IsDirectory", IsDirectory, 1, kDefaultPropsFlags),
			JS_FN("IsFile", IsFile, 1, kDefaultPropsFlags),
			JS_FN("IsKeyPressed", IsKeyPressed, 1, kDefaultPropsFlags),
			JS_FN("ListFonts", ListFonts, 0, kDefaultPropsFlags),
			JS_FN("MapString", MapString, 3, kDefaultPropsFlags),
			JS_FN("Now", Now, 0, kDefaultPropsFlags),
			JS_FN("PathWildcardMatch", PathWildcardMatch, 2, kDefaultPropsFlags),
			JS_FN("ReadINI", ReadINI, 3, kDefaultPropsFlags),
			JS_FN("ReadTextFile", ReadTextFile, 1, kDefaultPropsFlags),
			JS_FN("ReadUTF8", ReadUTF8, 1, kDefaultPropsFlags),
			JS_FN("RemovePath", RemovePath, 1, kDefaultPropsFlags),
			JS_FN("RenamePath", RenamePath, 2, kDefaultPropsFlags),
			JS_FN("ReplaceIllegalChars", ReplaceIllegalChars, 1, kDefaultPropsFlags),
			JS_FN("SetClipboardText", SetClipboardText, 1, kDefaultPropsFlags),
			JS_FN("ShowHtmlDialog", ShowHtmlDialog, 3, kDefaultPropsFlags),
			JS_FN("SplitFilePath", SplitFilePath, 1, kDefaultPropsFlags),
			JS_FN("WriteINI", WriteINI, 4, kDefaultPropsFlags),
			JS_FN("WriteTextFile", WriteTextFile, 2, kDefaultPropsFlags),
			JS_FS_END,
		});

	MJS_DEFINE_JS_FN_FROM_NATIVE(get_Version, Utils::get_Version)

	constexpr auto jsProperties = std::to_array<JSPropertySpec>(
		{
			JS_PSG("Version", get_Version, kDefaultPropsFlags),
			JS_PS_END,
		});
}

namespace mozjs
{
	using namespace smp;

	const JSClass Utils::JsClass = jsClass;
	const JSFunctionSpec* Utils::JsFunctions = jsFunctions.data();
	const JSPropertySpec* Utils::JsProperties = jsProperties.data();

	Utils::Utils(JSContext* ctx) : m_ctx(ctx) {}

	CDialogHtml::Options Utils::ParseHTMLOptions(JS::HandleValue options, wil::com_ptr<CDialogHtml::HostExternal>& host_external)
	{
		if (options.isNullOrUndefined())
		{
			return {};
		}

		QwrException::ExpectTrue(options.isObject(), "options argument is not an object");

		JS::RootedObject jsObject(m_ctx, &options.toObject());
		bool hasData{};

		if (JS_HasProperty(m_ctx, jsObject, "data", &hasData) && hasData)
		{
			JS::RootedValue jsValue(m_ctx);

			if (JS_GetProperty(m_ctx, jsObject, "data", &jsValue))
			{
				_variant_t data;
				convert::JsToVariant(m_ctx, jsValue, *data.GetAddress());
				host_external = new ComObject<CDialogHtml::HostExternal>(data);
			}
		}

		auto html_options = CDialogHtml::Options{
			.width = GetOptionalProperty<uint32_t>(m_ctx, jsObject, "width").value_or(250u),
			.height = GetOptionalProperty<uint32_t>(m_ctx, jsObject, "height").value_or(100u),
			.x = GetOptionalProperty<int32_t>(m_ctx, jsObject, "x").value_or(0),
			.y = GetOptionalProperty<int32_t>(m_ctx, jsObject, "y").value_or(0),
			.isCentered = GetOptionalProperty<bool>(m_ctx, jsObject, "center").value_or(true),
			.isContextMenuEnabled = GetOptionalProperty<bool>(m_ctx, jsObject, "context_menu").value_or(false),
			.isFormSelectionEnabled = GetOptionalProperty<bool>(m_ctx, jsObject, "selection").value_or(false),
			.isResizable = GetOptionalProperty<bool>(m_ctx, jsObject, "resizable").value_or(false),
			.isScrollEnabled = GetOptionalProperty<bool>(m_ctx, jsObject, "scroll").value_or(false)
		};

		return html_options;
	}

	std::unique_ptr<Utils> Utils::CreateNative(JSContext* ctx)
	{
		return std::unique_ptr<Utils>(new Utils(ctx));
	}

	uint32_t Utils::GetInternalSize()
	{
		return 0;
	}

	bool Utils::CheckComponent(const std::string& name, bool is_dll) const
	{
		pfc::string8 temp;

		for (auto ptr : componentversion::enumerate())
		{
			if (is_dll)
			{
				ptr->get_file_name(temp);
			}
			else
			{
				ptr->get_component_name(temp);
			}

			if (name == temp.get_ptr())
			{
				return true;
			}
		}

		return false;
	}

	bool Utils::CheckComponentWithOpt(size_t optArgCount, const std::string& name, bool is_dll) const
	{
		switch (optArgCount)
		{
		case 0: return CheckComponent(name, is_dll);
		case 1: return CheckComponent(name);
		default: throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	bool Utils::CheckFont(const std::wstring& name) const
	{
		return FontHelper::get().check_name(name);
	}

	uint32_t Utils::ColourPicker(uint32_t, uint32_t default_colour)
	{
		static std::array<COLORREF, 16> colours{};
		const auto wnd = GetPanelHwndForCurrentGlobal(m_ctx);
		QwrException::ExpectTrue(wnd, "Method called before fb2k was initialized completely");

		auto colour = smp::ArgbToColorref(default_colour);
		uChooseColor(&colour, wnd, colours.data());
		return smp::ColorrefToArgb(colour);
	}

	std::string Utils::ConvertToAscii(const std::string& str)
	{
		return pfc::stringcvt::string_ascii_from_utf8(str.c_str(), str.length()).get_ptr();
	}

	bool Utils::CopyFile(const std::wstring& from, const std::wstring& to, bool overwrite) const
	{
		return FileHelper(from).copy_file(to, overwrite);
	}

	bool Utils::CopyFileWithOpt(size_t optArgCount, const std::wstring& from, const std::wstring& to, bool overwrite) const
	{
		switch (optArgCount)
		{
		case 0: return CopyFile(from, to, overwrite);
		case 1: return CopyFile(from, to);
		default: throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	bool Utils::CopyFolder(const std::wstring& from, const std::wstring& to, bool overwrite, bool recur) const
	{
		return FileHelper(from).copy_folder(to, overwrite, recur);
	}

	bool Utils::CopyFolderWithOpt(size_t optArgCount, const std::wstring& from, const std::wstring& to, bool overwrite, bool recur) const
	{
		switch (optArgCount)
		{
		case 0: return CopyFolder(from, to, overwrite, recur);
		case 1: return CopyFolder(from, to, overwrite);
		case 2: return CopyFolder(from, to);
		default: throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	bool Utils::CreateFolder(const std::wstring& path) const
	{
		return FileHelper(path).create_folder();
	}

	uint32_t Utils::DetectCharset(const std::wstring& path) const
	{
		return TextFile(path).guess_codepage();
	}

	void Utils::DownloadFileAsync(const std::string& url, const std::wstring& path)
	{
		const auto wnd = GetPanelHwndForCurrentGlobal(m_ctx);
		QwrException::ExpectTrue(wnd, "Method called before fb2k was initialized completely");

		auto task = fb2k::service_new<::DownloadFileAsync>(wnd, url, path);
		fb2k::cpuThreadPool::get()->runSingle(task);
	}

	void Utils::EditTextFile(const std::wstring& path)
	{
		const auto wnd = GetPanelHwndForCurrentGlobal(m_ctx);
		QwrException::ExpectTrue(wnd, "Method called before fb2k was initialized completely");

		if (!modal_dialog_scope::can_create())
		{
			return;
		}

		modal_dialog_scope scope(wnd);

		// TODO: add options - editor_path, is_modal
		smp::EditTextFile(wnd, path, false, false);
	}

	bool Utils::FileExists(const std::wstring& path) const
	{
		return FileHelper(path).exists();
	}

	JS::Value Utils::FileTest(const std::wstring& path, const std::wstring& mode)
	{
		if (L"e" == mode) // exists
		{
			JS::RootedValue jsValue(m_ctx);
			convert::to_js::ToValue(m_ctx, FileExists(path), &jsValue);
			return jsValue;
		}
		else if (L"s" == mode)
		{
			JS::RootedValue jsValue(m_ctx);
			convert::to_js::ToValue(m_ctx, GetFileSize(path), &jsValue);
			return jsValue;
		}
		else if (L"d" == mode)
		{
			JS::RootedValue jsValue(m_ctx);
			convert::to_js::ToValue(m_ctx, IsDirectory(path), &jsValue);
			return jsValue;
		}
		else if (L"split" == mode)
		{
			return SplitFilePath(path);
		}
		else if (L"chardet" == mode)
		{
			JS::RootedValue jsValue(m_ctx);
			convert::to_js::ToValue(m_ctx, DetectCharset(path), &jsValue);
			return jsValue;
		}
		else
		{
			throw QwrException("Invalid value of mode argument: '{}'", smp::ToU8(mode));
		}
	}

	std::string Utils::FormatDuration(double p) const
	{
		return pfc::format_time_ex(p, 0).get_ptr();
	}

	std::string Utils::FormatFileSize(uint64_t p) const
	{
		return pfc::format_file_size_short(p).get_ptr();
	}

	void Utils::GetAlbumArtAsync(uint32_t, JsFbMetadbHandle* handle, uint32_t art_id, bool need_stub, bool only_embed, bool)
	{
		const auto wnd = GetPanelHwndForCurrentGlobal(m_ctx);
		QwrException::ExpectTrue(wnd, "Method called before fb2k was initialized completely");
		QwrException::ExpectTrue(handle, "handle argument is null");
		QwrException::ExpectTrue(AlbumArtStatic::check_type_id(art_id), "Invalid art_id");

		auto task = fb2k::service_new<::GetAlbumArtAsync>(wnd, handle->GetHandle(), art_id, need_stub, only_embed);
		fb2k::cpuThreadPool::get()->runSingle(task);
	}

	void Utils::GetAlbumArtAsyncWithOpt(size_t optArgCount, uint32_t hWnd, JsFbMetadbHandle* handle, uint32_t art_id, bool need_stub, bool only_embed, bool no_load)
	{
		switch (optArgCount)
		{
		case 0: return GetAlbumArtAsync(hWnd, handle, art_id, need_stub, only_embed, no_load);
		case 1: return GetAlbumArtAsync(hWnd, handle, art_id, need_stub, only_embed);
		case 2: return GetAlbumArtAsync(hWnd, handle, art_id, need_stub);
		case 3: return GetAlbumArtAsync(hWnd, handle, art_id);
		case 4: return GetAlbumArtAsync(hWnd, handle);
		default: throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	JSObject* Utils::GetAlbumArtAsyncV2(uint32_t /*window_id*/, JsFbMetadbHandle* handle, uint32_t art_id, bool need_stub, bool only_embed, bool)
	{
		const auto wnd = GetPanelHwndForCurrentGlobal(m_ctx);
		QwrException::ExpectTrue(wnd, "Method called before fb2k was initialized completely");
		QwrException::ExpectTrue(handle, "handle argument is null");
		QwrException::ExpectTrue(AlbumArtStatic::check_type_id(art_id), "Invalid art_id");

		return mozjs::GetAlbumArtPromise(m_ctx, wnd, handle->GetHandle(), art_id, need_stub, only_embed);
	}

	JSObject* Utils::GetAlbumArtAsyncV2WithOpt(size_t optArgCount, uint32_t hWnd, JsFbMetadbHandle* handle, uint32_t art_id, bool need_stub, bool only_embed, bool no_load)
	{
		switch (optArgCount)
		{
		case 0: return GetAlbumArtAsyncV2(hWnd, handle, art_id, need_stub, only_embed, no_load);
		case 1: return GetAlbumArtAsyncV2(hWnd, handle, art_id, need_stub, only_embed);
		case 2: return GetAlbumArtAsyncV2(hWnd, handle, art_id, need_stub);
		case 3: return GetAlbumArtAsyncV2(hWnd, handle, art_id);
		case 4: return GetAlbumArtAsyncV2(hWnd, handle);
		default: throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	JSObject* Utils::GetAlbumArtEmbedded(const std::string& rawpath, uint32_t art_id)
	{
		QwrException::ExpectTrue(AlbumArtStatic::check_type_id(art_id), "Invalid art_id");

		auto data = AlbumArtStatic::get_embedded(rawpath, art_id);
		auto bitmap = AlbumArtStatic::to_bitmap(data);
		if (!bitmap)
			return nullptr;

		return JsGdiBitmap::CreateJs(m_ctx, std::move(bitmap));
	}

	JSObject* Utils::GetAlbumArtEmbeddedWithOpt(size_t optArgCount, const std::string& rawpath, uint32_t art_id)
	{
		switch (optArgCount)
		{
		case 0: return GetAlbumArtEmbedded(rawpath, art_id);
		case 1: return GetAlbumArtEmbedded(rawpath);
		default: throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	JSObject* Utils::GetAlbumArtV2(JsFbMetadbHandle* handle, uint32_t art_id, bool need_stub)
	{
		QwrException::ExpectTrue(handle, "handle argument is null");
		QwrException::ExpectTrue(AlbumArtStatic::check_type_id(art_id), "Invalid art_id");

		std::string dummy_path;
		auto data = AlbumArtStatic::get(handle->GetHandle(), art_id, need_stub, false, dummy_path);
		auto bitmap = AlbumArtStatic::to_bitmap(data);
		if (!bitmap)
			return nullptr;

		return JsGdiBitmap::CreateJs(m_ctx, std::move(bitmap));
	}

	JSObject* Utils::GetAlbumArtV2WithOpt(size_t optArgCount, JsFbMetadbHandle* handle, uint32_t art_id, bool need_stub)
	{
		switch (optArgCount)
		{
		case 0: return GetAlbumArtV2(handle, art_id, need_stub);
		case 1: return GetAlbumArtV2(handle, art_id);
		case 2: return GetAlbumArtV2(handle);
		default: throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	std::string Utils::GetClipboardText() const
	{
		pfc::string8 text;
		uGetClipboardString(text);
		return text.get_ptr();
	}

	uint64_t Utils::GetFileSize(const std::wstring& path) const
	{
		return FileHelper(path).file_size();
	}

	uint64_t Utils::GetLastModified(const std::wstring& path) const
	{
		return FileHelper(path).last_modified();
	}

	JSObject* Utils::GetPackageInfo(const std::string& packageId) const
	{
		const auto packagePathOpt = PackageUtils::Find(packageId);
		if (!packagePathOpt)
		{
			return nullptr;
		}

		const auto settings = PackageUtils::GetSettingsFromPath(*packagePathOpt);

		JS::RootedObject jsDirs(m_ctx, JS_NewPlainObject(m_ctx));
		AddProperty(m_ctx, jsDirs, "Root", PackageUtils::GetPath(settings).wstring());
		AddProperty(m_ctx, jsDirs, "Assets", PackageUtils::GetAssetsDir(settings).wstring());
		AddProperty(m_ctx, jsDirs, "Scripts", PackageUtils::GetScriptsDir(settings).wstring());
		AddProperty(m_ctx, jsDirs, "Storage", PackageUtils::GetStorageDir(settings).wstring());

		JS::RootedObject jsObject(m_ctx, JS_NewPlainObject(m_ctx));
		AddProperty(m_ctx, jsObject, "Directories", static_cast<JS::HandleObject>(jsDirs));
		AddProperty(m_ctx, jsObject, "Version", settings.scriptVersion);

		return jsObject;
	}

	std::wstring Utils::GetPackagePath(const std::string& packageId) const
	{
		const auto packagePathOpt = PackageUtils::Find(packageId);
		QwrException::ExpectTrue(packagePathOpt.has_value(), "Unknown package: {}", packageId);

		return packagePathOpt->native();
	}

	uint32_t Utils::GetSysColour(uint32_t index) const
	{
		const auto hBrush = ::GetSysColorBrush(index); ///< no need to call DeleteObject here
		QwrException::ExpectTrue(hBrush, "Invalid color index: {}", index);

		return smp::ColorrefToArgb(::GetSysColor(index));
	}

	uint32_t Utils::GetSystemMetrics(uint32_t index) const
	{
		return ::GetSystemMetrics(index);
	}

	JS::Value Utils::Glob(const std::wstring& pattern, uint32_t exc_mask, uint32_t inc_mask)
	{
		std::vector<std::wstring> files;
		WIN32_FIND_DATA data{};
		auto hFindFile = wil::unique_hfind(FindFirstFileW(pattern.data(), &data));

		if (hFindFile)
		{
			const auto folder = std::filesystem::path(pattern).parent_path().native() + std::filesystem::path::preferred_separator;

			while (true)
			{
				const DWORD attr = data.dwFileAttributes;

				if (WI_IsAnyFlagSet(attr, inc_mask) && !WI_IsAnyFlagSet(attr, exc_mask))
				{
					files.emplace_back(folder + data.cFileName);
				}

				if (!FindNextFileW(hFindFile.get(), &data))
					break;
			}
		}

		std::ranges::sort(files, CmpW());

		JS::RootedValue jsValue(m_ctx);
		convert::to_js::ToArrayValue(m_ctx, files, &jsValue);
		return jsValue;
	}

	JS::Value Utils::GlobWithOpt(size_t optArgCount, const std::wstring& pattern, uint32_t exc_mask, uint32_t inc_mask)
	{
		switch (optArgCount)
		{
		case 0: return Glob(pattern, exc_mask, inc_mask);
		case 1: return Glob(pattern, exc_mask);
		case 2: return Glob(pattern);
		default: throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	uint32_t Utils::HTTPRequestAsync(uint32_t type, const std::string& url, const std::string& user_agent_or_headers, const std::string& body)
	{
		static uint32_t task_id{};

		const auto wnd = GetPanelHwndForCurrentGlobal(m_ctx);
		QwrException::ExpectTrue(wnd, "Method called before fb2k was initialized completely");
		QwrException::ExpectTrue(type <= 1, "Invalid type argument");

		const auto type_enum = static_cast<HTTPRequestAsync::Type>(type);
		auto task = fb2k::service_new<::HTTPRequestAsync>(type_enum, wnd, ++task_id, url, user_agent_or_headers, body);
		fb2k::cpuThreadPool::get()->runSingle(task);

		return task_id;
	}

	uint32_t Utils::HTTPRequestAsyncWithOpt(size_t optArgCount, uint32_t type, const std::string& url, const std::string& user_agent_or_headers, const std::string& body)
	{
		switch (optArgCount)
		{
		case 0: return HTTPRequestAsync(type, url, user_agent_or_headers, body);
		case 1: return HTTPRequestAsync(type, url, user_agent_or_headers);
		case 2: return HTTPRequestAsync(type, url);
		default: throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	std::string Utils::InputBox(uint32_t, const std::string& prompt, const std::string& caption, const std::string& def, bool error_on_cancel)
	{
		const auto wnd = GetPanelHwndForCurrentGlobal(m_ctx);
		QwrException::ExpectTrue(wnd, "Method called before fb2k was initialized completely");

		if (modal_dialog_scope::can_create())
		{
			modal_dialog_scope scope(wnd);

			CInputBox dlg(prompt.c_str(), caption.c_str(), def.c_str());
			const auto status = dlg.DoModal(wnd);

			if (status == IDCANCEL && error_on_cancel)
			{
				throw QwrException("Dialog window was closed");
			}

			if (status == IDOK)
			{
				return dlg.GetValue();
			}
		}

		return def;
	}

	std::string Utils::InputBoxWithOpt(size_t optArgCount, uint32_t hWnd, const std::string& prompt, const std::string& caption, const std::string& def, bool error_on_cancel)
	{
		switch (optArgCount)
		{
		case 0: return InputBox(hWnd, prompt, caption, def, error_on_cancel);
		case 1: return InputBox(hWnd, prompt, caption, def);
		case 2: return InputBox(hWnd, prompt, caption);
		default: throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	bool Utils::IsDirectory(const std::wstring& path) const
	{
		return FileHelper(path).is_folder();
	}

	bool Utils::IsFile(const std::wstring& path) const
	{
		return FileHelper(path).is_file();
	}

	bool Utils::IsKeyPressed(uint32_t vkey) const
	{
		return ::IsKeyPressed(vkey);
	}

	JS::Value Utils::ListFonts()
	{
		const auto fonts = FontHelper::get().get_names();

		JS::RootedValue jsValue(m_ctx);
		convert::to_js::ToArrayValue(m_ctx, fonts, &jsValue);
		return jsValue;
	}

	std::wstring Utils::MapString(const std::wstring& str, uint32_t lcid, uint32_t flags)
	{
		// WinAPI is weird: 0 - error (with LastError), > 0 - characters required
		int iRet = LCIDToLocaleName(lcid, nullptr, 0, LOCALE_ALLOW_NEUTRAL_NAMES);
		smp::CheckWinApi(iRet, "LCIDToLocaleName(nullptr)");

		std::wstring localeName(iRet, '\0');
		iRet = LCIDToLocaleName(lcid, localeName.data(), sizeu(localeName), LOCALE_ALLOW_NEUTRAL_NAMES);
		smp::CheckWinApi(iRet, "LCIDToLocaleName(data)");

		std::optional<NLSVERSIONINFOEX> versionInfo;
		try
		{
			if (_WIN32_WINNT_WIN7 > GetWindowsVersionCode())
			{
				NLSVERSIONINFOEX tmpVersionInfo{};
				BOOL bRet = GetNLSVersionEx(COMPARE_STRING, localeName.c_str(), &tmpVersionInfo);
				smp::CheckWinApi(bRet, "GetNLSVersionEx");

				versionInfo = tmpVersionInfo;
			}
		}
		catch (const std::exception&)
		{
		}

		auto* pVersionInfo = reinterpret_cast<NLSVERSIONINFO*>(versionInfo ? &(*versionInfo) : nullptr);

		iRet = LCMapStringEx(localeName.c_str(), flags, str.c_str(), lengthu(str) + 1, nullptr, 0, pVersionInfo, nullptr, 0);
		smp::CheckWinApi(iRet, "LCMapStringEx(nullptr)");

		std::wstring dst(iRet, '\0');
		iRet = LCMapStringEx(localeName.c_str(), flags, str.c_str(), lengthu(str) + 1, dst.data(), lengthu(dst), pVersionInfo, nullptr, 0);
		smp::CheckWinApi(iRet, "LCMapStringEx(data)");

		dst.resize(lengthu(dst));
		return dst;
	}

	uint32_t Utils::Now()
	{
		return to_uint(pfc::fileTimeWtoU(pfc::fileTimeNow()));
	}

	bool Utils::PathWildcardMatch(const std::wstring& pattern, const std::wstring& str)
	{
		return PathMatchSpecW(str.c_str(), pattern.c_str());
	}

	std::wstring Utils::ReadINI(const std::wstring& path, const std::wstring& section, const std::wstring& key, const std::wstring& defaultval)
	{
		std::array<wchar_t, MAX_PATH> buffer{};
		GetPrivateProfileStringW(section.data(), key.data(), defaultval.data(), buffer.data(), MAX_PATH, path.data());
		return buffer.data();
	}

	std::wstring Utils::ReadINIWithOpt(size_t optArgCount, const std::wstring& path, const std::wstring& section, const std::wstring& key, const std::wstring& defaultval)
	{
		switch (optArgCount)
		{
		case 0: return ReadINI(path, section, key, defaultval);
		case 1: return ReadINI(path, section, key);
		default: throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	std::wstring Utils::ReadTextFile(const std::wstring& path, uint32_t codepage)
	{
		std::wstring content;
		TextFile(path).read_wide(codepage, content);
		return content;
	}

	std::wstring Utils::ReadTextFileWithOpt(size_t optArgCount, const std::wstring& path, uint32_t codepage)
	{
		switch (optArgCount)
		{
		case 0: return ReadTextFile(path, codepage);
		case 1: return ReadTextFile(path);
		default: throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	std::string Utils::ReadUTF8(const std::wstring& path)
	{
		return TextFile(path).read();
	}

	int32_t Utils::RemovePath(const std::wstring& path) const
	{
		return FileHelper(path).remove_all();
	}

	bool Utils::RenamePath(const std::wstring& from, const std::wstring& to) const
	{
		return FileHelper::rename(from, to);
	}

	std::string Utils::ReplaceIllegalChars(const std::string& str, bool strip_trailing_periods)
	{
		const std::string ret = pfc::io::path::replaceIllegalNameChars(str.c_str(), false, pfc::io::path::charReplaceModern).get_ptr();
		size_t len = ret.length();

		if (strip_trailing_periods && ret.ends_with('.'))
		{
			for (const char c : ret | std::views::reverse)
			{
				if (c != '.')
					break;

				len--;
			}
		}

		return ret.substr(0uz, len);
	}

	std::string Utils::ReplaceIllegalCharsWithOpt(size_t optArgCount, const std::string& str, bool strip_trailing_periods)
	{
		switch (optArgCount)
		{
		case 0: return ReplaceIllegalChars(str, strip_trailing_periods);
		case 1: return ReplaceIllegalChars(str);
		default: throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	void Utils::SetClipboardText(const std::string& text)
	{
		uSetClipboardString(text.c_str());
	}

	void Utils::ShowHtmlDialog(uint32_t, const std::wstring& code_or_path, JS::HandleValue options)
	{
		const auto wnd = GetPanelHwndForCurrentGlobal(m_ctx);
		QwrException::ExpectTrue(wnd, "Method called before fb2k was initialized completely");

		if (modal_dialog_scope::can_create())
		{
			modal_dialog_scope scope(wnd);
			wil::com_ptr<CDialogHtml::HostExternal> host_external;

			auto html_options = ParseHTMLOptions(options, host_external);
			auto dlg = CDialogHtml(m_ctx, html_options, code_or_path, std::move(host_external));
			auto iRet = dlg.DoModal(wnd);

			if (-1 == iRet || IDABORT == iRet)
			{
				if (JS_IsExceptionPending(m_ctx))
				{
					throw JsException();
				}
				else
				{
					throw QwrException("DoModal failed: {}", iRet);
				}
			}
		}
	}

	void Utils::ShowHtmlDialogWithOpt(size_t optArgCount, uint32_t hWnd, const std::wstring& code_or_path, JS::HandleValue options)
	{
		switch (optArgCount)
		{
		case 0: ShowHtmlDialog(hWnd, code_or_path, options); break;
		case 1: ShowHtmlDialog(hWnd, code_or_path); break;
		default: throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	JS::Value Utils::SplitFilePath(const std::wstring& path)
	{
		const auto cleanedPath = std::filesystem::path(path).lexically_normal();

		std::vector<std::wstring> out(3);
		if (PathIsFileSpecW(cleanedPath.filename().c_str()))
		{
			out[0] = cleanedPath.parent_path() / "";
			out[1] = cleanedPath.stem();
			out[2] = cleanedPath.extension();
		}
		else
		{
			out[0] = cleanedPath / "";
		}

		JS::RootedValue jsValue(m_ctx);
		convert::to_js::ToArrayValue(m_ctx, out, &jsValue);

		return jsValue;
	}

	bool Utils::WriteINI(const std::wstring& path, const std::wstring& section, const std::wstring& key, const std::wstring& val)
	{
		return WritePrivateProfileStringW(section.c_str(), key.c_str(), val.c_str(), path.c_str());
	}

	bool Utils::WriteTextFile(const std::wstring& path, const std::string& content, bool write_bom)
	{
		return TextFile(path).write(content, write_bom);
	}

	bool Utils::WriteTextFileWithOpt(size_t optArgCount, const std::wstring& path, const std::string& content, bool write_bom)
	{
		switch (optArgCount)
		{
		case 0: return WriteTextFile(path, content, write_bom);
		case 1: return WriteTextFile(path, content);
		default: throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	std::string Utils::get_Version() const
	{
		return Component::version.data();
	}
}
