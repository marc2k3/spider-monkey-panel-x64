#pragma once

class IDropTargetImpl : public ImplementCOMRefCounter<IDropTarget>
{
public:
	IDropTargetImpl(HWND hWnd);
	virtual ~IDropTargetImpl();

	COM_QI_SIMPLE(IDropTarget)

	HRESULT RegisterDragDrop();
	HRESULT RevokeDragDrop();

	// IDropTarget
	STDMETHODIMP DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;
	STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;
	STDMETHODIMP DragLeave() override;
	STDMETHODIMP Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;

	// Overrides
	virtual DWORD OnDragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD dwEffect) = 0;
	virtual DWORD OnDragOver(DWORD grfKeyState, POINTL pt, DWORD dwEffect) = 0;
	virtual DWORD OnDrop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD dwEffect) = 0;
	virtual void OnDragLeave() = 0;

protected:
	HWND hWnd_;
};
