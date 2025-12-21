#pragma once

class FontHelper
{
public:
	using Name = std::array<wchar_t, LF_FACESIZE>;
	using Names = std::vector<Name>;

	FontHelper()
	{
		Gdiplus::InstalledFontCollection font_collection;
		const auto count = font_collection.GetFamilyCount();
		std::vector<Gdiplus::FontFamily> families(count);

		int found{};

		if (font_collection.GetFamilies(count, families.data(), &found) == Gdiplus::Ok)
		{
			for (const auto& family : families)
			{
				Name name{};
				family.GetFamilyName(name.data());
				m_names.emplace_back(name);
			}
		}
	}

	static FontHelper& get()
	{
		static FontHelper instance;
		return instance;
	}

	Names get_names()
	{
		return m_names;
	}

	bool check_name(const std::wstring& name_to_check)
	{
		const auto it = std::ranges::find_if(m_names, [name_to_check](const Name& name)
			{
				return _wcsicmp(name.data(), name_to_check.data()) == 0;
			});

		return it != m_names.end();
	}

	std::wstring get_name_checked(const std::wstring& name)
	{
		return check_name(name) ? name : L"Segoe UI";
	}

	wil::unique_hfont create(const std::wstring& name, int pxSize, Gdiplus::FontStyle style)
	{
		return wil::unique_hfont(CreateFontW(
			-pxSize,
			0,
			0,
			0,
			WI_IsFlagSet(style, Gdiplus::FontStyleBold) ? FW_BOLD : FW_NORMAL,
			WI_IsFlagSet(style, Gdiplus::FontStyleItalic) ? TRUE : FALSE,
			WI_IsFlagSet(style, Gdiplus::FontStyleUnderline) ? TRUE : FALSE,
			WI_IsFlagSet(style, Gdiplus::FontStyleStrikeout) ? TRUE : FALSE,
			DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE,
			name.data()));
	}

private:
	Names m_names;
};
