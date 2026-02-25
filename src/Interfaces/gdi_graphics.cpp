#include "PCH.hpp"
#include "gdi_graphics.h"
#include "gdi_bitmap.h"
#include "gdi_font.h"
#include "gdi_raw_bitmap.h"
#include "measure_string_info.h"

#include <Helpers/EstimateLineWrap.hpp>
#include <utils/colour_helpers.h>

namespace
{
	using namespace mozjs;

	DEFINE_JS_CLASS_OPS(JsGdiGraphics::FinalizeJsObject)

	DEFINE_JS_CLASS_NO_PROPERTIES("GdiGraphics")

	MJS_DEFINE_JS_FN_FROM_NATIVE(CalcTextHeight, JsGdiGraphics::CalcTextHeight)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(CalcTextWidth, JsGdiGraphics::CalcTextWidth, JsGdiGraphics::CalcTextWidthWithOpt, 1)
	MJS_DEFINE_JS_FN_FROM_NATIVE(DrawEllipse, JsGdiGraphics::DrawEllipse)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(DrawImage, JsGdiGraphics::DrawImage, JsGdiGraphics::DrawImageWithOpt, 2)
	MJS_DEFINE_JS_FN_FROM_NATIVE(DrawLine, JsGdiGraphics::DrawLine)
	MJS_DEFINE_JS_FN_FROM_NATIVE(DrawPolygon, JsGdiGraphics::DrawPolygon)
	MJS_DEFINE_JS_FN_FROM_NATIVE(DrawRect, JsGdiGraphics::DrawRect)
	MJS_DEFINE_JS_FN_FROM_NATIVE(DrawRoundRect, JsGdiGraphics::DrawRoundRect)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(DrawString, JsGdiGraphics::DrawString, JsGdiGraphics::DrawStringWithOpt, 1)
	MJS_DEFINE_JS_FN_FROM_NATIVE(EstimateLineWrap, JsGdiGraphics::EstimateLineWrap)
	MJS_DEFINE_JS_FN_FROM_NATIVE(FillEllipse, JsGdiGraphics::FillEllipse)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(FillGradRect, JsGdiGraphics::FillGradRect, JsGdiGraphics::FillGradRectWithOpt, 1)
	MJS_DEFINE_JS_FN_FROM_NATIVE(FillPolygon, JsGdiGraphics::FillPolygon)
	MJS_DEFINE_JS_FN_FROM_NATIVE(FillRoundRect, JsGdiGraphics::FillRoundRect)
	MJS_DEFINE_JS_FN_FROM_NATIVE(FillSolidRect, JsGdiGraphics::FillSolidRect)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(GdiAlphaBlend, JsGdiGraphics::GdiAlphaBlend, JsGdiGraphics::GdiAlphaBlendWithOpt, 1)
	MJS_DEFINE_JS_FN_FROM_NATIVE(GdiDrawBitmap, JsGdiGraphics::GdiDrawBitmap)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(GdiDrawText, JsGdiGraphics::GdiDrawText, JsGdiGraphics::GdiDrawTextWithOpt, 1)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(MeasureString, JsGdiGraphics::MeasureString, JsGdiGraphics::MeasureStringWithOpt, 1)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(SetInterpolationMode, JsGdiGraphics::SetInterpolationMode, JsGdiGraphics::SetInterpolationModeWithOpt, 1)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(SetSmoothingMode, JsGdiGraphics::SetSmoothingMode, JsGdiGraphics::SetSmoothingModeWithOpt, 1)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(SetTextRenderingHint, JsGdiGraphics::SetTextRenderingHint, JsGdiGraphics::SetTextRenderingHintWithOpt, 1)

	constexpr auto jsFunctions = std::to_array<JSFunctionSpec>(
		{
			JS_FN("CalcTextHeight", CalcTextHeight, 2, kDefaultPropsFlags),
			JS_FN("CalcTextWidth", CalcTextWidth, 2, kDefaultPropsFlags),
			JS_FN("DrawEllipse", DrawEllipse, 6, kDefaultPropsFlags),
			JS_FN("DrawImage", DrawImage, 9, kDefaultPropsFlags),
			JS_FN("DrawLine", DrawLine, 6, kDefaultPropsFlags),
			JS_FN("DrawPolygon", DrawPolygon, 3, kDefaultPropsFlags),
			JS_FN("DrawRect", DrawRect, 6, kDefaultPropsFlags),
			JS_FN("DrawRoundRect", DrawRoundRect, 8, kDefaultPropsFlags),
			JS_FN("DrawString", DrawString, 7, kDefaultPropsFlags),
			JS_FN("EstimateLineWrap", EstimateLineWrap, 3, kDefaultPropsFlags),
			JS_FN("FillEllipse", FillEllipse, 5, kDefaultPropsFlags),
			JS_FN("FillGradRect", FillGradRect, 7, kDefaultPropsFlags),
			JS_FN("FillPolygon", FillPolygon, 3, kDefaultPropsFlags),
			JS_FN("FillRoundRect", FillRoundRect, 7, kDefaultPropsFlags),
			JS_FN("FillSolidRect", FillSolidRect, 5, kDefaultPropsFlags),
			JS_FN("GdiAlphaBlend", GdiAlphaBlend, 9, kDefaultPropsFlags),
			JS_FN("GdiDrawBitmap", GdiDrawBitmap, 9, kDefaultPropsFlags),
			JS_FN("GdiDrawText", GdiDrawText, 7, kDefaultPropsFlags),
			JS_FN("MeasureString", MeasureString, 6, kDefaultPropsFlags),
			JS_FN("SetInterpolationMode", SetInterpolationMode, 0, kDefaultPropsFlags),
			JS_FN("SetSmoothingMode", SetSmoothingMode, 0, kDefaultPropsFlags),
			JS_FN("SetTextRenderingHint", SetTextRenderingHint, 0, kDefaultPropsFlags),
			JS_FS_END,
		});
}

namespace mozjs
{
	const JSClass JsGdiGraphics::JsClass = jsClass;
	const JSFunctionSpec* JsGdiGraphics::JsFunctions = jsFunctions.data();
	const JSPropertySpec* JsGdiGraphics::JsProperties = jsProperties.data();
	const JsPrototypeId JsGdiGraphics::PrototypeId = JsPrototypeId::GdiGraphics;

	JsGdiGraphics::JsGdiGraphics(JSContext* ctx) : m_ctx(ctx) {}

	std::unique_ptr<JsGdiGraphics> JsGdiGraphics::CreateNative(JSContext* ctx)
	{
		return std::unique_ptr<JsGdiGraphics>(new JsGdiGraphics(ctx));
	}

	uint32_t JsGdiGraphics::GetInternalSize()
	{
		return 0;
	}

	Gdiplus::Graphics* JsGdiGraphics::GetGraphicsObject() const
	{
		return m_graphics;
	}

	void JsGdiGraphics::SetGraphicsObject(Gdiplus::Graphics* graphics)
	{
		m_graphics = graphics;
	}

	uint32_t JsGdiGraphics::CalcTextHeight(const std::wstring& str, JsGdiFont* font)
	{
		QwrException::ExpectTrue(m_graphics, "Internal error: Gdiplus::Graphics object is null");
		QwrException::ExpectTrue(font, "font argument is null");

		auto dc = wil::GetDC(nullptr);
		auto scope = wil::SelectObject(dc.get(), font->GetHFont());

		SIZE size{};
		GetTextExtentPoint32W(dc.get(), str.data(), to_int(str.size()), &size);
		return size.cy;
	}

	uint32_t JsGdiGraphics::CalcTextWidth(const std::wstring& str, JsGdiFont* font, bool use_exact)
	{
		QwrException::ExpectTrue(m_graphics, "Internal error: Gdiplus::Graphics object is null");
		QwrException::ExpectTrue(font, "font argument is null");

		auto dc = wil::GetDC(nullptr);
		auto scope = wil::SelectObject(dc.get(), font->GetHFont());

		// If font has kerning pairs then GetTextExtentPoint32 will return an inaccurate width if those pairs exist in text.
		// DrawText returns a completely accurate value, but is slower and should not be called from inside estimate_line_wrap
		if (use_exact && str.length() > 1 && GetKerningPairsW(dc.get(), 0, 0) > 0)
		{
			auto rect = CRect(0, 0, 0, 0);
			DrawTextW(dc.get(), str.data(), -1, &rect, DT_CALCRECT | DT_NOPREFIX | DT_SINGLELINE);
			return rect.Width();
		}

		SIZE size{};
		GetTextExtentPoint32W(dc.get(), str.data(), to_int(str.length()), &size);
		return size.cx;
	}

	uint32_t JsGdiGraphics::CalcTextWidthWithOpt(size_t optArgCount, const std::wstring& str, JsGdiFont* font, bool use_exact)
	{
		switch (optArgCount)
		{
		case 0:
			return CalcTextWidth(str, font, use_exact);
		case 1:
			return CalcTextWidth(str, font);
		default:
			throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	void JsGdiGraphics::DrawEllipse(float x, float y, float w, float h, float line_width, uint32_t colour)
	{
		QwrException::ExpectTrue(m_graphics, "Internal error: Gdiplus::Graphics object is null");

		Gdiplus::Pen pen(colour, line_width);
		const auto status = m_graphics->DrawEllipse(&pen, x, y, w, h);
		smp::CheckGdi(status, "DrawEllipse");
	}

	void JsGdiGraphics::DrawImage(JsGdiBitmap* image,
		float dstX, float dstY, float dstW, float dstH,
		float srcX, float srcY, float srcW, float srcH,
		float angle, uint8_t alpha)
	{
		QwrException::ExpectTrue(m_graphics, "Internal error: Gdiplus::Graphics object is null");
		QwrException::ExpectTrue(image, "image argument is null");

		auto bitmap = image->GdiBitmap();
		const auto rect = Gdiplus::RectF(dstX, dstY, dstW, dstH);
		Gdiplus::ImageAttributes ia;
		Gdiplus::Status status{};

		if (alpha < UINT8_MAX)
		{
			Gdiplus::ColorMatrix cm = { 0.f };
			cm.m[0][0] = cm.m[1][1] = cm.m[2][2] = cm.m[4][4] = 1.f;
			cm.m[3][3] = static_cast<float>(alpha) / 255.f;

			status = ia.SetColorMatrix(&cm);
			smp::CheckGdi(status, "SetColorMatrix");
		}

		if (angle != 0.f)
		{
			Gdiplus::Matrix m, old_m;
			
			status = m.RotateAt(angle, Gdiplus::PointF(dstX + (dstW / 2), dstY + (dstH / 2)));
			smp::CheckGdi(status, "RotateAt");

			status = m_graphics->GetTransform(&old_m);
			smp::CheckGdi(status, "GetTransform");

			status = m_graphics->SetTransform(&m);
			smp::CheckGdi(status, "SetTransform");

			status = m_graphics->DrawImage(bitmap, rect, srcX, srcY, srcW, srcH, Gdiplus::UnitPixel, &ia);
			smp::CheckGdi(status, "DrawImage");

			status = m_graphics->SetTransform(&old_m);
			smp::CheckGdi(status, "SetTransform");
		}
		else
		{
			status = m_graphics->DrawImage(bitmap, rect, srcX, srcY, srcW, srcH, Gdiplus::UnitPixel, &ia);
			smp::CheckGdi(status, "DrawImage");
		}
	}

	void JsGdiGraphics::DrawImageWithOpt(size_t optArgCount, JsGdiBitmap* image,
		float dstX, float dstY, float dstW, float dstH,
		float srcX, float srcY, float srcW, float srcH, float angle,
		uint8_t alpha)
	{
		switch (optArgCount)
		{
		case 0:
			return DrawImage(image, dstX, dstY, dstW, dstH, srcX, srcY, srcW, srcH, angle, alpha);
		case 1:
			return DrawImage(image, dstX, dstY, dstW, dstH, srcX, srcY, srcW, srcH, angle);
		case 2:
			return DrawImage(image, dstX, dstY, dstW, dstH, srcX, srcY, srcW, srcH);
		default:
			throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	void JsGdiGraphics::DrawLine(float x1, float y1, float x2, float y2, float line_width, uint32_t colour)
	{
		QwrException::ExpectTrue(m_graphics, "Internal error: Gdiplus::Graphics object is null");

		Gdiplus::Pen pen(colour, line_width);
		const auto status = m_graphics->DrawLine(&pen, x1, y1, x2, y2);
		smp::CheckGdi(status, "DrawLine");
	}

	void JsGdiGraphics::DrawPolygon(uint32_t colour, float line_width, JS::HandleValue points)
	{
		QwrException::ExpectTrue(m_graphics, "Internal error: Gdiplus::Graphics object is null");

		std::vector<Gdiplus::PointF> gdiPoints;
		ParsePoints(points, gdiPoints);

		Gdiplus::Pen pen(colour, line_width);
		const auto status = m_graphics->DrawPolygon(&pen, gdiPoints.data(), to_int(gdiPoints.size()));
		smp::CheckGdi(status, "DrawPolygon");
	}

	void JsGdiGraphics::DrawRect(float x, float y, float w, float h, float line_width, uint32_t colour)
	{
		QwrException::ExpectTrue(m_graphics, "Internal error: Gdiplus::Graphics object is null");

		Gdiplus::Pen pen(colour, line_width);
		const auto status = m_graphics->DrawRectangle(&pen, x, y, w, h);
		smp::CheckGdi(status, "DrawRectangle");
	}

	void JsGdiGraphics::DrawRoundRect(float x, float y, float w, float h, float arc_width, float arc_height, float line_width, uint32_t colour)
	{
		QwrException::ExpectTrue(m_graphics, "Internal error: Gdiplus::Graphics object is null");
		QwrException::ExpectTrue(2 * arc_width <= w && 2 * arc_height <= h, "Arc argument has invalid value");

		Gdiplus::Pen pen(colour, line_width);
		Gdiplus::GraphicsPath gp;
		GetRoundRectPath(gp, Gdiplus::RectF(x, y, w, h), arc_width, arc_height);

		auto status = pen.SetStartCap(Gdiplus::LineCapRound);
		smp::CheckGdi(status, "SetStartCap");

		status = pen.SetEndCap(Gdiplus::LineCapRound);
		smp::CheckGdi(status, "SetEndCap");

		status = m_graphics->DrawPath(&pen, &gp);
		smp::CheckGdi(status, "DrawPath");
	}

	void JsGdiGraphics::DrawString(const std::wstring& str, JsGdiFont* font, uint32_t colour, float x, float y, float w, float h, uint32_t flags)
	{
		QwrException::ExpectTrue(m_graphics, "Internal error: Gdiplus::Graphics object is null");
		QwrException::ExpectTrue(font, "font argument is null");

		Gdiplus::Font* pGdiFont = font->GdiFont();
		QwrException::ExpectTrue(pGdiFont, "Internal error: GdiFont is null");

		Gdiplus::SolidBrush br(colour);
		Gdiplus::StringFormat fmt(Gdiplus::StringFormat::GenericTypographic());
		Gdiplus::Status status{};

		if (flags != 0)
		{
			status = fmt.SetAlignment(static_cast<Gdiplus::StringAlignment>((flags >> 28) & 0x3)); //0xf0000000
			smp::CheckGdi(status, "SetAlignment");

			status = fmt.SetLineAlignment(static_cast<Gdiplus::StringAlignment>((flags >> 24) & 0x3)); //0x0f000000
			smp::CheckGdi(status, "SetLineAlignment");

			status = fmt.SetTrimming(static_cast<Gdiplus::StringTrimming>((flags >> 20) & 0x7)); //0x00f00000
			smp::CheckGdi(status, "SetTrimming");

			status = fmt.SetFormatFlags(static_cast<Gdiplus::StringAlignment>(flags & 0x7FFF)); //0x0000ffff
			smp::CheckGdi(status, "SetFormatFlags");
		}

		status = m_graphics->DrawString(str.c_str(), -1, pGdiFont, Gdiplus::RectF(x, y, w, h), &fmt, &br);
		smp::CheckGdi(status, "DrawString");
	}

	void JsGdiGraphics::DrawStringWithOpt(size_t optArgCount, const std::wstring& str, JsGdiFont* font, uint32_t colour,
		float x, float y, float w, float h,
		uint32_t flags)
	{
		switch (optArgCount)
		{
		case 0:
			return DrawString(str, font, colour, x, y, w, h, flags);
		case 1:
			return DrawString(str, font, colour, x, y, w, h);
		default:
			throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	JSObject* JsGdiGraphics::EstimateLineWrap(const std::wstring& str, JsGdiFont* font, uint32_t max_width)
	{
		QwrException::ExpectTrue(m_graphics, "Internal error: Gdiplus::Graphics object is null");
		QwrException::ExpectTrue(font, "font argument is null");

		auto dc = wil::GetDC(nullptr);
		auto scope = wil::SelectObject(dc.get(), font->GetHFont());
		auto result = ::EstimateLineWrap(dc.get(), max_width).wrap(str);

		JS::RootedObject jsArray(m_ctx, JS::NewArrayObject(m_ctx, result.size() * 2));
		JsException::ExpectTrue(jsArray);

		JS::RootedValue jsValue(m_ctx);
		uint32_t i{};

		for (const auto& [text, width] : result)
		{
			convert::to_js::ToValue(m_ctx, text, &jsValue);

			if (!JS_SetElement(m_ctx, jsArray, i++, jsValue))
			{
				throw JsException();
			}

			jsValue.setNumber(to_uint(width));
			if (!JS_SetElement(m_ctx, jsArray, i++, jsValue))
			{
				throw JsException();
			}
		}

		return jsArray;
	}

	void JsGdiGraphics::FillEllipse(float x, float y, float w, float h, uint32_t colour)
	{
		QwrException::ExpectTrue(m_graphics, "Internal error: Gdiplus::Graphics object is null");

		Gdiplus::SolidBrush br(colour);
		const auto status = m_graphics->FillEllipse(&br, x, y, w, h);
		smp::CheckGdi(status, "FillEllipse");
	}

	void JsGdiGraphics::FillGradRect(float x, float y, float w, float h, float angle, uint32_t colour1, uint32_t colour2, float focus)
	{
		QwrException::ExpectTrue(m_graphics, "Internal error: Gdiplus::Graphics object is null");

		const auto rect = Gdiplus::RectF(x, y, w, h);
		Gdiplus::LinearGradientBrush brush(rect, colour1, colour2, angle, TRUE);
		auto status = brush.SetBlendTriangularShape(focus);
		smp::CheckGdi(status, "SetBlendTriangularShape");

		status = m_graphics->FillRectangle(&brush, rect);
		smp::CheckGdi(status, "FillRectangle");
	}

	void JsGdiGraphics::FillGradRectWithOpt(size_t optArgCount, float x, float y, float w, float h, float angle, uint32_t colour1, uint32_t colour2, float focus)
	{
		switch (optArgCount)
		{
		case 0:
			return FillGradRect(x, y, w, h, angle, colour1, colour2, focus);
		case 1:
			return FillGradRect(x, y, w, h, angle, colour1, colour2);
		default:
			throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	void JsGdiGraphics::FillPolygon(uint32_t colour, uint32_t fillmode, JS::HandleValue points)
	{
		QwrException::ExpectTrue(m_graphics, "Internal error: Gdiplus::Graphics object is null");

		std::vector<Gdiplus::PointF> gdiPoints;
		ParsePoints(points, gdiPoints);

		Gdiplus::SolidBrush br(colour);
		const auto status = m_graphics->FillPolygon(&br, gdiPoints.data(), to_int(gdiPoints.size()), static_cast<Gdiplus::FillMode>(fillmode));
		smp::CheckGdi(status, "FillPolygon");
	}

	void JsGdiGraphics::FillRoundRect(float x, float y, float w, float h, float arc_width, float arc_height, uint32_t colour)
	{
		QwrException::ExpectTrue(m_graphics, "Internal error: Gdiplus::Graphics object is null");
		QwrException::ExpectTrue(2 * arc_width <= w && 2 * arc_height <= h, "Arc argument has invalid value");

		Gdiplus::SolidBrush br(colour);
		Gdiplus::GraphicsPath gp;
		const auto rect = Gdiplus::RectF(x, y, w, h);
		GetRoundRectPath(gp, rect, arc_width, arc_height);

		const auto status = m_graphics->FillPath(&br, &gp);
		smp::CheckGdi(status, "FillPath");
	}

	void JsGdiGraphics::FillSolidRect(float x, float y, float w, float h, uint32_t colour)
	{
		QwrException::ExpectTrue(m_graphics, "Internal error: Gdiplus::Graphics object is null");

		Gdiplus::SolidBrush brush(colour);
		const auto status = m_graphics->FillRectangle(&brush, x, y, w, h);
		smp::CheckGdi(status, "FillRectangle");
	}

	void JsGdiGraphics::GdiAlphaBlend(JsGdiRawBitmap* bitmap,
		int32_t dstX, int32_t dstY, uint32_t dstW, uint32_t dstH,
		int32_t srcX, int32_t srcY, uint32_t srcW, uint32_t srcH,
		uint8_t alpha)
	{
		QwrException::ExpectTrue(m_graphics, "Internal error: Gdiplus::Graphics object is null");
		QwrException::ExpectTrue(bitmap, "bitmap argument is null");

		const auto dc = m_graphics->GetHDC();
		auto releaser = wil::scope_exit([this, dc]
			{
				m_graphics->ReleaseHDC(dc);
			});

		const auto srcDc = bitmap->GetHDC();
		const auto bRet = ::GdiAlphaBlend(dc, dstX, dstY, dstW, dstH, srcDc, srcX, srcY, srcW, srcH, BLENDFUNCTION{ AC_SRC_OVER, 0, alpha, AC_SRC_ALPHA });
		smp::CheckWinApi(bRet, "GdiAlphaBlend");
	}

	void JsGdiGraphics::GdiAlphaBlendWithOpt(size_t optArgCount, JsGdiRawBitmap* bitmap,
		int32_t dstX, int32_t dstY, uint32_t dstW, uint32_t dstH,
		int32_t srcX, int32_t srcY, uint32_t srcW, uint32_t srcH,
		uint8_t alpha)
	{
		switch (optArgCount)
		{
		case 0:
			return GdiAlphaBlend(bitmap, dstX, dstY, dstW, dstH, srcX, srcY, srcW, srcH, alpha);
		case 1:
			return GdiAlphaBlend(bitmap, dstX, dstY, dstW, dstH, srcX, srcY, srcW, srcH);
		default:
			throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	void JsGdiGraphics::GdiDrawBitmap(JsGdiRawBitmap* bitmap,
		int32_t dstX, int32_t dstY, uint32_t dstW, uint32_t dstH,
		int32_t srcX, int32_t srcY, uint32_t srcW, uint32_t srcH)
	{
		QwrException::ExpectTrue(m_graphics, "Internal error: Gdiplus::Graphics object is null");
		QwrException::ExpectTrue(bitmap, "bitmap argument is null");

		const auto dc = m_graphics->GetHDC();
		auto releaser = wil::scope_exit([this, dc]
			{
				m_graphics->ReleaseHDC(dc);
			});

		const auto srcDc = bitmap->GetHDC();
		BOOL bRet{};

		if (dstW == srcW && dstH == srcH)
		{
			bRet = BitBlt(dc, dstX, dstY, dstW, dstH, srcDc, srcX, srcY, SRCCOPY);
			smp::CheckWinApi(bRet, "BitBlt");
		}
		else
		{
			bRet = SetStretchBltMode(dc, HALFTONE);
			smp::CheckWinApi(bRet, "SetStretchBltMode");

			bRet = SetBrushOrgEx(dc, 0, 0, nullptr);
			smp::CheckWinApi(bRet, "SetBrushOrgEx");

			bRet = StretchBlt(dc, dstX, dstY, dstW, dstH, srcDc, srcX, srcY, srcW, srcH, SRCCOPY);
			smp::CheckWinApi(bRet, "StretchBlt");
		}
	}

	void JsGdiGraphics::GdiDrawText(const std::wstring& str, JsGdiFont* font, uint32_t colour, int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t format)
	{
		QwrException::ExpectTrue(m_graphics, "Internal error: Gdiplus::Graphics object is null");
		QwrException::ExpectTrue(font, "font argument is null");

		const auto dc = m_graphics->GetHDC();
		auto releaser = wil::scope_exit([this, dc]
			{
				m_graphics->ReleaseHDC(dc);
			});

		auto rect = CRect(x, y, x + w, y + h);
		auto scope = wil::SelectObject(dc, font->GetHFont());
		SetTextColor(dc, smp::ArgbToColorref(colour));

		auto iRet = SetBkMode(dc, TRANSPARENT);
		smp::CheckWinApi(CLR_INVALID != iRet, "SetBkMode");

		auto uRet = SetTextAlign(dc, TA_LEFT | TA_TOP | TA_NOUPDATECP);
		smp::CheckWinApi(GDI_ERROR != uRet, "SetTextAlign");

		if (WI_IsFlagSet(format, DT_MODIFYSTRING))
		{
			format &= ~DT_MODIFYSTRING;
		}

		if (WI_IsFlagSet(format, DT_CALCRECT))
		{
			auto rect_calc = CRect(rect);
			iRet = DrawTextW(dc, str.data(), -1, &rect_calc, format);
			smp::CheckWinApi(iRet, "DrawText");

			const auto nh = rect_calc.Height();

			format &= ~DT_CALCRECT;

			if (WI_IsFlagSet(format, DT_VCENTER))
			{
				rect.top += ((h - nh) >> 1);
				rect.bottom = rect.top + nh;
			}
			else if (WI_IsFlagSet(format, DT_BOTTOM))
			{
				rect.top = rect.bottom - nh;
			}
		}

		auto dtp = DRAWTEXTPARAMS(sizeof(DRAWTEXTPARAMS), 4, 0, 0, 0);
		iRet = DrawTextExW(dc, const_cast<LPWSTR>(str.data()), -1, &rect, format, &dtp);
		smp::CheckWinApi(iRet, "DrawTextExW");
	}

	void JsGdiGraphics::GdiDrawTextWithOpt(size_t optArgCount, const std::wstring& str, JsGdiFont* font, uint32_t colour,
		int32_t x, int32_t y, uint32_t w, uint32_t h,
		uint32_t format)
	{
		switch (optArgCount)
		{
		case 0:
			return GdiDrawText(str, font, colour, x, y, w, h, format);
		case 1:
			return GdiDrawText(str, font, colour, x, y, w, h);
		default:
			throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	JSObject* JsGdiGraphics::MeasureString(const std::wstring& str, JsGdiFont* font, float x, float y, float w, float h, uint32_t flags)
	{
		QwrException::ExpectTrue(m_graphics, "Internal error: Gdiplus::Graphics object is null");
		QwrException::ExpectTrue(font, "font argument is null");

		Gdiplus::Font* fn = font->GdiFont();
		Gdiplus::StringFormat fmt = Gdiplus::StringFormat::GenericTypographic();

		if (flags != 0)
		{
			fmt.SetAlignment(static_cast<Gdiplus::StringAlignment>((flags >> 28) & 0x3));     //0xf0000000
			fmt.SetLineAlignment(static_cast<Gdiplus::StringAlignment>((flags >> 24) & 0x3)); //0x0f000000
			fmt.SetTrimming(static_cast<Gdiplus::StringTrimming>((flags >> 20) & 0x7));       //0x00f00000
			fmt.SetFormatFlags(static_cast<Gdiplus::StringFormatFlags>(flags & 0x7FFF));        //0x0000ffff
		}

		Gdiplus::RectF bound;
		int32_t chars{}, lines{};
		const auto status = m_graphics->MeasureString(str.c_str(), -1, fn, Gdiplus::RectF(x, y, w, h), &fmt, &bound, &chars, &lines);
		smp::CheckGdi(status, "MeasureString");

		return JsMeasureStringInfo::CreateJs(m_ctx, bound.X, bound.Y, bound.Width, bound.Height, lines, chars);
	}

	JSObject* JsGdiGraphics::MeasureStringWithOpt(size_t optArgCount, const std::wstring& str, JsGdiFont* font,
		float x, float y, float w, float h,
		uint32_t flags)
	{
		switch (optArgCount)
		{
		case 0:
			return MeasureString(str, font, x, y, w, h, flags);
		case 1:
			return MeasureString(str, font, x, y, w, h);
		default:
			throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	void JsGdiGraphics::SetInterpolationMode(uint32_t mode)
	{
		QwrException::ExpectTrue(m_graphics, "Internal error: Gdiplus::Graphics object is null");

		const auto status = m_graphics->SetInterpolationMode(static_cast<Gdiplus::InterpolationMode>(mode));
		smp::CheckGdi(status, "SetInterpolationMode");
	}

	void JsGdiGraphics::SetInterpolationModeWithOpt(size_t optArgCount, uint32_t mode)
	{
		switch (optArgCount)
		{
		case 0:
			return SetInterpolationMode(mode);
		case 1:
			return SetInterpolationMode();
		default:
			throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	void JsGdiGraphics::SetSmoothingMode(uint32_t mode)
	{
		QwrException::ExpectTrue(m_graphics, "Internal error: Gdiplus::Graphics object is null");

		const auto status = m_graphics->SetSmoothingMode(static_cast<Gdiplus::SmoothingMode>(mode));
		smp::CheckGdi(status, "SetSmoothingMode");
	}

	void JsGdiGraphics::SetSmoothingModeWithOpt(size_t optArgCount, uint32_t mode)
	{
		switch (optArgCount)
		{
		case 0:
			return SetSmoothingMode(mode);
		case 1:
			return SetSmoothingMode();
		default:
			throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	void JsGdiGraphics::SetTextRenderingHint(uint32_t mode)
	{
		QwrException::ExpectTrue(m_graphics, "Internal error: Gdiplus::Graphics object is null");

		const auto status = m_graphics->SetTextRenderingHint(static_cast<Gdiplus::TextRenderingHint>(mode));
		smp::CheckGdi(status, "SetTextRenderingHint");
	}

	void JsGdiGraphics::SetTextRenderingHintWithOpt(size_t optArgCount, uint32_t mode)
	{
		switch (optArgCount)
		{
		case 0:
			return SetTextRenderingHint(mode);
		case 1:
			return SetTextRenderingHint();
		default:
			throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	void JsGdiGraphics::GetRoundRectPath(Gdiplus::GraphicsPath& gp, const Gdiplus::RectF& rect, float arc_width, float arc_height) const
	{
		const float arc_dia_w = arc_width * 2;
		const float arc_dia_h = arc_height * 2;
		auto corner = Gdiplus::RectF(rect.X, rect.Y, arc_dia_w, arc_dia_h);

		auto status = gp.Reset();
		smp::CheckGdi(status, "Reset");

		// top left
		status = gp.AddArc(corner, 180, 90);
		smp::CheckGdi(status, "AddArc");

		// top right
		corner.X += (rect.Width - arc_dia_w);
		status = gp.AddArc(corner, 270, 90);
		smp::CheckGdi(status, "AddArc");

		// bottom right
		corner.Y += (rect.Height - arc_dia_h);
		status = gp.AddArc(corner, 0, 90);
		smp::CheckGdi(status, "AddArc");

		// bottom left
		corner.X -= (rect.Width - arc_dia_w);
		status = gp.AddArc(corner, 90, 90);
		smp::CheckGdi(status, "AddArc");

		status = gp.CloseFigure();
		smp::CheckGdi(status, "CloseFigure");
	}

	void JsGdiGraphics::ParsePoints(JS::HandleValue jsValue, std::vector<Gdiplus::PointF>& gdiPoints)
	{
		bool isX = true;
		float x = 0.0;
		auto pointParser = [&gdiPoints, &isX, &x](float coordinate) {
			if (isX)
			{
				x = coordinate;
			}
			else
			{
				gdiPoints.emplace_back(Gdiplus::PointF{ x, coordinate });
			}

			isX = !isX;
			};

		gdiPoints.clear();
		convert::to_native::ProcessArray<float>(m_ctx, jsValue, pointParser);

		QwrException::ExpectTrue(isX, "Points count must be a multiple of two"); ///< Means that we were expecting `y` coordinate
	}
}
