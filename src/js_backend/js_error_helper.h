#pragma once

namespace mozjs
{
	[[nodiscard]] std::string ExceptionToText(JSContext* ctx);
	[[nodiscard]] std::string JsErrorToText(JSContext* ctx);
	void ExceptionToJsError(JSContext* ctx);
	void PrependTextToJsError(JSContext* ctx, const std::string& text);
	void SuppressException(JSContext* ctx);

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
}
