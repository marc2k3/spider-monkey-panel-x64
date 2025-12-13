#pragma once

namespace smp
{
	struct WrappedTextLine
	{
		std::wstring text;
		uint32_t width;
	};

	[[nodiscard]] uint32_t GetTextHeight(HDC hdc, std::wstring_view text);
	[[nodiscard]] uint32_t GetTextWidth(HDC hdc, std::wstring_view text, bool accurate = false);
	[[nodiscard]] std::vector<WrappedTextLine> WrapText(HDC hdc, const std::wstring& text, uint32_t maxWidth);
}
