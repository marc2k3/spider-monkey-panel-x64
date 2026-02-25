#include "PCH.hpp"
#include "js_engine.h"

#include "js_error_scope.h"

#include <ComUtils/com_destruction_handler.h>
#include <config/advanced_config.h>
#include <Panel/js_panel_window.h>
#include <Panel/modal_blocking_scope.h>
#include <Panel/user_message.h>
#include <JsBackend/Timeout/timer_manager_native.h>

namespace
{
	constexpr uint32_t kHeartbeatRateMs = 73;
	[[maybe_unused]] constexpr uint32_t kJobsMaxBudgetMs = 500;

	// Half the size of the actual C stack, to be safe (default stack size in VS is 1MB).
	constexpr size_t kMaxStackLimit = 1024LL * 1024 / 2;

	template <typename T, typename D>
	[[nodiscard]] std::unique_ptr<T, D> make_unique_with_dtor(T* t, D d)
	{
		return std::unique_ptr<T, D>(t, d);
	}

	void ReportException(const std::string& errorText) noexcept
	{
		const auto errorTextPadded = [&errorText]
			{
				const auto text = fmt::format("Critical JS engine error: {}", Component::name_with_version);

				if (errorText.empty())
				{
					return text;
				}
				else
				{
					return fmt::format("{}\n{}", text, errorText);
				}
			}();

		smp::ReportErrorWithPopup(errorTextPadded);
	}
}

namespace mozjs
{
	JsEngine::JsEngine()
	{
		JS_Init();
	}

	JsEngine::~JsEngine() {}

	JsEngine& JsEngine::GetInstance() noexcept
	{
		static JsEngine je;
		return je;
	}

	void JsEngine::PrepareForExit() noexcept
	{
		m_should_shutdown = true;

		if (m_registered_containers.empty())
		{
			// finalize immediately, since we don't have containers to care about
			Finalize();
		}
	}

	bool JsEngine::RegisterContainer(JsContainer& jsContainer) noexcept
	{
		if (m_registered_containers.empty() && !Initialize())
		{
			return false;
		}

		jsContainer.SetJsCtx(m_ctx);
		m_registered_containers.emplace(&jsContainer, jsContainer);
		m_monitor.AddContainer(jsContainer);

		return true;
	}

	void JsEngine::UnregisterContainer(JsContainer& jsContainer) noexcept
	{
		if (auto it = m_registered_containers.find(&jsContainer); it != m_registered_containers.end())
		{
			m_monitor.RemoveContainer(jsContainer);

			it->second.get().Finalize();
			m_registered_containers.erase(it);
		}

		if (m_registered_containers.empty())
		{
			Finalize();
		}
	}

	void JsEngine::MaybeRunJobs() noexcept
	{
		if (!m_is_initialised || m_are_jobs_in_progress)
			return;

		m_are_jobs_in_progress = true;

		auto autoJobs = wil::scope_exit([this]
			{
				m_are_jobs_in_progress = false;
			});

		{
			js::RunJobs(m_ctx);

			for (size_t i = 0; i < m_rejected_promises.length(); ++i)
			{
				const auto& rejectedPromise = m_rejected_promises[i];
				if (!rejectedPromise)
				{
					continue;
				}

				JSAutoRealm ac(m_ctx, rejectedPromise);
				AutoJsReport are(m_ctx);

				JS::RootedValue jsValue(m_ctx, JS::GetPromiseResult(rejectedPromise));
				if (!jsValue.isNullOrUndefined())
				{
					JS_SetPendingException(m_ctx, jsValue);
				}
				else
				{ // Should not reach here, mostly paranoia check
					JS_ReportErrorUTF8(m_ctx, "Unhandled promise rejection");
				}
			}
			m_rejected_promises.get().clear();
		}
	}

	void JsEngine::OnJsActionStart(JsContainer& jsContainer) noexcept
	{
		m_monitor.OnJsActionStart(jsContainer);
	}

	void JsEngine::OnJsActionEnd(JsContainer& jsContainer) noexcept
	{
		m_monitor.OnJsActionEnd(jsContainer);
	}

	JsGc& JsEngine::GetGcEngine() noexcept
	{
		return m_gc;
	}

	const JsGc& JsEngine::GetGcEngine() const noexcept
	{
		return m_gc;
	}

	JsScriptCache& JsEngine::GetScriptCache() noexcept
	{
		return *m_script_cache;
	}

	void JsEngine::OnHeartbeat() noexcept
	{
		if (!m_is_initialised || m_is_beating || m_should_stop_heartbeat_thread)
		{
			return;
		}

		m_is_beating = true;

		{
			if (!m_gc.MaybeGc())
			{ // OOM
				ReportOomError();
			}
		}

		m_is_beating = false;
	}

	bool JsEngine::Initialize() noexcept
	{
		if (m_is_initialised)
		{
			return true;
		}

		auto autoJsCtx = make_unique_with_dtor<JSContext>(nullptr, [](auto pCtx)
			{
				JS_DestroyContext(pCtx);
			});

		try
		{
			autoJsCtx.reset(JS_NewContext(JsGc::GetMaxHeap()));
			QwrException::ExpectTrue(autoJsCtx.get(), "JS_NewContext failed");

			JSContext* ctx = autoJsCtx.get();

			JS_SetNativeStackQuota(ctx, kMaxStackLimit);

			if (!JS_AddInterruptCallback(ctx, InterruptHandler))
			{
				throw JsException();
			}

			if (!js::UseInternalJobQueues(ctx))
			{
				throw JsException();
			}

			JS::SetPromiseRejectionTrackerCallback(ctx, RejectedPromiseHandler, this);

			// TODO: JS::SetWarningReporter(m_ctx)

			if (!JS::InitSelfHostedCode(ctx))
			{
				throw JsException();
			}

			m_gc.Initialize(ctx);
			m_rejected_promises.init(ctx, JS::GCVector<JSObject*, 0, js::SystemAllocPolicy>(js::SystemAllocPolicy()));
			m_script_cache = std::make_unique<JsScriptCache>();

			StartHeartbeatThread();
			m_monitor.Start(ctx);
		}
		catch (const JsException&)
		{
			ReportException(mozjs::JsErrorToText(autoJsCtx.get()));
			return false;
		}
		catch (const QwrException& e)
		{
			ReportException(e.what());
			return false;
		}

		m_ctx = autoJsCtx.release();
		m_is_initialised = true;

		return true;
	}

	void JsEngine::Finalize() noexcept
	{
		if (m_ctx)
		{
			m_monitor.Stop();
			// Stop the thread first, so that we don't get additional GC's during jsGc.Finalize
			StopHeartbeatThread();
			m_gc.Finalize();

			m_script_cache.reset();
			m_rejected_promises.reset();

			JS_DestroyContext(m_ctx);
			m_ctx = nullptr;
		}

		if (m_should_shutdown)
		{
			smp::TimerManager_Native::Get().Finalize();
			JS_ShutDown();
			smp::DeleteAllStoredObject();
		}

		m_is_initialised = false;
	}

	void JsEngine::StartHeartbeatThread() noexcept
	{
		if (!m_heartbeat_window)
		{
			m_heartbeat_window = smp::HeartbeatWindow::Create();
		}

		m_should_stop_heartbeat_thread = false;

		m_heartbeat_thread = std::thread([parent = this]
			{
				while (!parent->m_should_stop_heartbeat_thread)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(kHeartbeatRateMs));
					PostMessageW(parent->m_heartbeat_window->GetHwnd(), std::to_underlying(smp::MiscMessage::heartbeat), 0, 0);
				}
			});
	}

	void JsEngine::StopHeartbeatThread() noexcept
	{
		if (m_heartbeat_thread.joinable())
		{
			m_should_stop_heartbeat_thread = true;
			m_heartbeat_thread.join();
		}
	}

	bool JsEngine::InterruptHandler(JSContext*) noexcept
	{
		return JsEngine::GetInstance().OnInterrupt();
	}

	bool JsEngine::OnInterrupt() noexcept
	{
		return m_monitor.OnInterrupt();
	}

	void JsEngine::RejectedPromiseHandler(JSContext*, bool, JS::HandleObject promise, JS::PromiseRejectionHandlingState state, void* data) noexcept
	{
		JsEngine& self = *reinterpret_cast<JsEngine*>(data);

		if (JS::PromiseRejectionHandlingState::Handled == state)
		{
			auto& uncaughtRejections = self.m_rejected_promises;

			for (size_t i = 0; i < uncaughtRejections.length(); ++i)
			{
				if (uncaughtRejections[i] == promise)
				{
					// To avoid large amounts of memmoves, we don't shrink the vector here.
					// Instead, we filter out nullptrs when iterating over the vector later.
					uncaughtRejections[i].set(nullptr);
					break;
				}
			}
		}
		else
		{
			self.m_rejected_promises.get().append(promise);
		}
	}

	void JsEngine::ReportOomError() noexcept
	{
		for (auto& [hWnd, jsContainer] : m_registered_containers)
		{
			auto& jsContainerRef = jsContainer.get();

			if (JsContainer::JsStatus::Working != jsContainerRef.GetStatus())
			{
				continue;
			}

			jsContainerRef.Fail(fmt::format("Out of memory: {}/{} bytes", jsContainerRef.m_realm->GetCurrentHeapBytes(), JsGc::GetMaxHeap()));
		}
	}
}
