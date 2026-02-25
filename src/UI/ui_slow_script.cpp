#include "PCH.hpp"
#include "ui_slow_script.h"

CDialogSlowScript::CDialogSlowScript(const std::string& panelName, const std::string& scriptInfo, CDialogSlowScript::Data& data)
	: panelName_(panelName)
	, scriptInfo_(scriptInfo)
	, data_(data) {}

LRESULT CDialogSlowScript::OnInitDialog(HWND, LPARAM)
{
	CenterWindow();

	const auto text = [this]
		{
			std::string tmp;

			if (!panelName_.empty())
			{
				tmp += fmt::format("Panel: {}", panelName_);
			}

			if (!scriptInfo_.empty())
			{
				if (!tmp.empty())
				{
					tmp += "\n";
				}
				tmp += fmt::format("Script: {}", scriptInfo_);
			}

			if (tmp.empty())
			{
				tmp = "<Unable to fetch panel info>";
			}

			return tmp;
		}();

	uSetWindowText(GetDlgItem(IDC_SLOWSCRIPT_SCRIPT_NAME), text.c_str());
	m_hooks.AddDialogWithControls(m_hWnd);
	return FALSE; // set focus to default control
}

LRESULT CDialogSlowScript::OnContinueScript(WORD, WORD, HWND)
{
	data_.stop = false;
	EndDialog(IDOK);
	return 0;
}

LRESULT CDialogSlowScript::OnStopScript(WORD, WORD, HWND)
{
	data_.stop = true;
	data_.askAgain = true;
	EndDialog(IDOK);
	return 0;
}

LRESULT CDialogSlowScript::OnDontAskClick(WORD, WORD wID, HWND hWndCtl)
{
	data_.askAgain = uButton_GetCheck(hWndCtl, wID);
	return 0;
}

LRESULT CDialogSlowScript::OnCloseCmd(WORD, WORD wID, HWND)
{
	if (wID == IDCANCEL)
	{
		data_.stop = false;
		data_.askAgain = true;
	}

	EndDialog(wID);
	return 0;
}
