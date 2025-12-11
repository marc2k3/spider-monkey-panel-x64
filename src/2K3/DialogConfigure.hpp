#pragma once
#include "Scintilla/Scintilla.hpp"

class CDialogConfigure : public CDialogImpl<CDialogConfigure>
{
public:
	using SaveCallback = std::function<void()>;

	CDialogConfigure(std::wstring title, std::string& text, SaveCallback callback);

	BEGIN_MSG_MAP_EX(CDialogConfigure)
		CHAIN_MSG_MAP_MEMBER(m_resizer)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_NOTIFY(OnNotify)
		COMMAND_ID_HANDLER_EX(IDC_BTN_CODE, OnCode)
		COMMAND_ID_HANDLER_EX(IDC_BTN_STYLE, OnStyle)
		COMMAND_ID_HANDLER_EX(IDOK, OnApplyOrOK)
		COMMAND_ID_HANDLER_EX(IDC_BTN_APPLY, OnApplyOrOK)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	enum { IDD = IDD_DIALOG_CONFIGURE };

private:
	BOOL OnInitDialog(CWindow, LPARAM);
	LRESULT OnNotify(int32_t, LPNMHDR pnmh);
	void InitScintilla();
	void InitTitle();
	void OnApplyOrOK(uint32_t, int32_t nID, CWindow);
	void OnCancel(uint32_t, int32_t nID, CWindow);
	void OnCode(uint32_t, int32_t, CWindow);
	void OnStyle(uint32_t, int32_t, CWindow);

	CDialogResizeHelper m_resizer;
	CScintilla m_scintilla;
	SaveCallback m_callback;
	fb2k::CCoreDarkModeHooks m_hooks;
	std::wstring m_title;
	std::string& m_text;
};
