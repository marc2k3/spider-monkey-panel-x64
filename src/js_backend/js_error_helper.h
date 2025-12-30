#pragma once

namespace mozjs
{
	[[nodiscard]] std::string ExceptionToText(JSContext* cx);
	[[nodiscard]] std::string JsErrorToText(JSContext* cx);
	void ExceptionToJsError(JSContext* cx);
	void PrependTextToJsError(JSContext* cx, const std::string& text);
	void SuppressException(JSContext* cx);

	template <typename F, typename... Args>
	[[nodiscard]] bool Execute_JsSafe(JSContext* cx, std::string_view functionName, F&& func, Args&&... args)
	{
		try
		{
			func(cx, std::forward<Args>(args)...);
		}
		catch (...)
		{
			ExceptionToJsError(cx);
		}

		if (JS_IsExceptionPending(cx))
		{
			PrependTextToJsError(cx, fmt::format("{} failed", functionName));
			return false;
		}

		return true;
	}
}
