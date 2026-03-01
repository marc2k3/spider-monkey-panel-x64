#pragma once
#include "PropertyList.hpp"

#include <Panel/js_panel_window.h>

class CDialogProperties : public CDialogImpl<CDialogProperties>
{
public:
	CDialogProperties(smp::js_panel_window* parent);

	BEGIN_MSG_MAP_EX(CDialogProperties)
		CHAIN_MSG_MAP_MEMBER(m_resizer)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CONTEXTMENU(OnContextMenu)
		COMMAND_ID_HANDLER_EX(IDC_BTN_IMPORT, OnImport)
		COMMAND_ID_HANDLER_EX(IDC_BTN_EXPORT, OnExport)
		COMMAND_ID_HANDLER_EX(IDC_BTN_CLEAR, OnClear)
		COMMAND_ID_HANDLER_EX(IDOK, OnApplyOrOK)
		COMMAND_ID_HANDLER_EX(IDC_BTN_APPLY, OnApplyOrOK)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
	END_MSG_MAP()

	enum { IDD = IDD_DIALOG_PROPERTIES };

private:
	BOOL OnInitDialog(CWindow, LPARAM);
	void OnApplyOrOK(uint32_t, int32_t nID, CWindow);
	void OnCancel(uint32_t, int32_t nID, CWindow);
	void OnClear(uint32_t, int32_t, CWindow);
	void OnContextMenu(CWindow, CPoint pt);
	void OnExport(uint32_t, int32_t, CWindow);
	void OnImport(uint32_t, int32_t, CWindow);

	CButton m_btn_clear, m_btn_export;
	CDialogResizeHelper m_resizer;
	smp::js_panel_window* m_parent;
	PropertyList m_list;
	fb2k::CCoreDarkModeHooks m_hooks;
};
