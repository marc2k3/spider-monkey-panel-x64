#pragma once

namespace mozjs
{
	class JsMonitor final
	{
	public:
		JsMonitor();
		~JsMonitor() = default;
		JsMonitor(const JsMonitor&) = delete;
		JsMonitor& operator=(const JsMonitor&) = delete;

		/// @detail Assumes that JSContext is freshly created
		///
		/// @throw QwrException
		void Start(JSContext* ctx);
		void Stop();

		void AddContainer(JsContainer& jsContainer);
		void RemoveContainer(JsContainer& jsContainer);

		void OnJsActionStart(JsContainer& jsContainer);
		void OnJsActionEnd(JsContainer& jsContainer);

		[[nodiscard]] bool OnInterrupt();

	private:
		/// @throw QwrException
		void StartMonitorThread();
		void StopMonitorThread();

		[[nodiscard]] bool HasActivePopup() const;

	private:
		struct ContainerData
		{
			ContainerData(JsContainer* pContainer) : pContainer(pContainer) {}

			JsContainer* pContainer{};
			bool ignoreSlowScriptCheck{};
			std::chrono::milliseconds slowScriptCheckpoint{};
			bool slowScriptSecondHalf{};
		};

		JSContext* m_ctx{};
		HWND m_main_window{};
		const std::chrono::seconds m_slow_script_limit;
		std::unordered_map<JsContainer*, ContainerData> m_monitored_containers;

		std::mutex m_watcher_mutex;
		std::thread m_watcher_thread;
		std::atomic_bool m_should_stop_thread{};
		std::condition_variable m_has_action;
		// Contains the same time as slowScriptCheckpoint in monitoredContainers_
		std::unordered_map<JsContainer*, std::chrono::milliseconds> m_active_containers;
		bool m_is_in_interrupt{};

		std::atomic_bool m_was_in_modal = false;
	};
}
