#include "PCH.hpp"
#include "fb.h"

#include <2K3/AlbumArtStatic.hpp>
#include <2K3/ContextMenuCommand.hpp>
#include <2K3/MainMenuCommand.hpp>
#include <com_utils/drop_source_impl.h>
#include <events/event_dispatcher.h>
#include <events/event_js_callback.h>
#include <interfaces/context_menu_manager.h>
#include <interfaces/fb_audio_chunk.h>
#include <interfaces/fb_metadb_handle.h>
#include <interfaces/fb_metadb_handle_list.h>
#include <interfaces/fb_profiler.h>
#include <interfaces/fb_title_format.h>
#include <interfaces/fb_ui_selection_holder.h>
#include <interfaces/gdi_bitmap.h>
#include <interfaces/main_menu_manager.h>
#include <panel/modal_blocking_scope.h>
#include <panel/user_message.h>
#include <utils/mainmenu_dynamic.h>
#include <utils/menu_helpers.h>

namespace
{
	using namespace mozjs;

	DEFINE_JS_CLASS_OPS(Fb::FinalizeJsObject)

	DEFINE_JS_CLASS("FbUtils")

	MJS_DEFINE_JS_FN_FROM_NATIVE(AcquireUiSelectionHolder, Fb::AcquireUiSelectionHolder)
	MJS_DEFINE_JS_FN_FROM_NATIVE(AddDirectory, Fb::AddDirectory)
	MJS_DEFINE_JS_FN_FROM_NATIVE(AddFiles, Fb::AddFiles)
	MJS_DEFINE_JS_FN_FROM_NATIVE(AddLocationsAsync, Fb::AddLocationsAsync);
	MJS_DEFINE_JS_FN_FROM_NATIVE(CheckClipboardContents, Fb::CheckClipboardContents)
	MJS_DEFINE_JS_FN_FROM_NATIVE(ClearPlaylist, Fb::ClearPlaylist)
	MJS_DEFINE_JS_FN_FROM_NATIVE(CopyHandleListToClipboard, Fb::CopyHandleListToClipboard)
	MJS_DEFINE_JS_FN_FROM_NATIVE(CreateContextMenuManager, Fb::CreateContextMenuManager)
	MJS_DEFINE_JS_FN_FROM_NATIVE(CreateHandleList, Fb::CreateHandleList)
	MJS_DEFINE_JS_FN_FROM_NATIVE(CreateMainMenuManager, Fb::CreateMainMenuManager)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(CreateProfiler, Fb::CreateProfiler, Fb::CreateProfilerWithOpt, 1)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(DoDragDrop, Fb::DoDragDrop, Fb::DoDragDropWithOpt, 1)
	MJS_DEFINE_JS_FN_FROM_NATIVE(Exit, Fb::Exit)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(GetAudioChunk, Fb::GetAudioChunk, Fb::GetAudioChunkWithOpt, 1)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(GetClipboardContents, Fb::GetClipboardContents, Fb::GetClipboardContentsWithOpt, 1)
	MJS_DEFINE_JS_FN_FROM_NATIVE(GetDSPPresets, Fb::GetDSPPresets)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(GetFocusItem, Fb::GetFocusItem, Fb::GetFocusItemWithOpt, 1)
	MJS_DEFINE_JS_FN_FROM_NATIVE(GetLibraryItems, Fb::GetLibraryItems)
	MJS_DEFINE_JS_FN_FROM_NATIVE(GetLibraryRelativePath, Fb::GetLibraryRelativePath)
	MJS_DEFINE_JS_FN_FROM_NATIVE(GetNowPlaying, Fb::GetNowPlaying)
	MJS_DEFINE_JS_FN_FROM_NATIVE(GetOutputDevices, Fb::GetOutputDevices)
	MJS_DEFINE_JS_FN_FROM_NATIVE(GetQueryItems, Fb::GetQueryItems)
	MJS_DEFINE_JS_FN_FROM_NATIVE(GetSelection, Fb::GetSelection)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(GetSelections, Fb::GetSelections, Fb::GetSelectionsWithOpt, 1)
	MJS_DEFINE_JS_FN_FROM_NATIVE(GetSelectionType, Fb::GetSelectionType)
	MJS_DEFINE_JS_FN_FROM_NATIVE(IsLibraryEnabled, Fb::IsLibraryEnabled)
	MJS_DEFINE_JS_FN_FROM_NATIVE(IsMainMenuCommandChecked, Fb::IsMainMenuCommandChecked)
	MJS_DEFINE_JS_FN_FROM_NATIVE(IsMetadbInMediaLibrary, Fb::IsMetadbInMediaLibrary)
	MJS_DEFINE_JS_FN_FROM_NATIVE(LoadPlaylist, Fb::LoadPlaylist)
	MJS_DEFINE_JS_FN_FROM_NATIVE(Next, Fb::Next)
	MJS_DEFINE_JS_FN_FROM_NATIVE(Pause, Fb::Pause)
	MJS_DEFINE_JS_FN_FROM_NATIVE(Play, Fb::Play)
	MJS_DEFINE_JS_FN_FROM_NATIVE(PlayOrPause, Fb::PlayOrPause)
	MJS_DEFINE_JS_FN_FROM_NATIVE(Prev, Fb::Prev)
	MJS_DEFINE_JS_FN_FROM_NATIVE(Random, Fb::Random)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(RegisterMainMenuCommand, Fb::RegisterMainMenuCommand, Fb::RegisterMainMenuCommandWithOpt, 1)
	MJS_DEFINE_JS_FN_FROM_NATIVE(RequestBacklog, Fb::RequestBacklog)
	MJS_DEFINE_JS_FN_FROM_NATIVE(Restart, Fb::Restart)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(RunContextCommand, Fb::RunContextCommand, Fb::RunContextCommandWithOpt, 1)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(RunContextCommandWithMetadb, Fb::RunContextCommandWithMetadb, Fb::RunContextCommandWithMetadbWithOpt, 1)
	MJS_DEFINE_JS_FN_FROM_NATIVE(RunMainMenuCommand, Fb::RunMainMenuCommand)
	MJS_DEFINE_JS_FN_FROM_NATIVE(SavePlaylist, Fb::SavePlaylist)
	MJS_DEFINE_JS_FN_FROM_NATIVE(SetDSPPreset, Fb::SetDSPPreset)
	MJS_DEFINE_JS_FN_FROM_NATIVE(SetOutputDevice, Fb::SetOutputDevice)
	MJS_DEFINE_JS_FN_FROM_NATIVE(ShowConsole, Fb::ShowConsole)
	MJS_DEFINE_JS_FN_FROM_NATIVE(ShowLibrarySearchUI, Fb::ShowLibrarySearchUI)
	MJS_DEFINE_JS_FN_FROM_NATIVE(ShowPictureViewer, Fb::ShowPictureViewer)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(ShowPopupMessage, Fb::ShowPopupMessage, Fb::ShowPopupMessageWithOpt, 1)
	MJS_DEFINE_JS_FN_FROM_NATIVE(ShowPreferences, Fb::ShowPreferences)
	MJS_DEFINE_JS_FN_FROM_NATIVE(Stop, Fb::Stop)
	MJS_DEFINE_JS_FN_FROM_NATIVE(TitleFormat, Fb::TitleFormat)
	MJS_DEFINE_JS_FN_FROM_NATIVE(UnregisterMainMenuCommand, Fb::UnregisterMainMenuCommand)
	MJS_DEFINE_JS_FN_FROM_NATIVE(VolumeDown, Fb::VolumeDown)
	MJS_DEFINE_JS_FN_FROM_NATIVE(VolumeMute, Fb::VolumeMute)
	MJS_DEFINE_JS_FN_FROM_NATIVE(VolumeUp, Fb::VolumeUp)

	constexpr auto jsFunctions = std::to_array<JSFunctionSpec>(
		{
			JS_FN("AcquireUiSelectionHolder", AcquireUiSelectionHolder, 0, kDefaultPropsFlags),
			JS_FN("AddDirectory", AddDirectory, 0, kDefaultPropsFlags),
			JS_FN("AddFiles", AddFiles, 0, kDefaultPropsFlags),
			JS_FN("AddLocationsAsync", AddLocationsAsync, 1, kDefaultPropsFlags),
			JS_FN("CheckClipboardContents", CheckClipboardContents, 0, kDefaultPropsFlags),
			JS_FN("ClearPlaylist", ClearPlaylist, 0, kDefaultPropsFlags),
			JS_FN("CopyHandleListToClipboard", CopyHandleListToClipboard, 1, kDefaultPropsFlags),
			JS_FN("CreateContextMenuManager", CreateContextMenuManager, 0, kDefaultPropsFlags),
			JS_FN("CreateHandleList", CreateHandleList, 0, kDefaultPropsFlags),
			JS_FN("CreateMainMenuManager", CreateMainMenuManager, 0, kDefaultPropsFlags),
			JS_FN("CreateProfiler", CreateProfiler, 0, kDefaultPropsFlags),
			JS_FN("DoDragDrop", DoDragDrop, 3, kDefaultPropsFlags),
			JS_FN("Exit", Exit, 0, kDefaultPropsFlags),
			JS_FN("GetAudioChunk", GetAudioChunk, 2, kDefaultPropsFlags),
			JS_FN("GetClipboardContents", GetClipboardContents, 0, kDefaultPropsFlags),
			JS_FN("GetDSPPresets", GetDSPPresets, 0, kDefaultPropsFlags),
			JS_FN("GetFocusItem", GetFocusItem, 0, kDefaultPropsFlags),
			JS_FN("GetLibraryItems", GetLibraryItems, 0, kDefaultPropsFlags),
			JS_FN("GetLibraryRelativePath", GetLibraryRelativePath, 1, kDefaultPropsFlags),
			JS_FN("GetNowPlaying", GetNowPlaying, 0, kDefaultPropsFlags),
			JS_FN("GetOutputDevices", GetOutputDevices, 0, kDefaultPropsFlags),
			JS_FN("GetQueryItems", GetQueryItems, 2, kDefaultPropsFlags),
			JS_FN("GetSelection", GetSelection, 0, kDefaultPropsFlags),
			JS_FN("GetSelections", GetSelections, 0, kDefaultPropsFlags),
			JS_FN("GetSelectionType", GetSelectionType, 0, kDefaultPropsFlags),
			JS_FN("IsLibraryEnabled", IsLibraryEnabled, 0, kDefaultPropsFlags),
			JS_FN("IsMainMenuCommandChecked", IsMainMenuCommandChecked, 1, kDefaultPropsFlags),
			JS_FN("IsMetadbInMediaLibrary", IsMetadbInMediaLibrary, 1, kDefaultPropsFlags),
			JS_FN("LoadPlaylist", LoadPlaylist, 0, kDefaultPropsFlags),
			JS_FN("Next", Next, 0, kDefaultPropsFlags),
			JS_FN("Pause", Pause, 0, kDefaultPropsFlags),
			JS_FN("Play", Play, 0, kDefaultPropsFlags),
			JS_FN("PlayOrPause", PlayOrPause, 0, kDefaultPropsFlags),
			JS_FN("Prev", Prev, 0, kDefaultPropsFlags),
			JS_FN("Random", Random, 0, kDefaultPropsFlags),
			JS_FN("RegisterMainMenuCommand", RegisterMainMenuCommand, 2, kDefaultPropsFlags),
			JS_FN("RequestBacklog", RequestBacklog, 1, kDefaultPropsFlags),
			JS_FN("Restart", Restart, 0, kDefaultPropsFlags),
			JS_FN("RunContextCommand", RunContextCommand, 1, kDefaultPropsFlags),
			JS_FN("RunContextCommandWithMetadb", RunContextCommandWithMetadb, 2, kDefaultPropsFlags),
			JS_FN("RunMainMenuCommand", RunMainMenuCommand, 1, kDefaultPropsFlags),
			JS_FN("SavePlaylist", SavePlaylist, 0, kDefaultPropsFlags),
			JS_FN("SetDSPPreset", SetDSPPreset, 1, kDefaultPropsFlags),
			JS_FN("SetOutputDevice", SetOutputDevice, 2, kDefaultPropsFlags),
			JS_FN("ShowConsole", ShowConsole, 0, kDefaultPropsFlags),
			JS_FN("ShowLibrarySearchUI", ShowLibrarySearchUI, 1, kDefaultPropsFlags),
			JS_FN("ShowPictureViewer", ShowPictureViewer, 1, kDefaultPropsFlags),
			JS_FN("ShowPopupMessage", ShowPopupMessage, 1, kDefaultPropsFlags),
			JS_FN("ShowPreferences", ShowPreferences, 0, kDefaultPropsFlags),
			JS_FN("Stop", Stop, 0, kDefaultPropsFlags),
			JS_FN("TitleFormat", TitleFormat, 1, kDefaultPropsFlags),
			JS_FN("UnregisterMainMenuCommand", UnregisterMainMenuCommand, 1, kDefaultPropsFlags),
			JS_FN("VolumeDown", VolumeDown, 0, kDefaultPropsFlags),
			JS_FN("VolumeMute", VolumeMute, 0, kDefaultPropsFlags),
			JS_FN("VolumeUp", VolumeUp, 0, kDefaultPropsFlags),
			JS_FS_END,
		});

	MJS_DEFINE_JS_FN_FROM_NATIVE(get_AlwaysOnTop, Fb::get_AlwaysOnTop)
	MJS_DEFINE_JS_FN_FROM_NATIVE(get_ComponentPath, Fb::get_ComponentPath)
	MJS_DEFINE_JS_FN_FROM_NATIVE(get_CursorFollowPlayback, Fb::get_CursorFollowPlayback)
	MJS_DEFINE_JS_FN_FROM_NATIVE(get_CustomVolume, Fb::get_CustomVolume)
	MJS_DEFINE_JS_FN_FROM_NATIVE(get_FoobarPath, Fb::get_FoobarPath)
	MJS_DEFINE_JS_FN_FROM_NATIVE(get_IsPaused, Fb::get_IsPaused)
	MJS_DEFINE_JS_FN_FROM_NATIVE(get_IsPlaying, Fb::get_IsPlaying)
	MJS_DEFINE_JS_FN_FROM_NATIVE(get_PlaybackFollowCursor, Fb::get_PlaybackFollowCursor)
	MJS_DEFINE_JS_FN_FROM_NATIVE(get_PlaybackLength, Fb::get_PlaybackLength)
	MJS_DEFINE_JS_FN_FROM_NATIVE(get_PlaybackTime, Fb::get_PlaybackTime)
	MJS_DEFINE_JS_FN_FROM_NATIVE(get_ProfilePath, Fb::get_ProfilePath)
	MJS_DEFINE_JS_FN_FROM_NATIVE(get_ReplaygainMode, Fb::get_ReplaygainMode)
	MJS_DEFINE_JS_FN_FROM_NATIVE(get_StopAfterCurrent, Fb::get_StopAfterCurrent)
	MJS_DEFINE_JS_FN_FROM_NATIVE(get_Version, Fb::get_Version)
	MJS_DEFINE_JS_FN_FROM_NATIVE(get_Volume, Fb::get_Volume)
	MJS_DEFINE_JS_FN_FROM_NATIVE(put_AlwaysOnTop, Fb::put_AlwaysOnTop)
	MJS_DEFINE_JS_FN_FROM_NATIVE(put_CursorFollowPlayback, Fb::put_CursorFollowPlayback)
	MJS_DEFINE_JS_FN_FROM_NATIVE(put_PlaybackFollowCursor, Fb::put_PlaybackFollowCursor)
	MJS_DEFINE_JS_FN_FROM_NATIVE(put_PlaybackTime, Fb::put_PlaybackTime)
	MJS_DEFINE_JS_FN_FROM_NATIVE(put_ReplaygainMode, Fb::put_ReplaygainMode)
	MJS_DEFINE_JS_FN_FROM_NATIVE(put_StopAfterCurrent, Fb::put_StopAfterCurrent)
	MJS_DEFINE_JS_FN_FROM_NATIVE(put_Volume, Fb::put_Volume)

	constexpr auto jsProperties = std::to_array<JSPropertySpec>(
		{
			JS_PSGS("AlwaysOnTop", get_AlwaysOnTop, put_AlwaysOnTop, kDefaultPropsFlags),
			JS_PSG("ComponentPath", get_ComponentPath, kDefaultPropsFlags),
			JS_PSGS("CursorFollowPlayback", get_CursorFollowPlayback, put_CursorFollowPlayback, kDefaultPropsFlags),
			JS_PSG("CustomVolume", get_CustomVolume, kDefaultPropsFlags),
			JS_PSG("FoobarPath", get_FoobarPath, kDefaultPropsFlags),
			JS_PSG("IsPaused", get_IsPaused, kDefaultPropsFlags),
			JS_PSG("IsPlaying", get_IsPlaying, kDefaultPropsFlags),
			JS_PSGS("PlaybackFollowCursor", get_PlaybackFollowCursor, put_PlaybackFollowCursor, kDefaultPropsFlags),
			JS_PSG("PlaybackLength", get_PlaybackLength, kDefaultPropsFlags),
			JS_PSGS("PlaybackTime", get_PlaybackTime, put_PlaybackTime, kDefaultPropsFlags),
			JS_PSG("ProfilePath", get_ProfilePath, kDefaultPropsFlags),
			JS_PSGS("ReplaygainMode", get_ReplaygainMode, put_ReplaygainMode, kDefaultPropsFlags),
			JS_PSGS("StopAfterCurrent", get_StopAfterCurrent, put_StopAfterCurrent, kDefaultPropsFlags),
			JS_PSG("Version", get_Version, kDefaultPropsFlags),
			JS_PSGS("Volume", get_Volume, put_Volume, kDefaultPropsFlags),
			JS_PS_END,
		});
	}

	namespace mozjs
	{
		using namespace smp;

		const JSClass Fb::JsClass = jsClass;
		const JSFunctionSpec* Fb::JsFunctions = jsFunctions.data();
		const JSPropertySpec* Fb::JsProperties = jsProperties.data();

		Fb::Fb(JSContext* ctx) : m_ctx(ctx)
		{
			visualisation_manager::get()->create_stream(vis_, visualisation_manager::KStreamFlagNewFFT);
		}

		std::unique_ptr<Fb> Fb::CreateNative(JSContext* ctx)
		{
			return std::unique_ptr<Fb>(new Fb(ctx));
		}

		uint32_t Fb::GetInternalSize()
		{
			return 0;
		}

		JSObject* Fb::AcquireUiSelectionHolder()
		{
			return JsFbUiSelectionHolder::CreateJs(m_ctx, ui_selection_manager::get()->acquire());
		}

		void Fb::AddDirectory()
		{
			standard_commands::main_add_directory();
		}

		void Fb::AddFiles()
		{
			standard_commands::main_add_files();
		}

		uint32_t Fb::AddLocationsAsync(JS::HandleValue locations)
		{
			static uint32_t s_task_id{};
			static constexpr uint32_t s_flags = playlist_incoming_item_filter_v2::op_flag_no_filter | playlist_incoming_item_filter_v2::op_flag_delay_ui;

			const auto wnd = GetPanelHwndForCurrentGlobal(m_ctx);
			QwrException::ExpectTrue(wnd, "Method called before fb2k was initialized completely");

			pfc::string_list_impl location_list;
			convert::to_native::ProcessArray<std::string>(
				m_ctx,
				locations,
				[&location_list](const auto& location)
				{
					location_list.add_item(location.c_str());
				}
			);

			if (location_list.get_count() == 0)
				return 0;

			auto func = [wnd, task_id = ++s_task_id](metadb_handle_list_cref handles)
				{
					EventDispatcher::Get().PutEvent(
						wnd,
						GenerateEvent_JsCallback(
							EventId::kInternalLocationsAdded,
							task_id,
							std::make_shared<metadb_handle_list>(handles)
						)
					);
				};

			auto ptr = process_locations_notify::create(func);
			playlist_incoming_item_filter_v2::get()->process_locations_async(location_list, s_flags, nullptr, nullptr, nullptr, ptr);
			return s_task_id;
		}

		bool Fb::CheckClipboardContents()
		{
			wil::com_ptr<IDataObject> obj;

			if SUCCEEDED(OleGetClipboard(&obj))
			{
				DWORD effect = DROPEFFECT_COPY;
				bool native{};

				return SUCCEEDED(ole_interaction::get()->check_dataobject(obj.get(), effect, native));
			}

			return false;
		}

		void Fb::ClearPlaylist()
		{
			standard_commands::main_clear_playlist();
		}

		bool Fb::CopyHandleListToClipboard(JsFbMetadbHandleList* handles)
		{
			QwrException::ExpectTrue(handles, "handles argument is null");

			auto obj = ole_interaction::get()->create_dataobject(handles->GetHandleList());
			return SUCCEEDED(OleSetClipboard(obj.get_ptr()));
		}

		JSObject* Fb::CreateContextMenuManager()
		{
			return JsContextMenuManager::CreateJs(m_ctx);
		}

		JSObject* Fb::CreateHandleList()
		{
			return JsFbMetadbHandleList::Constructor(m_ctx, JS::UndefinedHandleValue);
		}

		JSObject* Fb::CreateMainMenuManager()
		{
			return JsMainMenuManager::CreateJs(m_ctx);
		}

		JSObject* Fb::CreateProfiler(const std::string& name)
		{
			return JsFbProfiler::Constructor(m_ctx, name);
		}

		JSObject* Fb::CreateProfilerWithOpt(size_t optArgCount, const std::string& name)
		{
			switch (optArgCount)
			{
			case 0:
				return CreateProfiler(name);
			case 1:
				return CreateProfiler();
			default:
				throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
			}
		}

		uint32_t Fb::DoDragDrop(uint32_t, JsFbMetadbHandleList* handles, uint32_t okEffects, JS::HandleValue options)
		{
			const auto wnd = GetPanelHwndForCurrentGlobal(m_ctx);
			QwrException::ExpectTrue(wnd, "Method called before fb2k was initialized completely");
			QwrException::ExpectTrue(handles, "handles argument is null");

			const metadb_handle_list& handleList = handles->GetHandleList();
			const auto handleCount = handleList.get_count();

			if (modal::IsModalBlocked() || handleCount == 0uz || okEffects == DROPEFFECT_NONE)
				return DROPEFFECT_NONE;

			std::unique_ptr<Gdiplus::Bitmap> bitmap;
			auto parsedOptions = ParseDoDragDropOptions(options);
			std::wstring text;

			if (parsedOptions.show_album_art && !parsedOptions.custom_image)
			{
				metadb_handle_ptr handle;

				if (!fb2k::api::pm->activeplaylist_get_focus_item_handle(handle) || handleList.find_item(handle) == SIZE_MAX)
				{
					handle = handleList[0];
				}

				std::string dummy;
				auto data = AlbumArtStatic::get(handle, 0uz, false, false, dummy);
				bitmap = AlbumArtStatic::to_bitmap(data);

				if (bitmap)
				{
					parsedOptions.custom_image = bitmap.get();
				}
			}

			if (parsedOptions.show_text)
			{
				text = fmt::format(L"{} {}", handleCount, (handleCount > 1 ? L"tracks" : L"track"));
			}

			modal::MessageBlockingScope scope;
			auto data = ole_interaction::get()->create_dataobject(handleList);

			wil::com_ptr_t<IDropSourceImpl> pIDropSource = new IDropSourceImpl(
				wnd,
				data.get_ptr(),
				text,
				parsedOptions.custom_image
			);

			SendMessageW(wnd, std::to_underlying(InternalSyncMessage::wnd_internal_drag_start), 0, 0);

			DWORD returnEffect{};
			const auto hr = SHDoDragDrop(nullptr, data.get_ptr(), pIDropSource.get(), okEffects, &returnEffect);

			SendMessageW(wnd, std::to_underlying(InternalSyncMessage::wnd_internal_drag_stop), 0, 0);

			return (DRAGDROP_S_CANCEL == hr ? DROPEFFECT_NONE : returnEffect);
		}

		uint32_t Fb::DoDragDropWithOpt(size_t optArgCount, uint32_t hWnd, JsFbMetadbHandleList* handles, uint32_t okEffects, JS::HandleValue options)
		{
			switch (optArgCount)
			{
			case 0:
				return DoDragDrop(hWnd, handles, okEffects, options);
			case 1:
				return DoDragDrop(hWnd, handles, okEffects);
			default:
				throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
			}
		}

		void Fb::Exit()
		{
			standard_commands::main_exit();
		}

		JSObject* Fb::GetAudioChunk(double requested_length, double offset)
		{
			audio_chunk_impl chunk;
			double time{};

			if (vis_->get_absolute_time(time) && vis_->get_chunk_absolute(chunk, time + offset, requested_length))
			{
				return JsFbAudioChunk::CreateJs(m_ctx, chunk);
			}

			return nullptr;
		}

		JSObject* Fb::GetAudioChunkWithOpt(size_t optArgCount, double requested_length, double offset)
		{
			switch (optArgCount)
			{
			case 0: return GetAudioChunk(requested_length, offset);
			case 1: return GetAudioChunk(requested_length);
			default: throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
			}
		}

		JSObject* Fb::GetClipboardContents(uint32_t)
		{
			wil::com_ptr<IDataObject> obj;
			metadb_handle_list items;

			if SUCCEEDED(OleGetClipboard(&obj))
			{
				dropped_files_data_impl data;

				if SUCCEEDED(ole_interaction::get()->parse_dataobject(obj.get(), data))
				{
					data.to_handles(items, false, core_api::get_main_window());
				}
			}

			return JsFbMetadbHandleList::CreateJs(m_ctx, items);
		}

		JSObject* Fb::GetClipboardContentsWithOpt(size_t optArgCount, uint32_t hWnd)
		{
			switch (optArgCount)
			{
			case 0:
				return GetClipboardContents(hWnd);
			case 1:
				return GetClipboardContents();
			default:
				throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
			}
		}

		std::string Fb::GetDSPPresets()
		{
			auto j = JSON::array();
			auto api = dsp_config_manager_v2::get();
			const auto selectedPreset = api->get_selected_preset();

			for (const auto i : indices(api->get_preset_count()))
			{
				pfc::string8 name;
				api->get_preset_name(i, name);

				j.push_back({
					{ "active", selectedPreset == i },
					{ "name", name.get_ptr() }
				});
			}

			return j.dump(2);
		}

		JSObject* Fb::GetFocusItem(bool)
		{
			metadb_handle_ptr metadb;

			if (fb2k::api::pm->activeplaylist_get_focus_item_handle(metadb))
			{
				return JsFbMetadbHandle::CreateJs(m_ctx, metadb);
			}

			return nullptr;
		}

		JSObject* Fb::GetFocusItemWithOpt(size_t optArgCount, bool force)
		{
			switch (optArgCount)
			{
			case 0:
				return GetFocusItem(force);
			case 1:
				return GetFocusItem();
			default:
				throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
			}
		}

		JSObject* Fb::GetLibraryItems()
		{
			metadb_handle_list items;
			library_manager::get()->get_all_items(items);

			return JsFbMetadbHandleList::CreateJs(m_ctx, items);
		}

		std::wstring Fb::GetLibraryRelativePath(JsFbMetadbHandle* handle)
		{
			QwrException::ExpectTrue(handle, "handle argument is null");

			pfc::string8 path;
			library_manager::get()->get_relative_path(handle->GetHandle(), path);
			return smp::ToWide(path);
		}

		JSObject* Fb::GetNowPlaying()
		{
			metadb_handle_ptr metadb;
			
			if (fb2k::api::pc->get_now_playing(metadb))
			{
				return JsFbMetadbHandle::CreateJs(m_ctx, metadb);
			}

			return nullptr;
		}

		std::string Fb::GetOutputDevices()
		{
			auto j = JSON::array();
			auto api = output_manager_v2::get();

			outputCoreConfig_t config{};
			api->getCoreConfig(config);

			api->listDevices([&j, &config](const std::string& name, const GUID& output_id, const GUID& device_id)
				{
					const std::string output_string = fmt::format("{{{}}}", pfc::print_guid(output_id).get_ptr());
					const std::string device_string = fmt::format("{{{}}}", pfc::print_guid(device_id).get_ptr());

					j.push_back({
						{ "name", name },
						{ "output_id", output_string },
						{ "device_id", device_string },
						{ "active", config.m_output == output_id && config.m_device == device_id }
					});
				});

			return j.dump(2);
		}

		JSObject* Fb::GetQueryItems(JsFbMetadbHandleList* handles, const std::string& query)
		{
			QwrException::ExpectTrue(handles, "handles argument is null");

			const metadb_handle_list& handles_ptr = handles->GetHandleList();
			metadb_handle_list dst_list(handles_ptr);
			search_filter_v2::ptr filter;

			try
			{
				filter = search_filter_manager_v2::get()->create_ex(
					query.c_str(),
					fb2k::service_new<completion_notify_dummy>(),
					search_filter_manager_v2::KFlagSuppressNotify
				);
			}
			catch (const pfc::exception& e)
			{
				throw QwrException(e.what());
			}

			pfc::array_t<bool> mask;
			mask.set_size(dst_list.get_count());
			filter->test_multi(dst_list, mask.get_ptr());
			dst_list.filter_mask(mask.get_ptr());

			return JsFbMetadbHandleList::CreateJs(m_ctx, dst_list);
		}

		JSObject* Fb::GetSelection()
		{
			metadb_handle_list items;
			ui_selection_manager::get()->get_selection(items);

			if (items.get_count())
			{
				return JsFbMetadbHandle::CreateJs(m_ctx, items[0]);
			}

			return nullptr;
		}

		JSObject* Fb::GetSelections(uint32_t flags)
		{
			metadb_handle_list items;
			ui_selection_manager_v2::get()->get_selection(items, flags);

			return JsFbMetadbHandleList::CreateJs(m_ctx, items);
		}

		JSObject* Fb::GetSelectionsWithOpt(size_t optArgCount, uint32_t flags)
		{
			switch (optArgCount)
			{
			case 0:
				return GetSelections(flags);
			case 1:
				return GetSelections();
			default:
				throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
			}
		}

		uint32_t Fb::GetSelectionType()
		{
			const auto type = ui_selection_manager_v2::get()->get_selection_type(0u);
			const auto it = std::ranges::find_if(JsFbUiSelectionHolder::selection_ids, [type](const GUID* g)
				{
					return *g == type;
				});

			return static_cast<uint32_t>(std::ranges::distance(JsFbUiSelectionHolder::selection_ids.begin(), it));
		}

		bool Fb::IsLibraryEnabled()
		{
			return library_manager::get()->is_library_enabled();
		}

		bool Fb::IsMainMenuCommandChecked(const std::string& command)
		{
			const auto status = GetMainmenuCommandStatusByName(command);
			return WI_IsAnyFlagSet(status, mainmenu_commands::flag_checked | mainmenu_commands::flag_radiochecked);
		}

		bool Fb::IsMetadbInMediaLibrary(JsFbMetadbHandle* handle)
		{
			QwrException::ExpectTrue(handle, "handle argument is null");

			return library_manager::get()->is_item_in_library(handle->GetHandle());
		}

		void Fb::LoadPlaylist()
		{
			standard_commands::main_load_playlist();
		}

		void Fb::Next()
		{
			standard_commands::main_next();
		}

		void Fb::Pause()
		{
			standard_commands::main_pause();
		}

		void Fb::Play()
		{
			standard_commands::main_play();
		}

		void Fb::PlayOrPause()
		{
			standard_commands::main_play_or_pause();
		}

		void Fb::Prev()
		{
			standard_commands::main_previous();
		}

		void Fb::Random()
		{
			standard_commands::main_random();
		}

		void Fb::RegisterMainMenuCommand(uint32_t id, const std::string& name, const std::string& description)
		{
			const auto wnd = GetPanelHwndForCurrentGlobal(m_ctx);
			QwrException::ExpectTrue(wnd, "Method called before fb2k was initialized completely");

			DynamicMainMenuManager::Get().RegisterCommand(wnd, id, name, description);
		}

		void Fb::RegisterMainMenuCommandWithOpt(size_t optArgCount, uint32_t id, const std::string& name, const std::string& description)
		{
			switch (optArgCount)
			{
			case 0:
				return RegisterMainMenuCommand(id, name, description);
			case 1:
				return RegisterMainMenuCommand(id, name);
			default:
				throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
			}
		}

		void Fb::RequestBacklog(double time)
		{
			vis_->request_backlog(time);
		}

		void Fb::Restart()
		{
			standard_commands::main_restart();
		}

		bool Fb::RunContextCommand(const std::string& command, uint32_t flags)
		{
			return ContextMenuCommand(command, flags).execute();
		}

		bool Fb::RunContextCommandWithOpt(size_t optArgCount, const std::string& command, uint32_t flags)
		{
			switch (optArgCount)
			{
			case 0:
				return RunContextCommand(command, flags);
			case 1:
				return RunContextCommand(command);
			default:
				throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
			}
		}

		bool Fb::RunContextCommandWithMetadb(const std::string& command, JS::HandleValue handle, uint32_t flags)
		{
			QwrException::ExpectTrue(handle.isObject(), "handle argument is invalid");

			JS::RootedObject jsObject(m_ctx, &handle.toObject());
			metadb_handle_list handle_list;

			auto* jsHandle = JsFbMetadbHandle::ExtractNative(m_ctx, jsObject);
			auto* jsHandleList = JsFbMetadbHandleList::ExtractNative(m_ctx, jsObject);
			QwrException::ExpectTrue(jsHandle || jsHandleList, "handle argument is invalid");

			if (jsHandleList)
			{
				handle_list = jsHandleList->GetHandleList();
			}
			else
			{
				handle_list.add_item(jsHandle->GetHandle());
			}

			return ContextMenuCommand(command, handle_list, flags).execute();
		}

		bool Fb::RunContextCommandWithMetadbWithOpt(size_t optArgCount, const std::string& command, JS::HandleValue handle, uint32_t flags)
		{
			switch (optArgCount)
			{
			case 0:
				return RunContextCommandWithMetadb(command, handle, flags);
			case 1:
				return RunContextCommandWithMetadb(command, handle);
			default:
				throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
			}
		}

		bool Fb::RunMainMenuCommand(const std::string& command)
		{
			return MainMenuCommand(command).execute();
		}

		void Fb::SavePlaylist()
		{
			standard_commands::main_save_playlist();
		}

		void Fb::SetDSPPreset(uint32_t idx)
		{
			auto api = dsp_config_manager_v2::get();
			const auto count = api->get_preset_count();

			QwrException::ExpectTrue(idx < count, "Index is out of bounds");

			api->select_preset(idx);
		}

		void Fb::SetOutputDevice(const std::wstring& output, const std::wstring& device)
		{
			GUID output_id{};
			GUID device_id{};

			if (CLSIDFromString(output.c_str(), &output_id) == NOERROR && CLSIDFromString(device.c_str(), &device_id) == NOERROR)
			{
				output_manager_v2::get()->setCoreConfigDevice(output_id, device_id);
			}
		}

		void Fb::ShowConsole()
		{
			standard_commands::main_show_console();
		}

		void Fb::ShowLibrarySearchUI(const std::string& query)
		{
			library_search_ui::get()->show(query.c_str());
		}

		void Fb::ShowPictureViewer(const std::wstring& image_path)
		{
			auto data = AlbumArtStatic::to_data(image_path);
			AlbumArtStatic::show_viewer(data);
		}

		void Fb::ShowPopupMessage(const std::string& msg, const std::string& title)
		{
			fb2k::inMainThread([msg, title]
				{
					popup_message::g_show(msg.c_str(), title.c_str());
				});
		}

		void Fb::ShowPopupMessageWithOpt(size_t optArgCount, const std::string& msg, const std::string& title)
		{
			switch (optArgCount)
			{
			case 0:
				return ShowPopupMessage(msg, title);
			case 1:
				return ShowPopupMessage(msg);
			default:
				throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
			}
		}

		void Fb::ShowPreferences()
		{
			standard_commands::main_preferences();
		}

		void Fb::Stop()
		{
			standard_commands::main_stop();
		}

		JSObject* Fb::TitleFormat(const std::string& expression)
		{
			return JsFbTitleFormat::Constructor(m_ctx, expression);
		}

		void Fb::UnregisterMainMenuCommand(uint32_t id)
		{
			const auto wnd = GetPanelHwndForCurrentGlobal(m_ctx);
			QwrException::ExpectTrue(wnd, "Method called before fb2k was initialized completely");

			DynamicMainMenuManager::Get().UnregisterCommand(wnd, id);
		}

		void Fb::VolumeDown()
		{
			standard_commands::main_volume_down();
		}

		void Fb::VolumeMute()
		{
			standard_commands::main_volume_mute();
		}

		void Fb::VolumeUp()
		{
			standard_commands::main_volume_up();
		}

		bool Fb::get_AlwaysOnTop()
		{
			return config_object::g_get_data_bool_simple(standard_config_objects::bool_ui_always_on_top, false);
		}

		std::wstring Fb::get_ComponentPath()
		{
			static const auto component_path = path::Component().native() + std::filesystem::path::preferred_separator;
			return component_path;
		}

		bool Fb::get_CursorFollowPlayback()
		{
			return config_object::g_get_data_bool_simple(standard_config_objects::bool_cursor_follows_playback, false);
		}

		int32_t Fb::get_CustomVolume()
		{
			if (fb2k::api::pc->custom_volume_is_active())
			{
				return fb2k::api::pc->custom_volume_get();
			}

			return -1;
		}

		std::wstring Fb::get_FoobarPath()
		{
			static const auto fb2k_path = path::Foobar2000().native() + std::filesystem::path::preferred_separator;
			return fb2k_path;
		}

		bool Fb::get_IsPaused()
		{
			return fb2k::api::pc->is_paused();
		}

		bool Fb::get_IsPlaying()
		{
			return fb2k::api::pc->is_playing();
		}

		bool Fb::get_PlaybackFollowCursor()
		{
			return config_object::g_get_data_bool_simple(standard_config_objects::bool_playback_follows_cursor, false);
		}

		double Fb::get_PlaybackLength()
		{
			return fb2k::api::pc->playback_get_length();
		}

		double Fb::get_PlaybackTime()
		{
			return fb2k::api::pc->playback_get_position();
		}

		std::wstring Fb::get_ProfilePath()
		{
			static const auto profile_path = path::Profile().native() + std::filesystem::path::preferred_separator;
			return profile_path;
		}

		uint32_t Fb::get_ReplaygainMode()
		{
			t_replaygain_config rg;
			replaygain_manager::get()->get_core_settings(rg);
			return rg.m_source_mode;
		}

		bool Fb::get_StopAfterCurrent()
		{
			return fb2k::api::pc->get_stop_after_current();
		}

		std::string Fb::get_Version()
		{
			return core_version_info_v2::get()->get_version_as_text();
		}

		float Fb::get_Volume()
		{
			return fb2k::api::pc->get_volume();
		}

		void Fb::put_AlwaysOnTop(bool p)
		{
			config_object::g_set_data_bool(standard_config_objects::bool_ui_always_on_top, p);
		}

		void Fb::put_CursorFollowPlayback(bool p)
		{
			config_object::g_set_data_bool(standard_config_objects::bool_cursor_follows_playback, p);
		}

		void Fb::put_PlaybackFollowCursor(bool p)
		{
			config_object::g_set_data_bool(standard_config_objects::bool_playback_follows_cursor, p);
		}

		void Fb::put_PlaybackTime(double time)
		{
			fb2k::api::pc->playback_seek(time);
		}

		void Fb::put_ReplaygainMode(uint32_t mode)
		{
			QwrException::ExpectTrue(mode <= t_replaygain_config::source_mode_byPlaybackOrder, "Invalid replay gain mode: {}", mode);

			auto api = replaygain_manager::get();

			t_replaygain_config config;
			api->get_core_settings(config);
			config.m_source_mode = mode;
			api->set_core_settings(config);

			fb2k::api::pc->restart();
		}

		void Fb::put_StopAfterCurrent(bool p)
		{
			fb2k::api::pc->set_stop_after_current(p);
		}

		void Fb::put_Volume(float value)
		{
			if (!fb2k::api::pc->custom_volume_is_active())
			{
				fb2k::api::pc->set_volume(value);
			}
		}

		Fb::DoDragDropOptions Fb::ParseDoDragDropOptions(JS::HandleValue options)
		{
			if (options.isNullOrUndefined())
				return {};

			QwrException::ExpectTrue(options.isObject(), "options argument is not an object");
			auto jsOptions = JS::RootedObject(m_ctx, &options.toObject());

			DoDragDropOptions parsedOptions;
			parsedOptions.show_album_art = GetOptionalProperty<bool>(m_ctx, jsOptions, "show_album_art").value_or(true);
			parsedOptions.show_text = GetOptionalProperty<bool>(m_ctx, jsOptions, "show_text").value_or(true);

			if (parsedOptions.show_album_art)
			{
				auto jsImage = GetOptionalProperty<JsGdiBitmap*>(m_ctx, jsOptions, "custom_image").value_or(nullptr);

				if (jsImage)
				{
					parsedOptions.custom_image = jsImage->GdiBitmap();
				}
			}

			return parsedOptions;
		}
	}
