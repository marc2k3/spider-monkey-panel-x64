#pragma once

#define DDWM_SETCURSOR    (WM_USER + 2)
#define DDWM_UPDATEWINDOW (WM_USER + 3)

namespace drag
{
	HRESULT GetIsShowingLayered(IDataObject* pDataObj, BOOL& p_out);
	HRESULT SetDefaultImage(IDataObject* pDataObj);
	HRESULT SetDropText(IDataObject* pDataObj, DROPIMAGETYPE dit, const wchar_t* msg, const wchar_t* insert);
	HRESULT GetDragWindow(IDataObject* pDataObj, HWND& p_wnd);
}
