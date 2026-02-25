#include "PCH.hpp"
#include "js_container.h"
#include "js_async_task.h"

#include <Panel/js_panel_window.h>

using namespace smp;

namespace mozjs
{
	JsContainer::JsContainer(js_panel_window& parent_window)
	{
		m_parent_window = &parent_window;

		bool bRet = JsEngine::GetInstance().RegisterContainer(*this);
		m_js_status = (bRet ? JsStatus::Ready : JsStatus::EngineFailed);
	}

	JsContainer::~JsContainer()
	{
		Finalize();
		JsEngine::GetInstance().UnregisterContainer(*this);
		m_ctx = nullptr;
	}

	bool JsContainer::Initialize()
	{
		if (JsStatus::EngineFailed == m_js_status)
		{
			Fail("JS engine failed to initialize");
			return false;
		}

		if (JsStatus::Working == m_js_status)
		{
			return true;
		}

		if (m_global.initialized() || m_graphics.initialized())
		{
			m_graphics.reset();
			m_global.reset();
		}

		try
		{
			m_global.init(m_ctx, JsGlobalObject::CreateNative(m_ctx, *this));

			auto autoGlobal = wil::scope_exit([this]
				{
					m_global.reset();
				});

			JSAutoRealm ac(m_ctx, m_global);
			m_graphics.init(m_ctx, JsGdiGraphics::CreateJs(m_ctx));
			m_realm = static_cast<JsRealmInner*>(JS::GetRealmPrivate(js::GetContextRealm(m_ctx)));
			autoGlobal.release();
		}
		catch (...)
		{
			Fail(mozjs::ExceptionToText(m_ctx));
			return false;
		}

		m_native_global = static_cast<JsGlobalObject*>(GetMaybePtrFromReservedSlot(m_global, kReservedObjectSlot));
		m_native_graphics = JsGdiGraphics::ExtractNativeUnchecked(m_graphics);
		m_js_status = JsStatus::Working;
		return true;
	}

	void JsContainer::Finalize()
	{
		if (JsStatus::Ready == m_js_status)
		{
			return;
		}

		if (JsStatus::Failed != m_js_status && JsStatus::EngineFailed != m_js_status)
		{ // Don't suppress error: it should be cleared only on initialization
			m_js_status = JsStatus::Ready;
		}

		m_native_graphics = nullptr;
		m_graphics.reset();
		m_drop_action.reset();

		if (!m_global.initialized())
		{
			return;
		}

		{
			JSAutoRealm ac(m_ctx, m_global);
			JsGlobalObject::PrepareForGc(m_ctx, m_global);

			auto realm = static_cast<JsRealmInner*>(JS::GetRealmPrivate(js::GetContextRealm(m_ctx)));
			m_realm = nullptr;
			realm->MarkForDeletion();
		}

		m_native_global = nullptr;
		m_global.reset();
		JsEngine::GetInstance().GetGcEngine().TriggerGc();
	}

	void JsContainer::Fail(const std::string& errorText)
	{
		Finalize();

		if (JsStatus::EngineFailed != m_js_status)
		{
			// Don't suppress error
			m_js_status = JsStatus::Failed;
		}

		const auto errorTextPadded = [this, &errorText]
			{
				const auto text = fmt::format("Error: {} ({})", Component::name_with_version, m_parent_window->GetPanelDescription());

				if (errorText.empty())
				{
					return text;
				}
				else
				{
					return fmt::format("{}\n{}", text, errorText);
				}
			}();

		m_parent_window->Fail(errorTextPadded);
	}

	JsContainer::JsStatus JsContainer::GetStatus() const
	{
		return m_js_status;
	}

	bool JsContainer::ExecuteScript(const std::string& scriptCode)
	{
		auto selfSaver = shared_from_this();
		m_is_parsing_script = true;

		const auto autoParseState = wil::scope_exit([&]
			{
				m_is_parsing_script = false;
			});

		JSAutoRealm ac(m_ctx, m_global);

		try
		{
			JS::SourceText<mozilla::Utf8Unit> source;
			if (!source.init(m_ctx, scriptCode.c_str(), scriptCode.length(), JS::SourceOwnership::Borrowed))
			{
				throw JsException();
			}

			JS::CompileOptions opts(m_ctx);
			opts.setFileAndLine("", 1);

			OnJsActionStart();

			auto autoAction = wil::scope_exit([this]
				{
					OnJsActionEnd();
				});

			JS::RootedValue dummyRval(m_ctx);

			if (!JS::Evaluate(m_ctx, opts, source, &dummyRval))
			{
				throw JsException();
			}

			return true;
		}
		catch (...)
		{
			mozjs::ExceptionToJsError(m_ctx);
			Fail(mozjs::JsErrorToText(m_ctx));
			return false;
		}
	}

	bool JsContainer::ExecuteScriptFile(const std::filesystem::path& scriptPath)
	{
		auto selfSaver = shared_from_this();
		m_is_parsing_script = true;

		auto autoParseState = wil::scope_exit([&]
			{
				m_is_parsing_script = false;
			});

		JSAutoRealm ac(m_ctx, m_global);
		try
		{
			OnJsActionStart();

			auto autoAction = wil::scope_exit([this]
				{
					OnJsActionEnd();
				});

			m_native_global->IncludeScript(scriptPath);
			return true;
		}
		catch (...)
		{
			mozjs::ExceptionToJsError(m_ctx);
			Fail(mozjs::JsErrorToText(m_ctx));
			return false;
		}
	}

	void JsContainer::RunJobs()
	{
		JsEngine::GetInstance().MaybeRunJobs();
	}

	smp::js_panel_window& JsContainer::GetParentPanel() const
	{
		return *m_parent_window;
	}

	bool JsContainer::InvokeOnDragAction(const std::string& functionName, const POINTL& pt, uint32_t keyState, DragActionParams& actionParams)
	{
		if (!IsReadyForCallback())
		{
			return false;
		}

		auto selfSaver = shared_from_this();
		JsAutoRealmWithErrorReport autoScope(m_ctx, m_global);

		if (!CreateDropActionIfNeeded())
		{
			// reports
			return false;
		}

		m_native_drop_action->AccessDropActionParams() = actionParams;

		auto retVal = InvokeJsCallback(functionName,
			static_cast<JS::HandleObject>(m_drop_action),
			static_cast<int32_t>(pt.x),
			static_cast<int32_t>(pt.y),
			static_cast<uint32_t>(keyState));
		if (!retVal)
		{
			return false;
		}

		actionParams = m_native_drop_action->AccessDropActionParams();
		return true;
	}

	void JsContainer::InvokeOnNotify(const std::wstring& name, JS::HandleValue info)
	{
		if (!IsReadyForCallback())
		{
			return;
		}

		auto selfSaver = shared_from_this();
		JsAutoRealmWithErrorReport autoScope(m_ctx, m_global);

		// Bind object to current realm
		JS::RootedValue jsValue(m_ctx, info);
		if (!JS_WrapValue(m_ctx, &jsValue))
		{
			// reports
			return;
		}

		autoScope.DisableReport(); ///< InvokeJsCallback has it's own AutoReportException
		InvokeJsCallback("on_notify_data", name, static_cast<JS::HandleValue>(jsValue));

		if (jsValue.isObject())
		{
			// this will remove all wrappers (e.g. during callback re-entrancy)
			js::NukeCrossCompartmentWrappers(
				m_ctx,
				js::SingleCompartment{ js::GetContextCompartment(m_ctx) },
				js::GetNonCCWObjectRealm(js::UncheckedUnwrap(&jsValue.toObject())),
				js::NukeReferencesToWindow::DontNukeWindowReferences, ///< browser specific flag, irrelevant to us
				js::NukeReferencesFromTarget::NukeIncomingReferences
			);
		}
	}

	void JsContainer::InvokeOnPaint(Gdiplus::Graphics& gr)
	{
		if (!IsReadyForCallback())
		{
			return;
		}

		auto selfSaver = shared_from_this();
		m_native_graphics->SetGraphicsObject(&gr);
		InvokeJsCallback("on_paint", static_cast<JS::HandleObject>(m_graphics));

		if (m_native_graphics)
		{
			// InvokeJsCallback invokes Fail() on error, which resets pNativeGraphics_
			m_native_graphics->SetGraphicsObject(nullptr);
		}
	}

	bool JsContainer::InvokeJsAsyncTask(JsAsyncTask& jsTask)
	{
		if (!IsReadyForCallback())
		{
			return true;
		}

		auto selfSaver = shared_from_this();
		JsAutoRealmWithErrorReport autoScope(m_ctx, m_global);

		OnJsActionStart();

		auto autoAction = wil::scope_exit([this]
			{
				OnJsActionEnd();
			});

		return jsTask.InvokeJs();
	}

	void JsContainer::SetJsCtx(JSContext* ctx)
	{
		m_ctx = ctx;
	}

	bool JsContainer::IsReadyForCallback() const
	{
		return (JsStatus::Working == m_js_status) && !m_is_parsing_script;
	}

	bool JsContainer::CreateDropActionIfNeeded()
	{
		if (m_drop_action.initialized())
		{
			return true;
		}

		try
		{
			m_drop_action.init(m_ctx, JsDropSourceAction::CreateJs(m_ctx));
		}
		catch (...)
		{
			mozjs::ExceptionToJsError(m_ctx);
			return false;
		}

		m_native_drop_action = JsDropSourceAction::ExtractNativeUnchecked(m_drop_action);
		return true;
	}

	void JsContainer::OnJsActionStart()
	{
		if (m_nested_js_counter++ == 0)
		{
			JsEngine::GetInstance().OnJsActionStart(*this);
		}
	}

	void JsContainer::OnJsActionEnd()
	{
		if (--m_nested_js_counter == 0)
		{
			JsEngine::GetInstance().OnJsActionEnd(*this);
		}
	}
}
