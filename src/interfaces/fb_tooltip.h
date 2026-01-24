#pragma once

namespace mozjs
{
	class JsFbTooltip : public JsObjectBase<JsFbTooltip>
	{
	public:
		// @remark No need to cleanup JS here, since it must be performed manually beforehand anyway
		~JsFbTooltip() override = default;

		DEFINE_JS_INTERFACE_VARS

		static std::unique_ptr<JsFbTooltip> CreateNative(JSContext* ctx, HWND parent_wnd);
		uint32_t GetInternalSize();

		void PrepareForGc();
		void Update();

	public:
		void Activate();
		void Deactivate();
		uint32_t GetDelayTime(uint32_t type);
		void SetDelayTime(uint32_t type, int32_t time);
		void SetFont(const std::wstring& name, uint32_t pxSize = 12, uint32_t style = 0);
		void SetFontWithOpt(size_t optArgCount, const std::wstring& name, uint32_t pxSize, uint32_t style);
		void SetMaxWidth(uint32_t width);
		void TrackPosition(int32_t x, int32_t y);

	public:
		std::wstring get_Text();
		void put_Text(const std::wstring& text);
		void put_TrackActivate(bool activate);

	private:
		JsFbTooltip(JSContext* ctx, HWND hParentWnd);

	private:
		JSContext* m_ctx;

		CToolTipCtrl m_ctrl;
		HWND m_parent_wnd{};
		wil::unique_hfont m_font;
		std::unique_ptr<CToolInfo> m_info;
		std::wstring m_buffer;
	};
}
