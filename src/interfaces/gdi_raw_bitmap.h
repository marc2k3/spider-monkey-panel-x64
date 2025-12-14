#pragma once
#include <utils/gdi_helpers.h>

namespace mozjs
{
	class JsGdiRawBitmap : public JsObjectBase<JsGdiRawBitmap>
	{
	public:
		~JsGdiRawBitmap() override = default;

		DEFINE_JS_INTERFACE_VARS

		static std::unique_ptr<JsGdiRawBitmap> CreateNative(JSContext* cx, Gdiplus::Bitmap* pBmp);
		uint32_t GetInternalSize();

	public:
		[[nodiscard]] __notnull HDC GetHDC() const;

	public: // props
		std::uint32_t get_Height();
		std::uint32_t get_Width();

	private:
		JsGdiRawBitmap(JSContext* cx, wil::unique_hbitmap hBmp, uint32_t width, uint32_t height);

		static wil::unique_hbitmap CreateHBitmapFromGdiPlusBitmap(Gdiplus::Bitmap& bitmap);

		[[maybe_unused]] JSContext* pJsCtx_ = nullptr;
		const wil::unique_hdc pDc_;
		const wil::unique_hbitmap hBmp_;
		const wil::unique_select_object autoBmp_;
		uint32_t width_;
		uint32_t height_;
	};
}
