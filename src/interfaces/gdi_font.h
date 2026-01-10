#pragma once

namespace mozjs
{
	class JsGdiFont : public JsObjectBase<JsGdiFont>
	{
	public:
		~JsGdiFont() override;

		DEFINE_JS_INTERFACE_VARS_GLOBAL_PROTO

		static std::unique_ptr<JsGdiFont> CreateNative(JSContext* cx, LOGFONT& lf);
		static std::unique_ptr<JsGdiFont> CreateNative(JSContext* cx, const std::wstring& name, int32_t pxSize, int32_t style);
		uint32_t GetInternalSize();

	public:
		[[nodiscard]] Gdiplus::Font* GdiFont() const;
		[[nodiscard]] HFONT GetHFont() const;

	public:
		static JSObject* Constructor(JSContext* cx, const std::wstring& fontName, int32_t pxSize, int32_t style = 0);
		static JSObject* ConstructorWithOpt(JSContext* cx, size_t optArgCount, const std::wstring& fontName, int32_t pxSize, int32_t style);

	public:
		[[nodiscard]] uint32_t get_Height() const;
		[[nodiscard]] std::wstring get_Name() const;
		[[nodiscard]] float get_Size() const;
		[[nodiscard]] uint32_t get_Style() const;

	private:
		JsGdiFont(JSContext* cx, LOGFONT& lf);
		JsGdiFont(JSContext* cx, const std::wstring& name, int32_t pxSize, int32_t style);

	private:
		[[maybe_unused]] JSContext* m_ctx{};
		std::wstring m_name = L"Segoe UI";
		std::unique_ptr<Gdiplus::Font> m_font;
		wil::unique_hfont m_hFont;
	};
}
