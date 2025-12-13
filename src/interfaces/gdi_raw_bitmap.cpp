#include <stdafx.h>
#include "gdi_raw_bitmap.h"

#include <utils/gdi_error_helpers.h>
#include <utils/gdi_helpers.h>

namespace
{
	using namespace mozjs;

	DEFINE_JS_CLASS_OPS(JsGdiRawBitmap::FinalizeJsObject)

	DEFINE_JS_CLASS("GdiRawBitMap")

	constexpr auto jsFunctions = std::to_array<JSFunctionSpec>(
		{
			JS_FS_END,
		});

	MJS_DEFINE_JS_FN_FROM_NATIVE(get_Height, JsGdiRawBitmap::get_Height)
	MJS_DEFINE_JS_FN_FROM_NATIVE(get_Width, JsGdiRawBitmap::get_Width)

	constexpr auto jsProperties = std::to_array<JSPropertySpec>(
		{
			JS_PSG("Height", get_Height, kDefaultPropsFlags),
			JS_PSG("Width", get_Width, kDefaultPropsFlags),
			JS_PS_END,
		});
}

namespace mozjs
{
	const JSClass JsGdiRawBitmap::JsClass = jsClass;
	const JSFunctionSpec* JsGdiRawBitmap::JsFunctions = jsFunctions.data();
	const JSPropertySpec* JsGdiRawBitmap::JsProperties = jsProperties.data();
	const JsPrototypeId JsGdiRawBitmap::PrototypeId = JsPrototypeId::GdiRawBitmap;

	JsGdiRawBitmap::JsGdiRawBitmap(JSContext* cx, wil::unique_hbitmap hBmp, uint32_t width, uint32_t height)
		: pJsCtx_(cx)
		, pDc_(CreateCompatibleDC(nullptr))
		, hBmp_(std::move(hBmp))
		, autoBmp_(SelectObject(pDc_.get(), hBmp_.get()))
		, width_(width)
		, height_(height) {}

	wil::unique_hbitmap JsGdiRawBitmap::CreateHBitmapFromGdiPlusBitmap(Gdiplus::Bitmap& bitmap)
	{
		const Gdiplus::Rect rect{ 0, 0, static_cast<int>(bitmap.GetWidth()), static_cast<int>(bitmap.GetHeight()) };
		Gdiplus::BitmapData bmpdata{};

		if (bitmap.LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppPARGB, &bmpdata) != Gdiplus::Ok)
			return nullptr;

		BITMAP bm{};
		bm.bmType = 0;
		bm.bmWidth = bmpdata.Width;
		bm.bmHeight = bmpdata.Height;
		bm.bmWidthBytes = bmpdata.Stride;
		bm.bmPlanes = 1;
		bm.bmBitsPixel = 32;
		bm.bmBits = bmpdata.Scan0;

		auto hBitmap = wil::unique_hbitmap(CreateBitmapIndirect(&bm));
		bitmap.UnlockBits(&bmpdata);

		return hBitmap;
	}

	std::unique_ptr<JsGdiRawBitmap> JsGdiRawBitmap::CreateNative(JSContext* cx, Gdiplus::Bitmap* pBmp)
	{
		QwrException::ExpectTrue(pBmp, "Internal error: Gdiplus::Bitmap is null");

		auto hBitmap = CreateHBitmapFromGdiPlusBitmap(*pBmp);
		QwrException::ExpectTrue(hBitmap.get(), "Internal error: failed to get HBITMAP from Gdiplus::Bitmap");

		return std::unique_ptr<JsGdiRawBitmap>(new JsGdiRawBitmap(cx, std::move(hBitmap), pBmp->GetWidth(), pBmp->GetHeight()));
	}

	uint32_t JsGdiRawBitmap::GetInternalSize()
	{
		// We generate only PixelFormat32bppPARGB images
		return width_ * height_ * Gdiplus::GetPixelFormatSize(PixelFormat32bppPARGB) / 8;
	}

	HDC JsGdiRawBitmap::GetHDC() const
	{
		return pDc_.get();
	}

	std::uint32_t JsGdiRawBitmap::get_Height()
	{
		return height_;
	}

	std::uint32_t JsGdiRawBitmap::get_Width()
	{
		return width_;
	}
}
