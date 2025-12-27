#pragma once

namespace mozjs
{
	class JsFbMetadbHandleList;

	class JsFbUiSelectionHolder : public JsObjectBase<JsFbUiSelectionHolder>
	{
	public:
		~JsFbUiSelectionHolder() override = default;

		DEFINE_JS_INTERFACE_VARS

		static std::unique_ptr<JsFbUiSelectionHolder> CreateNative(JSContext* cx, const ui_selection_holder::ptr& holder);
		uint32_t GetInternalSize();

	public:
		static constexpr std::array selection_ids =
		{
			&contextmenu_item::caller_undefined,
			&contextmenu_item::caller_active_playlist_selection,
			&contextmenu_item::caller_active_playlist,
			&contextmenu_item::caller_playlist_manager,
			&contextmenu_item::caller_now_playing,
			&contextmenu_item::caller_keyboard_shortcut_list,
			&contextmenu_item::caller_media_library_viewer
		};

		void SetPlaylistSelectionTracking();
		void SetPlaylistTracking();
		void SetSelection(JsFbMetadbHandleList* handles, uint8_t type = 0);
		void SetSelectionWithOpt(size_t optArgCount, JsFbMetadbHandleList* handles, uint8_t type);

	private:
		JsFbUiSelectionHolder(JSContext* cx, const ui_selection_holder::ptr& holder);

	private:
		[[maybe_unused]] JSContext* pJsCtx_ = nullptr;
		ui_selection_holder::ptr holder_;
	};
}
