#include "PCH.hpp"
#include "plman.h"

#include <2K3/CustomSort.hpp>
#include <2K3/PlaylistLock.hpp>
#include <2K3/ProcessLocationsNotify.hpp>
#include <interfaces/fb_metadb_handle.h>
#include <interfaces/fb_metadb_handle_list.h>
#include <interfaces/fb_playback_queue_item.h>
#include <interfaces/fb_playlist_recycler.h>

namespace
{
	using namespace mozjs;

	DEFINE_JS_CLASS_OPS(Plman::FinalizeJsObject)

	DEFINE_JS_CLASS("FbPlaylistManager")

	MJS_DEFINE_JS_FN_FROM_NATIVE(AddItemToPlaybackQueue, Plman::AddItemToPlaybackQueue);
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(AddLocations, Plman::AddLocations, Plman::AddLocationsWithOpt, 1);
	MJS_DEFINE_JS_FN_FROM_NATIVE(AddPlaylistItemToPlaybackQueue, Plman::AddPlaylistItemToPlaybackQueue);
	MJS_DEFINE_JS_FN_FROM_NATIVE(ClearPlaylist, Plman::ClearPlaylist);
	MJS_DEFINE_JS_FN_FROM_NATIVE(ClearPlaylistSelection, Plman::ClearPlaylistSelection);
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(CreateAutoPlaylist, Plman::CreateAutoPlaylist, Plman::CreateAutoPlaylistWithOpt, 2);
	MJS_DEFINE_JS_FN_FROM_NATIVE(CreatePlaylist, Plman::CreatePlaylist);
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(DuplicatePlaylist, Plman::DuplicatePlaylist, Plman::DuplicatePlaylistWithOpt, 1);
	MJS_DEFINE_JS_FN_FROM_NATIVE(EnsurePlaylistItemVisible, Plman::EnsurePlaylistItemVisible);
	MJS_DEFINE_JS_FN_FROM_NATIVE(ExecutePlaylistDefaultAction, Plman::ExecutePlaylistDefaultAction);
	MJS_DEFINE_JS_FN_FROM_NATIVE(FindByGUID, Plman::FindByGUID);
	MJS_DEFINE_JS_FN_FROM_NATIVE(FindOrCreatePlaylist, Plman::FindOrCreatePlaylist);
	MJS_DEFINE_JS_FN_FROM_NATIVE(FindPlaybackQueueItemIndex, Plman::FindPlaybackQueueItemIndex);
	MJS_DEFINE_JS_FN_FROM_NATIVE(FindPlaylist, Plman::FindPlaylist);
	MJS_DEFINE_JS_FN_FROM_NATIVE(FlushPlaybackQueue, Plman::FlushPlaybackQueue);
	MJS_DEFINE_JS_FN_FROM_NATIVE(GetGUID, Plman::GetGUID);
	MJS_DEFINE_JS_FN_FROM_NATIVE(GetPlaybackQueueContents, Plman::GetPlaybackQueueContents);
	MJS_DEFINE_JS_FN_FROM_NATIVE(GetPlaybackQueueHandles, Plman::GetPlaybackQueueHandles);
	MJS_DEFINE_JS_FN_FROM_NATIVE(GetPlayingItemLocation, Plman::GetPlayingItemLocation);
	MJS_DEFINE_JS_FN_FROM_NATIVE(GetPlaylistFocusItemIndex, Plman::GetPlaylistFocusItemIndex);
	MJS_DEFINE_JS_FN_FROM_NATIVE(GetPlaylistItems, Plman::GetPlaylistItems);
	MJS_DEFINE_JS_FN_FROM_NATIVE(GetPlaylistLockedActions, Plman::GetPlaylistLockedActions);
	MJS_DEFINE_JS_FN_FROM_NATIVE(GetPlaylistLockName, Plman::GetPlaylistLockName);
	MJS_DEFINE_JS_FN_FROM_NATIVE(GetPlaylistName, Plman::GetPlaylistName);
	MJS_DEFINE_JS_FN_FROM_NATIVE(GetPlaylistSelectedItems, Plman::GetPlaylistSelectedItems);
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(InsertPlaylistItems, Plman::InsertPlaylistItems, Plman::InsertPlaylistItemsWithOpt, 1);
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(InsertPlaylistItemsFilter, Plman::InsertPlaylistItemsFilter, Plman::InsertPlaylistItemsFilterWithOpt, 1);
	MJS_DEFINE_JS_FN_FROM_NATIVE(IsAutoPlaylist, Plman::IsAutoPlaylist);
	MJS_DEFINE_JS_FN_FROM_NATIVE(IsPlaylistItemSelected, Plman::IsPlaylistItemSelected);
	MJS_DEFINE_JS_FN_FROM_NATIVE(IsPlaylistLocked, Plman::IsPlaylistLocked);
	MJS_DEFINE_JS_FN_FROM_NATIVE(IsRedoAvailable, Plman::IsRedoAvailable);
	MJS_DEFINE_JS_FN_FROM_NATIVE(IsUndoAvailable, Plman::IsUndoAvailable);
	MJS_DEFINE_JS_FN_FROM_NATIVE(MovePlaylist, Plman::MovePlaylist);
	MJS_DEFINE_JS_FN_FROM_NATIVE(MovePlaylistSelection, Plman::MovePlaylistSelection);
	MJS_DEFINE_JS_FN_FROM_NATIVE(PlaylistItemCount, Plman::PlaylistItemCount);
	MJS_DEFINE_JS_FN_FROM_NATIVE(Redo, Plman::Redo);
	MJS_DEFINE_JS_FN_FROM_NATIVE(RemoveItemFromPlaybackQueue, Plman::RemoveItemFromPlaybackQueue);
	MJS_DEFINE_JS_FN_FROM_NATIVE(RemoveItemsFromPlaybackQueue, Plman::RemoveItemsFromPlaybackQueue);
	MJS_DEFINE_JS_FN_FROM_NATIVE(RemovePlaylist, Plman::RemovePlaylist);
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(RemovePlaylistSelection, Plman::RemovePlaylistSelection, Plman::RemovePlaylistSelectionWithOpt, 1);
	MJS_DEFINE_JS_FN_FROM_NATIVE(RemovePlaylistSwitch, Plman::RemovePlaylistSwitch);
	MJS_DEFINE_JS_FN_FROM_NATIVE(RenamePlaylist, Plman::RenamePlaylist);
	MJS_DEFINE_JS_FN_FROM_NATIVE(SetActivePlaylistContext, Plman::SetActivePlaylistContext);
	MJS_DEFINE_JS_FN_FROM_NATIVE(SetPlaylistFocusItem, Plman::SetPlaylistFocusItem);
	MJS_DEFINE_JS_FN_FROM_NATIVE(SetPlaylistFocusItemByHandle, Plman::SetPlaylistFocusItemByHandle);
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(SetPlaylistLockedActions, Plman::SetPlaylistLockedActions, Plman::SetPlaylistLockedActionsWithOpt, 1);
	MJS_DEFINE_JS_FN_FROM_NATIVE(SetPlaylistSelection, Plman::SetPlaylistSelection);
	MJS_DEFINE_JS_FN_FROM_NATIVE(SetPlaylistSelectionSingle, Plman::SetPlaylistSelectionSingle);
	MJS_DEFINE_JS_FN_FROM_NATIVE(ShowAutoPlaylistUI, Plman::ShowAutoPlaylistUI);
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(SortByFormat, Plman::SortByFormat, Plman::SortByFormatWithOpt, 1);
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(SortByFormatV2, Plman::SortByFormatV2, Plman::SortByFormatV2WithOpt, 1);
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(SortPlaylistsByName, Plman::SortPlaylistsByName, Plman::SortPlaylistsByNameWithOpt, 1);
	MJS_DEFINE_JS_FN_FROM_NATIVE(Undo, Plman::Undo);
	MJS_DEFINE_JS_FN_FROM_NATIVE(UndoBackup, Plman::UndoBackup);

	constexpr auto jsFunctions = std::to_array<JSFunctionSpec>(
		{
			JS_FN("AddItemToPlaybackQueue", AddItemToPlaybackQueue, 1, kDefaultPropsFlags),
			JS_FN("AddLocations", AddLocations, 2, kDefaultPropsFlags),
			JS_FN("AddPlaylistItemToPlaybackQueue", AddPlaylistItemToPlaybackQueue, 2, kDefaultPropsFlags),
			JS_FN("ClearPlaylist", ClearPlaylist, 1, kDefaultPropsFlags),
			JS_FN("ClearPlaylistSelection", ClearPlaylistSelection, 1, kDefaultPropsFlags),
			JS_FN("CreateAutoPlaylist", CreateAutoPlaylist, 3, kDefaultPropsFlags),
			JS_FN("CreatePlaylist", CreatePlaylist, 2, kDefaultPropsFlags),
			JS_FN("DuplicatePlaylist", DuplicatePlaylist, 1, kDefaultPropsFlags),
			JS_FN("EnsurePlaylistItemVisible", EnsurePlaylistItemVisible, 2, kDefaultPropsFlags),
			JS_FN("ExecutePlaylistDefaultAction", ExecutePlaylistDefaultAction, 2, kDefaultPropsFlags),
			JS_FN("FindByGUID", FindByGUID, 1, kDefaultPropsFlags),
			JS_FN("FindOrCreatePlaylist", FindOrCreatePlaylist, 2, kDefaultPropsFlags),
			JS_FN("FindPlaybackQueueItemIndex", FindPlaybackQueueItemIndex, 3, kDefaultPropsFlags),
			JS_FN("FindPlaylist", FindPlaylist, 1, kDefaultPropsFlags),
			JS_FN("FlushPlaybackQueue", FlushPlaybackQueue, 0, kDefaultPropsFlags),
			JS_FN("GetGUID", GetGUID, 1, kDefaultPropsFlags),
			JS_FN("GetPlaybackQueueContents", GetPlaybackQueueContents, 0, kDefaultPropsFlags),
			JS_FN("GetPlaybackQueueHandles", GetPlaybackQueueHandles, 0, kDefaultPropsFlags),
			JS_FN("GetPlayingItemLocation", GetPlayingItemLocation, 0, kDefaultPropsFlags),
			JS_FN("GetPlaylistFocusItemIndex", GetPlaylistFocusItemIndex, 1, kDefaultPropsFlags),
			JS_FN("GetPlaylistItems", GetPlaylistItems, 1, kDefaultPropsFlags),
			JS_FN("GetPlaylistLockedActions", GetPlaylistLockedActions, 1, kDefaultPropsFlags),
			JS_FN("GetPlaylistLockName", GetPlaylistLockName, 1, kDefaultPropsFlags),
			JS_FN("GetPlaylistName", GetPlaylistName, 1, kDefaultPropsFlags),
			JS_FN("GetPlaylistSelectedItems", GetPlaylistSelectedItems, 1, kDefaultPropsFlags),
			JS_FN("InsertPlaylistItems", InsertPlaylistItems, 3, kDefaultPropsFlags),
			JS_FN("InsertPlaylistItemsFilter", InsertPlaylistItemsFilter, 3, kDefaultPropsFlags),
			JS_FN("IsAutoPlaylist", IsAutoPlaylist, 1, kDefaultPropsFlags),
			JS_FN("IsPlaylistItemSelected", IsPlaylistItemSelected, 2, kDefaultPropsFlags),
			JS_FN("IsPlaylistLocked", IsPlaylistLocked, 1, kDefaultPropsFlags),
			JS_FN("IsRedoAvailable", IsRedoAvailable, 1, kDefaultPropsFlags),
			JS_FN("IsUndoAvailable", IsUndoAvailable, 1, kDefaultPropsFlags),
			JS_FN("MovePlaylist", MovePlaylist, 2, kDefaultPropsFlags),
			JS_FN("MovePlaylistSelection", MovePlaylistSelection, 2, kDefaultPropsFlags),
			JS_FN("PlaylistItemCount", PlaylistItemCount, 1, kDefaultPropsFlags),
			JS_FN("Redo", Redo, 1, kDefaultPropsFlags),
			JS_FN("RemoveItemFromPlaybackQueue", RemoveItemFromPlaybackQueue, 1, kDefaultPropsFlags),
			JS_FN("RemoveItemsFromPlaybackQueue", RemoveItemsFromPlaybackQueue, 1, kDefaultPropsFlags),
			JS_FN("RemovePlaylist", RemovePlaylist, 1, kDefaultPropsFlags),
			JS_FN("RemovePlaylistSelection", RemovePlaylistSelection, 1, kDefaultPropsFlags),
			JS_FN("RemovePlaylistSwitch", RemovePlaylistSwitch, 1, kDefaultPropsFlags),
			JS_FN("RenamePlaylist", RenamePlaylist, 2, kDefaultPropsFlags),
			JS_FN("SetActivePlaylistContext", SetActivePlaylistContext, 0, kDefaultPropsFlags),
			JS_FN("SetPlaylistFocusItem", SetPlaylistFocusItem, 2, kDefaultPropsFlags),
			JS_FN("SetPlaylistFocusItemByHandle", SetPlaylistFocusItemByHandle, 2, kDefaultPropsFlags),
			JS_FN("SetPlaylistLockedActions", SetPlaylistLockedActions, 1, kDefaultPropsFlags),
			JS_FN("SetPlaylistSelection", SetPlaylistSelection, 3, kDefaultPropsFlags),
			JS_FN("SetPlaylistSelectionSingle", SetPlaylistSelectionSingle, 3, kDefaultPropsFlags),
			JS_FN("ShowAutoPlaylistUI", ShowAutoPlaylistUI, 1, kDefaultPropsFlags),
			JS_FN("SortByFormat", SortByFormat, 2, kDefaultPropsFlags),
			JS_FN("SortByFormatV2", SortByFormatV2, 2, kDefaultPropsFlags),
			JS_FN("SortPlaylistsByName", SortPlaylistsByName, 0, kDefaultPropsFlags),
			JS_FN("Undo", Undo, 1, kDefaultPropsFlags),
			JS_FN("UndoBackup", UndoBackup, 1, kDefaultPropsFlags),
			JS_FS_END,
		});

	MJS_DEFINE_JS_FN_FROM_NATIVE(get_ActivePlaylist, Plman::get_ActivePlaylist);
	MJS_DEFINE_JS_FN_FROM_NATIVE(get_PlaybackOrder, Plman::get_PlaybackOrder);
	MJS_DEFINE_JS_FN_FROM_NATIVE(get_PlayingPlaylist, Plman::get_PlayingPlaylist);
	MJS_DEFINE_JS_FN_FROM_NATIVE(get_PlaylistCount, Plman::get_PlaylistCount);
	MJS_DEFINE_JS_FN_FROM_NATIVE(get_PlaylistRecycler, Plman::get_PlaylistRecycler);
	MJS_DEFINE_JS_FN_FROM_NATIVE(put_ActivePlaylist, Plman::put_ActivePlaylist);
	MJS_DEFINE_JS_FN_FROM_NATIVE(put_PlaybackOrder, Plman::put_PlaybackOrder);
	MJS_DEFINE_JS_FN_FROM_NATIVE(put_PlayingPlaylist, Plman::put_PlayingPlaylist);

	constexpr auto jsProperties = std::to_array<JSPropertySpec>(
		{
			JS_PSGS("ActivePlaylist", get_ActivePlaylist, put_ActivePlaylist, kDefaultPropsFlags),
			JS_PSGS("PlaybackOrder", get_PlaybackOrder, put_PlaybackOrder, kDefaultPropsFlags),
			JS_PSGS("PlayingPlaylist", get_PlayingPlaylist, put_PlayingPlaylist, kDefaultPropsFlags),
			JS_PSG("PlaylistCount", get_PlaylistCount, kDefaultPropsFlags),
			JS_PSG("PlaylistRecycler", get_PlaylistRecycler, kDefaultPropsFlags),
			JS_PS_END,
		});
}

namespace mozjs
{
	const JSClass Plman::JsClass = jsClass;
	const JSFunctionSpec* Plman::JsFunctions = jsFunctions.data();
	const JSPropertySpec* Plman::JsProperties = jsProperties.data();

	Plman::Plman(JSContext* ctx) : m_ctx(ctx) {}

	Plman::~Plman()
	{
		PrepareForGc();
	}

	std::unique_ptr<Plman> Plman::CreateNative(JSContext* ctx)
	{
		return std::unique_ptr<Plman>(new Plman(ctx));
	}

	uint32_t Plman::GetInternalSize()
	{
		return 0;
	}

	void Plman::PrepareForGc()
	{
		m_recycler.reset();
	}

	void Plman::AddItemToPlaybackQueue(JsFbMetadbHandle* handle)
	{
		QwrException::ExpectTrue(handle, "handle argument is null");

		fb2k::api::pm->queue_add_item(handle->GetHandle());
	}

	void Plman::AddLocations(uint32_t playlistIndex, JS::HandleValue locations, bool select)
	{
		QwrException::ExpectTrue(playlistIndex < fb2k::api::pm->get_playlist_count(), "playlistIndex is invalid");
		QwrException::ExpectTrue(
			!WI_IsFlagSet(fb2k::api::pm->playlist_lock_get_filter_mask(playlistIndex), playlist_lock::filter_add),
			"a playlist lock prevents adding new items to the specified playlistIndex"
		);

		pfc::string_list_impl location_list;
		convert::to_native::ProcessArray<std::string>(
			m_ctx,
			locations,
			[&location_list](const auto& location) { location_list.add_item(location.c_str()); });

		if (location_list.get_count() == 0)
			return; // no error

		const auto base = fb2k::api::pm->playlist_get_item_count(playlistIndex);
		const auto g = fb2k::api::pm->playlist_get_guid(playlistIndex);
		auto cb = fb2k::service_new<ProcessLocationsNotify>(g, base, select);
		ProcessLocationsNotify::init(location_list, cb);
	}

	void Plman::AddLocationsWithOpt(size_t optArgCount, uint32_t playlistIndex, JS::HandleValue locations, bool select)
	{
		switch (optArgCount)
		{
		case 0:
			return AddLocations(playlistIndex, locations, select);
		case 1:
			return AddLocations(playlistIndex, locations);
		default:
			throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	void Plman::AddPlaylistItemToPlaybackQueue(uint32_t playlistIndex, uint32_t playlistItemIndex)
	{
		fb2k::api::pm->queue_add_item_playlist(playlistIndex, playlistItemIndex);
	}

	void Plman::ClearPlaylist(uint32_t playlistIndex)
	{
		fb2k::api::pm->playlist_clear(playlistIndex);
	}

	void Plman::ClearPlaylistSelection(uint32_t playlistIndex)
	{
		fb2k::api::pm->playlist_clear_selection(playlistIndex);
	}

	uint32_t Plman::CreateAutoPlaylist(uint32_t playlistIndex, const std::string& name, const std::string& query, const std::string& sort, uint32_t flags)
	{
		const auto upos = CreatePlaylist(playlistIndex, name);

		try
		{
			autoplaylist_manager::get()->add_client_simple(query.c_str(), sort.c_str(), upos, flags);
			return upos;
		}
		catch (const pfc::exception& e)
		{
			fb2k::api::pm->remove_playlist(upos);
			throw QwrException(e.what());
		}
	}

	uint32_t Plman::CreateAutoPlaylistWithOpt(size_t optArgCount, uint32_t playlistIndex, const std::string& name, const std::string& query, const std::string& sort, uint32_t flags)
	{
		switch (optArgCount)
		{
		case 0:
			return CreateAutoPlaylist(playlistIndex, name, query, sort, flags);
		case 1:
			return CreateAutoPlaylist(playlistIndex, name, query, sort);
		case 2:
			return CreateAutoPlaylist(playlistIndex, name, query);
		default:
			throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	uint32_t Plman::CreatePlaylist(uint32_t playlistIndex, const std::string& name)
	{
		if (name.empty())
		{
			return to_uint(fb2k::api::pm->create_playlist_autoname(playlistIndex));
		}
		else
		{
			return to_uint(fb2k::api::pm->create_playlist(name.c_str(), name.length(), playlistIndex));
		}
	}

	uint32_t Plman::DuplicatePlaylist(uint32_t from, const std::string& name)
	{
		QwrException::ExpectTrue(from < fb2k::api::pm->get_playlist_count(), "Index is out of bounds");

		pfc::string8 new_name(name.c_str());

		if (new_name.is_empty())
		{
			fb2k::api::pm->playlist_get_name(from, new_name);
		}

		metadb_handle_list items;
		fb2k::api::pm->playlist_get_all_items(from, items);

		const auto pos = fb2k::api::pm->create_playlist(new_name, new_name.get_length(), ++from);
		fb2k::api::pm->playlist_insert_items(pos, 0uz, items, pfc::bit_array_false());
		return to_uint(pos);
	}

	uint32_t Plman::DuplicatePlaylistWithOpt(size_t optArgCount, uint32_t from, const std::string& name)
	{
		switch (optArgCount)
		{
		case 0:
			return DuplicatePlaylist(from, name);
		case 1:
			return DuplicatePlaylist(from);
		default:
			throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	void Plman::EnsurePlaylistItemVisible(uint32_t playlistIndex, uint32_t playlistItemIndex)
	{
		fb2k::api::pm->playlist_ensure_visible(playlistIndex, playlistItemIndex);
	}

	bool Plman::ExecutePlaylistDefaultAction(uint32_t playlistIndex, uint32_t playlistItemIndex)
	{
		return fb2k::api::pm->playlist_execute_default_action(playlistIndex, playlistItemIndex);
	}

	int32_t Plman::FindByGUID(const std::string& str)
	{
		const auto guid = pfc::GUID_from_text(str.c_str());
		return to_int(fb2k::api::pm->find_playlist_by_guid(guid));
	}

	uint32_t Plman::FindOrCreatePlaylist(const std::string& name, bool unlocked)
	{
		if (unlocked)
		{
			return to_uint(fb2k::api::pm->find_or_create_playlist_unlocked(name.c_str(), name.length()));
		}
		else
		{
			return to_uint(fb2k::api::pm->find_or_create_playlist(name.c_str(), name.length()));
		}
	}

	int32_t Plman::FindPlaybackQueueItemIndex(JsFbMetadbHandle* handle, uint32_t playlistIndex, uint32_t playlistItemIndex)
	{
		QwrException::ExpectTrue(handle, "handle argument is null");

		t_playback_queue_item item;
		item.m_handle = handle->GetHandle();
		item.m_playlist = playlistIndex;
		item.m_item = playlistItemIndex;

		const auto upos = fb2k::api::pm->queue_find_index(item);
		return to_int(upos);
	}

	int32_t Plman::FindPlaylist(const std::string& name)
	{
		const auto upos = fb2k::api::pm->find_playlist(name.c_str(), name.length());
		return to_int(upos);
	}

	void Plman::FlushPlaybackQueue()
	{
		fb2k::api::pm->queue_flush();
	}

	std::string Plman::GetGUID(uint32_t playlistIndex)
	{
		QwrException::ExpectTrue(playlistIndex < fb2k::api::pm->get_playlist_count(), "Index is out of bounds");

		const auto guid = fb2k::api::pm->playlist_get_guid(playlistIndex);
		return pfc::print_guid(guid).get_ptr();
	}

	JS::Value Plman::GetPlaybackQueueContents()
	{
		pfc::list_t<t_playback_queue_item> queue_items;
		fb2k::api::pm->queue_get_contents(queue_items);

		JS::RootedValue jsValue(m_ctx);
		convert::to_js::ToArrayValue(m_ctx, queue_items, &jsValue);
		return jsValue;
	}

	JSObject* Plman::GetPlaybackQueueHandles()
	{
		pfc::list_t<t_playback_queue_item> queue_items;
		fb2k::api::pm->queue_get_contents(queue_items);

		metadb_handle_list items;

		for (auto&& queue_item : queue_items)
		{
			items.add_item(queue_item.m_handle);
		}

		return JsFbMetadbHandleList::CreateJs(m_ctx, items);
	}

	JSObject* Plman::GetPlayingItemLocation()
	{
		size_t playlistIndex{};
		size_t playlistItemIndex{};
		bool isValid = fb2k::api::pm->get_playing_item_location(&playlistIndex, &playlistItemIndex);

		JS::RootedObject jsObject(m_ctx, JS_NewPlainObject(m_ctx));
		AddProperty(m_ctx, jsObject, "IsValid", isValid);
		AddProperty(m_ctx, jsObject, "PlaylistIndex", isValid ? to_int(playlistIndex) : -1);
		AddProperty(m_ctx, jsObject, "PlaylistItemIndex", isValid ? to_int(playlistItemIndex) : -1);
		return jsObject;
	}

	int32_t Plman::GetPlaylistFocusItemIndex(uint32_t playlistIndex)
	{
		const auto upos = fb2k::api::pm->playlist_get_focus_item(playlistIndex);
		return to_int(upos);
	}

	JSObject* Plman::GetPlaylistItems(uint32_t playlistIndex)
	{
		metadb_handle_list items;
		fb2k::api::pm->playlist_get_all_items(playlistIndex, items);

		return JsFbMetadbHandleList::CreateJs(m_ctx, items);
	}

	std::wstring Plman::GetPlaylistLockName(uint32_t playlistIndex)
	{
		QwrException::ExpectTrue(playlistIndex < fb2k::api::pm->get_playlist_count(), "Index is out of bounds");

		pfc::string8 lockName;
		if (fb2k::api::pm->playlist_lock_query_name(playlistIndex, lockName))
			return smp::ToWide(lockName);

		return {};
	}

	JS::Value Plman::GetPlaylistLockedActions(uint32_t playlistIndex)
	{
		QwrException::ExpectTrue(playlistIndex < fb2k::api::pm->get_playlist_count(), "Index is out of bounds");

		Strings actions;
		const auto lockMask = fb2k::api::pm->playlist_lock_get_filter_mask(playlistIndex);

		if (lockMask > 0u)
		{
			for (auto&& [action, mask] : s_actionToMask)
			{
				if (WI_IsAnyFlagSet(lockMask, mask))
					actions.emplace_back(action);
			}
		}

		JS::RootedValue jsValue(m_ctx);
		convert::to_js::ToArrayValue(m_ctx, actions, &jsValue);
		return jsValue;
	}

	std::wstring Plman::GetPlaylistName(uint32_t playlistIndex)
	{
		pfc::string8 name;
		fb2k::api::pm->playlist_get_name(playlistIndex, name);
		return smp::ToWide(name);
	}

	JSObject* Plman::GetPlaylistSelectedItems(uint32_t playlistIndex)
	{
		metadb_handle_list items;
		fb2k::api::pm->playlist_get_selected_items(playlistIndex, items);

		return JsFbMetadbHandleList::CreateJs(m_ctx, items);
	}

	void Plman::InsertPlaylistItems(uint32_t playlistIndex, uint32_t base, JsFbMetadbHandleList* handles, bool select)
	{
		QwrException::ExpectTrue(handles, "handles argument is null");

		pfc::bit_array_val selection(select);
		fb2k::api::pm->playlist_insert_items(playlistIndex, base, handles->GetHandleList(), selection);
	}

	void Plman::InsertPlaylistItemsWithOpt(size_t optArgCount, uint32_t playlistIndex, uint32_t base, JsFbMetadbHandleList* handles, bool select)
	{
		switch (optArgCount)
		{
		case 0:
			return InsertPlaylistItems(playlistIndex, base, handles, select);
		case 1:
			return InsertPlaylistItems(playlistIndex, base, handles);
		default:
			throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	void Plman::InsertPlaylistItemsFilter(uint32_t playlistIndex, uint32_t base, JsFbMetadbHandleList* handles, bool select)
	{
		QwrException::ExpectTrue(handles, "handles argument is null");

		fb2k::api::pm->playlist_insert_items_filter(playlistIndex, base, handles->GetHandleList(), select);
	}

	void Plman::InsertPlaylistItemsFilterWithOpt(size_t optArgCount, uint32_t playlistIndex, uint32_t base, JsFbMetadbHandleList* handles, bool select)
	{
		switch (optArgCount)
		{
		case 0:
			return InsertPlaylistItemsFilter(playlistIndex, base, handles, select);
		case 1:
			return InsertPlaylistItemsFilter(playlistIndex, base, handles);
		default:
			throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	bool Plman::IsAutoPlaylist(uint32_t playlistIndex)
	{
		QwrException::ExpectTrue(playlistIndex < fb2k::api::pm->get_playlist_count(), "Index is out of bounds");

		return autoplaylist_manager::get()->is_client_present(playlistIndex);
	}

	bool Plman::IsPlaylistItemSelected(uint32_t playlistIndex, uint32_t playlistItemIndex)
	{
		return fb2k::api::pm->playlist_is_item_selected(playlistIndex, playlistItemIndex);
	}

	bool Plman::IsPlaylistLocked(uint32_t playlistIndex)
	{
		QwrException::ExpectTrue(playlistIndex < fb2k::api::pm->get_playlist_count(), "Index is out of bounds");

		return fb2k::api::pm->playlist_lock_is_present(playlistIndex);
	}

	bool Plman::IsRedoAvailable(uint32_t playlistIndex)
	{
		QwrException::ExpectTrue(playlistIndex < fb2k::api::pm->get_playlist_count(), "Index is out of bounds");

		return fb2k::api::pm->playlist_is_redo_available(playlistIndex);
	}

	bool Plman::IsUndoAvailable(uint32_t playlistIndex)
	{
		QwrException::ExpectTrue(playlistIndex < fb2k::api::pm->get_playlist_count(), "Index is out of bounds");

		return fb2k::api::pm->playlist_is_undo_available(playlistIndex);
	}

	bool Plman::MovePlaylist(uint32_t from, uint32_t to)
	{
		const auto count = fb2k::api::pm->get_playlist_count();

		if (from < count && to < count && from != to)
		{
			auto sort_order = CustomSort::order(count);
			pfc::create_move_items_permutation(sort_order.get_ptr(), count, pfc::bit_array_one(from), to_int(to - from));
			return fb2k::api::pm->reorder(sort_order.get_ptr(), count);
		}

		return false;
	}

	bool Plman::MovePlaylistSelection(uint32_t playlistIndex, int32_t delta)
	{
		return fb2k::api::pm->playlist_move_selection(playlistIndex, delta);
	}

	uint32_t Plman::PlaylistItemCount(uint32_t playlistIndex)
	{
		return to_uint(fb2k::api::pm->playlist_get_item_count(playlistIndex));
	}

	void Plman::Redo(uint32_t playlistIndex)
	{
		QwrException::ExpectTrue(IsRedoAvailable(playlistIndex), "Redo is not available");

		fb2k::api::pm->playlist_redo_restore(playlistIndex);
	}

	void Plman::RemoveItemFromPlaybackQueue(uint32_t index)
	{
		fb2k::api::pm->queue_remove_mask(pfc::bit_array_one(index));
	}

	void Plman::RemoveItemsFromPlaybackQueue(JS::HandleValue affectedItems)
	{
		pfc::bit_array_bittable affected(fb2k::api::pm->queue_get_count());
		convert::to_native::ProcessArray<uint32_t>(m_ctx, affectedItems, [&affected](uint32_t index) { affected.set(index, true); });
		fb2k::api::pm->queue_remove_mask(affected);
	}

	bool Plman::RemovePlaylist(uint32_t playlistIndex)
	{
		return fb2k::api::pm->remove_playlist(playlistIndex);
	}

	void Plman::RemovePlaylistSelection(uint32_t playlistIndex, bool crop)
	{
		fb2k::api::pm->playlist_remove_selection(playlistIndex, crop);
	}

	void Plman::RemovePlaylistSelectionWithOpt(size_t optArgCount, uint32_t playlistIndex, bool crop)
	{
		switch (optArgCount)
		{
		case 0:
			return RemovePlaylistSelection(playlistIndex, crop);
		case 1:
			return RemovePlaylistSelection(playlistIndex);
		default:
			throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	bool Plman::RemovePlaylistSwitch(uint32_t playlistIndex)
	{
		return fb2k::api::pm->remove_playlist_switch(playlistIndex);
	}

	bool Plman::RenamePlaylist(uint32_t playlistIndex, const std::string& name)
	{
		return fb2k::api::pm->playlist_rename(playlistIndex, name.c_str(), name.length());
	}

	void Plman::SetActivePlaylistContext()
	{
		ui_edit_context_manager::get()->set_context_active_playlist();
	}

	void Plman::SetPlaylistFocusItem(uint32_t playlistIndex, uint32_t playlistItemIndex)
	{
		fb2k::api::pm->playlist_set_focus_item(playlistIndex, playlistItemIndex);
	}

	void Plman::SetPlaylistFocusItemByHandle(uint32_t playlistIndex, JsFbMetadbHandle* handle)
	{
		QwrException::ExpectTrue(handle, "handle argument is null");

		fb2k::api::pm->playlist_set_focus_by_handle(playlistIndex, handle->GetHandle());
	}

	void Plman::SetPlaylistLockedActions(uint32_t playlistIndex, JS::HandleValue lockedActions)
	{
		QwrException::ExpectTrue(playlistIndex < fb2k::api::pm->get_playlist_count(), "Index is out of bounds");
		QwrException::ExpectTrue(lockedActions.isObjectOrNull(), "`lockedActions` argument is not an object nor null");

		const auto is_my_lock = PlaylistLock::is_my_lock(playlistIndex);
		const auto other_lock = fb2k::api::pm->playlist_lock_is_present(playlistIndex) && !is_my_lock;
		QwrException::ExpectTrue(!other_lock, "This lock is owned by a different component");

		uint32_t newLockMask{};

		if (lockedActions.isObject())
		{
			const auto lockedActionsVec = convert::to_native::ToValue<Strings>(m_ctx, lockedActions);

			for (const auto& action : lockedActionsVec)
			{
				const auto it = s_actionToMask.find(action);
				QwrException::ExpectTrue(it != s_actionToMask.end(), "Unknown action name: {}", action);
				newLockMask |= it->second;
			}
		}

		if (newLockMask == fb2k::api::pm->playlist_lock_get_filter_mask(playlistIndex))
			return;

		if (is_my_lock)
			PlaylistLock::remove(playlistIndex);

		if (newLockMask > 0u)
			PlaylistLock::add(playlistIndex, newLockMask);
	}

	void Plman::SetPlaylistLockedActionsWithOpt(size_t optArgCount, uint32_t playlistIndex, JS::HandleValue lockedActions)
	{
		switch (optArgCount)
		{
		case 0:
			return SetPlaylistLockedActions(playlistIndex, lockedActions);
		case 1:
			return SetPlaylistLockedActions(playlistIndex);
		default:
			throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	void Plman::SetPlaylistSelection(uint32_t playlistIndex, JS::HandleValue affectedItems, bool state)
	{
		pfc::bit_array_bittable affected(fb2k::api::pm->playlist_get_item_count(playlistIndex));

		convert::to_native::ProcessArray<uint32_t>(
			m_ctx,
			affectedItems,
			[&affected](uint32_t index) { affected.set(index, true); });

		pfc::bit_array_val status(state);
		fb2k::api::pm->playlist_set_selection(playlistIndex, affected, status);
	}

	void Plman::SetPlaylistSelectionSingle(uint32_t playlistIndex, uint32_t playlistItemIndex, bool state)
	{
		fb2k::api::pm->playlist_set_selection_single(playlistIndex, playlistItemIndex, state);
	}

	bool Plman::ShowAutoPlaylistUI(uint32_t playlistIndex)
	{
		QwrException::ExpectTrue(playlistIndex < fb2k::api::pm->get_playlist_count(), "Index is out of bounds");

		auto api = autoplaylist_manager::get();
		if (!api->is_client_present(playlistIndex))
		{ // TODO v2: replace with error
			return false;
		}

		autoplaylist_client_ptr client = api->query_client(playlistIndex);
		client->show_ui(playlistIndex);

		return true;
	}

	bool Plman::SortByFormat(uint32_t playlistIndex, const std::string& pattern, bool selOnly)
	{
		return fb2k::api::pm->playlist_sort_by_format(playlistIndex, pattern.empty() ? nullptr : pattern.c_str(), selOnly);
	}

	bool Plman::SortByFormatWithOpt(size_t optArgCount, uint32_t playlistIndex, const std::string& pattern, bool selOnly)
	{
		switch (optArgCount)
		{
		case 0:
			return SortByFormat(playlistIndex, pattern, selOnly);
		case 1:
			return SortByFormat(playlistIndex, pattern);
		default:
			throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	bool Plman::SortByFormatV2(uint32_t playlistIndex, const std::string& pattern, int32_t direction)
	{
		metadb_handle_list handles;
		fb2k::api::pm->playlist_get_all_items(playlistIndex, handles);

		auto sort_order = CustomSort::order(handles.get_count());

		titleformat_object::ptr script;
		titleformat_compiler::get()->compile_safe(script, pattern.c_str());

		metadb_handle_list_helper::sort_by_format_get_order(handles, sort_order.get_ptr(), script, nullptr, direction);
		return fb2k::api::pm->playlist_reorder_items(playlistIndex, sort_order.get_ptr(), sort_order.get_count());
	}

	bool Plman::SortByFormatV2WithOpt(size_t optArgCount, uint32_t playlistIndex, const std::string& pattern, int32_t direction)
	{
		switch (optArgCount)
		{
		case 0:
			return SortByFormatV2(playlistIndex, pattern, direction);
		case 1:
			return SortByFormatV2(playlistIndex, pattern);
		default:
			throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	void Plman::SortPlaylistsByName(int32_t direction)
	{
		const auto count = fb2k::api::pm->get_playlist_count();
		auto items = pfc_array<CustomSort::Item>(count);

		pfc::string8 name;
		name.prealloc(512);

		for (auto&& [index, item] : std::views::enumerate(items))
		{
			fb2k::api::pm->playlist_get_name(index, name);
			item.index = index;
			item.text = pfc::wideFromUTF8(name);
		}

		auto order = CustomSort::sort(items, direction);
		fb2k::api::pm->reorder(order.get_ptr(), count);
	}

	void Plman::SortPlaylistsByNameWithOpt(size_t optArgCount, int32_t direction)
	{
		switch (optArgCount)
		{
		case 0:
			return SortPlaylistsByName(direction);
		case 1:
			return SortPlaylistsByName();
		default:
			throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	void Plman::Undo(uint32_t playlistIndex)
	{
		QwrException::ExpectTrue(IsUndoAvailable(playlistIndex), "Undo is not available");

		fb2k::api::pm->playlist_undo_restore(playlistIndex);
	}

	void Plman::UndoBackup(uint32_t playlistIndex)
	{
		QwrException::ExpectTrue(playlistIndex < fb2k::api::pm->get_playlist_count(), "Index is out of bounds");

		fb2k::api::pm->playlist_undo_backup(playlistIndex);
	}

	int32_t Plman::get_ActivePlaylist()
	{
		const auto upos = fb2k::api::pm->get_active_playlist();
		return to_int(upos);
	}

	uint32_t Plman::get_PlaybackOrder()
	{
		return to_uint(fb2k::api::pm->playback_order_get_active());
	}

	int32_t Plman::get_PlayingPlaylist()
	{
		const auto upos = fb2k::api::pm->get_playing_playlist();
		return to_int(upos);
	}

	uint32_t Plman::get_PlaylistCount()
	{
		return to_uint(playlist_manager::get()->get_playlist_count());
	}

	JSObject* Plman::get_PlaylistRecycler()
	{
		if (!m_recycler.initialized())
		{
			m_recycler.init(m_ctx, JsFbPlaylistRecycler::CreateJs(m_ctx));
		}

		return m_recycler;
	}

	void Plman::put_ActivePlaylist(uint32_t playlistIndex)
	{
		QwrException::ExpectTrue(playlistIndex < fb2k::api::pm->get_playlist_count(), "Index is out of bounds");

		fb2k::api::pm->set_active_playlist(playlistIndex);
	}

	void Plman::put_PlaybackOrder(uint32_t order)
	{
		QwrException::ExpectTrue(order < fb2k::api::pm->playback_order_get_count(), "Unknown playback order id: {}", order);

		fb2k::api::pm->playback_order_set_active(order);
	}

	void Plman::put_PlayingPlaylist(uint32_t playlistIndex)
	{
		QwrException::ExpectTrue(playlistIndex < fb2k::api::pm->get_playlist_count(), "Index is out of bounds");

		fb2k::api::pm->set_playing_playlist(playlistIndex);
	}
}
