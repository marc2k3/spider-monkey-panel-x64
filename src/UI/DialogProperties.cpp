#include "PCH.hpp"
#include "DialogProperties.hpp"

namespace
{
	static const CDialogResizeHelper::Param resize_data[] =
	{
		{ IDC_LIST_PROPERTIES, 0, 0, 1, 1 },
		{ IDC_BTN_IMPORT, 0, 1, 0, 1 },
		{ IDC_BTN_EXPORT, 0, 1, 0, 1 },
		{ IDC_BTN_CLEAR, 0, 1, 0, 1 },
		{ IDOK, 1, 1, 1, 1 },
		{ IDCANCEL, 1, 1, 1, 1 },
		{ IDC_BTN_APPLY, 1, 1, 1, 1 },
	};

	static const CRect resize_min_max(620, 400, 0, 0);
}

CDialogProperties::CDialogProperties(smp::js_panel_window* parent)
	: m_parent(parent)
	, m_resizer(resize_data, resize_min_max) {}

BOOL CDialogProperties::OnInitDialog(CWindow, LPARAM)
{
	SetIcon(ui_control::get()->get_main_icon());

	m_list.CreateInDialog(m_hWnd, IDC_LIST_PROPERTIES);
	m_list.SetWindowLongPtrW(GWL_EXSTYLE, 0L);
	m_list.SetProperties(m_parent->GetPanelProperties());

	m_btn_clear = GetDlgItem(IDC_BTN_CLEAR);
	m_btn_export = GetDlgItem(IDC_BTN_EXPORT);

	m_hooks.AddDialogWithControls(m_hWnd);
	Component::dialog_position.apply_to_window(m_hWnd);
	return TRUE;
}

void CDialogProperties::OnApplyOrOK(uint32_t, int32_t nID, CWindow)
{
	Component::dialog_position.read_from_window(m_hWnd);

	auto& properties = m_parent->GetPanelProperties();
	properties.values = m_list.GetProperties().values;
	m_parent->ReloadScript();
	
	if (nID == IDC_BTN_APPLY)
	{
		m_list.SetProperties(properties);
	}
	else if (nID == IDOK)
	{
		EndDialog(nID);
	}
}

void CDialogProperties::OnCancel(uint32_t, int32_t nID, CWindow)
{
	EndDialog(nID);
}

void CDialogProperties::OnClear(uint32_t, int32_t, CWindow)
{
	m_list.SelectAll();
	m_list.RequestRemoveSelection();
}

void CDialogProperties::OnContextMenu(CWindow, CPoint pt)
{
	m_list.OnContextMenu(pt);
}

void CDialogProperties::OnExport(uint32_t, int32_t, CWindow)
{
	auto path_func = [this](fb2k::stringRef path)
		{
			auto properties = m_list.GetProperties();
			auto json = properties.ToJson();
			auto wpath = nativeW(path);
			TextFile(wpath).write(json);
		};

	FileDialog::save(m_hWnd, "Save as", "Property files|*.json", "json", path_func);
}

void CDialogProperties::OnImport(uint32_t, int32_t, CWindow)
{
	auto path_func = [this](fb2k::stringRef path)
		{
			auto wpath = nativeW(path);
			auto json = TextFile(wpath).read();

			try
			{
				auto properties = config::PanelProperties::FromJson(json);
				m_list.SetProperties(properties);
			}
			catch (const std::exception& e)
			{
				smp::ReportErrorWithPopup(e.what());
			}
		};

	FileDialog::open(m_hWnd, "Import from", "Property files|*.json|All files|*.*", path_func);
}
