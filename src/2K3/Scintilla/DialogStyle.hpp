#pragma once
#include "Scintilla.hpp"

class CDialogStyle : public CDialogImpl<CDialogStyle>, private IListControlOwnerDataSource
{
public:
	CDialogStyle(CScintilla* parent);

	BEGIN_MSG_MAP_EX(CDialogStyle)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
		COMMAND_HANDLER_EX(IDC_BTN_IMPORT, BN_CLICKED, OnImportBnClicked)
		COMMAND_HANDLER_EX(IDC_BTN_EXPORT, BN_CLICKED, OnExportBnClicked)
		COMMAND_HANDLER_EX(IDC_BTN_PRESETS, BN_CLICKED, OnPresetsBnClicked)
	END_MSG_MAP()

	enum { IDD = IDD_DIALOG_STYLE };

private:
	BOOL OnInitDialog(CWindow, LPARAM);
	void OnCancel(uint32_t, int32_t nID, CWindow);
	void OnExportBnClicked(uint32_t, int32_t, CWindow);
	void OnImportBnClicked(uint32_t, int32_t, CWindow);
	void OnPresetsBnClicked(uint32_t, int32_t, CWindow);

	bool listIsColumnEditable(ctx_t, size_t column) final;
	size_t listGetItemCount(ctx_t) final;
	pfc::string8 listGetSubItemText(ctx_t, size_t row, size_t column) final;
	void listSetEditField(ctx_t, size_t row, size_t column, const char* value) final;
	void listSubItemClicked(ctx_t, size_t row, size_t column) final;

	CListControlOwnerData m_list;
	CScintilla* m_parent;
	fb2k::CCoreDarkModeHooks m_hooks;
};
