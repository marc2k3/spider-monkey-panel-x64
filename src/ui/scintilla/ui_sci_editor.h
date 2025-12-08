// Extracted from SciTE
// Copyright 1998-2005 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.
#pragma once

#include <2K3/ScintillaImpl.hpp>
#include <panel/user_message.h>
#include <ui/scintilla/ui_sci_find_replace.h>
#include <ui/scintilla/ui_sci_goto.h>

namespace smp::config::sci
{
struct ScintillaProp;
}

namespace smp::ui::sci
{

class CScriptEditorCtrl
	: public CScintillaImpl<CScriptEditorCtrl>
	, public CScintillaFindReplaceImpl<CScriptEditorCtrl>
	, public CScintillaGotoImpl
{
public:
	CScriptEditorCtrl();

	BEGIN_MSG_MAP(CScriptEditorCtrl)
		CHAIN_MSG_MAP(CScintillaGotoImpl)
		CHAIN_MSG_MAP_ALT(CScintillaFindReplaceImpl<CScriptEditorCtrl>, 1)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(SCN_UPDATEUI, OnUpdateUI)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(SCN_CHARADDED, OnCharAdded)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(SCN_ZOOM, OnZoom)
		REFLECTED_COMMAND_CODE_HANDLER_EX(SCEN_CHANGE, OnChange)
	END_MSG_MAP()

	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnUpdateUI(LPNMHDR pnmn);
	LRESULT OnCharAdded(LPNMHDR pnmh);
	LRESULT OnZoom(LPNMHDR pnmn);
	LRESULT OnChange(UINT uNotifyCode, int nID, HWND wndCtl);

	bool ProcessKey(uint32_t vk);

	void Init();
	void ReadAPI();
	void SetJScript();
	void SetContent(const char* text, bool clear_undo_buffer = false);
	void ReloadScintillaSettings();

private:
	struct Range
	{
		intptr_t cpMin{}, cpMax{};
	};

	enum class IndentationStatus : intptr_t
	{
		isNone,        // no effect on indentation
		isBlockStart,  // indentation block begin such as "{" or VB "function"
		isBlockEnd,    // indentation end indicator such as "}" or VB "end"
		isKeyWordStart // Keywords that cause indentation
	};

	struct KeyWordComparator
	{
		bool operator()(const std::string& a, const std::string& b) const;
	};

	struct BracePosition
	{
		std::optional<intptr_t> current;
		std::optional<intptr_t> matching;
	};

	struct StyledPart
	{
		StyledPart(std::string value, int style)
			: value(std::move(value))
			, style(style)
		{
		}
		std::string value;
		int style;
	};

private:
	// Operations and Implementation
	Range GetSelection();
	intptr_t GetCaretInLine();
	std::string GetCurrentLine();
	IndentationStatus GetIndentState(intptr_t line);
	std::vector<StyledPart> GetStyledParts(intptr_t line, std::span<const int> styles, size_t maxParts);
	bool RangeIsAllWhitespace(intptr_t start, intptr_t end);
	std::optional<DWORD> GetPropertyColor(const char* key);
	void RestoreDefaultStyle();
	void TrackWidth();
	void LoadStyleFromProperties();
	void AutoMarginWidth();
	bool StartCallTip();
	void ContinueCallTip();
	void FillFunctionDefinition(intptr_t pos = -1);
	bool StartAutoComplete();
	int IndentOfBlock(intptr_t line);
	void AutomaticIndentation(char ch);
	BracePosition FindBraceMatchPos();
	std::optional<std::vector<std::string_view>> GetNearestWords(std::string_view wordPart, std::optional<char> separator = std::nullopt);
	std::optional<std::string_view> GetFullDefinitionForWord(std::string_view word);
	void SetIndentation(intptr_t line, int indent);
	std::optional<std::string> GetPropertyExpanded_Opt(const char* key);

private:
	intptr_t m_nBraceCount = 0;
	intptr_t m_nCurrentCallTip = 0;
	intptr_t m_nStartCalltipWord = 0;
	intptr_t m_nLastPosCallTip = 0;
	static constexpr intptr_t m_nStatementLookback = 10;

	std::string m_szCurrentCallTipWord;
	std::string m_szFunctionDefinition;

	std::set<std::string, KeyWordComparator> m_apis;
};

} // namespace smp::ui::sci
