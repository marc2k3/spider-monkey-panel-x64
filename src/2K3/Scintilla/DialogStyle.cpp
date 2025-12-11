#include <stdafx.h>
#include "DialogStyle.hpp"

#include <2K3/FileDialog.hpp>

CDialogStyle::CDialogStyle(CScintilla* parent) : m_parent(parent), m_list(this) {}

#pragma region IListControlOwnerDataSource
bool CDialogStyle::listIsColumnEditable(ctx_t, size_t column)
{
	return column == 1uz;
}

size_t CDialogStyle::listGetItemCount(ctx_t)
{
	return scintilla_config.m_data.size();
}

pfc::string8 CDialogStyle::listGetSubItemText(ctx_t, size_t row, size_t column)
{
	switch (column)
	{
	case 0uz:
		return scintilla_config.m_data[row].first.c_str();
	case 1uz:
		return scintilla_config.m_data[row].second.c_str();
	}

	return "";
}

void CDialogStyle::listSetEditField(ctx_t, size_t row, size_t column, const char* value)
{
	if (column == 1uz)
	{
		scintilla_config.set_data_item(row, value);
		m_parent->SetStyles();
	}
}

void CDialogStyle::listSubItemClicked(ctx_t, size_t row, size_t column)
{
	if (column == 1uz)
	{
		m_list.TableEdit_Start(row, column);
	}
}
#pragma endregion

BOOL CDialogStyle::OnInitDialog(CWindow, LPARAM)
{
	m_list.CreateInDialog(m_hWnd, IDC_LIST_STYLE);
	m_list.SetWindowLongPtrW(GWL_EXSTYLE, 0L);
	m_list.InitializeHeaderCtrl(HDS_NOSIZING);
	m_list.SetSelectionModeNone();

	const auto dpi = m_list.GetDPI().cx;
	m_list.AddColumn("Name", MulDiv(150, dpi, 96));
	m_list.AddColumnAutoWidth("Value");

	m_hooks.AddDialogWithControls(m_hWnd);
	return TRUE;
}

void CDialogStyle::OnCancel(uint32_t, int32_t nID, CWindow)
{
	EndDialog(nID);
}

void CDialogStyle::OnExportBnClicked(uint32_t, int32_t, CWindow)
{
	auto path_func = [](fb2k::stringRef path)
		{
			const auto wpath = nativeW(path);
			scintilla_config.export_to_file(wpath);
		};

	FileDialog::save(m_hWnd, "Save as", "Configuration files|*.cfg", "cfg", path_func);
}

void CDialogStyle::OnImportBnClicked(uint32_t, int32_t, CWindow)
{
	auto path_func = [this](fb2k::stringRef path)
		{
			const auto wpath = nativeW(path);
			scintilla_config.import_from_file(wpath);
			m_parent->SetStyles();
			m_list.ReloadData();
		};

	FileDialog::open(m_hWnd, "Import from", "Configuration files|*.cfg|All files|*.*", path_func);
}

void CDialogStyle::OnPresetsBnClicked(uint32_t, int32_t, CWindow)
{
	CRect rect;
	HMENU menu = CreatePopupMenu();

	AppendMenuW(menu, MF_STRING, IDR_CFG_DEFAULT, L"Default");
	AppendMenuW(menu, MF_STRING, IDR_CFG_BRIGHT, L"Bright");
	AppendMenuW(menu, MF_STRING, IDR_CFG_DARK_GRAY, L"Dark Gray");
	AppendMenuW(menu, MF_STRING, IDR_CFG_RUBY_BLUE, L"Ruby Blue");

	GetDlgItem(IDC_BTN_PRESETS).GetWindowRect(&rect);
	const auto id = TrackPopupMenuEx(menu, TPM_BOTTOMALIGN | TPM_NONOTIFY | TPM_RETURNCMD, rect.left, rect.top, m_hWnd, nullptr);
	DestroyMenu(menu);

	if (id > 0)
	{
		scintilla_config.load_preset(id);
		m_parent->SetStyles();
		m_list.ReloadData();
	}
}
