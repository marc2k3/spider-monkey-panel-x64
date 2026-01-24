#include "PCH.hpp"
#include "gdi_font.h"

#include <2K3/FontHelper.hpp>

namespace
{
	using namespace mozjs;

	DEFINE_JS_CLASS_OPS(JsGdiFont::FinalizeJsObject)

	DEFINE_JS_CLASS_NO_FUNCTIONS("GdiFont")

	MJS_DEFINE_JS_FN_FROM_NATIVE(get_Height, JsGdiFont::get_Height)
	MJS_DEFINE_JS_FN_FROM_NATIVE(get_Name, JsGdiFont::get_Name)
	MJS_DEFINE_JS_FN_FROM_NATIVE(get_Size, JsGdiFont::get_Size)
	MJS_DEFINE_JS_FN_FROM_NATIVE(get_Style, JsGdiFont::get_Style)

	constexpr auto jsProperties = std::to_array<JSPropertySpec>(
		{
			JS_PSG("Height", get_Height, kDefaultPropsFlags),
			JS_PSG("Name", get_Name, kDefaultPropsFlags),
			JS_PSG("Size", get_Size, kDefaultPropsFlags),
			JS_PSG("Style", get_Style, kDefaultPropsFlags),
			JS_PS_END,
		});

	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(GdiFont_Constructor, JsGdiFont::Constructor, JsGdiFont::ConstructorWithOpt, 1)
}

namespace mozjs
{
	const JSClass JsGdiFont::JsClass = jsClass;
	const JSFunctionSpec* JsGdiFont::JsFunctions = jsFunctions.data();
	const JSPropertySpec* JsGdiFont::JsProperties = jsProperties.data();
	const JsPrototypeId JsGdiFont::PrototypeId = JsPrototypeId::GdiFont;
	const JSNative JsGdiFont::JsConstructor = ::GdiFont_Constructor;

	JsGdiFont::JsGdiFont(JSContext* cx, LOGFONT& lf) : m_ctx(cx)
	{
		if (FontHelper::get().check_name(lf.lfFaceName))
		{
			m_name = lf.lfFaceName;
		}
		else
		{
			wcsncpy_s(lf.lfFaceName, LF_FACESIZE, m_name.data(), m_name.length());
		}

		auto dc = wil::GetDC(nullptr);
		m_hFont.reset(CreateFontIndirectW(&lf));
		m_font = std::make_unique<Gdiplus::Font>(dc.get(), m_hFont.get());
	}

	JsGdiFont::JsGdiFont(JSContext* cx, const std::wstring& name, int32_t pxSize, int32_t style) : m_ctx(cx)
	{
		m_name = FontHelper::get().get_name_checked(name);
		m_hFont = FontHelper::get().create(m_name, pxSize, static_cast<Gdiplus::FontStyle>(style));
		m_font = std::make_unique<Gdiplus::Font>(m_name.data(), static_cast<Gdiplus::REAL>(pxSize), style, Gdiplus::UnitPixel);
	}

	JsGdiFont::~JsGdiFont()
	{
		m_font.reset();
		m_hFont.reset();
	}

	std::unique_ptr<JsGdiFont> JsGdiFont::CreateNative(JSContext* cx, LOGFONT& lf)
	{
		return std::unique_ptr<JsGdiFont>(new JsGdiFont(cx, lf));
	}

	std::unique_ptr<JsGdiFont> JsGdiFont::CreateNative(JSContext* cx, const std::wstring& name, int32_t pxSize, int32_t style)
	{
		return std::unique_ptr<JsGdiFont>(new JsGdiFont(cx, name, pxSize, style));
	}

	uint32_t JsGdiFont::GetInternalSize()
	{
		return sizeof(Gdiplus::Font) + sizeof(HFONT);
	}

	Gdiplus::Font* JsGdiFont::GdiFont() const
	{
		return m_font.get();
	}

	HFONT JsGdiFont::GetHFont() const
	{
		return m_hFont.get();
	}

	JSObject* JsGdiFont::Constructor(JSContext* cx, const std::wstring& fontName, int32_t pxSize, int32_t style)
	{
		return JsGdiFont::CreateJs(cx, fontName, pxSize, style);
	}

	JSObject* JsGdiFont::ConstructorWithOpt(JSContext* cx, size_t optArgCount, const std::wstring& fontName, int32_t pxSize, int32_t style)
	{
		switch (optArgCount)
		{
		case 0:
			return Constructor(cx, fontName, pxSize, style);
		case 1:
			return Constructor(cx, fontName, pxSize);
		default:
			throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	uint32_t JsGdiFont::get_Height() const
	{
		static const auto dpi = static_cast<Gdiplus::REAL>(QueryScreenDPI());
		return static_cast<uint32_t>(m_font->GetHeight(dpi));
	}

	std::wstring JsGdiFont::get_Name() const
	{
		return m_name;
	}

	float JsGdiFont::get_Size() const
	{
		return m_font->GetSize();
	}

	uint32_t JsGdiFont::get_Style() const
	{
		return m_font->GetStyle();
	}
}
