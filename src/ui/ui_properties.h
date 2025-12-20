#pragma once
#include <config/panel_config.h>
#include <PropertyList/PropertyList.h>
#include <panel/js_panel_window.h>

namespace smp::ui
{
	class CDialogProperties : public CDialogImpl<CDialogProperties>
	{
	public:
		enum
		{
			IDD = IDD_DIALOG_PROPERTIES
		};

		BEGIN_MSG_MAP(CDialogProperties)
			CHAIN_MSG_MAP_MEMBER(m_resizer)
			MSG_WM_INITDIALOG(OnInitDialog)
			COMMAND_ID_HANDLER_EX(IDOK, OnApplyOrOK)
			COMMAND_ID_HANDLER_EX(IDC_BTN_APPLY, OnApplyOrOK)
			COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
			COMMAND_ID_HANDLER_EX(IDC_BTN_CLEAR, OnClearAllBnClicked)
			COMMAND_ID_HANDLER_EX(IDC_BTN_DEL, OnDelBnClicked)
			COMMAND_ID_HANDLER_EX(IDC_BTN_IMPORT, OnImportBnClicked)
			COMMAND_ID_HANDLER_EX(IDC_BTN_EXPORT, OnExportBnClicked)
#pragma warning(push)
#pragma warning(disable : 26454) // Arithmetic overflow
			NOTIFY_CODE_HANDLER_EX(PIN_ITEMCHANGED, OnPinItemChanged)
			NOTIFY_CODE_HANDLER_EX(PIN_SELCHANGED, OnSelChanged)
#pragma warning(pop)
			REFLECT_NOTIFICATIONS()
		END_MSG_MAP()

		CDialogProperties(js_panel_window& parent, config::PanelProperties& properties);

	private:
		LRESULT OnInitDialog(HWND hwndFocus, LPARAM lParam);
		LRESULT OnPinItemChanged(LPNMHDR pnmh);
		LRESULT OnSelChanged(LPNMHDR pnmh);
		void OnClearAllBnClicked(uint32_t, int32_t, CWindow);
		void OnDelBnClicked(uint32_t, int32_t, CWindow);
		void OnExportBnClicked(uint32_t, int32_t, CWindow);
		void OnImportBnClicked(uint32_t, int32_t, CWindow);
		void OnApplyOrOK(uint32_t, int32_t nID, CWindow);
		void OnCancel(uint32_t, int32_t nID, CWindow);

		void UpdateUiFromData();
		void UpdateUiDelButton();

	private:
		CDialogResizeHelper m_resizer;
		CPropertyListCtrl m_list_ctrl;
		js_panel_window& m_parent;
		config::PanelProperties& m_properties;
	};
}
