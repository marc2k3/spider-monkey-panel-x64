#pragma once

namespace qwr
{
	template <typename T>
	std::optional<T> GetNumber(std::string_view strView, int base = 10)
	{
		T number{};

		if (auto [pos, ec] = std::from_chars(strView.data(), strView.data() + strView.size(), number, base); ec == std::errc{})
		{
			return number;
		}
		else
		{
			return std::nullopt;
		}
	}
}
