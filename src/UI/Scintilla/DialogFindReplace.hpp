#pragma once
#include "Scintilla.hpp"

class CDialogFindReplace : public CDialogImpl<CDialogFindReplace>
{
public:
	enum class Mode
	{
		Find,
		Replace
	};

	CDialogFindReplace(CScintilla* parent);

	BEGIN_MSG_MAP_EX(CDialogFindReplace)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDC_BTN_NEXT, OnFindNext)
		COMMAND_ID_HANDLER_EX(IDC_BTN_PREVIOUS, OnFindPrevious)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER_EX(IDC_BTN_REPLACE, OnReplace)
		COMMAND_ID_HANDLER_EX(IDC_BTN_REPLACE_ALL, OnReplaceAll)
		COMMAND_HANDLER_EX(IDC_EDIT_FIND, EN_CHANGE, OnFindTextChange)
		COMMAND_HANDLER_EX(IDC_EDIT_REPLACE, EN_CHANGE, OnReplaceTextChange)
		COMMAND_RANGE_HANDLER_EX(IDC_CHECK_MATCHCASE, IDC_CHECK_REGEXP, OnCheckBoxChange)
	END_MSG_MAP()

	enum { IDD = IDD_DIALOG_FIND_REPLACE };

	void Update(Mode mode, std::string_view selected_text);

	FindOption m_flags{};
	std::string m_find_text, m_replace_text;

private:
	BOOL OnInitDialog(CWindow, LPARAM);
	void OnCancel(uint32_t, int32_t, CWindow);
	void OnCheckBoxChange(uint32_t, int32_t nID, CWindow);
	void OnFindNext(uint32_t, int32_t, CWindow);
	void OnFindPrevious(uint32_t, int32_t, CWindow);
	void OnFindTextChange(uint32_t, int32_t, CWindow);
	void OnReplace(uint32_t, int32_t, CWindow);
	void OnReplaceAll(uint32_t, int32_t, CWindow);
	void OnReplaceTextChange(uint32_t, int32_t, CWindow);

	CScintilla* m_parent;
	bool m_havefound{};
	fb2k::CCoreDarkModeHooks m_hooks;
	std::unordered_map<int32_t, CCheckBox> m_check_box_map;
	std::unordered_map<int32_t, CWindow> m_window_map;
};
