#pragma once

namespace mozjs
{
	[[nodiscard]] std::string ExceptionToText(JSContext* ctx);
	[[nodiscard]] std::string JsErrorToText(JSContext* ctx);
	void ExceptionToJsError(JSContext* ctx);
	void PrependTextToJsError(JSContext* ctx, const std::string& text);
	void SuppressException(JSContext* ctx);
}
