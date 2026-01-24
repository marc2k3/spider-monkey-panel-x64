#pragma once
#include "com_tools.h"

class IDropSourceImpl : public ImplementCOMRefCounter<IDropSource>
{
public:
	/// @throw QwrException
	IDropSourceImpl(HWND hWnd, IDataObject* pDataObj, std::wstring_view text, Gdiplus::Bitmap* pUserImage);
	virtual ~IDropSourceImpl();

	COM_QI_SIMPLE(IDropSource)

	// IDropSource
	STDMETHODIMP QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState) override;
	STDMETHODIMP GiveFeedback(DWORD dwEffect) override;

private:
	wil::com_ptr<IDataObject> m_data;
	wil::com_ptr<IDragSourceHelper2> m_drag_source_helper;
	SHDRAGIMAGE m_dragImage{};
	DWORD m_effect = DROPEFFECT_NONE;
	bool m_prev_is_showing_layered{};
};
