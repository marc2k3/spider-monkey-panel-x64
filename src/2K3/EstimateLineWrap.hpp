#pragma once

class EstimateLineWrap
{
public:
	struct WrappedItem
	{
		std::wstring text;
		size_t width{};
	};

	using WrappedItems = std::vector<WrappedItem>;

	EstimateLineWrap(HDC hdc, size_t width) : m_hdc(hdc), m_width(width) {}

	WrappedItems wrap(std::wstring_view text)
	{
		const auto delims = text.contains(CRLF_WIDE) ? CRLF_WIDE : LF_WIDE;
		const auto lines = std::views::split(text, delims) | std::ranges::to<WStrings>();
		WrappedItems items;

		for (auto&& line : lines)
		{
			wrap_recur(line, items);
		}
		return items;
	}

private:
	bool not_wrap_char(wchar_t ch)
	{
		return iswalnum(ch) || iswpunct(ch);
	}

	size_t get_text_width(std::wstring_view text, size_t length)
	{
		SIZE size;
		GetTextExtentPoint32W(m_hdc, text.data(), to_int(length), &size);
		return size.cx;
	}

	void wrap_recur(std::wstring_view text, WrappedItems& out)
	{
		const auto text_width = get_text_width(text, text.length());

		if (text_width <= m_width)
		{
			out.emplace_back(text.data(), text_width);
		}
		else
		{
			auto text_length = text.length() * m_width / text_width;

			if (get_text_width(text, text_length) < m_width)
			{
				while (get_text_width(text, std::min(text.length(), text_length + 1uz)) <= m_width)
				{
					++text_length;
				}
			}
			else
			{
				while (get_text_width(text, text_length) > m_width && text_length > 1)
				{
					--text_length;
				}
			}

			const auto fallback_length = std::max(text_length, 1uz);

			while (text_length > 0uz && not_wrap_char(text.at(text_length - 1uz)))
			{
				--text_length;
			}

			if (text_length == 0uz)
				text_length = fallback_length;

			out.emplace_back(std::wstring(text.substr(0uz, text_length)), get_text_width(text, text_length));

			if (text_length < text.length())
			{
				wrap_recur(text.substr(text_length), out);
			}
		}
	}

	static constexpr std::wstring_view CRLF_WIDE = L"\r\n";
	static constexpr std::wstring_view LF_WIDE = L"\n";

	HDC m_hdc{};
	size_t m_width{};
};
