#include "PCH.hpp"
#include "DialogFindReplace.hpp"

#include "KeyHook.hpp"

namespace
{
	FindOption operator ~(FindOption rhs)
	{
		return static_cast<FindOption>(~std::to_underlying(rhs));
	}

	FindOption& operator &=(FindOption& lhs, FindOption rhs)
	{
		lhs = static_cast<FindOption>(std::to_underlying(lhs) & std::to_underlying(rhs));
		return lhs;
	}

	static const std::unordered_map<int32_t, FindOption> id_option_map =
	{
		{ IDC_CHECK_MATCHCASE, FindOption::MatchCase },
		{ IDC_CHECK_WHOLEWORD, FindOption::WholeWord },
		{ IDC_CHECK_REGEXP, FindOption::RegExp | FindOption::Cxx11RegEx }
	};

	static constexpr std::array ids =
	{
		IDC_EDIT_FIND,
		IDC_EDIT_REPLACE,
		IDC_BTN_NEXT,
		IDC_BTN_PREVIOUS,
		IDC_BTN_REPLACE,
		IDC_BTN_REPLACE_ALL,
		IDC_LABEL_REPLACE,
		IDCANCEL
	};
}

CDialogFindReplace::CDialogFindReplace(CScintilla* parent) : m_parent(parent) {}

BOOL CDialogFindReplace::OnInitDialog(CWindow, LPARAM)
{
	for (const auto id : id_option_map | std::views::keys)
	{
		auto wnd = CCheckBox(GetDlgItem(id));
		PP::subclassThisWindow<KeyHook>(wnd, IDC_BTN_NEXT);
		m_check_box_map.emplace(id, wnd);
	}

	for (const auto id : ids)
	{
		auto wnd = GetDlgItem(id);
		auto cmd = id == IDC_EDIT_FIND || id == IDC_EDIT_REPLACE ? IDC_BTN_NEXT : id;
		PP::subclassThisWindow<KeyHook>(wnd, cmd);
		m_window_map.emplace(id, wnd);
	}

	m_hooks.AddDialogWithControls(m_hWnd);
	return TRUE;
}

void CDialogFindReplace::OnCancel(uint32_t, int32_t, CWindow)
{
	ShowWindow(SW_HIDE);
}

void CDialogFindReplace::OnCheckBoxChange(uint32_t, int32_t nID, CWindow)
{
	const auto option = id_option_map.at(nID);

	if (m_check_box_map.at(nID).IsChecked())
		m_flags |= option;
	else
		m_flags &= ~option;

	const auto regexp = FlagSet(m_flags, FindOption::RegExp);
	m_check_box_map.at(IDC_CHECK_WHOLEWORD).EnableWindow(!regexp);
	m_window_map.at(IDC_BTN_PREVIOUS).EnableWindow(m_find_text.length() > 0uz && !regexp);
}

void CDialogFindReplace::OnFindNext(uint32_t, int32_t, CWindow)
{
	m_havefound = m_parent->Find(true);
}

void CDialogFindReplace::OnFindPrevious(uint32_t, int32_t, CWindow)
{
	m_parent->Find(false);
}

void CDialogFindReplace::OnFindTextChange(uint32_t, int32_t, CWindow)
{
	m_find_text = pfc::getWindowText(m_window_map.at(IDC_EDIT_FIND));
	const auto enabled = m_find_text.length() > 0uz;
	m_window_map.at(IDC_BTN_NEXT).EnableWindow(enabled);
	m_window_map.at(IDC_BTN_PREVIOUS).EnableWindow(enabled && !FlagSet(m_flags, FindOption::RegExp));
	m_window_map.at(IDC_BTN_REPLACE).EnableWindow(enabled);
	m_window_map.at(IDC_BTN_REPLACE_ALL).EnableWindow(enabled);
}

void CDialogFindReplace::OnReplace(uint32_t, int32_t, CWindow)
{
	if (m_havefound)
	{
		m_parent->Replace();
		m_havefound = false;
	}

	OnFindNext(0u, 0u, nullptr);
}

void CDialogFindReplace::OnReplaceAll(uint32_t, int32_t, CWindow)
{
	m_parent->ReplaceAll();
}

void CDialogFindReplace::OnReplaceTextChange(uint32_t, int32_t, CWindow)
{
	m_replace_text = pfc::getWindowText(m_window_map.at(IDC_EDIT_REPLACE));
}

void CDialogFindReplace::Update(Mode mode, std::string_view selected_text)
{
	const auto find = mode == Mode::Find;
	const auto sw = find ? SW_HIDE : SW_SHOW;
	m_window_map.at(IDC_LABEL_REPLACE).ShowWindow(sw);
	m_window_map.at(IDC_EDIT_REPLACE).ShowWindow(sw);
	m_window_map.at(IDC_BTN_REPLACE).ShowWindow(sw);
	m_window_map.at(IDC_BTN_REPLACE_ALL).ShowWindow(sw);

	if (selected_text.length())
	{
		m_find_text = selected_text;
		pfc::setWindowText(m_window_map.at(IDC_EDIT_FIND), selected_text.data());
	}

	pfc::setWindowText(m_hWnd, find ? "Find" : "Replace");
	ShowWindow(SW_SHOW);
	SetFocus();
}
