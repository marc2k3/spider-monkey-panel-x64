#include <stdafx.h>
#include "ui_editor_config.h"

#include <2K3/FileDialog.hpp>
#include <ui/scintilla/sci_prop_sets.h>

namespace smp::ui
{

CDialogEditorConfig::CDialogEditorConfig() : m_list(this) {}

#pragma region IListControlOwnerDataSource
bool CDialogEditorConfig::listIsColumnEditable(ctx_t, size_t column)
{
	return column == 1;
}

size_t CDialogEditorConfig::listGetItemCount(ctx_t)
{
	return config::sci::props.m_data.size();
}

pfc::string8 CDialogEditorConfig::listGetSubItemText(ctx_t, size_t row, size_t column)
{
	switch (column)
	{
	case 0:
		return config::sci::props.m_data.at(row).key.c_str();
	case 1:
		return config::sci::props.m_data.at(row).val.c_str();
	}

	return "";
}

void CDialogEditorConfig::listSetEditField(ctx_t, size_t row, size_t column, const char* value)
{
	if (column == 1)
	{
		config::sci::props.m_data.at(row).val = value;
	}
}

void CDialogEditorConfig::listSubItemClicked(ctx_t, size_t row, size_t column)
{
	if (column == 1)
	{
		m_list.TableEdit_Start(row, column);
	}
}
#pragma endregion

BOOL CDialogEditorConfig::OnInitDialog(HWND, LPARAM)
{
	m_list.CreateInDialog(m_hWnd, IDC_LIST_EDITOR_PROP);
	m_list.SetWindowLongPtrW(GWL_EXSTYLE, 0L);
	m_list.InitializeHeaderCtrl(HDS_NOSIZING);
	m_list.SetSelectionModeNone();

	const auto dpi = m_list.GetDPI().cx;
	m_list.AddColumn("Name", MulDiv(150, dpi, 96));
	m_list.AddColumnAutoWidth("Value");
	return TRUE;
}

LRESULT CDialogEditorConfig::OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	EndDialog(wID);
	return 0;
}

void CDialogEditorConfig::OnButtonReset(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	config::sci::props.reset();
	m_list.ReloadData();
}

void CDialogEditorConfig::OnButtonExportBnClicked(WORD, WORD, HWND)
{
	auto path_func = [this](fb2k::stringRef path)
		{
			config::sci::props.export_to_file(path);
		};

	FileDialog::save(m_hWnd, "Save as", "Configuration files|*.cfg|All files|*.*", "cfg", path_func);
}

void CDialogEditorConfig::OnButtonImportBnClicked(WORD, WORD, HWND)
{
	auto path_func = [this](fb2k::stringRef path)
		{
			config::sci::props.import_from_file(path);
			m_list.ReloadData();
		};

	FileDialog::open(m_hWnd, "Import from", "Configuration files|*.cfg|All files|*.*", path_func);
}

} // namespace smp::ui
