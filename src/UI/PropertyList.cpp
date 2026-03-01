#include "PCH.hpp"
#include "PropertyList.hpp"

#pragma region CListControlComplete
bool PropertyList::GetCellCheckState(size_t row, size_t column) const
{
	if (column == 1uz && m_items[row].is_bool)
		return m_items[row].bool_value;

	return false;
}

bool PropertyList::GetSubItemText(size_t row, size_t column, pfc::string_base& out) const
{
	switch(column)
	{
	case 0uz:
		out = m_items[row].key.c_str();
		return true;
	case 1uz:
		if (m_items[row].is_bool)
			return false;

		out = m_items[row].value.c_str();
		return true;
	default:
		return false;
	}
}

bool PropertyList::TableEdit_IsColumnEditable(size_t column) const
{
	return column == 1uz;
}

PropertyList::cellType_t PropertyList::GetCellType(size_t row, size_t column) const
{
	if (column == 1uz && m_items[row].is_bool)
		return &PFC_SINGLETON(CListCell_Checkbox);

	return &PFC_SINGLETON(CListCell_Text);
}

size_t PropertyList::GetItemCount() const
{
	return m_items.size();
}

void PropertyList::OnSubItemClicked(size_t row, size_t column, CPoint pt)
{
	if (column == 1uz && !m_items[row].is_bool)
	{
		TableEdit_Start(row, column);
		return;
	}

	__super::OnSubItemClicked(row, column, pt);
}

void PropertyList::RequestRemoveSelection()
{
	const auto mask = GetSelectionMask();
	const auto old_count = GetItemCount();
	pfc::remove_mask_t(m_items, mask);
	OnItemsRemoved(mask, old_count);

	if (m_items.empty())
	{
		m_btn_clear.EnableWindow(FALSE);
		m_btn_export.EnableWindow(FALSE);
	}
}

void PropertyList::SetCellCheckState(size_t row, size_t column, bool value)
{
	if (column == 1uz && m_items[row].is_bool)
	{
		m_items[row].bool_value = value;
		__super::SetCellCheckState(row, column, value);
	}
}

void PropertyList::TableEdit_SetField(size_t row, size_t column, const char* value)
{
	if (column == 1uz && !m_items[row].is_bool)
	{
		m_items[row].value = value;
		ReloadItem(row);
	}
}
#pragma endregion

config::PanelProperties PropertyList::GetProperties()
{
	config::PanelProperties properties;

	for (auto&& item : m_items)
	{
		config::SerializedJsValue serializedValue;

		if (item.is_bool)
		{
			serializedValue = item.bool_value;
		}
		else if (item.is_string)
		{
			serializedValue = item.value;
		}
		else
		{
			const auto wstr = smp::ToWide(item.value);
			auto src = _variant_t(wstr.data());
			_variant_t dst;

			if SUCCEEDED(VariantChangeType(&dst, &src, 0, VT_R8))
			{
				serializedValue = dst.dblVal;
			}
			else
			{
				serializedValue = item.value;
			}
		}

		const auto wkey = smp::ToWide(item.key);
		const auto value = std::make_shared<config::SerializedJsValue>(serializedValue);
		properties.values.emplace(wkey, value);
	}

	return properties;
}

int32_t PropertyList::OnCreate(LPCREATESTRUCT)
{
	AddColumn("Name", MulDiv(360, m_dpi.cx, 96));
	AddColumnAutoWidth("Value");

	m_btn_clear = GetParent().GetDlgItem(IDC_BTN_CLEAR);
	m_btn_export = GetParent().GetDlgItem(IDC_BTN_EXPORT);
	return 0;
}

void PropertyList::OnContextMenu(CPoint pt)
{
	if (GetSelectedCount() == 0uz)
		return;

	auto menu = CreatePopupMenu();
	AppendMenuW(menu, MF_STRING, ID_MENU_SELECTALL, L"Select all\tCtrl+A");
	AppendMenuW(menu, MF_STRING, ID_MENU_SELECTNONE, L"Select none");
	AppendMenuW(menu, MF_STRING, ID_MENU_INVERTSEL, L"Invert selection");
	AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
	AppendMenuW(menu, MF_STRING, ID_MENU_REMOVE, L"Remove\tDel");

	pt = GetContextMenuPoint(pt);
	const auto id = TrackPopupMenuEx(menu, TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, m_hWnd, nullptr);
	DestroyMenu(menu);

	switch (id)
	{
	case ID_MENU_SELECTALL:
		SelectAll();
		break;
	case ID_MENU_SELECTNONE:
		SelectNone();
		break;
	case ID_MENU_INVERTSEL:
		SetSelection(pfc::bit_array_true(), pfc::bit_array_not(GetSelectionMask()));
		break;
	case ID_MENU_REMOVE:
		RequestRemoveSelection();
		break;
	}
}

void PropertyList::SetProperties(const config::PanelProperties& properties)
{
	m_items.clear();

	for (const auto& [key, serialisedValue] : properties.values)
	{
		ListItem item;
		item.key = smp::ToU8(key);

		std::visit([&item](auto&& value)
			{
				using T = std::decay_t<decltype(value)>;

				if constexpr (std::is_same_v<T, bool>)
				{
					item.is_bool = true;
					item.bool_value = value;
				}
				else if constexpr (std::is_same_v<T, std::string>)
				{
					item.is_string = true;
					item.value = value;
				}
				else if constexpr (std::is_same_v<T, int32_t>)
				{
					item.value = fmt::to_string(value);
				}
				else if constexpr (std::is_same_v<T, double>)
				{
					item.value = fmt::format("{:.16g}", value);
				}
				else
				{
					static_assert(smp::always_false_v<T>, "non-exhaustive visitor!");
				}
			}, *serialisedValue);

		m_items.emplace_back(item);
	}

	ReloadData();
	m_btn_clear.EnableWindow(m_items.size() > 0uz);
	m_btn_export.EnableWindow(m_items.size() > 0uz);
}
