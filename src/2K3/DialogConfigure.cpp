#include <stdafx.h>
#include "DialogConfigure.hpp"

namespace
{
	static const CDialogResizeHelper::Param resize_data[] =
	{
		{ IDC_SCINTILLA, 0, 0, 1, 1 },
		{ IDC_BTN_CODE, 0, 1, 0, 1 },
		{ IDC_BTN_STYLE, 0, 1, 0, 1 },
		{ IDC_BTN_SAMPLES, 0, 1, 0, 1 },
		{ IDOK, 1, 1, 1, 1 },
		{ IDCANCEL, 1, 1, 1, 1 },
		{ IDC_BTN_APPLY, 1, 1, 1, 1 },
	};

	static const CRect resize_min_max(620, 400, 0, 0);

	cfgDialogPosition dialog_position(smp::guid::dialog_position);
}

CDialogConfigure::CDialogConfigure(std::wstring title, std::string& text, SaveCallback callback)
	: m_title(title)
	, m_text(text)
	, m_callback(callback)
	, m_resizer(resize_data, resize_min_max) {}

BOOL CDialogConfigure::OnInitDialog(CWindow, LPARAM)
{
	InitTitle();
	InitScintilla();

	m_hooks.AddDialogWithControls(m_hWnd);
	dialog_position.apply_to_window(m_hWnd);
	return TRUE;
}

LRESULT CDialogConfigure::OnNotify(int32_t, LPNMHDR pnmh)
{
	const auto code = static_cast<Notification>(pnmh->code);

	switch (code)
	{
	case Notification::SavePointLeft:
		SetWindowTextW(fmt::format(L"*{}", m_title).c_str());
		break;
	case Notification::SavePointReached:
		SetWindowTextW(m_title.c_str());
		break;
	}

	SetMsgHandled(FALSE);
	return 0;
}

void CDialogConfigure::InitScintilla()
{
	const auto mode = scintilla_config.get_mode();

	m_scintilla.SubclassWindow(GetDlgItem(IDC_SCINTILLA));
	m_scintilla.Init(mode, m_text);
}

void CDialogConfigure::InitTitle()
{
	SetIcon(ui_control::get()->get_main_icon());
	SetWindowTextW(m_title.c_str());
}

void CDialogConfigure::OnApplyOrOK(uint32_t, int32_t nID, CWindow)
{
	dialog_position.read_from_window(m_hWnd);

	m_text = m_scintilla.GetCode();
	m_callback();

	if (nID == IDC_BTN_APPLY)
	{
		m_scintilla.SetSavePoint();
	}
	else if (nID == IDOK)
	{
		EndDialog(nID);
	}
}

void CDialogConfigure::OnCancel(uint32_t, int32_t nID, CWindow)
{
	if (m_scintilla.GetModify())
	{
		const auto result = popup_message_v3::get()->messageBox(
			m_hWnd,
			"Unsaved changes will be lost. Are you sure?",
			SMP_NAME,
			MB_YESNO
		);

		if (result != IDYES)
			return;
	}

	EndDialog(nID);
}

void CDialogConfigure::OnCode(uint32_t, int32_t, CWindow)
{
	CRect rect;
	auto menu = CreatePopupMenu();

	AppendMenuW(menu, MF_STRING, ID_MENU_RESET, L"Reset");
	AppendMenuW(menu, MF_SEPARATOR, 0, 0);
	AppendMenuW(menu, MF_STRING, ID_MENU_IMPORT, L"Import");
	AppendMenuW(menu, MF_STRING, ID_MENU_EXPORT, L"Export");

	GetDlgItem(IDC_BTN_CODE).GetWindowRect(&rect);
	const auto id = TrackPopupMenuEx(menu, TPM_BOTTOMALIGN | TPM_NONOTIFY | TPM_RETURNCMD, rect.left, rect.top, m_hWnd, nullptr);
	DestroyMenu(menu);

	switch (id)
	{
	case ID_MENU_RESET:
		m_scintilla.SetCode(get_resource_text(IDR_DEFAULT_SCRIPT));
		break;
	case ID_MENU_IMPORT:
		m_scintilla.Import();
		break;
	case ID_MENU_EXPORT:
		m_scintilla.Export();
		break;
	}
}

void CDialogConfigure::OnStyle(uint32_t, int32_t, CWindow)
{
	using enum ScintillaConfig::Mode;

	CRect rect;
	auto menu = CreatePopupMenu();
	uint32_t check{}, edit_flag{};

	if (m_scintilla.m_mode == JavaScriptCustom)
	{
		check = ID_MENU_STYLE_CUSTOM;
		edit_flag = MF_STRING;
	}
	else
	{
		check = ID_MENU_STYLE_AUTO;
		edit_flag = MF_GRAYED;
	}

	AppendMenuW(menu, MF_STRING, ID_MENU_STYLE_AUTO, L"Auto");
	AppendMenuW(menu, MF_STRING, ID_MENU_STYLE_CUSTOM, L"Custom");
	AppendMenuW(menu, MF_SEPARATOR, 0, 0);
	AppendMenuW(menu, edit_flag, ID_MENU_STYLE_EDIT, L"Edit...");
	CheckMenuRadioItem(menu, ID_MENU_STYLE_AUTO, ID_MENU_STYLE_CUSTOM, check, MF_BYCOMMAND);

	GetDlgItem(IDC_BTN_STYLE).GetWindowRect(&rect);
	const auto id = TrackPopupMenuEx(menu, TPM_BOTTOMALIGN | TPM_NONOTIFY | TPM_RETURNCMD, rect.left, rect.top, m_hWnd, nullptr);
	DestroyMenu(menu);

	switch (id)
	{
	case ID_MENU_STYLE_AUTO:
		m_scintilla.SetMode(JavaScriptAuto);
		break;
	case ID_MENU_STYLE_CUSTOM:
		m_scintilla.SetMode(JavaScriptCustom);
		break;
	case ID_MENU_STYLE_EDIT:
		m_scintilla.OpenStyleDialog();
		break;
	}
}
