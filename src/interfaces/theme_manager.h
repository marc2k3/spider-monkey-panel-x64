#pragma once

namespace mozjs
{
	class JsThemeManager : public JsObjectBase<JsThemeManager>
	{
	public:
		~JsThemeManager() override;

		DEFINE_JS_INTERFACE_VARS

		static std::unique_ptr<JsThemeManager> CreateNative(JSContext* ctx, wil::unique_htheme theme);
		uint32_t GetInternalSize();

	public:
		void DrawThemeBackground(JsGdiGraphics* gr,
			int32_t x, int32_t y, uint32_t w, uint32_t h,
			int32_t clip_x = 0, int32_t clip_y = 0, uint32_t clip_w = 0, uint32_t clip_h = 0);
		void DrawThemeBackgroundWithOpt(size_t optArgCount, JsGdiGraphics* gr,
			int32_t x, int32_t y, uint32_t w, uint32_t h,
			int32_t clip_x, int32_t clip_y, uint32_t clip_w, uint32_t clip_h);
		bool IsThemePartDefined(int32_t partid, int32_t stateId);
		void SetPartAndStateID(int32_t partid, int32_t stateId = 0);
		void SetPartAndStateIDWithOpt(size_t optArgCount, int32_t partid, int32_t stateId);

	private:
		JsThemeManager(JSContext* ctx, wil::unique_htheme theme);

	private:
		JSContext* m_ctx{};
		wil::unique_htheme m_theme;
		int32_t m_part_id{}, m_state_id{};
	};
}
