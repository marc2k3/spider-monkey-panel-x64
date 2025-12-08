#include <stdafx.h>
#include "text_helpers.h"

namespace
{
	using namespace smp::utils;

	bool is_wrap_char(wchar_t current, wchar_t next)
	{
		if (std::iswpunct(current))
			return false;

		if (next == '\0')
			return true;

		if (std::iswspace(current))
			return true;

		bool currentAlphaNum = !!std::iswalnum(current);

		if (currentAlphaNum)
		{
			if (std::iswpunct(next))
			{
				return false;
			}
		}

		return !currentAlphaNum || !std::iswalnum(next);
	}

	void WrapTextRecur(HDC hdc, const std::wstring& text, uint32_t width, std::vector<WrappedTextLine>& out)
	{
		const auto textWidth = GetTextWidth(hdc, text);

		if (textWidth <= width || text.size() <= 1uz)
		{
			out.emplace_back(text, textWidth);
		}
		else
		{
			uint32_t textLength = lengthu(text) * width / textWidth;

			if (GetTextWidth(hdc, text.substr(0, textLength)) < width)
			{
				while (GetTextWidth(hdc, text.substr(0, std::min(lengthu(text), textLength + 1u))) <= width)
				{
					++textLength;
				}
			}
			else
			{
				while (GetTextWidth(hdc, text.substr(0, textLength)) > width && textLength > 1)
				{
					--textLength;
				}
			}

			{
				uint32_t fallbackTextLength = std::max(textLength, 1u);

				while (textLength > 0 && !is_wrap_char(text[textLength - 1], text[textLength]))
				{
					--textLength;
				}

				if (!textLength)
				{
					textLength = fallbackTextLength;
				}

				out.emplace_back(text.substr(0, textLength), GetTextWidth(hdc, text.substr(0, textLength)));
			}

			if (textLength < text.size())
			{
				WrapTextRecur(hdc, text.substr(textLength), width, out);
			}
		}
	}
}

namespace smp::utils
{
	uint32_t GetTextHeight(HDC hdc, std::wstring_view text)
	{
		SIZE size;
		GetTextExtentPoint32W(hdc, text.data(), to_int(text.size()), &size);
		return size.cy;
	}

	uint32_t GetTextWidth(HDC hdc, std::wstring_view text, bool accurate)
	{
		// If font has kerning pairs then GetTextExtentPoint32 will return an inaccurate width if those pairs exist in text.
		// DrawText returns a completely accurate value, but is slower and should not be called from inside estimate_line_wrap
		if (accurate && text.size() > 1 && GetKerningPairs(hdc, 0, 0) > 0)
		{
			RECT rc_calc{ 0, 0, 0, 0 };
			DrawTextW(hdc, text.data(), -1, &rc_calc, DT_CALCRECT | DT_NOPREFIX | DT_SINGLELINE);
			return rc_calc.right;
		}

		SIZE size;
		// TODO: add error checks
		GetTextExtentPoint32W(hdc, text.data(), static_cast<int>(text.size()), &size);
		return size.cx;
	}

	std::vector<WrappedTextLine> WrapText(HDC hdc, const std::wstring& text, uint32_t maxWidth)
	{
		std::vector<WrappedTextLine> lines;
		const wchar_t* curTextPos = text.c_str();

		while (true)
		{
			const wchar_t* next = wcschr(curTextPos, '\n');
			if (!next)
			{
				WrapTextRecur(hdc, curTextPos, maxWidth, lines);
				break;
			}

			const wchar_t* walk = next;
			while (walk > curTextPos && walk[-1] == '\r')
			{
				--walk;
			}

			WrapTextRecur(hdc, std::wstring(curTextPos, walk - curTextPos), maxWidth, lines);
			curTextPos = next + 1;
		}

		return lines;
	}
}
