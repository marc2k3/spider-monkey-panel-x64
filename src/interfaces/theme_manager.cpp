#include "PCH.hpp"
#include "theme_manager.h"

namespace
{
	using namespace mozjs;

	DEFINE_JS_CLASS_OPS(JsThemeManager::FinalizeJsObject)

	DEFINE_JS_CLASS_NO_PROPERTIES("ThemeManager")

	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(DrawThemeBackground, JsThemeManager::DrawThemeBackground, JsThemeManager::DrawThemeBackgroundWithOpt, 4)
	MJS_DEFINE_JS_FN_FROM_NATIVE(IsThemePartDefined, JsThemeManager::IsThemePartDefined)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(SetPartAndStateID, JsThemeManager::SetPartAndStateID, JsThemeManager::SetPartAndStateIDWithOpt, 1)

	constexpr auto jsFunctions = std::to_array<JSFunctionSpec>(
		{
			JS_FN("DrawThemeBackground", DrawThemeBackground, 5, kDefaultPropsFlags),
			JS_FN("IsThemePartDefined", IsThemePartDefined, 1, kDefaultPropsFlags),
			JS_FN("SetPartAndStateID", SetPartAndStateID, 1, kDefaultPropsFlags),
			JS_FS_END,
		});
}

namespace mozjs
{
	const JSClass JsThemeManager::JsClass = jsClass;
	const JSFunctionSpec* JsThemeManager::JsFunctions = jsFunctions.data();
	const JSPropertySpec* JsThemeManager::JsProperties = jsProperties.data();
	const JsPrototypeId JsThemeManager::PrototypeId = JsPrototypeId::ThemeManager;

	JsThemeManager::JsThemeManager(JSContext* ctx, wil::unique_htheme theme) : m_ctx(ctx), m_theme(std::move(theme)) {}

	JsThemeManager::~JsThemeManager()
	{
		m_theme.reset();
	}

	std::unique_ptr<JsThemeManager> JsThemeManager::CreateNative(JSContext* ctx, wil::unique_htheme theme)
	{
		return std::unique_ptr<JsThemeManager>(new JsThemeManager(ctx, std::move(theme)));
	}

	uint32_t JsThemeManager::GetInternalSize()
	{
		return 0;
	}

	void JsThemeManager::DrawThemeBackground(JsGdiGraphics* gr,
		int32_t x, int32_t y, uint32_t w, uint32_t h,
		int32_t clip_x, int32_t clip_y, uint32_t clip_w, uint32_t clip_h)
	{
		QwrException::ExpectTrue(gr, "gr argument is null");

		const auto rect = CRect(x, y, x + w, y + h);
		auto graphics = gr->GetGraphicsObject();
		const auto dc = graphics->GetHDC();

		auto releaser = wil::scope_exit([graphics, dc]
			{
				graphics->ReleaseHDC(dc);
			});

		if (clip_x == 0 && clip_y == 0 && clip_w == 0 && clip_h == 0)
		{
			const auto hr = ::DrawThemeBackground(m_theme.get(), dc, m_part_id, m_state_id, &rect, nullptr);
			smp::CheckHR(hr, "DrawThemeBackground");
		}
		else
		{
			const auto rect_clip = CRect(clip_x, clip_y, clip_x + clip_w, clip_y + clip_h);
			const auto hr = ::DrawThemeBackground(m_theme.get(), dc, m_part_id, m_state_id, &rect, rect_clip);
			smp::CheckHR(hr, "DrawThemeBackground");
		}
	}

	void JsThemeManager::DrawThemeBackgroundWithOpt(size_t optArgCount, JsGdiGraphics* gr,
		int32_t x, int32_t y, uint32_t w, uint32_t h,
		int32_t clip_x, int32_t clip_y, uint32_t clip_w, uint32_t clip_h)
	{
		switch (optArgCount)
		{
		case 0:
			return DrawThemeBackground(gr, x, y, w, h, clip_x, clip_y, clip_w, clip_h);
		case 1:
			return DrawThemeBackground(gr, x, y, w, h, clip_x, clip_y, clip_w);
		case 2:
			return DrawThemeBackground(gr, x, y, w, h, clip_x, clip_y);
		case 3:
			return DrawThemeBackground(gr, x, y, w, h, clip_x);
		case 4:
			return DrawThemeBackground(gr, x, y, w, h);
		default:
			throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	bool JsThemeManager::IsThemePartDefined(int32_t partid, int32_t stateId)
	{
		return ::IsThemePartDefined(m_theme.get(), partid, stateId);
	}

	void JsThemeManager::SetPartAndStateID(int32_t partid, int32_t stateId)
	{
		m_part_id = partid;
		m_state_id = stateId;
	}

	void JsThemeManager::SetPartAndStateIDWithOpt(size_t optArgCount, int32_t partid, int32_t stateId)
	{
		switch (optArgCount)
		{
		case 0:
			return SetPartAndStateID(partid, stateId);
		case 1:
			return SetPartAndStateID(partid);
		default:
			throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}
}
