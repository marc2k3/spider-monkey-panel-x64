#pragma once
#include <panel/js_panel_window.h>

namespace mozjs
{
	class Window : public JsObjectBase<Window>
	{
	public:
		~Window() override;

		DEFINE_JS_NAMESPACE_VARS

		static std::unique_ptr<Window> CreateNative(JSContext* ctx, smp::js_panel_window& parentPanel);
		uint32_t GetInternalSize();

	public:
		void PrepareForGc();
		[[nodiscard]] HWND GetHwnd() const;

	public: // methods
		void ClearInterval(uint32_t intervalId) const;
		void ClearTimeout(uint32_t timeoutId) const;
		JSObject* CreatePopupMenu();
		JSObject* CreateThemeManager(const std::wstring& classid);
		// TODO v2: remove
		JSObject* CreateTooltip(const std::wstring& name = L"Segoe UI", uint32_t pxSize = 12, uint32_t style = 0);
		// TODO v2: remove
		JSObject* CreateTooltipWithOpt(size_t optArgCount, const std::wstring& name, uint32_t pxSize, uint32_t style);
		// TODO v2: remove
		void DefinePanel(const std::string& name, JS::HandleValue options = JS::UndefinedHandleValue);
		// TODO v2: remove
		void DefinePanelWithOpt(size_t optArgCount, const std::string& name, JS::HandleValue options = JS::UndefinedHandleValue);
		void DefineScript(const std::string& name, JS::HandleValue options = JS::UndefinedHandleValue);
		void DefineScriptWithOpt(size_t optArgCount, const std::string& name, JS::HandleValue options = JS::UndefinedHandleValue);
		void EditScript();
		uint32_t GetColourCUI(uint32_t type);
		uint32_t GetColourDUI(uint32_t type);
		JSObject* GetFontCUI(uint32_t type);
		JSObject* GetFontDUI(uint32_t type);
		JS::Value GetProperty(const std::wstring& name, JS::HandleValue defaultval = JS::NullHandleValue);
		JS::Value GetPropertyWithOpt(size_t optArgCount, const std::wstring& name, JS::HandleValue defaultval);
		void NotifyOthers(const std::wstring& name, JS::HandleValue info);
		void Reload();
		void Repaint(bool force = false);
		void RepaintWithOpt(size_t optArgCount, bool force);
		void RepaintRect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, bool force = false);
		void RepaintRectWithOpt(size_t optArgCount, uint32_t x, uint32_t y, uint32_t w, uint32_t h, bool force);
		void SetCursor(uint32_t id);
		uint32_t SetInterval(JS::HandleValue func, uint32_t delay, JS::HandleValueArray funcArgs = JS::HandleValueArray{ JS::UndefinedHandleValue });
		uint32_t SetIntervalWithOpt(size_t optArgCount, JS::HandleValue func, uint32_t delay, JS::HandleValueArray funcArgs);
		void SetProperty(const std::wstring& name, JS::HandleValue val = JS::NullHandleValue);
		void SetPropertyWithOpt(size_t optArgCount, const std::wstring& name, JS::HandleValue val);
		uint32_t SetTimeout(JS::HandleValue func, uint32_t delay, JS::HandleValueArray funcArgs = JS::HandleValueArray{ JS::UndefinedHandleValue });
		uint32_t SetTimeoutWithOpt(size_t optArgCount, JS::HandleValue func, uint32_t delay, JS::HandleValueArray funcArgs);
		void ShowConfigure();
		void ShowConfigureV2();
		void ShowProperties();

	public: // props
		uint32_t get_DlgCode();
		uint32_t get_DPI();
		int32_t get_Height();
		uint64_t get_ID() const;
		uint32_t get_InstanceType();
		bool get_IsDark();
		bool get_IsTransparent();
		bool get_IsVisible();
		JSObject* get_JsMemoryStats();
		int32_t get_MaxHeight();
		int32_t get_MaxWidth();
		// TODO v2: remove
		uint32_t get_MemoryLimit() const;
		int32_t get_MinHeight();
		int32_t get_MinWidth();
		std::string get_Name();
		// TODO v2: remove
		uint64_t get_PanelMemoryUsage();
		JSObject* get_ScriptInfo();
		JSObject* get_Tooltip();
		// TODO v2: remove
		uint64_t get_TotalMemoryUsage() const;
		int32_t get_Width();
		void put_DlgCode(uint32_t code);
		void put_MaxHeight(int32_t height);
		void put_MaxWidth(int32_t width);
		void put_MinHeight(int32_t height);
		void put_MinWidth(int32_t width);

	private:
		Window(JSContext* ctx, smp::js_panel_window& parent);

		struct DefineScriptOptions
		{
			std::string author;
			std::string version;
			struct Features
			{
				bool dragAndDrop{};
				bool grabFocus = true;
			} features;
		};

		DefineScriptOptions ParseDefineScriptOptions(JS::HandleValue options);

	private:
		JSContext* m_ctx{};
		smp::js_panel_window& m_parent;
		bool m_isFinalized{}, m_isScriptDefined{};
		JS::PersistentRootedObject m_tooltip;
	};
}
