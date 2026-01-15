#include "PCH.hpp"
#include "drop_target_impl.h"

namespace
{
	/// @throw QwrException
	wil::com_ptr<IDropTargetHelper> GetDropTargetHelper()
	{
		// delay helper initialization, since it's pretty expensive
		static wil::com_ptr<IDropTargetHelper> dth = []
			{
				auto tmp = wil::CoCreateInstanceNoThrow<IDropTargetHelper>(CLSID_DragDropHelper);
				
				if (!tmp)
					throw QwrException("CoCreateInstance");

				return tmp;
			}();

		return dth;
	}

	void log_dnd_error(std::string_view what)
	{
		Component::log("DnD initialization failed:\n {}", what);
	}
}

IDropTargetImpl::IDropTargetImpl(HWND hWnd) : hWnd_(hWnd) {}

IDropTargetImpl::~IDropTargetImpl()
{
	RevokeDragDrop();
}

HRESULT IDropTargetImpl::RegisterDragDrop()
{
	return ::RegisterDragDrop(hWnd_, this);
}

HRESULT IDropTargetImpl::RevokeDragDrop()
{
	return ::RevokeDragDrop(hWnd_);
}

STDMETHODIMP IDropTargetImpl::DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	RETURN_HR_IF(E_INVALIDARG, !pDataObj || !pdwEffect);

	POINT point{ pt.x, pt.y };

	try
	{
		GetDropTargetHelper()->DragEnter(hWnd_, pDataObj, &point, *pdwEffect);
	}
	catch (const QwrException& e)
	{
		log_dnd_error(e.what());
		return E_FAIL;
	}

	*pdwEffect = OnDragEnter(pDataObj, grfKeyState, pt, *pdwEffect);
	return S_OK;
}

STDMETHODIMP IDropTargetImpl::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	RETURN_HR_IF(E_INVALIDARG, !pdwEffect);

	POINT point{ pt.x, pt.y };

	try
	{
		GetDropTargetHelper()->DragOver(&point, *pdwEffect);
	}
	catch (const QwrException& e)
	{
		log_dnd_error(e.what());
		return E_FAIL;
	}

	*pdwEffect = OnDragOver(grfKeyState, pt, *pdwEffect);
	return S_OK;
}

STDMETHODIMP IDropTargetImpl::DragLeave()
{
	try
	{
		GetDropTargetHelper()->DragLeave();
	}
	catch (const QwrException& e)
	{
		log_dnd_error(e.what());
		return E_FAIL;
	}

	OnDragLeave();
	return S_OK;
}

STDMETHODIMP IDropTargetImpl::Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	RETURN_HR_IF(E_INVALIDARG, !pDataObj || !pdwEffect);

	POINT point{ pt.x, pt.y };

	try
	{
		GetDropTargetHelper()->Drop(pDataObj, &point, *pdwEffect);
	}
	catch (const QwrException& e)
	{
		log_dnd_error(e.what());
		return E_FAIL;
	}

	*pdwEffect = OnDrop(pDataObj, grfKeyState, pt, *pdwEffect);
	return S_OK;
}
