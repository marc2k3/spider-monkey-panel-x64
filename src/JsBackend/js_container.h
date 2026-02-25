#pragma once
#include <interfaces/drop_source_action.h>
#include <interfaces/gdi_graphics.h>

#include "js_realm_inner.h"
#include "native_to_js_invoker.h"

struct DragActionParams;

namespace mozjs
{
	class JsAsyncTask;

	// Must not leak exceptions!
	class JsContainer final : public std::enable_shared_from_this<JsContainer>
	{
		friend class JsEngine;

	public:
		JsContainer(smp::js_panel_window& parent_window);
		JsContainer(const JsContainer&) = delete;
		~JsContainer();

	public:
		enum class JsStatus
		{
			EngineFailed,
			Failed,
			Ready,
			Working
		};

	public:
		[[nodiscard]] bool Initialize();
		void Finalize();

		void Fail(const std::string& errorText);

		[[nodiscard]] JsStatus GetStatus() const;

		[[nodiscard]] bool ExecuteScript(const std::string& scriptCode);
		[[nodiscard]] bool ExecuteScriptFile(const std::filesystem::path& scriptPath);

		static void RunJobs();

	public:
		smp::js_panel_window& GetParentPanel() const;

	public:
		template <typename ReturnType = std::nullptr_t, typename... ArgTypes>
		std::optional<ReturnType> InvokeJsCallback(std::string functionName, ArgTypes&&... args)
		{
			if (!IsReadyForCallback())
			{
				return std::nullopt;
			}

			auto selfSaver = shared_from_this();

			OnJsActionStart();
			auto autoAction = wil::scope_exit([&]
				{
					OnJsActionEnd();
				});

			return mozjs::InvokeJsCallback<ReturnType>(m_ctx, m_global, functionName, std::forward<ArgTypes>(args)...);
		}

		[[nodiscard]] bool InvokeOnDragAction(const std::string& functionName, const POINTL& pt, uint32_t keyState, DragActionParams& actionParams);
		void InvokeOnNotify(const std::wstring& name, JS::HandleValue info);
		void InvokeOnPaint(Gdiplus::Graphics& gr);
		bool InvokeJsAsyncTask(JsAsyncTask& jsTask);

	private:
		void SetJsCtx(JSContext* ctx);

		[[nodiscard]] bool IsReadyForCallback() const;

		/// @return true on success, false with JS report on failure
		[[nodiscard]] bool CreateDropActionIfNeeded();

		void OnJsActionStart();
		void OnJsActionEnd();

	private:
		JSContext* m_ctx{};
		smp::js_panel_window* m_parent_window{};

		JS::PersistentRootedObject m_global;
		JS::PersistentRootedObject m_graphics;
		JS::PersistentRootedObject m_drop_action;

		JsRealmInner* m_realm;
		JsGlobalObject* m_native_global{};
		JsGdiGraphics* m_native_graphics{};
		JsDropSourceAction* m_native_drop_action{};

		JsStatus m_js_status = JsStatus::EngineFailed;
		bool m_is_parsing_script{};
		uint32_t m_nested_js_counter{};
	};
}
