#pragma once

template <std::integral T>
static auto indices(T to)
{
	return std::views::iota(T{}, to);
}

template <typename T>
static auto pfc_array(size_t count) noexcept
{
	pfc::array_t<T> arr;
	arr.set_size(count);
	return arr;
}

template <typename T>
static auto pfc_list(const T& item) noexcept
{
	return pfc::list_single_ref_t<T>(item);
}

static void set_window_theme(HWND wnd, bool is_dark) noexcept
{
	SetWindowTheme(wnd, is_dark ? L"DarkMode_Explorer" : nullptr, nullptr);
}

static std::wstring nativeW(fb2k::stringRef path) noexcept
{
	const auto native_path = filesystem::g_get_native_path(path->c_str());
	return smp::ToWide(native_path);
}

static int32_t hex_digit_to_int(char ch) noexcept
{
	if (ch >= '0' && ch <= '9')
		return ch - '0';
	else if (ch >= 'A' && ch <= 'F')
		return ch - 'A' + 10;
	else if (ch >= 'a' && ch <= 'f')
		return ch - 'a' + 10;
	else
		return 0;
}

static COLORREF to_colorref(std::string_view hex) noexcept
{
	const int32_t r = hex_digit_to_int(hex.at(0uz)) << 4 | hex_digit_to_int(hex.at(1uz));
	const int32_t g = hex_digit_to_int(hex.at(2uz)) << 4 | hex_digit_to_int(hex.at(3uz));
	const int32_t b = hex_digit_to_int(hex.at(4uz)) << 4 | hex_digit_to_int(hex.at(5uz));
	return RGB(r, g, b);
}

static std::optional<int32_t> to_int(std::string_view str) noexcept
{
	if (pfc::string_is_numeric(str.data()))
		return std::stoi(str.data());

	return std::nullopt;
}

static int to_int(size_t val) noexcept
{
	if (val > INT_MAX)
		return -1;
	else
		return static_cast<int>(val);
}

static uint32_t to_uint(size_t val) noexcept
{
	return static_cast<uint32_t>(val);
}

static uint32_t lengthu(auto blah) noexcept
{
	return static_cast<uint32_t>(blah.length());
}

static uint32_t sizeu(auto blah) noexcept
{
	return static_cast<uint32_t>(blah.size());
}
