#include "PCH.hpp"
#include "drag_image.h"

namespace
{
	// Ripped from win32_helpers.cpp

	SIZE get_system_dpi()
	{
		HDC dc = GetDC(nullptr);
		SIZE ret = { GetDeviceCaps(dc, LOGPIXELSX), GetDeviceCaps(dc, LOGPIXELSY) };
		ReleaseDC(nullptr, dc);
		return ret;
	}

	SIZE get_system_dpi_cached()
	{
		static const SIZE size = get_system_dpi();
		return size;
	}

	bool UsesTheming(HTHEME theme, int partId, int stateId)
	{
		return theme && IsThemePartDefined(theme, partId, stateId);
	}

	SIZE GetResizedImageSize(const SIZE& currentDimension, const SIZE& maxDimensions) noexcept
	{
		const auto& [imgWidth, imgHeight] = currentDimension;
		const auto& [maxWidth, maxHeight] = maxDimensions;

		if (imgWidth <= maxWidth && imgHeight <= maxHeight)
			return currentDimension;

		SIZE newSize{};
		const auto imgRatio = static_cast<double>(imgHeight) / imgWidth;
		const auto constraintsRatio = static_cast<double>(maxHeight) / maxWidth;

		if (imgRatio > constraintsRatio)
		{
			newSize.cy = maxHeight;
			newSize.cx = lround(maxHeight / imgRatio);
		}
		else
		{
			newSize.cx = maxWidth;
			newSize.cy = lround(maxWidth * imgRatio);
		}

		return newSize;
	}

	void draw_drag_image_label(HWND wnd, HTHEME theme, HDC dc, const RECT& rc, std::wstring_view text)
	{
		auto font = wil::unique_hfont(uCreateIconFont());
		auto select = wil::SelectObject(dc, font.get());

		constexpr int theme_state = 0;
		const bool useTheming = UsesTheming(theme, DD_TEXTBG, theme_state);

		DWORD text_flags = DT_CENTER | DT_WORDBREAK;
		RECT rc_text{};

		if (useTheming)
		{
			GetThemeTextExtent(theme, dc, DD_TEXTBG, theme_state, text.data(), to_int(text.length()), text_flags, &rc, &rc_text);
		}
		else
		{
			rc_text = rc;
			DrawTextW(dc, text.data(), to_int(text.length()), &rc_text, text_flags | DT_CALCRECT);
		}

		auto x_offset = (wil::rect_width(rc) - wil::rect_width(rc_text)) / 2;
		auto y_offset = (wil::rect_height(rc) - wil::rect_height(rc_text)) / 2;
		rc_text.left += x_offset;
		rc_text.right += x_offset;
		rc_text.top += y_offset;
		rc_text.bottom += y_offset;

		if (useTheming)
		{
			MARGINS margins{};
			GetThemeMargins(theme, dc, DD_TEXTBG, theme_state, TMT_CONTENTMARGINS, &rc_text, &margins);

			RECT background_rect = rc_text;
			background_rect.left -= margins.cxLeftWidth;
			background_rect.top -= margins.cyTopHeight;
			background_rect.bottom += margins.cyBottomHeight;
			background_rect.right += margins.cxRightWidth;

			if (IsThemeBackgroundPartiallyTransparent(theme, DD_TEXTBG, 0))
			{
				DrawThemeParentBackground(wnd, dc, &background_rect);
			}

			DrawThemeBackground(theme, dc, DD_TEXTBG, theme_state, &background_rect, nullptr);
			DrawThemeText(theme, dc, DD_TEXTBG, theme_state, text.data(), to_int(text.length()), text_flags, 0, &rc_text);
		}
		else
		{
			auto previousColour = GetTextColor(dc);
			auto previousBackgroundMode = GetBkMode(dc);
			SetTextColor(dc, GetSysColor(COLOR_HIGHLIGHTTEXT));
			SetBkMode(dc, TRANSPARENT);
			DrawTextW(dc, text.data(), to_int(text.length()), &rc_text, text_flags);
			SetTextColor(dc, previousColour);
			SetBkMode(dc, previousBackgroundMode);
		}
	}

	bool draw_drag_custom_image(HDC dc, const SIZE& max_size, Gdiplus::Bitmap& customImage)
	{
		const auto size = SIZE(customImage.GetWidth(), customImage.GetHeight());
		const auto [newWidth, newHeight] = GetResizedImageSize(size, max_size);

		auto gdiGraphics = Gdiplus::Graphics(dc);

		const auto status = gdiGraphics.DrawImage(
			&customImage,
			Gdiplus::Rect(
				lround(static_cast<float>(max_size.cx - newWidth) / 2.f),
				lround(static_cast<float>(max_size.cy - newHeight) / 2.f),
				newWidth,
				newHeight
			),
			0,
			0,
			size.cx,
			size.cy,
			Gdiplus::UnitPixel
		);

		return Gdiplus::Ok == status;
	}

	std::tuple<SIZE, POINT> GetDragImageContentSizeAndOffset(HDC dc, HTHEME theme)
	{
		constexpr int themeState = 0;

		auto sz = get_system_dpi_cached();
		POINT offset{};

		if (!UsesTheming(theme, DD_IMAGEBG, themeState))
		{
			return { sz, offset };
		}

		if FAILED(GetThemePartSize(theme, dc, DD_IMAGEBG, themeState, nullptr, TS_DRAW, &sz))
		{
			return { sz, offset };
		}

		MARGINS margins{};

		if SUCCEEDED(GetThemeMargins(theme, dc, DD_IMAGEBG, themeState, TMT_CONTENTMARGINS, nullptr, &margins))
		{
			sz.cx -= margins.cxLeftWidth + margins.cxRightWidth;
			sz.cy -= margins.cyBottomHeight + margins.cyTopHeight;
		}

		return { sz, offset };
	}
}

namespace uih
{
	bool create_drag_image(
		HWND wnd,
		std::wstring_view text,
		Gdiplus::Bitmap* bitmap,
		SHDRAGIMAGE& dragImage)
	{
		wil::unique_htheme dd_theme;

		if (IsThemeActive() && IsAppThemed())
		{
			dd_theme.reset(OpenThemeData(wnd, VSCLASS_DRAGDROP));
		}

		HDC dc = GetDC(wnd);
		HDC dc_mem = CreateCompatibleDC(dc);

		auto [size, offset] = GetDragImageContentSizeAndOffset(dc, dd_theme.get());
		const auto rc = RECT(0, 0, size.cx, size.cy);

		HBITMAP bm_mem = CreateCompatibleBitmap(dc, size.cx, size.cy);
		auto select = wil::SelectObject(dc_mem, bm_mem);

		if (bitmap)
		{
			if (!draw_drag_custom_image(dc_mem, size, *bitmap))
			{
				return false;
			}
		}

		if (text.length())
		{
			draw_drag_image_label(wnd, dd_theme.get(), dc_mem, rc, text);
		}

		DeleteDC(dc_mem);
		ReleaseDC(wnd, dc);

		dragImage.sizeDragImage.cx = size.cx;
		dragImage.sizeDragImage.cy = size.cy;
		dragImage.ptOffset.x = size.cx / 2;
		dragImage.ptOffset.y = (size.cy - offset.y) - (size.cy - offset.y) / 10;
		dragImage.hbmpDragImage = bm_mem;
		dragImage.crColorKey = CLR_NONE;

		return true;
	}
}
