#pragma once
#include "event.h"

namespace smp
{
	class js_panel_window;

	class Event_JsTask : public Event_JsExecutor
	{
	public:
		Event_JsTask(EventId id, std::shared_ptr<mozjs::JsAsyncTask> pTask);

		std::optional<bool> JsExecute(mozjs::JsContainer& jsContainer) override;

	private:
		std::shared_ptr<mozjs::JsAsyncTask> pTask_;
	};
}
