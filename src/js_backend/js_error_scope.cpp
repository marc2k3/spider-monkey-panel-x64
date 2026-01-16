#include "PCH.hpp"
#include "js_error_scope.h"
#include "js_error_helper.h"

namespace mozjs
{
	AutoJsReport::AutoJsReport(JSContext* ctx) : m_ctx(ctx) {}

	AutoJsReport::~AutoJsReport() noexcept
	{
		if (m_is_disabled)
		{
			return;
		}

		if (!JS_IsExceptionPending(m_ctx))
		{
			return;
		}

		try
		{
			const auto errorText = JsErrorToText(m_ctx);
			JS_ClearPendingException(m_ctx);
			JS::RootedObject global(m_ctx, JS::CurrentGlobalOrNull(m_ctx));

			if (global)
			{
				auto globalCtx = static_cast<JsGlobalObject*>(GetMaybePtrFromReservedSlot(global, kReservedObjectSlot));

				if (globalCtx)
				{
					globalCtx->Fail(errorText);
					JS_ClearPendingException(m_ctx);
				}
			}

		}
		catch (...) {}
	}

	void AutoJsReport::Disable()
	{
		m_is_disabled = true;
	}

	JsAutoRealmWithErrorReport::JsAutoRealmWithErrorReport(JSContext* ctx, JS::HandleObject global) : m_ac(ctx, global), m_are(ctx) {}

	void JsAutoRealmWithErrorReport::DisableReport()
	{
		m_are.Disable();
	}
}
