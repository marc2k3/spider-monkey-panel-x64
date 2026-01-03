#pragma once
#include "js_error_helper.h"
#include "js_error_scope.h"
#include "js_to_native.h"
#include "native_to_js.h"

namespace mozjs::internal
{
	template <size_t ArgArraySize>
	void NativeToJsArguments(
		[[maybe_unused]] JSContext* cx,
		[[maybe_unused]] JS::RootedValueArray<ArgArraySize>& wrappedArgs,
		[[maybe_unused]] uint8_t argIndex) {}

	template <size_t ArgArraySize, typename ArgType, typename... ArgTypes>
	void NativeToJsArguments(
		JSContext* cx,
		JS::RootedValueArray<ArgArraySize>& wrappedArgs,
		uint8_t argIndex, ArgType&& arg,
		ArgTypes&&... args)
	{
		convert::to_js::ToValue(cx, std::forward<ArgType>(arg), wrappedArgs[argIndex]);
		NativeToJsArguments(cx, wrappedArgs, argIndex + 1, std::forward<ArgTypes>(args)...);
	}

	inline bool InvokeJsCallback_Impl(
		JSContext* cx,
		JS::HandleObject globalObject,
		JS::HandleValue functionValue,
		const JS::HandleValueArray& args,
		JS::MutableHandleValue rval)
	{
		JS::RootedFunction func(cx, JS_ValueToFunction(cx, functionValue));

		if (func)
		{
			return JS::Call(cx, globalObject, func, args, rval);
		}

		return false;
	}
}

namespace mozjs
{
	template <typename ReturnType = std::nullptr_t, typename... ArgTypes>
	std::optional<ReturnType> InvokeJsCallback(
		JSContext* cx,
		JS::HandleObject globalObject,
		std::string functionName,
		ArgTypes&&... args)
	{
		JsAutoRealmWithErrorReport autoScope(cx, globalObject);
		JS::RootedValue funcValue(cx);

		if (!JS_GetProperty(cx, globalObject, functionName.c_str(), &funcValue))
		{
			// Reports
			return std::nullopt;
		}

		if (funcValue.isUndefined())
		{
			// Not an error: user didn't define a callback
			return std::nullopt;
		}

		try
		{
			JS::RootedValue retVal(cx);

			if constexpr (sizeof...(ArgTypes) > 0)
			{
				JS::RootedValueArray<sizeof...(ArgTypes)> wrappedArgs(cx);
				internal::NativeToJsArguments(cx, wrappedArgs, 0, std::forward<ArgTypes>(args)...);

				if (internal::InvokeJsCallback_Impl(cx, globalObject, funcValue, wrappedArgs, &retVal))
				{
					return convert::to_native::ToValue<ReturnType>(cx, retVal);
				}
			}
			else
			{
				if (internal::InvokeJsCallback_Impl(cx, globalObject, funcValue, JS::HandleValueArray::empty(), &retVal))
				{
					return convert::to_native::ToValue<ReturnType>(cx, retVal);
				}
			}

			return std::nullopt;
		}
		catch (...)
		{
			ExceptionToJsError(cx);
			return std::nullopt;
		}
	}
}
