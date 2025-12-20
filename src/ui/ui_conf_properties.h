#pragma once
#include <config/panel_config.h>
#include <PropertyList/PropertyList.h>
#include <panel/js_panel_window.h>
#include <ui/impl/ui_itab.h>

namespace smp::ui
{
	class CConfigProperties : public CDialogImpl<CConfigProperties>, public CDialogResize<CConfigProperties>
	{
	public:
		enum
		{
			IDD = IDD_DIALOG_CONF_PROPERTIES
		};

		BEGIN_DLGRESIZE_MAP(CConfigProperties)
			DLGRESIZE_CONTROL(IDC_LIST_PROPERTIES, DLSZ_SIZE_X | DLSZ_SIZE_Y)
			DLGRESIZE_CONTROL(IDC_DEL, DLSZ_MOVE_Y)
			DLGRESIZE_CONTROL(IDC_CLEARALL, DLSZ_MOVE_Y)
			DLGRESIZE_CONTROL(IDC_IMPORT, DLSZ_MOVE_Y)
			DLGRESIZE_CONTROL(IDC_EXPORT, DLSZ_MOVE_Y)
			DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X | DLSZ_MOVE_Y)
			DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X | DLSZ_MOVE_Y)
			DLGRESIZE_CONTROL(IDAPPLY, DLSZ_MOVE_X | DLSZ_MOVE_Y)
		END_DLGRESIZE_MAP()

		BEGIN_MSG_MAP(CConfigProperties)
			MSG_WM_INITDIALOG(OnInitDialog)
			COMMAND_ID_HANDLER_EX(IDOK, OnCloseCmd)
			COMMAND_ID_HANDLER_EX(IDCANCEL, OnCloseCmd)
			COMMAND_ID_HANDLER_EX(IDAPPLY, OnCloseCmd)
			COMMAND_HANDLER_EX(IDC_CLEARALL, BN_CLICKED, OnClearAllBnClicked)
			COMMAND_HANDLER_EX(IDC_DEL, BN_CLICKED, OnDelBnClicked)
			COMMAND_HANDLER_EX(IDC_IMPORT, BN_CLICKED, OnImportBnClicked)
			COMMAND_HANDLER_EX(IDC_EXPORT, BN_CLICKED, OnExportBnClicked)
#pragma warning(push)
#pragma warning(disable : 26454) // Arithmetic overflow
			NOTIFY_CODE_HANDLER_EX(PIN_ITEMCHANGED, OnPinItemChanged)
			NOTIFY_CODE_HANDLER_EX(PIN_SELCHANGED, OnSelChanged)
#pragma warning(pop)
			CHAIN_MSG_MAP(CDialogResize<CConfigProperties>)
			REFLECT_NOTIFICATIONS()
		END_MSG_MAP()

		CConfigProperties(js_panel_window& parent, config::PanelProperties& properties);

	private:
		LRESULT OnInitDialog(HWND hwndFocus, LPARAM lParam);
		LRESULT OnClearAllBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl);
		LRESULT OnDelBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl);
		LRESULT OnExportBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl);
		LRESULT OnImportBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl);
		LRESULT OnPinItemChanged(LPNMHDR pnmh);
		LRESULT OnSelChanged(LPNMHDR pnmh);
		LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl);

		void UpdateUiFromData();
		void UpdateUiDelButton();

	private:
		CPropertyListCtrl m_list_ctrl;
		js_panel_window& m_parent;
		config::PanelProperties& m_properties;
	};
}
