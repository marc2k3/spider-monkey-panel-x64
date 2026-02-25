#pragma once

namespace smp
{
	[[nodiscard]] constexpr COLORREF ArgbToColorref(DWORD argb)
	{
		return RGB(argb >> RED_SHIFT, argb >> GREEN_SHIFT, argb >> BLUE_SHIFT);
	}

	[[nodiscard]] constexpr DWORD ColorrefToArgb(COLORREF color)
	{
		// COLORREF : 0x00bbggrr
		// ARGB : 0xaarrggbb
		return (GetRValue(color) << RED_SHIFT) | (GetGValue(color) << GREEN_SHIFT) | (GetBValue(color) << BLUE_SHIFT) | 0xff000000;
	}
}
