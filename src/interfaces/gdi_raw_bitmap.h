#pragma once

namespace mozjs
{
	class JsGdiRawBitmap : public JsObjectBase<JsGdiRawBitmap>
	{
	public:
		~JsGdiRawBitmap() override = default;

		DEFINE_JS_INTERFACE_VARS

		static std::unique_ptr<JsGdiRawBitmap> CreateNative(JSContext* ctx, Gdiplus::Bitmap* bitmap);
		uint32_t GetInternalSize();

	public:
		[[nodiscard]] HDC GetHDC() const;

	public: // props
		uint32_t get_Height();
		uint32_t get_Width();

	private:
		JsGdiRawBitmap(JSContext* ctx, wil::unique_hbitmap hbitmap, uint32_t width, uint32_t height);

		static wil::unique_hbitmap CreateHBitmapFromGdiPlusBitmap(Gdiplus::Bitmap& bitmap);

		[[maybe_unused]] JSContext* m_ctx{};
		const wil::unique_hdc m_hdc;
		const wil::unique_hbitmap m_hbitmap;
		const wil::unique_select_object m_obj;
		uint32_t m_width{}, m_height{};
	};
}
