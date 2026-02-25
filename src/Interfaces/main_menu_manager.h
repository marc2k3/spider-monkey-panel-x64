#pragma once

namespace mozjs
{
	class JsMenuObject;

	class JsMainMenuManager : public JsObjectBase<JsMainMenuManager>
	{
	public:
		~JsMainMenuManager() override = default;

		DEFINE_JS_INTERFACE_VARS

		static std::unique_ptr<JsMainMenuManager> CreateNative(JSContext* cx);
		uint32_t GetInternalSize();

	public:
		void BuildMenu(JsMenuObject* menu, int32_t base_id, int32_t count);
		bool ExecuteByID(uint32_t id);
		void Init(const std::string& root_name);

	private:
		JsMainMenuManager(JSContext* cx);

	private:
		[[maybe_unused]] JSContext* pJsCtx_ = nullptr;
		mainmenu_manager::ptr menuManager_;
	};
}
