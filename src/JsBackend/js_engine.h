#pragma once
#include "heartbeat_window.h"
#include "js_gc.h"
#include "js_monitor.h"
#include "js_script_cache.h"

namespace mozjs
{
	class JsEngine final
	{
	public:
		~JsEngine();
		JsEngine(const JsEngine&) = delete;
		JsEngine& operator=(const JsEngine&) = delete;

		[[nodiscard]] static JsEngine& GetInstance() noexcept;
		void PrepareForExit() noexcept;

	public: // methods accessed by JsContainer
		[[nodiscard]] bool RegisterContainer(JsContainer& jsContainer) noexcept;
		void UnregisterContainer(JsContainer& jsContainer) noexcept;

		void MaybeRunJobs() noexcept;

		void OnJsActionStart(JsContainer& jsContainer) noexcept;
		void OnJsActionEnd(JsContainer& jsContainer) noexcept;

	public: // methods accessed by js objects
		[[nodiscard]] JsGc& GetGcEngine() noexcept;
		[[nodiscard]] const JsGc& GetGcEngine() const noexcept;
		[[nodiscard]] JsScriptCache& GetScriptCache() noexcept;

	public: // methods accessed by other internals
		void OnHeartbeat() noexcept;
		[[nodiscard]] bool OnInterrupt() noexcept;

	private:
		JsEngine();

	private:
		bool Initialize() noexcept;
		void Finalize() noexcept;

		/// @throw QwrException
		void StartHeartbeatThread() noexcept;
		void StopHeartbeatThread() noexcept;

		static bool InterruptHandler(JSContext* ctx) noexcept;

		static void RejectedPromiseHandler(
			JSContext* ctx,
			bool mutedErrors,
			JS::HandleObject promise,
			JS::PromiseRejectionHandlingState state,
			void* data) noexcept;

		void ReportOomError() noexcept;

	private:
		JSContext* m_ctx{};

		bool m_is_initialised{};
		bool m_should_shutdown{};

		std::map<void*, std::reference_wrapper<JsContainer>> m_registered_containers;

		bool m_is_beating{};
		std::unique_ptr<smp::HeartbeatWindow> m_heartbeat_window;
		std::thread m_heartbeat_thread;
		std::atomic_bool m_should_stop_heartbeat_thread{};

		JsGc m_gc;
		JsMonitor m_monitor;
		JS::PersistentRooted<JS::GCVector<JSObject*, 0, js::SystemAllocPolicy>> m_rejected_promises;
		bool m_are_jobs_in_progress = false;
		std::unique_ptr<JsScriptCache> m_script_cache;
	};
}
