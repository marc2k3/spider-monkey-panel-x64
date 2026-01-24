#include "PCH.hpp"
#include "file_drop_target.h"

FileDropTarget::FileDropTarget(HWND hDropWnd, HWND hNotifyWnd) : IDropTargetImpl(hDropWnd), hDropWnd_(hDropWnd), hNotifyWnd_(hNotifyWnd) {}

#pragma region static
bool FileDropTarget::IsFile(IDataObject* pDataObj)
{
	FORMATETC fmte{ CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	return SUCCEEDED(pDataObj->QueryGetData(&fmte));
}

uint32_t FileDropTarget::GetOnDropMsg()
{
	static const auto msgId = ::RegisterWindowMessageW(L"smp_file_drop");
	return msgId;
}
#pragma endregion

DWORD FileDropTarget::OnDragEnter(IDataObject* pDataObj, DWORD, POINTL, DWORD)
{
	isFile_ = IsFile(pDataObj);
	return GetEffect();
}

DWORD FileDropTarget::OnDragOver(DWORD, POINTL, DWORD)
{
	return GetEffect();
}

DWORD FileDropTarget::OnDrop(IDataObject* pDataObj, DWORD, POINTL, DWORD)
{
	isFile_ = IsFile(pDataObj);
	const auto newEffect = GetEffect();
	pDataObj->AddRef();

	if (!PostMessageW(hNotifyWnd_, GetOnDropMsg(), reinterpret_cast<WPARAM>(hDropWnd_), reinterpret_cast<LPARAM>(pDataObj)))
	{
		pDataObj->Release();
	}

	return newEffect;
}

DWORD FileDropTarget::GetEffect() const
{
	return (isFile_ ? DROPEFFECT_COPY : DROPEFFECT_NONE);
}
