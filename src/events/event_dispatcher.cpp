#include "PCH.hpp"
#include "event_dispatcher.h"
#include "task_controller.h"

#include <panel/user_message.h>

namespace smp
{
	EventDispatcher& EventDispatcher::Get() noexcept
	{
		static EventDispatcher em;
		return em;
	}

	void EventDispatcher::AddWindow(HWND hWnd, std::shared_ptr<PanelTarget> pTarget) noexcept
	{
		std::unique_lock ul(taskControllerMapMutex_);
		taskControllerMap_.try_emplace(hWnd, std::make_shared<TaskController>(pTarget));
		nextEventMsgStatusMap_.try_emplace(hWnd, true);
	}

	void EventDispatcher::RemoveWindow(HWND hWnd) noexcept
	{
		std::unique_lock ul(taskControllerMapMutex_);
		taskControllerMap_.erase(hWnd);
		nextEventMsgStatusMap_.erase(hWnd);
	}

	void EventDispatcher::NotifyAllAboutExit() noexcept
	{
		std::vector<HWND> hWnds;
		hWnds.reserve(taskControllerMap_.size());

		{
			std::scoped_lock sl(taskControllerMapMutex_);
			for (auto& [hLocalWnd, pTaskController] : taskControllerMap_)
			{
				hWnds.emplace_back(hLocalWnd);
			}
		}

		for (const auto& hWnd : hWnds)
		{
			SendMessageW(hWnd, std::to_underlying(InternalSyncMessage::prepare_for_exit), 0, 0);
		}
	}

	bool EventDispatcher::IsRequestEventMessage(UINT msg) noexcept
	{
		return msg == std::to_underlying(InternalSyncMessage::run_next_event);
	}

	bool EventDispatcher::ProcessNextEvent(HWND hWnd) noexcept
	{
		auto it = [&]
			{
				std::unique_lock ul(taskControllerMapMutex_);
				return taskControllerMap_.find(hWnd);
			}();

		if (it == taskControllerMap_.end() || !it->second)
		{
			return false;
		}
		return it->second->ExecuteNextTask();
	}

	void EventDispatcher::RequestNextEvent(HWND hWnd) noexcept
	{
		std::scoped_lock sl(taskControllerMapMutex_);

		auto taskControllerIt = taskControllerMap_.find(hWnd);
		if (taskControllerIt == taskControllerMap_.end() || !taskControllerIt->second)
		{ // task controller might be missing when invoked before window initialization
			return;
		}

		RequestNextEventImpl(hWnd, *taskControllerIt->second);
	}

	void EventDispatcher::RequestNextEventImpl(HWND hWnd, TaskController& taskController) noexcept
	{
		if (!taskController.HasTasks())
		{
			return;
		}

		const auto isWaitingForMsgIt = nextEventMsgStatusMap_.find(hWnd);
		if (isWaitingForMsgIt == nextEventMsgStatusMap_.end())
		{
			return;
		}

		if (isWaitingForMsgIt->second)
		{
			isWaitingForMsgIt->second = false;
			PostMessageW(hWnd, std::to_underlying(InternalSyncMessage::run_next_event), 0, 0);
		}
		else
		{
			isWaitingForMsgIt->second = false;
		}
	}

	void EventDispatcher::OnRequestEventMessageReceived(HWND hWnd) noexcept
	{
		std::scoped_lock sl(taskControllerMapMutex_);

		auto isWaitingForMsgIt = nextEventMsgStatusMap_.find(hWnd);
		if (isWaitingForMsgIt == nextEventMsgStatusMap_.end())
		{
			return;
		}

		isWaitingForMsgIt->second = true;
	}

	void EventDispatcher::PutRunnable(HWND hWnd, std::shared_ptr<Runnable> pRunnable, EventPriority priority) noexcept
	{
		std::scoped_lock sl(taskControllerMapMutex_);

		auto taskControllerIt = taskControllerMap_.find(hWnd);
		if (taskControllerIt == taskControllerMap_.end() || !taskControllerIt->second)
		{
			return;
		}

		auto pTaskController = taskControllerIt->second;
		pTaskController->AddRunnable(std::move(pRunnable), priority);

		RequestNextEventImpl(hWnd, *pTaskController);
	}

	void EventDispatcher::PutEvent(HWND hWnd, std::unique_ptr<EventBase> pEvent, EventPriority priority) noexcept
	{
		std::scoped_lock sl(taskControllerMapMutex_);

		auto taskControllerIt = taskControllerMap_.find(hWnd);
		if (taskControllerIt == taskControllerMap_.end() || !taskControllerIt->second)
		{
			return;
		}

		auto pTaskController = taskControllerIt->second;
		pEvent->SetTarget(pTaskController->GetTarget());
		pTaskController->AddRunnable(std::move(pEvent), priority);

		RequestNextEventImpl(hWnd, *pTaskController);
	}

	void EventDispatcher::PutEventToAll(std::unique_ptr<EventBase> pEvent, EventPriority priority) noexcept
	{
		std::scoped_lock sl(taskControllerMapMutex_);

		for (auto& [hLocalWnd, pTaskController] : taskControllerMap_)
		{
			if (!pTaskController)
			{
				continue;
			}

			auto pClonedEvent = pEvent->Clone();
			if (!pClonedEvent)
			{
				return;
			}

			pClonedEvent->SetTarget(pTaskController->GetTarget());
			pTaskController->AddRunnable(std::move(pClonedEvent), priority);

			RequestNextEventImpl(hLocalWnd, *pTaskController);
		}
	}

	void EventDispatcher::PutEventToOthers(HWND hWnd, std::unique_ptr<EventBase> pEvent, EventPriority priority) noexcept
	{
		std::scoped_lock sl(taskControllerMapMutex_);

		for (auto& [hLocalWnd, pTaskController] : taskControllerMap_)
		{
			if (!pTaskController || hLocalWnd == hWnd)
			{
				continue;
			}

			auto pClonedEvent = pEvent->Clone();
			if (!pClonedEvent)
			{
				return;
			}

			pClonedEvent->SetTarget(pTaskController->GetTarget());
			pTaskController->AddRunnable(std::move(pClonedEvent), priority);

			RequestNextEventImpl(hLocalWnd, *pTaskController);
		}
	}

	void EventDispatcher::NotifyOthers(HWND hWnd, std::unique_ptr<EventBase> pEvent) noexcept
	{
		std::vector<std::pair<HWND, std::unique_ptr<EventBase>>> hWndToEvent;
		hWndToEvent.reserve(taskControllerMap_.size());

		{
			std::scoped_lock sl(taskControllerMapMutex_);
			for (auto& [hLocalWnd, pTaskController] : taskControllerMap_)
			{
				if (!pTaskController || hLocalWnd == hWnd)
				{
					continue;
				}

				auto pClonedEvent = pEvent->Clone();
				if (!pClonedEvent)
				{
					return;
				}

				pClonedEvent->SetTarget(pTaskController->GetTarget());

				hWndToEvent.emplace_back(hLocalWnd, std::move(pClonedEvent));
			}
		}

		for (const auto& [wnd, pClonedEvent] : hWndToEvent)
		{
			SendMessageW(wnd, std::to_underlying(InternalSyncMessage::legacy_notify_others), 0, reinterpret_cast<LPARAM>(pClonedEvent.get()));
		}
	}
}
