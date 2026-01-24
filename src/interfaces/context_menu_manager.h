#pragma once

namespace mozjs
{
	class JsMenuObject;
	class JsFbMetadbHandleList;

	class JsContextMenuManager : public JsObjectBase<JsContextMenuManager>
	{
	public:
		~JsContextMenuManager() override = default;

		DEFINE_JS_INTERFACE_VARS

		static std::unique_ptr<JsContextMenuManager> CreateNative(JSContext* cx);
		uint32_t GetInternalSize();

	public:
		void BuildMenu(JsMenuObject* menuObject, int32_t base_id, int32_t max_id = -1);
		void BuildMenuWithOpt(size_t optArgCount, JsMenuObject* menuObject, int32_t base_id, int32_t max_id);
		bool ExecuteByID(uint32_t id);
		void InitContext(JsFbMetadbHandleList* handles);
		void InitContextPlaylist();
		void InitNowPlaying();

	private:
		JsContextMenuManager(JSContext* cx);

	private:
		[[maybe_unused]] JSContext* pJsCtx_ = nullptr;
		contextmenu_manager::ptr contextMenu_;
	};
}
