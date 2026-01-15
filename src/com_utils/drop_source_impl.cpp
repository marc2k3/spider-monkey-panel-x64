#include "PCH.hpp"
#include "drop_source_impl.h"
#include "drag_utils.h"
#include "drag_image.h"

IDropSourceImpl::IDropSourceImpl(HWND hWnd, IDataObject* data, std::wstring_view text, Gdiplus::Bitmap* bitmap) : m_data(data)
{
	auto drag_source_helper = wil::CoCreateInstanceNoThrow<IDragSourceHelper>(CLSID_DragDropHelper);

	if (!drag_source_helper)
		return;

	m_drag_source_helper = drag_source_helper.try_query<IDragSourceHelper2>();

	if (!m_drag_source_helper)
		return;

	m_drag_source_helper->SetFlags(DSH_ALLOWDROPDESCRIPTIONTEXT);

	if (uih::create_drag_image(hWnd, text, bitmap, m_dragImage))
	{
		m_drag_source_helper->InitializeFromBitmap(&m_dragImage, data);
	}

	if (IsThemeActive() && IsAppThemed())
	{
		drag::SetDefaultImage(data);
	}
};

IDropSourceImpl::~IDropSourceImpl()
{
	if (m_dragImage.hbmpDragImage)
	{
		DeleteObject(m_dragImage.hbmpDragImage);
	}
}

STDMETHODIMP IDropSourceImpl::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{
	if (fEscapePressed || WI_IsAnyFlagSet(grfKeyState, MK_RBUTTON | MK_MBUTTON))
		return DRAGDROP_S_CANCEL;
	else if (WI_IsFlagSet(grfKeyState, MK_LBUTTON))
		return S_OK;
	else if (m_effect == DROPEFFECT_NONE)
		return DRAGDROP_S_CANCEL;
	else
		return DRAGDROP_S_DROP;
}

STDMETHODIMP IDropSourceImpl::GiveFeedback(DWORD dwEffect)
{
	HWND wnd_drag{};
	BOOL isShowingLayered{};

	if (IsThemeActive())
		drag::GetIsShowingLayered(m_data.get(), isShowingLayered);

	if (SUCCEEDED(drag::GetDragWindow(m_data.get(), wnd_drag)) && wnd_drag)
		PostMessageW(wnd_drag, DDWM_UPDATEWINDOW, NULL, NULL);

	if (isShowingLayered)
	{
		if (!m_prev_is_showing_layered)
		{
			SetCursor(LoadCursorW(nullptr, IDC_ARROW));
		}

		if (wnd_drag && dwEffect == DROPEFFECT_NONE)
		{
			PostMessageW(wnd_drag, DDWM_SETCURSOR, 1, NULL);
		}
	}

	m_prev_is_showing_layered = isShowingLayered != 0;
	m_effect = dwEffect;
	return isShowingLayered ? S_OK : DRAGDROP_S_USEDEFAULTCURSORS;
}
