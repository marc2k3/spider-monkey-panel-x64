#pragma once
#include "event_mouse.h"

#include <panel/drag_action_params.h>

namespace smp
{
	class js_panel_window;
	struct StorageObject;

	class Event_Drag : public Event_Mouse
	{
	public:
		/// @remark Should be called only from the main thread
		Event_Drag(EventId id, int32_t x, int32_t y, uint32_t mask, uint32_t modifiers, const DragActionParams& dragParams, IDataObject* pDataObj);
		~Event_Drag() override;

		[[nodiscard]] Event_Drag* AsDragEvent() override;

		std::optional<bool> JsExecute(mozjs::JsContainer& jsContainer) override;

		[[nodiscard]] const DragActionParams& GetDragParams() const;
		[[nodiscard]] IDataObject* GetStoredData() const;

		/// @remark Should be called only from the main thread
		void DisposeStoredData();

	private:
		const DragActionParams dragParams_;
		wil::com_ptr<IDataObject> pDataObject_;
		StorageObject* pStorage_;
	};
}
