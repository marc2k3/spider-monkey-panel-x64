#pragma once

namespace CustomSort
{
	struct Item
	{
		pfc::wstringLite text;
		size_t index{};
	};

	using Order = pfc::array_t<size_t>;

	template <int32_t direction>
	static bool sort_compare(const Item& a, const Item& b) noexcept
	{
		const auto ret = direction * StrCmpLogicalW(a.text, b.text);

		if (ret == 0)
			return a.index < b.index;

		return ret < 0;
	}

	static Order order(size_t count) noexcept
	{
		auto sort_order = pfc_array<size_t>(count);
		std::ranges::iota(sort_order, 0uz);
		return sort_order;
	}

	static Order sort(pfc::array_t<Item>& items, int32_t direction = 1) noexcept
	{
		std::ranges::sort(items, direction > 0 ? sort_compare<1> : sort_compare<-1>);

		auto sort_order = order(items.get_count());
		std::ranges::transform(items, sort_order.begin(), [](const Item& item) { return item.index; });
		return sort_order;
	}
}
