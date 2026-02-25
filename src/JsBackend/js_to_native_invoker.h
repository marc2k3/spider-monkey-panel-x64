#pragma once
#include "js_error_helper.h"

namespace mozjs::internal
{
	template <typename... ArgTypes, typename FuncType, uint32_t... Indexes>
	auto ProcessJsArgs_Impl(const JS::CallArgs& jsArgs, FuncType&& func, std::index_sequence<Indexes...>)
	{
		return std::make_tuple(func(jsArgs, std::type_identity<ArgTypes>{}, Indexes)...);
	}

	template <typename... ArgTypes, typename FuncType>
	auto ProcessJsArgs(const JS::CallArgs& jsArgs, FuncType&& func)
	{
		return ProcessJsArgs_Impl<ArgTypes...>(jsArgs, func, std::make_index_sequence<sizeof...(ArgTypes)>{});
	}

	template <typename BaseClass>
	BaseClass* InvokeNativeCallback_GetThisObject(JSContext* ctx, JS::HandleValue jsThis)
	{
		if (jsThis.isObject())
		{
			JS::RootedObject jsObject(ctx, &jsThis.toObject());
			return JsObjectBase<BaseClass>::ExtractNative(ctx, jsObject);
		}

		if constexpr (std::is_same_v<BaseClass, JsGlobalObject>)
		{
			if (jsThis.isUndefined())
			{ // global has undefined `this`
				JS::RootedObject jsObject(ctx, JS::CurrentGlobalOrNull(ctx));
				return JsGlobalObject::ExtractNative(ctx, jsObject);
			}
		}

		return nullptr;
	}

	template <typename FirstArg, typename... Args>
	constexpr bool InvokeNativeCallback_ParseArguments_Check()
	{
		// Check that only last argument has JS::HandleValueArray type
		if constexpr (sizeof...(Args) <= 1)
		{
			return true;
		}
		else
		{
			return !std::is_same_v<FirstArg, JS::HandleValueArray> && InvokeNativeCallback_ParseArguments_Check<Args...>();
		}
	}

	template <typename... ArgTypes>
	auto InvokeNativeCallback_ParseArguments(JSContext* ctx, JS::MutableHandleValueVector valueVector, const JS::CallArgs& jsArgs)
	{
		constexpr uint32_t argCount = sizeof...(ArgTypes);
		if constexpr (argCount > 0)
		{
			static_assert(InvokeNativeCallback_ParseArguments_Check<ArgTypes...>());
		}

		if constexpr (argCount > 0)
		{
			if constexpr (std::is_same_v<typename std::tuple_element<argCount - 1, std::tuple<ArgTypes...>>::type, JS::HandleValueArray>)
			{
				if (argCount <= jsArgs.length())
				{
					// Reserve memory, so that we can initialize JS::HandleValueArray correctly
					if (!valueVector.resize(jsArgs.length() - (argCount - 1)))
					{
						throw std::bad_alloc();
					}
				}
			}
		}

		auto callbackArguments =
			ProcessJsArgs<ArgTypes...>(
				jsArgs,
				[ctx, &valueVector](const JS::CallArgs& jsArgs, auto argTypeStruct, uint32_t index) {
					using ArgType = std::decay_t<typename decltype(argTypeStruct)::type>;
					constexpr uint32_t MaxArgCount = sizeof...(ArgTypes);
					// Not an error: default value might be set in callback
					const bool isDefaultValue = (index >= jsArgs.length() || index > MaxArgCount);

					if constexpr (std::is_same_v<ArgType, JS::HandleValue>)
					{
						if (isDefaultValue)
						{
							return ArgType(JS::UndefinedHandleValue);
						}
						else
						{
							return ArgType(jsArgs[index]);
						}
					}
					else if constexpr (std::is_same_v<ArgType, JS::HandleValueArray>)
					{
						if (isDefaultValue)
						{
							return JS::HandleValueArray::empty();
						}
						else
						{
							return JS::HandleValueArray::fromMarkedLocation(valueVector.length(), valueVector.begin());
						}
					}
					else
					{
						if (isDefaultValue)
						{
							return ArgType(); ///< Dummy value
						}
						else
						{
							return convert::to_native::ToValue<ArgType>(ctx, jsArgs[index]);
						}
					}
				});

		if constexpr (argCount > 0)
		{
			if constexpr (std::is_same_v<typename std::tuple_element<argCount - 1, std::tuple<ArgTypes...>>::type, JS::HandleValueArray>)
			{
				if (!valueVector.empty())
				{
					uint32_t startIdx = 0;
					for (auto i : std::views::iota(argCount - 1, jsArgs.length()))
					{
						valueVector[startIdx++].set(jsArgs[i]);
					}
				}
			}
		}

		return callbackArguments;
	}

	template <bool HasOptArg,  typename ReturnType, typename BaseClass, typename FuncType, typename FuncOptType, typename ArgTupleType>
	ReturnType InvokeNativeCallback_Call_Member(
		BaseClass* baseClass,
		FuncType fn,
		FuncOptType fnWithOpt,
		const ArgTupleType& argTuple,
		uint32_t optArgCount)
	{
		if constexpr (!HasOptArg)
		{
			(void)fnWithOpt;
			(void)optArgCount;
			return std::apply(fn, std::tuple_cat(std::make_tuple(baseClass), argTuple));
		}
		else
		{ // Invoke callback with optional argument handler
			(void)fn;
			return std::apply(fnWithOpt, std::tuple_cat(std::make_tuple(baseClass, optArgCount), argTuple));
		}
	}

	template <bool HasOptArg, typename ReturnType, typename FuncType, typename FuncOptType, typename ArgTupleType>
	ReturnType InvokeNativeCallback_Call_Static(JSContext* ctx, FuncType fn, FuncOptType fnWithOpt, const ArgTupleType& argTuple, uint32_t optArgCount)
	{
		if constexpr (!HasOptArg)
		{
			return std::apply(fn, std::tuple_cat(std::make_tuple(ctx), argTuple));
		}
		else
		{
			return std::apply(fnWithOpt, std::tuple_cat(std::make_tuple(ctx, optArgCount), argTuple));
		}
	}

	template <uint32_t OptArgCount, typename BaseClass, typename ReturnType, typename FuncType, typename FuncOptType, typename... ArgTypes>
	void InvokeNativeCallback_Member(JSContext* ctx, FuncType fn, FuncOptType fnWithOpt, unsigned argc, JS::Value* vp)
	{
		constexpr uint32_t maxArgCount = sizeof...(ArgTypes);
		static_assert(OptArgCount <= maxArgCount);

		JS::CallArgs jsArgs = JS::CallArgsFromVp(argc, vp);
		if (jsArgs.length() < (maxArgCount - OptArgCount))
		{
			throw QwrException("Invalid number of arguments");
		}

		BaseClass* baseClass = InvokeNativeCallback_GetThisObject<BaseClass>(ctx, jsArgs.thisv());
		if (!baseClass)
		{
			throw QwrException("Invalid `this` context");
		}

		JS::RootedValueVector handleValueVector(ctx);
		auto callbackArguments = InvokeNativeCallback_ParseArguments<ArgTypes...>(ctx, &handleValueVector, jsArgs);

		// May return raw JS pointer! (see below)
		const auto invokeNative = [&]
			{
				return InvokeNativeCallback_Call_Member<!!OptArgCount, ReturnType>(
					baseClass,
					fn,
					fnWithOpt,
					callbackArguments,
					(maxArgCount > jsArgs.length() ? maxArgCount - jsArgs.length() : 0)
				);
			};

		// Return value
		if constexpr (std::is_same_v<ReturnType, JSObject*>)
		{ // A raw JS pointer! Be careful when editing this code!
			jsArgs.rval().setObjectOrNull(invokeNative());
		}
		else if constexpr (std::is_same_v<ReturnType, JS::Value>)
		{ // Unrooted JS::Value! Be careful when editing this code!
			jsArgs.rval().set(invokeNative());
		}
		else if constexpr (std::is_same_v<ReturnType, void>)
		{
			invokeNative();
			jsArgs.rval().setUndefined();
		}
		else
		{
			convert::to_js::ToValue(ctx, invokeNative(), jsArgs.rval());
		}
	}

	template <uint32_t OptArgCount, typename ReturnType, typename FuncType, typename FuncOptType, typename... ArgTypes>
	void InvokeNativeCallback_Static(JSContext* ctx, FuncType fn, FuncOptType fnWithOpt, unsigned argc, JS::Value* vp)
	{
		constexpr uint32_t maxArgCount = sizeof...(ArgTypes);
		static_assert(OptArgCount <= maxArgCount);

		JS::CallArgs jsArgs = JS::CallArgsFromVp(argc, vp);
		if (jsArgs.length() < (maxArgCount - OptArgCount))
		{
			throw QwrException("Invalid number of arguments");
		}

		JS::RootedValueVector handleValueVector(ctx);
		auto callbackArguments = InvokeNativeCallback_ParseArguments<ArgTypes...>(ctx, &handleValueVector, jsArgs);

		// May return raw JS pointer! (see below)
		const auto invokeNative = [&]
			{
				return InvokeNativeCallback_Call_Static<!!OptArgCount, ReturnType>(
					ctx,
					fn,
					fnWithOpt,
					callbackArguments,
					(maxArgCount > jsArgs.length() ? maxArgCount - jsArgs.length() : 0)
				);
			};

		// Return value
		if constexpr (std::is_same_v<ReturnType, JSObject*>)
		{ // A raw JS pointer! Be careful when editing this code!
			jsArgs.rval().setObjectOrNull(invokeNative());
		}
		else if constexpr (std::is_same_v<ReturnType, JS::Value>)
		{ // Unrooted JS::Value! Be careful when editing this code!
			jsArgs.rval().set(invokeNative());
		}
		else if constexpr (std::is_same_v<ReturnType, void>)
		{
			invokeNative();
			jsArgs.rval().setUndefined();
		}
		else
		{
			convert::to_js::ToValue(ctx, invokeNative(), jsArgs.rval());
		}
	}
}

namespace mozjs
{
	template <typename F, typename... Args>
	[[nodiscard]] bool Execute_JsSafe(JSContext* ctx, std::string_view functionName, F&& func, Args&&... args)
	{
		try
		{
			func(ctx, std::forward<Args>(args)...);
		}
		catch (...)
		{
			ExceptionToJsError(ctx);
		}

		if (JS_IsExceptionPending(ctx))
		{
			PrependTextToJsError(ctx, fmt::format("{} failed", functionName));
			return false;
		}

		return true;
	}

	template <uint32_t OptArgCount = 0, typename BaseClass, typename ReturnType, typename FuncOptType, typename... ArgTypes>
	void InvokeNativeCallback(JSContext* ctx, ReturnType(BaseClass::* fn)(ArgTypes...), FuncOptType fnWithOpt, unsigned argc, JS::Value* vp)
	{
		mozjs::internal::InvokeNativeCallback_Member<
			OptArgCount,
			BaseClass,
			ReturnType,
			decltype(fn),
			FuncOptType,
			ArgTypes...>(ctx, fn, fnWithOpt, argc, vp);
	}

	template <uint32_t OptArgCount = 0, typename BaseClass, typename ReturnType, typename FuncOptType, typename... ArgTypes>
	void InvokeNativeCallback(JSContext* ctx, ReturnType(BaseClass::* fn)(ArgTypes...) const, FuncOptType fnWithOpt, unsigned argc, JS::Value* vp)
	{
		mozjs::internal::InvokeNativeCallback_Member<
			OptArgCount,
			BaseClass,
			ReturnType,
			decltype(fn),
			FuncOptType,
			ArgTypes...>(ctx, fn, fnWithOpt, argc, vp);
	}

	template <uint32_t OptArgCount = 0, typename ReturnType, typename FuncOptType, typename... ArgTypes>
	void InvokeNativeCallback(JSContext* ctx, ReturnType(__cdecl* fn)(JSContext*, ArgTypes...), FuncOptType fnWithOpt, unsigned argc, JS::Value* vp)
	{
		mozjs::internal::InvokeNativeCallback_Static<
			OptArgCount,
			ReturnType,
			decltype(fn),
			FuncOptType,
			ArgTypes...>(ctx, fn, fnWithOpt, argc, vp);
	}
}

/// @brief Defines a function named `functionName`, which executes `functionImpl` in a safe way:
///        traps C++ exceptions and converts them to JS exceptions,
///        while adding `logName` to error report.
#define MJS_DEFINE_JS_FN_FULL(functionName, logName, functionImpl) \
	bool functionName(JSContext* ctx, unsigned argc, JS::Value* vp) \
	{ \
		return mozjs::Execute_JsSafe(ctx, logName, functionImpl, argc, vp); \
	} \

/// @brief Same as MJS_DEFINE_JS_FN_FULL, but uses `functionName` for logging.
#define MJS_DEFINE_JS_FN(functionName, functionImpl) \
	MJS_DEFINE_JS_FN_FULL(functionName, #functionName, functionImpl)

/// @brief Defines a function named `functionName`, which converts all JS arguments to corresponding arguments
///        of `functionImpl` prototype and executes it in a safe way (see MJS_DEFINE_JS_FN_FULL).
///        Allows for execution with less arguments than in `functionImpl` prototype
///        (`optArgCount` is the maximum amount of optional arguments),
///        in that case `functionImplWithOpt` will be called.
#define MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT_FULL(functionName, logName, functionImpl, functionImplWithOpt, optArgCount) \
	bool functionName(JSContext* ctx, unsigned argc, JS::Value* vp) \
	{ \
		const auto wrappedFunc = [](JSContext* ctx, unsigned argc, JS::Value* vp) \
			{ \
				InvokeNativeCallback<optArgCount>(ctx, &functionImpl, &functionImplWithOpt, argc, vp); \
			}; \
		return mozjs::Execute_JsSafe(ctx, logName, wrappedFunc, argc, vp); \
	} \

/// @brief Same as MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT_FULL, but uses `functionName` for logging.
#define MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(functionName, functionImpl, functionImplWithOpt, optArgCount) \
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT_FULL(functionName, #functionName, functionImpl, functionImplWithOpt, optArgCount)

/// @brief Same as MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT_FULL, but with zero optional arguments.
#define MJS_DEFINE_JS_FN_FROM_NATIVE_FULL(functionName, logName, functionImpl) \
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT_FULL(functionName, logName, functionImpl, functionImpl, 0)

/// @brief Same as MJS_DEFINE_JS_FN_FROM_NATIVE_FULL, but uses `functionName` for logging.
#define MJS_DEFINE_JS_FN_FROM_NATIVE(functionName, functionImpl) \
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(functionName, functionImpl, functionImpl, 0)
