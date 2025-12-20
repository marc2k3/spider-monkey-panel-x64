#include <stdafx.h>
#include "ui_conf_properties.h"

#include <2K3/FileDialog.hpp>
#include <2K3/TextFile.hpp>
#include <panel/js_panel_window.h>
#include <ui/ui_conf.h>

namespace fs = std::filesystem;

namespace smp::ui
{
	CConfigProperties::CConfigProperties(js_panel_window& parent, config::PanelProperties& properties)
		: m_parent(parent)
		, m_properties(properties) {}

	LRESULT CConfigProperties::OnInitDialog(HWND, LPARAM)
	{
		DlgResize_Init(false, false, WS_CHILD);

		// Subclassing
		m_list_ctrl.SubclassWindow(GetDlgItem(IDC_LIST_PROPERTIES));
		m_list_ctrl.ModifyStyle(0, LBS_SORT | LBS_HASSTRINGS);
		m_list_ctrl.SetExtendedListStyle(PLS_EX_SORTED | PLS_EX_XPLOOK);

		CWindow{ GetDlgItem(IDC_DEL) }.EnableWindow(m_list_ctrl.GetCurSel() != -1);

		UpdateUiFromData();

		return TRUE; // set focus to default control
	}

	LRESULT CConfigProperties::OnPinItemChanged(LPNMHDR pnmh)
	{
		auto pnpi = (LPNMPROPERTYITEM)pnmh;

		const auto hasChanged = [pnpi, &properties = m_properties]() {
			auto& propValues = properties.values;

			if (!propValues.contains(pnpi->prop->GetName()))
			{
				return false;
			}

			auto& val = *propValues.at(pnpi->prop->GetName());
			_variant_t var;

			if (!pnpi->prop->GetValue(&var))
			{
				return false;
			}

			return std::visit([&var](auto& arg)
				{
					using T = std::decay_t<decltype(arg)>;
					const auto prevArgValue = arg;
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
						arg = smp::ToU8(std::wstring_view{ var.bstrVal ? var.bstrVal : L"" });
					}
					else
					{
						static_assert(smp::always_false_v<T>, "non-exhaustive visitor!");
					}

					return (prevArgValue != arg);
					}, val);
				}();

		if (hasChanged)
		{
			m_parent.ReloadScript();
		}

		return 0;
	}

	LRESULT CConfigProperties::OnSelChanged(LPNMHDR)
	{
		UpdateUiDelButton();
		return 0;
	}

	LRESULT CConfigProperties::OnClearAllBnClicked(WORD, WORD, HWND)
	{
		m_properties.values.clear();
		m_list_ctrl.ResetContent();
		m_parent.ReloadScript();
		return 0;
	}

	LRESULT CConfigProperties::OnDelBnClicked(WORD, WORD, HWND)
	{
		if (int idx = m_list_ctrl.GetCurSel(); idx >= 0)
		{
			HPROPERTY hproperty = m_list_ctrl.GetProperty(idx);
			std::wstring name = hproperty->GetName();

			m_properties.values.erase(name);
			m_list_ctrl.DeleteItem(hproperty);
		}

		UpdateUiDelButton();
		m_parent.ReloadScript();
		return 0;
	}

	LRESULT CConfigProperties::OnImportBnClicked(WORD, WORD, HWND)
	{
		auto path_func = [this](fb2k::stringRef path)
			{
				const auto wpath = nativeW(path);
				const auto str = TextFile(wpath).read();

				try
				{
					m_properties = smp::config::PanelProperties::FromJson(str);
					UpdateUiFromData();
				}
				catch (const QwrException& e)
				{
					smp::ReportErrorWithPopup(SMP_UNDERSCORE_NAME, e.what());
				}
				catch (const pfc::exception& e)
				{
					smp::ReportErrorWithPopup(SMP_UNDERSCORE_NAME, e.what());
				}

				m_parent.ReloadScript();
			};

		FileDialog::open(m_hWnd, "Import from", "Property files|*.json|All files|*.*", path_func);
		return 0;
	}

	LRESULT CConfigProperties::OnExportBnClicked(WORD, WORD, HWND)
	{
		auto path_func = [this](fb2k::stringRef path)
			{
				const auto wpath = nativeW(path);
				TextFile(wpath).write(m_properties.ToJson());
			};

		FileDialog::save(m_hWnd, "Import from", "Property files|*.json|All files|*.*", "json", path_func);
		return 0;
	}

	void CConfigProperties::UpdateUiFromData()
	{
		m_list_ctrl.ResetContent();

		struct LowerLexCmp
		{ // lexicographical comparison but with lower cased chars
			bool operator()(const std::wstring& a, const std::wstring& b) const
			{
				return (_wcsicmp(a.c_str(), b.c_str()) < 0);
			}
		};
		std::map<std::wstring, HPROPERTY, LowerLexCmp> propMap;
		for (const auto& [name, pSerializedValue] : m_properties.values)
		{
			HPROPERTY hProp = std::visit([&name = name](auto&& arg) {
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, bool> || std::is_same_v<T, int32_t>)
				{
					return PropCreateSimple(name.c_str(), arg);
				}
				else if constexpr (std::is_same_v<T, double>)
				{
					const std::wstring strNumber = [arg] {
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
	}

	void CConfigProperties::UpdateUiDelButton()
	{
		CWindow{ GetDlgItem(IDC_DEL) }.EnableWindow(m_list_ctrl.GetCurSel() != -1);
	}

	LRESULT CConfigProperties::OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl)
	{
		if (wID != IDCANCEL)
		{
			m_parent.ReloadScript();
		}

		if (wID != IDAPPLY)
			EndDialog(wID);

		return 0;
	}
}
