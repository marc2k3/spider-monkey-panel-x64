// A lot of logic ripped from <js/xpconnect/src/XPCJSContext.cpp>

#include "PCH.hpp"
#include "js_monitor.h"

#include <config/advanced_config.h>
#include <panel/js_panel_window.h>
#include <panel/modal_blocking_scope.h>
#include <ui/ui_slow_script.h>

namespace
{
	constexpr auto kMonitorRate = std::chrono::seconds(1);

	auto GetLowResTime()
	{
		return std::chrono::milliseconds(GetTickCount64());
	}
}

namespace mozjs
{
	JsMonitor::JsMonitor() : m_slow_script_limit(config::advanced::slow_script_limit.get())
	{
		// JsMonitor might be created before fb2k is fully initialized
		fb2k::inMainThread([this]
			{
				m_main_window = core_api::get_main_window();
			});
	}

	void JsMonitor::Start(JSContext* ctx)
	{
		m_ctx = ctx;
		if (m_slow_script_limit != std::chrono::seconds::zero())
		{
			StartMonitorThread();
		}
	}

	void JsMonitor::Stop()
	{
		if (m_slow_script_limit != std::chrono::seconds::zero())
		{
			StopMonitorThread();
		}
		m_ctx = nullptr;
	}

	void JsMonitor::AddContainer(JsContainer& jsContainer)
	{
		m_monitored_containers.emplace(&jsContainer, &jsContainer);
	}

	void JsMonitor::RemoveContainer(JsContainer& jsContainer)
	{
		m_monitored_containers.erase(&jsContainer);
	}

	void JsMonitor::OnJsActionStart(JsContainer& jsContainer)
	{
		auto it = m_monitored_containers.find(&jsContainer);

		auto& [key, data] = *it;
		if (data.ignoreSlowScriptCheck)
		{
			return;
		}

		const auto curTime = GetLowResTime();
		data.slowScriptCheckpoint = curTime;

		{
			std::unique_lock<std::mutex> ul(m_watcher_mutex);
			m_active_containers.emplace(&jsContainer, curTime);
			m_has_action.notify_one();
		}
	}

	void JsMonitor::OnJsActionEnd(JsContainer& jsContainer)
	{
		auto it = m_monitored_containers.find(&jsContainer);
		it->second.slowScriptSecondHalf = false;

		{
			std::unique_lock<std::mutex> ul(m_watcher_mutex);
			if (const auto itActive = m_active_containers.find(&jsContainer); itActive != m_active_containers.cend())
			{
				// container might or might not be in `m_active_containers` depending on if and when it's `ignoreSlowScriptCheck` was set
				m_active_containers.erase(itActive);
			}
		}
	}

	bool JsMonitor::OnInterrupt()
	{
		if (!m_ctx || m_slow_script_limit == std::chrono::seconds::zero())
		{
			return true;
		}

		{
			std::unique_lock<std::mutex> lock(m_watcher_mutex);
			if (m_is_in_interrupt)
			{
				return true;
			}
			m_is_in_interrupt = true;
		}
		auto autoBool = wil::scope_exit([&]
			{
				std::unique_lock<std::mutex> lock(m_watcher_mutex);
				m_is_in_interrupt = false;
			});

		const auto curTime = GetLowResTime();

		{ // Action might've been blocked by modal window
			const bool isInModal = HasActivePopup();

			if (m_was_in_modal && !isInModal)
			{
				for (auto& [pContainer, containerData] : m_monitored_containers)
				{
					containerData.slowScriptCheckpoint = curTime;
				}
				{
					std::unique_lock<std::mutex> lock(m_watcher_mutex);
					for (auto& [pContainer, startTime] : m_active_containers)
					{
						startTime = curTime;
					}
				}
			}

			m_was_in_modal = isInModal;

			if (isInModal)
			{
				return true;
			}
		}

		auto containerDataToProcess = [&]
			{
				auto lock = std::unique_lock(m_watcher_mutex);
				std::vector<std::pair<JsContainer*, ContainerData*>> dataToProcess;

				for (auto& [pContainer, containerData] : m_monitored_containers)
				{
					const auto it = std::ranges::find_if(m_active_containers, [pContainer = pContainer](auto& elem)
						{
							return (elem.first == pContainer);
						});

					if (m_active_containers.cend() != it)
					{
						dataToProcess.emplace_back(pContainer, &containerData);
					}
				}
				return dataToProcess;
			}();

		for (auto [pContainer, pContainerData] : containerDataToProcess)
		{
			auto& containerData = *pContainerData;

			if (containerData.ignoreSlowScriptCheck || (curTime - containerData.slowScriptCheckpoint) < m_slow_script_limit / 2.0)
			{
				continue;
			}

			// In order to guard against time changes or laptops going to sleep, we
			// don't trigger the slow script warning until (limit/2) seconds have
			// elapsed twice.
			if (!containerData.slowScriptSecondHalf)
			{ // use current time, since we might wait on warning dialog
				containerData.slowScriptCheckpoint = GetLowResTime();
				containerData.slowScriptSecondHalf = true;
				continue;
			}

			if (JsContainer::JsStatus::EngineFailed == pContainer->GetStatus() || JsContainer::JsStatus::Failed == pContainer->GetStatus())
			{ // possible if the interrupt was requested again after the script was aborted,
				// but before the container was removed from active
				continue;
			}

			CDialogSlowScript::Data dlgData;
			{
				std::string panelName;
				HWND parentHwnd;
				switch (pContainer->GetStatus())
				{
				case JsContainer::JsStatus::Working:
				{
					auto& parentPanel = pContainer->GetParentPanel();
					panelName = parentPanel.GetPanelDescription(false);
					parentHwnd = parentPanel.GetHWND();
					break;
				}
				case JsContainer::JsStatus::Ready:
				{ // possible if script destroyed the parent panel (e.g. by switching layout)
					parentHwnd = GetActiveWindow();
					break;
				}
				default:
					parentHwnd = GetActiveWindow();
				}

				std::string scriptInfo;
				JS::AutoFilename filename;
				unsigned lineno;
				if (!JS::DescribeScriptedCaller(m_ctx, &filename, &lineno))
				{
					JS_ClearPendingException(m_ctx);
					scriptInfo = "<failed to fetch script info>";
				}
				else
				{
					if (filename.get())
					{
						if (strlen(filename.get()))
						{
							scriptInfo += filename.get();
						}
						else
						{
							scriptInfo += "<unknown file>";
						}
						scriptInfo += ": " + std::to_string(lineno);
					}
				}

				CDialogSlowScript dlg(panelName, scriptInfo, dlgData);
				// TODO: fix dialog centering (that is lack of thereof)
				(void)dlg.DoModal(parentHwnd);
			}

			containerData.ignoreSlowScriptCheck = !dlgData.askAgain;

			if (dlgData.stop)
			{ // TODO: this might stop the script different from the one in currently iterated container,
				// we should get the container corresponding to the currently active realm.
				// Example: panel_1(reported): window.NotifyOthers > panel_2(stopped): on_notify_data
				JS_ReportErrorUTF8(m_ctx, "Script aborted by user");
				return false;
			}

			containerData.slowScriptCheckpoint = GetLowResTime();
			containerData.slowScriptSecondHalf = false;
		}

		return true;
	}

	void JsMonitor::StartMonitorThread()
	{
		m_should_stop_thread = false;

		m_watcher_thread = std::thread([this]
			{
				while (!m_should_stop_thread)
				{
					// We want to avoid showing the slow script dialog if the user's laptop
					// goes to sleep in the middle of running a script. To ensure this, we
					// invoke the interrupt callback after only half the timeout has
					// elapsed. The callback simply records the fact that it was called in
					// the mSlowScriptSecondHalf flag. Then we wait another (timeout/2)
					// seconds and invoke the callback again. This time around it sees
					// mSlowScriptSecondHalf is set and so it shows the slow script
					// dialog. If the computer is put to sleep during one of the (timeout/2)
					// periods, the script still has the other (timeout/2) seconds to
					// finish.

					std::this_thread::sleep_for(kMonitorRate);
					bool hasPotentiallySlowScripts = false;
					{
						std::unique_lock<std::mutex> lock(m_watcher_mutex);

						if (m_active_containers.empty())
						{
							m_has_action.wait(lock, [&]
								{
									return (m_should_stop_thread || (!m_active_containers.empty() && !m_is_in_interrupt));
								});
						}
						else if (m_is_in_interrupt)
						{ // Can't interrupt
							continue;
						}

						if (m_should_stop_thread)
						{
							break;
						}

						hasPotentiallySlowScripts = [&]
							{
								if (HasActivePopup())
								{ // popup detected, delay monitoring
									m_was_in_modal = true;
									return false;
								}

								const auto curTime = GetLowResTime();

								const auto it = std::ranges::find_if(m_active_containers, [&curTime, &slowScriptLimit = m_slow_script_limit](auto& elem)
									{
										auto& [pContainer, startTime] = elem;
										return ((curTime - startTime) > slowScriptLimit / 2.0);
									});

								return it != m_active_containers.cend();
							}();
					}

					if (hasPotentiallySlowScripts)
					{
						JS_RequestInterruptCallback(m_ctx);
					}
				}
			});
	}

	void JsMonitor::StopMonitorThread()
	{
		{
			std::unique_lock<std::mutex> lock(m_watcher_mutex);
			m_should_stop_thread = true;
			m_has_action.notify_one();
		}

		if (m_watcher_thread.joinable())
		{
			m_watcher_thread.join();
		}
	}

	bool JsMonitor::HasActivePopup() const
	{
		if (smp::modal::IsInWhitelistedModal())
		{
			return false;
		}

		if (smp::modal::IsModalBlocked())
		{
			return true;
		}

		if (m_main_window && GetLastActivePopup(m_main_window) != m_main_window)
		{
			return true;
		}

		return false;
	}
}
