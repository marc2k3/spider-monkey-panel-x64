#include "PCH.hpp"
#include "track_drop_target.h"
#include "drag_utils.h"

#include <2K3/ProcessLocationsNotify.hpp>
#include <events/event_dispatcher.h>
#include <events/event_drag.h>
#include <interfaces/drop_source_action.h>
#include <panel/js_panel_window.h>

namespace
{
	DROPIMAGETYPE GetDropImageFromEffect(DWORD dwEffect)
	{
		if (dwEffect & DROPEFFECT_MOVE)
		{
			return DROPIMAGE_MOVE;
		}
		if (dwEffect & DROPEFFECT_COPY)
		{
			return DROPIMAGE_COPY;
		}
		if (dwEffect & DROPEFFECT_LINK)
		{
			return DROPIMAGE_LINK;
		}
		if (dwEffect & DROPEFFECT_NONE)
		{
			return DROPIMAGE_NONE;
		}
		return DROPIMAGE_INVALID;
	}

	const wchar_t* GetDropTextFromEffect(DWORD dwEffect)
	{
		if (dwEffect & DROPEFFECT_MOVE)
		{
			return L"Move";
		}
		if (dwEffect & DROPEFFECT_COPY)
		{
			return L"Copy";
		}
		if (dwEffect & DROPEFFECT_LINK)
		{
			return L"Link";
		}
		return L"";
	}
}

TrackDropTarget::TrackDropTarget(smp::js_panel_window& panel) : IDropTargetImpl(panel.GetHWND()), pPanel_(&panel) {}

DWORD TrackDropTarget::OnDragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD dwEffect)
{
	pDataObject_ = pDataObj;

	bool native{};
	const auto hr = ole_interaction::get()->check_dataobject(pDataObj, fb2kAllowedEffect_, native);
		
	if (FAILED(hr))
	{
		fb2kAllowedEffect_ = DROPEFFECT_NONE;
	}
	else if (native && (WI_IsFlagSet(dwEffect, DROPEFFECT_MOVE)))
	{
		fb2kAllowedEffect_ |= DROPEFFECT_MOVE; // Remove check_dataobject move suppression for intra fb2k interactions
	}

	ScreenToClient(hWnd_, reinterpret_cast<LPPOINT>(&pt));
	std::ignore = PutDragEvent(smp::EventId::kMouseDragEnter, grfKeyState, pt, dwEffect & fb2kAllowedEffect_);

	return DROPEFFECT_NONE;
}

DWORD TrackDropTarget::OnDragOver(DWORD grfKeyState, POINTL pt, DWORD dwEffect)
{
	ScreenToClient(hWnd_, reinterpret_cast<LPPOINT>(&pt));
	const auto lastDragParamsOpt = PutDragEvent(smp::EventId::kMouseDragOver, grfKeyState, pt, dwEffect & fb2kAllowedEffect_);

	if (!lastDragParamsOpt)
	{
		return DROPEFFECT_NONE;
	}

	const auto& lastDragParams = *lastDragParamsOpt;
	const wchar_t* dragText = (lastDragParams.text.empty() ? GetDropTextFromEffect(lastDragParams.effect) : lastDragParams.text.c_str());
	drag::SetDropText(pDataObject_.get(), GetDropImageFromEffect(lastDragParams.effect), dragText, L"");

	return lastDragParams.effect;
}

DWORD TrackDropTarget::OnDrop(IDataObject* /*pDataObj*/, DWORD grfKeyState, POINTL pt, DWORD dwEffect)
{
	const auto lastDragParamsOpt = pPanel_->GetLastDragParams();

	ScreenToClient(hWnd_, reinterpret_cast<LPPOINT>(&pt));
	std::ignore = PutDragEvent(smp::EventId::kMouseDragDrop, grfKeyState, pt, dwEffect & fb2kAllowedEffect_);

	drag::SetDropText(pDataObject_.get(), DROPIMAGE_INVALID, L"", L"");
	pDataObject_.reset();

	if (!lastDragParamsOpt || dwEffect == DROPEFFECT_NONE)
	{
		return DROPEFFECT_NONE;
	}

	return lastDragParamsOpt->effect;
}

void TrackDropTarget::OnDragLeave()
{
	std::ignore = PutDragEvent(smp::EventId::kMouseDragLeave, {}, {}, {});
	drag::SetDropText(pDataObject_.get(), DROPIMAGE_INVALID, L"", L"");
	pDataObject_.reset();
}

void TrackDropTarget::ProcessDropEvent(IDataObject* pDataObj, std::optional<DragActionParams> dragParamsOpt)
{
	if (!pDataObj || !dragParamsOpt || dragParamsOpt->effect == DROPEFFECT_NONE)
	{
		return;
	}

	const auto& dragParams = *dragParamsOpt;
	dropped_files_data_impl droppedData;
	const auto hr = ole_interaction::get()->parse_dataobject(pDataObj, droppedData);

	if SUCCEEDED(hr)
	{
		const auto g = fb2k::api::pm->playlist_get_guid(dragParams.playlistIdx);
		auto cb = fb2k::service_new<ProcessLocationsNotify>(g, dragParams.base, dragParams.toSelect);

		droppedData.to_handles_async_ex(
			playlist_incoming_item_filter_v2::op_flag_delay_ui,
			core_api::get_main_window(),
			cb
		);
	}
}

std::optional<DragActionParams> TrackDropTarget::PutDragEvent(smp::EventId eventId, DWORD grfKeyState, POINTL pt, DWORD allowedEffects)
{
	if (!pPanel_)
	{
		return std::nullopt;
	}

	static std::unordered_map<smp::EventId, smp::InternalSyncMessage> eventToMsg{
		{ smp::EventId::kMouseDragEnter, smp::InternalSyncMessage::wnd_drag_enter },
		{ smp::EventId::kMouseDragLeave, smp::InternalSyncMessage::wnd_drag_leave },
		{ smp::EventId::kMouseDragOver, smp::InternalSyncMessage::wnd_drag_over },
		{ smp::EventId::kMouseDragDrop, smp::InternalSyncMessage::wnd_drag_drop }
	};

	DragActionParams dragParams{};
	dragParams.effect = allowedEffects;
	dragParams.isInternal = pPanel_->HasInternalDrag();

	// process system stuff first (e.g. mouse capture)
	SendMessageW(pPanel_->GetHWND(), std::to_underlying(eventToMsg.at(eventId)), 0, 0);

	smp::EventDispatcher::Get().PutEvent(
		hWnd_,
		std::make_unique<smp::Event_Drag>(
			eventId,
			pt.x,
			pt.y,
			grfKeyState,
			GetHotkeyModifierFlags(),
			dragParams,
			pDataObject_.get()
		),
		smp::EventPriority::kInput);

	if (eventId == smp::EventId::kMouseDragEnter)
	{
		// don't want to catch left-overs from the last operation
		pPanel_->ResetLastDragParams();
	}

	return pPanel_->GetLastDragParams();
}
