#include <PCH.hpp>
#include "js_error_scope.h"
#include "js_error_helper.h"

namespace mozjs
{
	AutoJsReport::AutoJsReport(JSContext* cx) : cx(cx) {}

	AutoJsReport::~AutoJsReport() noexcept
	{
		if (isDisabled_)
		{
			return;
		}

		if (!JS_IsExceptionPending(cx))
		{
			return;
		}

		try
		{
			const auto errorText = JsErrorToText(cx);
			JS_ClearPendingException(cx);
			JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));

			if (global)
			{
				auto globalCtx = static_cast<JsGlobalObject*>(GetMaybePtrFromReservedSlot(global, kReservedObjectSlot));

				if (globalCtx)
				{
					globalCtx->Fail(errorText);
					JS_ClearPendingException(cx);
				}
			}

		}
		catch (...) {}
	}

	void AutoJsReport::Disable()
	{
		isDisabled_ = true;
	}

	JsAutoRealmWithErrorReport::JsAutoRealmWithErrorReport(JSContext* cx, JS::HandleObject global) : ac_(cx, global), are_(cx) {}

	void JsAutoRealmWithErrorReport::DisableReport()
	{
		are_.Disable();
	}
}
