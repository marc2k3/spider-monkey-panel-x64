#include "PCH.hpp"
#include "gdi_raw_bitmap.h"

namespace
{
	using namespace mozjs;

	DEFINE_JS_CLASS_OPS(JsGdiRawBitmap::FinalizeJsObject)

	DEFINE_JS_CLASS_NO_FUNCTIONS("GdiRawBitMap")

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

	JsGdiRawBitmap::JsGdiRawBitmap(JSContext* ctx, wil::unique_hbitmap hbitmap, uint32_t width, uint32_t height)
		: m_ctx(ctx)
		, m_hdc(CreateCompatibleDC(nullptr))
		, m_hbitmap(std::move(hbitmap))
		, m_obj(SelectObject(m_hdc.get(), m_hbitmap.get()))
		, m_width(width)
		, m_height(height) {}

	wil::unique_hbitmap JsGdiRawBitmap::CreateHBitmapFromGdiPlusBitmap(Gdiplus::Bitmap& bitmap)
	{
		const auto rect = Gdiplus::Rect(0, 0, static_cast<int32_t>(bitmap.GetWidth()), static_cast<int32_t>(bitmap.GetHeight()));
		Gdiplus::BitmapData bmpdata{};

		if (Gdiplus::Ok != bitmap.LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppPARGB, &bmpdata))
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

	std::unique_ptr<JsGdiRawBitmap> JsGdiRawBitmap::CreateNative(JSContext* ctx, Gdiplus::Bitmap* bitmap)
	{
		QwrException::ExpectTrue(bitmap, "Internal error: Gdiplus::Bitmap is null");

		auto hBitmap = CreateHBitmapFromGdiPlusBitmap(*bitmap);
		QwrException::ExpectTrue(hBitmap.get(), "Internal error: failed to get HBITMAP from Gdiplus::Bitmap");

		return std::unique_ptr<JsGdiRawBitmap>(new JsGdiRawBitmap(ctx, std::move(hBitmap), bitmap->GetWidth(), bitmap->GetHeight()));
	}

	uint32_t JsGdiRawBitmap::GetInternalSize()
	{
		// We generate only PixelFormat32bppPARGB images
		return m_width * m_height * Gdiplus::GetPixelFormatSize(PixelFormat32bppPARGB) / 8;
	}

	HDC JsGdiRawBitmap::GetHDC() const
	{
		return m_hdc.get();
	}

	uint32_t JsGdiRawBitmap::get_Height()
	{
		return m_height;
	}

	uint32_t JsGdiRawBitmap::get_Width()
	{
		return m_width;
	}
}
