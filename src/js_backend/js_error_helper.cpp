#include "PCH.hpp"
#include "js_error_helper.h"

#include "cached_utf8_paths_hack.h"

namespace
{
	using namespace mozjs;

	std::string GetStackTraceString(JSContext* ctx, JS::HandleObject exn)
	{
		try
		{
			// Must not throw errors in error handler
			// Note: exceptions thrown while compiling top-level script have no stack.
			JS::RootedObject stackObj(ctx, JS::ExceptionStackOrNull(exn));
			if (!stackObj)
			{
				// quack?
				return GetOptionalProperty<std::string>(ctx, exn, "stack").value_or("");
			}

			JS::RootedString stackStr(ctx);
			if (!JS::BuildStackString(ctx, nullptr, stackObj, &stackStr, 2))
			{
				return "";
			}

			const auto& cachedPaths = hack::GetAllCachedUtf8Paths();
			auto pfcStackStr = convert::to_native::ToValue<pfc::string>(ctx, stackStr);

			for (const auto& [id, path] : cachedPaths)
			{
				const auto filename = pfc::string_filename_ext(path.c_str());
				pfcStackStr = pfcStackStr.replace(id.c_str(), filename.c_str());
			}

			return pfcStackStr.get_ptr();
		}
		catch (...)
		{
			SuppressException(ctx);
			return "";
		}
	}

	bool PrependTextToJsStringException(JSContext* ctx, JS::HandleValue excn, const std::string& text)
	{
		std::string currentMessage;

		try
		{
			// Must not throw errors in error handler
			currentMessage = convert::to_native::ToValue<std::string>(ctx, excn);
		}
		catch (const JsException&)
		{
			return false;
		}
		catch (const QwrException&)
		{
			return false;
		}

		if (currentMessage == std::string("out of memory"))
		{
			// Can't modify the message since we're out of memory
			return true;
		}

		std::string newMessage;

		if (currentMessage.empty())
		{
			newMessage = text;
		}
		else
		{
			newMessage = fmt::format("{}:\n{}", text, currentMessage);
		}

		JS::RootedValue jsMessage(ctx);

		try
		{
			// Must not throw errors in error handler
			convert::to_js::ToValue<std::string>(ctx, newMessage, &jsMessage);
		}
		catch (const JsException&)
		{
			return false;
		}
		catch (const QwrException&)
		{
			return false;
		}

		JS_SetPendingException(ctx, jsMessage);
		return true;
	}

	bool PrependTextToJsObjectException(JSContext* ctx, JS::HandleValue excn, const std::string& text)
	{
		auto autoClearOnError = wil::scope_exit([ctx]
			{
				JS_ClearPendingException(ctx);
			});

		JS::RootedObject excnObject(ctx, &excn.toObject());
		JS_ClearPendingException(ctx); ///< need this for js::ErrorReport::init
		JS::RootedObject excnStackObject(ctx, JS::ExceptionStackOrNull(excnObject));

		if (!excnStackObject)
		{
			// Sometimes happens with custom JS errors
			return false;
		}

		JS::ExceptionStack excnStack(ctx, excn, excnStackObject);
		JS::ErrorReportBuilder reportBuilder(ctx);

		if (!reportBuilder.init(ctx, excnStack, JS::ErrorReportBuilder::SniffingBehavior::WithSideEffects))
		{
			// Sometimes happens with custom JS errors
			return false;
		}

		JSErrorReport* pReport = reportBuilder.report();

		std::string newMessage;
		std::string currentMessage = pReport->message().c_str();

		if (currentMessage.empty())
		{
			newMessage = text;
		}
		else
		{
			newMessage = fmt::format("{}:\n{}", text, currentMessage);
		}

		JS::RootedValue jsFilename(ctx);
		JS::RootedValue jsMessage(ctx);

		try
		{
			// Must not throw errors in error handler
			convert::to_js::ToValue<std::string>(ctx, pReport->filename, &jsFilename);
			convert::to_js::ToValue<std::string>(ctx, newMessage, &jsMessage);
		}
		catch (...)
		{
			SuppressException(ctx);
			return false;
		}

		JS::RootedValue newExcn(ctx);
		JS::RootedString jsFilenameStr(ctx, jsFilename.toString());
		JS::RootedString jsMessageStr(ctx, jsMessage.toString());

		if (!JS_WrapObject(ctx, &excnStackObject))
		{
			// Need wrapping for the case when exception is thrown from internal global
			return false;
		}

		JS::Rooted<mozilla::Maybe<JS::Value>> cause(ctx, mozilla::Nothing{});

		if (!JS::CreateError(ctx, static_cast<JSExnType>(pReport->exnType), excnStackObject, jsFilenameStr, pReport->lineno, pReport->column, nullptr, jsMessageStr, cause, &newExcn))
		{
			return false;
		}

		autoClearOnError.release();
		JS_SetPendingException(ctx, newExcn);
		return true;
	}

	std::string ComErrorToText(const _com_error& e)
	{
		auto text = fmt::format("COM error:\n  hresult: {}", format_hresult(e.Error()).get_ptr());

		if (e.ErrorMessage())
		{
			text += fmt::format("\n  message: {}", smp::ToU8(e.ErrorMessage()));
		}

		if (e.Source().length())
		{
			text += fmt::format("\n  source: {}", smp::ToU8(e.Source().GetBSTR()));
		}

		if (e.Description().length())
		{
			text += fmt::format("\n  description: {}", smp::ToU8(e.Description().GetBSTR()));
		}

		return text;
	}

	std::string WilExceptionToText(const wil::ResultException& e)
	{
		std::array<wchar_t, 2048uz> msg{};

		if SUCCEEDED(wil::GetFailureLogString(msg.data(), msg.size(), e.GetFailureInfo()))
			return smp::ToU8(msg.data());

		return format_hresult(e.GetErrorCode()).get_ptr();
	}
}

namespace mozjs
{
	std::string ExceptionToText(JSContext* ctx)
	{
		try
		{
			throw;
		}
		catch (const JsException&)
		{
			return JsErrorToText(ctx);
		}
		catch (const wil::ResultException& e)
		{
			JS_ClearPendingException(ctx);
			return WilExceptionToText(e);
		}
		catch (const QwrException& e)
		{
			JS_ClearPendingException(ctx);
			return e.what();
		}
		catch (const _com_error& e)
		{
			JS_ClearPendingException(ctx);
			return ComErrorToText(e);
		}
		catch (const std::bad_alloc& e)
		{
			JS_ClearPendingException(ctx);
			return e.what();
		}
		// SM is not designed to handle uncaught exceptions, so we are risking here,
		// hoping that this exception will reach fb2k handler.
	}

	std::string JsErrorToText(JSContext* ctx)
	{
		JS::RootedValue excn(ctx);
		JS_GetPendingException(ctx, &excn);
		JS_ClearPendingException(ctx); ///< need this for js::ErrorReport::init

		auto autoErrorClear = wil::scope_exit([ctx]
			{
				// There should be no exceptions on function exit
				JS_ClearPendingException(ctx);
			});

		std::string errorText;
		if (excn.isString())
		{
			try
			{
				// Must not throw errors in error handler
				errorText = convert::to_native::ToValue<std::string>(ctx, excn);
			}
			catch (...)
			{
				SuppressException(ctx);
			}
		}
		else if (excn.isObject())
		{
			JS::RootedObject excnObject(ctx, &excn.toObject());

			JSErrorReport* pReport = JS_ErrorFromException(ctx, excnObject);
			if (!pReport)
			{
				// Sometimes happens with custom JS errors
				return errorText;
			}

			errorText = pReport->message().c_str();

			if (pReport->filename)
			{
				errorText += "\n";
				std::string stdPath;

				if (std::string(pReport->filename) == "")
				{
					stdPath = "<main>";
				}
				else
				{
					const auto pathOpt = hack::GetCachedUtf8Path(pReport->filename);

					if (pathOpt)
					{
						stdPath = pathOpt->filename().u8string();
					}
					else
					{
						stdPath = pReport->filename;
					}
				}

				errorText += fmt::format("\nFile: {}", stdPath);
				errorText += fmt::format("\nLine: {}, Column: {}", std::to_string(pReport->lineno), std::to_string(pReport->column));

				if (pReport->linebufLength())
				{
					pfc::string8 tmpBuf = pfc::stringcvt::string_utf8_from_utf16(pReport->linebuf(), pReport->linebufLength()).get_ptr();
					tmpBuf.truncate_eol();

					errorText += "\nSource: ";
					errorText += tmpBuf;
				}

				const auto stackTrace = GetStackTraceString(ctx, excnObject);

				if (!stackTrace.empty())
				{
					errorText += "\nStack trace:\n" + stackTrace;
				}
			}
		}

		return errorText;
	}

	void ExceptionToJsError(JSContext* ctx)
	{
		try
		{
			throw;
		}
		catch (const JsException&)
		{
		}
		catch (const wil::ResultException& e)
		{
			JS_ClearPendingException(ctx);
			const auto text = WilExceptionToText(e);
			JS_ReportErrorUTF8(ctx, text.c_str());
		}
		catch (const QwrException& e)
		{
			JS_ClearPendingException(ctx);
			JS_ReportErrorUTF8(ctx, e.what());
		}
		catch (const _com_error& e)
		{
			JS_ClearPendingException(ctx);
			const auto text = ComErrorToText(e);
			JS_ReportErrorUTF8(ctx, text.c_str());
		}
		catch (const std::bad_alloc&)
		{
			JS_ClearPendingException(ctx);
			JS_ReportAllocationOverflow(ctx);
		}
		// SM is not designed to handle uncaught exceptions, so we are risking here,
		// hoping that this exception will reach fb2k handler.
	}

	void PrependTextToJsError(JSContext* ctx, const std::string& text)
	{
		auto autoJsReport = wil::scope_exit([ctx, text]
			{
				JS_ReportErrorUTF8(ctx, "%s", text.c_str());
			});

		if (JS_IsExceptionPending(ctx))
		{
			// Get exception object before printing and clearing exception.
			JS::RootedValue excn(ctx);
			JS_GetPendingException(ctx, &excn);

			if (excn.isString() && PrependTextToJsStringException(ctx, excn, text))
			{
				autoJsReport.release();
			}
			else if (excn.isObject() && PrependTextToJsObjectException(ctx, excn, text))
			{
				autoJsReport.release();
			}
		}
	}

	void SuppressException(JSContext* ctx)
	{
		try
		{
			throw;
		}
		catch (const JsException&) {}
		catch (const wil::ResultException&) {}
		catch (const QwrException&) {}
		catch (const _com_error&) {}
		catch (const std::bad_alloc&) {}
		// SM is not designed to handle uncaught exceptions, so we are risking here,
		// hoping that this exception will reach fb2k handler.

		JS_ClearPendingException(ctx);
	}
}
