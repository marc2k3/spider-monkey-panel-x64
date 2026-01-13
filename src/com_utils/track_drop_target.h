#pragma once
#include "com_tools.h"
#include "drop_target_impl.h"

#include <events/event.h>
#include <panel/drag_action_params.h>

class TrackDropTarget : public IDropTargetImpl
{
public:
	TrackDropTarget(smp::js_panel_window& panel);
	~TrackDropTarget() override = default;

	// IDropTargetImpl
	DWORD OnDragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD dwEffect) override;
	DWORD OnDragOver(DWORD grfKeyState, POINTL pt, DWORD dwEffect) override;
	DWORD OnDrop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD dwEffect) override;
	void OnDragLeave() override;

	static void ProcessDropEvent(IDataObject* pDataObj, std::optional<DragActionParams> dragParamsOpt);

private:
	[[nodiscard]] std::optional<DragActionParams>
	PutDragEvent(smp::EventId eventId, DWORD grfKeyState, POINTL pt, DWORD allowedEffects);

private:
	smp::js_panel_window* pPanel_{};
	wil::com_ptr<IDataObject> pDataObject_;
	DWORD fb2kAllowedEffect_ = DROPEFFECT_NONE;
};
