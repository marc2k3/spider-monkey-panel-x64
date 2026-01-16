#pragma once
#include "js_error_helper.h"
#include "js_error_scope.h"
#include "js_to_native.h"
#include "native_to_js.h"

namespace mozjs::internal
{
	template <size_t ArgArraySize>
	void NativeToJsArguments(
		[[maybe_unused]] JSContext* ctx,
		[[maybe_unused]] JS::RootedValueArray<ArgArraySize>& wrappedArgs,
		[[maybe_unused]] uint8_t argIndex) {}

	template <size_t ArgArraySize, typename ArgType, typename... ArgTypes>
	void NativeToJsArguments(
		JSContext* ctx,
		JS::RootedValueArray<ArgArraySize>& wrappedArgs,
		uint8_t argIndex, ArgType&& arg,
		ArgTypes&&... args)
	{
		convert::to_js::ToValue(ctx, std::forward<ArgType>(arg), wrappedArgs[argIndex]);
		NativeToJsArguments(ctx, wrappedArgs, argIndex + 1, std::forward<ArgTypes>(args)...);
	}

	inline bool InvokeJsCallback_Impl(
		JSContext* ctx,
		JS::HandleObject globalObject,
		JS::HandleValue functionValue,
		const JS::HandleValueArray& args,
		JS::MutableHandleValue rval)
	{
		JS::RootedFunction func(ctx, JS_ValueToFunction(ctx, functionValue));

		if (func)
		{
			return JS::Call(ctx, globalObject, func, args, rval);
		}

		return false;
	}
}

namespace mozjs
{
	template <typename ReturnType = std::nullptr_t, typename... ArgTypes>
	std::optional<ReturnType> InvokeJsCallback(
		JSContext* ctx,
		JS::HandleObject globalObject,
		std::string functionName,
		ArgTypes&&... args)
	{
		JsAutoRealmWithErrorReport autoScope(ctx, globalObject);
		JS::RootedValue funcValue(ctx);

		if (!JS_GetProperty(ctx, globalObject, functionName.c_str(), &funcValue))
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
			JS::RootedValue retVal(ctx);

			if constexpr (sizeof...(ArgTypes) > 0)
			{
				JS::RootedValueArray<sizeof...(ArgTypes)> wrappedArgs(ctx);
				internal::NativeToJsArguments(ctx, wrappedArgs, 0, std::forward<ArgTypes>(args)...);

				if (internal::InvokeJsCallback_Impl(ctx, globalObject, funcValue, wrappedArgs, &retVal))
				{
					return convert::to_native::ToValue<ReturnType>(ctx, retVal);
				}
			}
			else
			{
				if (internal::InvokeJsCallback_Impl(ctx, globalObject, funcValue, JS::HandleValueArray::empty(), &retVal))
				{
					return convert::to_native::ToValue<ReturnType>(ctx, retVal);
				}
			}

			return std::nullopt;
		}
		catch (...)
		{
			ExceptionToJsError(ctx);
			return std::nullopt;
		}
	}
}
