#include "PCH.hpp"
#include "ui_properties.h"

#include <Panel/js_panel_window.h>
#include <UI/Scintilla/DialogEditor.hpp>

namespace
{
	static const CDialogResizeHelper::Param resize_data[] =
	{
		{ IDC_LIST_PROPERTIES, 0, 0, 1, 1 },
		{ IDC_BTN_DEL, 0, 1, 0, 1 },
		{ IDC_BTN_CLEAR, 0, 1, 0, 1 },
		{ IDC_BTN_IMPORT, 0, 1, 0, 1 },
		{ IDC_BTN_EXPORT, 0, 1, 0, 1 },
		{ IDOK, 1, 1, 1, 1 },
		{ IDCANCEL, 1, 1, 1, 1 },
		{ IDC_BTN_APPLY, 1, 1, 1, 1 },
	};

	static const CRect resize_min_max(620, 400, 0, 0);
}

CDialogProperties::CDialogProperties(smp::js_panel_window& parent)
	: m_parent(parent)
	, m_resizer(resize_data, resize_min_max) {}

LRESULT CDialogProperties::OnInitDialog(HWND, LPARAM)
{
	// Subclassing
	m_list_ctrl.SubclassWindow(GetDlgItem(IDC_LIST_PROPERTIES));
	m_list_ctrl.ModifyStyle(0, LBS_SORT | LBS_HASSTRINGS);
	m_list_ctrl.SetExtendedListStyle(PLS_EX_SORTED | PLS_EX_XPLOOK);

	m_property_values = m_parent.GetPanelProperties().values;
	UpdateUiFromData();
	dialog_position.apply_to_window(m_hWnd);
	return TRUE; // set focus to default control
}

LRESULT CDialogProperties::OnPinItemChanged(LPNMHDR pnmh)
{
	auto pnpi = (LPNMPROPERTYITEM)pnmh;

	if (m_property_values.contains(pnpi->prop->GetName()))
	{
		auto& val = *m_property_values.at(pnpi->prop->GetName());
		_variant_t var;

		if (pnpi->prop->GetValue(&var))
		{
			std::visit([&var](auto& arg)
				{
					using T = std::decay_t<decltype(arg)>;
					if constexpr (std::is_same_v<T, bool>)
					{
						var.ChangeType(VT_BOOL);
						arg = static_cast<bool>(var.boolVal);
					}
					else if constexpr (std::is_same_v<T, int32_t>)
					{
						var.ChangeType(VT_I4);
						arg = static_cast<int32_t>(var.lVal);
					}
					else if constexpr (std::is_same_v<T, double>)
					{
						if (VT_BSTR == var.vt)
						{
							arg = std::stod(var.bstrVal);
						}
						else
						{
							var.ChangeType(VT_R8);
							arg = var.dblVal;
						}
					}
					else if constexpr (std::is_same_v<T, std::string>)
					{
						var.ChangeType(VT_BSTR);

						if (var.bstrVal)
							arg = smp::ToU8(var.bstrVal);
						else
							arg.clear();
					}
					else
					{
						static_assert(smp::always_false_v<T>, "non-exhaustive visitor!");
					}
				}, val);
		}
	}

	return 0;
}

LRESULT CDialogProperties::OnSelChanged(LPNMHDR)
{
	UpdateButtons();
	return 0;
}

void CDialogProperties::OnClearAllBnClicked(uint32_t, int32_t, CWindow)
{
	m_property_values.clear();
	m_list_ctrl.ResetContent();
	UpdateButtons();
}

void CDialogProperties::OnDelBnClicked(uint32_t, int32_t, CWindow)
{
	if (int idx = m_list_ctrl.GetCurSel(); idx >= 0)
	{
		HPROPERTY hproperty = m_list_ctrl.GetProperty(idx);
		std::wstring name = hproperty->GetName();

		m_property_values.erase(name);
		m_list_ctrl.DeleteItem(hproperty);
	}

	UpdateButtons();
}

void CDialogProperties::OnImportBnClicked(uint32_t, int32_t, CWindow)
{
	auto path_func = [this](fb2k::stringRef path)
		{
			const auto wpath = nativeW(path);
			const auto str = TextFile(wpath).read();

			try
			{
				m_property_values = config::PanelProperties::FromJson(str).values;
				UpdateUiFromData();
			}
			catch (const QwrException& e)
			{
				smp::ReportErrorWithPopup(e.what());
			}
		};

	FileDialog::open(m_hWnd, "Import from", "Property files|*.json|All files|*.*", path_func);
}

void CDialogProperties::OnExportBnClicked(uint32_t, int32_t, CWindow)
{
	auto path_func = [this](fb2k::stringRef path)
		{
			config::PanelProperties properties;
			properties.values = m_property_values;

			const auto wpath = nativeW(path);
			TextFile(wpath).write(properties.ToJson());
		};

	FileDialog::save(m_hWnd, "Save as", "Property files|*.json|All files|*.*", "json", path_func);
}

void CDialogProperties::UpdateUiFromData()
{
	Map propMap;
	m_list_ctrl.ResetContent();

	for (const auto& [name, pSerializedValue] : m_property_values)
	{
		HPROPERTY hProp = std::visit([&name = name](auto&& arg)
			{
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, bool> || std::is_same_v<T, int32_t>)
				{
					return PropCreateSimple(name.c_str(), arg);
				}
				else if constexpr (std::is_same_v<T, double>)
				{
					const std::wstring strNumber = [arg]
						{
							if (std::trunc(arg) == arg)
							{ // Most likely uint64_t
								return std::to_wstring(static_cast<uint64_t>(arg));
							}

							// std::to_string(double) has precision of float
							return fmt::format(L"{:.16g}", arg);
						}();

					return PropCreateSimple(name.c_str(), strNumber.c_str());
				}
				else if constexpr (std::is_same_v<T, std::string>)
				{
					return PropCreateSimple(name.c_str(), smp::ToWide(arg).c_str());
				}
				else
				{
					static_assert(smp::always_false_v<T>, "non-exhaustive visitor!");
				}
			}, *pSerializedValue);

		propMap.emplace(name, hProp);
	}

	for (auto& [name, hProp] : propMap)
	{
		m_list_ctrl.AddItem(hProp);
	}

	UpdateButtons();
}

void CDialogProperties::UpdateButtons()
{
	const auto count = m_list_ctrl.GetCount();

	GetDlgItem(IDC_BTN_DEL).EnableWindow(m_list_ctrl.GetCurSel() != -1);
	GetDlgItem(IDC_BTN_EXPORT).EnableWindow(count > 0);
	GetDlgItem(IDC_BTN_CLEAR).EnableWindow(count > 0);
}

void CDialogProperties::OnApplyOrOK(uint32_t, int32_t nID, CWindow)
{
	dialog_position.read_from_window(m_hWnd);

	auto& properties = m_parent.GetPanelProperties();
	properties.values = m_property_values;

	m_parent.ReloadScript();

	if (nID == IDOK)
	{
		EndDialog(nID);
	}
	else
	{
		m_property_values = properties.values;
		UpdateUiFromData();
	}
}

void CDialogProperties::OnCancel(uint32_t, int32_t nID, CWindow)
{
	EndDialog(nID);
}
