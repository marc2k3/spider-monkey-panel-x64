#include "PCH.hpp"
#include "event_drag.h"

#include <ComUtils/com_destruction_handler.h>
#include <Panel/js_panel_window.h>

namespace smp
{
	Event_Drag::Event_Drag(EventId id, int32_t x, int32_t y, uint32_t mask, uint32_t modifiers, const DragActionParams& dragParams, IDataObject* pDataObj)
		: Event_Mouse(id, x, y, mask, modifiers)
		, dragParams_(dragParams)
		, pDataObject_(pDataObj)
		, pStorage_(GetNewStoredObject())
	{
		pDataObject_->AddRef();
		pStorage_->pUnknown = pDataObject_.get();
	}

	Event_Drag::~Event_Drag() {}

	Event_Drag* Event_Drag::AsDragEvent()
	{
		return this;
	}

	std::optional<bool> Event_Drag::JsExecute(mozjs::JsContainer& /*jsContainer*/)
	{
		return std::nullopt;
	}

	const DragActionParams& Event_Drag::GetDragParams() const
	{
		return dragParams_;
	}

	IDataObject* Event_Drag::GetStoredData() const
	{
		return pDataObject_.get();
	}

	void Event_Drag::DisposeStoredData()
	{
		if (pStorage_)
		{
			pDataObject_.reset();
			MarkStoredObjectAsToBeDeleted(pStorage_);
			pStorage_ = nullptr;
		}
	}
}
