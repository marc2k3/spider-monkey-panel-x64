#pragma once

class CDialogGoto : public CDialogImpl<CDialogGoto>
{
public:
	CDialogGoto(intptr_t line_number);

	BEGIN_MSG_MAP_EX(CDialogGoto)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_RANGE_HANDLER_EX(IDOK, IDCANCEL, OnCloseCmd)
		COMMAND_CODE_HANDLER_EX(EN_UPDATE, OnUpdate)
	END_MSG_MAP()

	enum { IDD = IDD_DIALOG_GOTO };

	intptr_t m_line_number{};

private:
	BOOL OnInitDialog(CWindow, LPARAM);
	void OnCloseCmd(uint32_t, int32_t nID, CWindow);
	void OnUpdate(uint32_t, int32_t, CWindow);

	CButton m_button_ok;
	CEdit m_edit_line_number;
	fb2k::CCoreDarkModeHooks m_hooks;
};
