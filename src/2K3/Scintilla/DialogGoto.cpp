#include "PCH.hpp"
#include "DialogGoto.hpp"

CDialogGoto::CDialogGoto(intptr_t line_number) : m_line_number(line_number) {}

BOOL CDialogGoto::OnInitDialog(CWindow, LPARAM)
{
	m_button_ok = GetDlgItem(IDOK);
	m_edit_line_number = GetDlgItem(IDC_EDIT_LINE_NUMBER);
	m_edit_line_number.SetWindowTextW(std::to_wstring(m_line_number).data());
	m_hooks.AddDialogWithControls(m_hWnd);
	return TRUE;
}

void CDialogGoto::OnCloseCmd(uint32_t, int32_t nID, CWindow)
{
	m_line_number = static_cast<intptr_t>(GetDlgItemInt(IDC_EDIT_LINE_NUMBER));
	EndDialog(nID);
}

void CDialogGoto::OnUpdate(uint32_t, int32_t, CWindow)
{
	m_button_ok.EnableWindow(m_edit_line_number.GetWindowTextLengthW() > 0);
}
