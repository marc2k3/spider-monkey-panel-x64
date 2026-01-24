#include "PCH.hpp"
#include "console.h"

namespace
{
	constexpr uint32_t kMaxLogDepth = 20;

	using namespace mozjs;

	std::string ParseJsValue(JSContext* ctx, JS::HandleValue jsValue, JS::MutableHandleObjectVector curObjects, uint32_t& logDepth, bool isParentObject);

	std::string ParseJsArray(JSContext* ctx, JS::HandleObject jsObject, JS::MutableHandleObjectVector curObjects, uint32_t& logDepth)
	{
		std::string output;

		output += "[";

		uint32_t arraySize;
		if (!JS::GetArrayLength(ctx, jsObject, &arraySize))
		{
			throw JsException();
		}

		JS::RootedValue arrayElement(ctx);
		for (uint32_t i = 0; i < arraySize; ++i)
		{
			if (!JS_GetElement(ctx, jsObject, i, &arrayElement))
			{
				throw JsException();
			}

			output += ParseJsValue(ctx, arrayElement, curObjects, logDepth, true);
			if (i != arraySize - 1)
			{
				output += ", ";
			}
		}

		output += "]";

		return output;
	}

	std::string ParseJsObject(JSContext* ctx, JS::HandleObject jsObject, JS::MutableHandleObjectVector curObjects, uint32_t& logDepth)
	{
		std::string output;

		{
			JS::RootedObject jsUnwrappedObject(ctx, jsObject);
			if (js::IsWrapper(jsObject))
			{
				jsUnwrappedObject = js::UncheckedUnwrap(jsObject);
			}
			if (js::IsProxy(jsUnwrappedObject) && js::GetProxyHandler(jsUnwrappedObject)->family() == GetSmpProxyFamily())
			{
				jsUnwrappedObject = js::GetProxyTargetObject(jsUnwrappedObject);
			}

			output += JS::InformalValueTypeName(JS::ObjectValue(*jsUnwrappedObject));
		}
		output += " {";

		JS::RootedIdVector jsVector(ctx);
		if (!js::GetPropertyKeys(ctx, jsObject, 0, &jsVector))
		{
			throw JsException();
		}

		JS::RootedValue jsIdValue(ctx);
		JS::RootedValue jsValue(ctx);
		bool hasFunctions = false;
		const auto length = jsVector.length();

		for (const auto i : indices(length))
		{
			const auto& jsId = jsVector[i];
			if (!JS_GetPropertyById(ctx, jsObject, jsId, &jsValue))
			{
				throw JsException();
			}

			if (jsValue.isObject() && JS_ObjectIsFunction(&jsValue.toObject()))
			{
				hasFunctions = true;
			}
			else
			{
				jsIdValue = js::IdToValue(jsId);
				output += convert::to_native::ToValue<std::string>(ctx, jsIdValue);
				output += "=";
				output += ParseJsValue(ctx, jsValue, curObjects, logDepth, true);

				if (i != length - 1uz || hasFunctions)
				{
					output += ", ";
				}
			}
		}

		if (hasFunctions)
		{
			output += "...";
		}

		output += "}";

		return output;
	}

	std::string ParseJsValue(JSContext* ctx, JS::HandleValue jsValue, JS::MutableHandleObjectVector curObjects, uint32_t& logDepth, bool isParentObject)
	{
		std::string output;

		++logDepth;
		auto autoDecrement = wil::scope_exit([&logDepth]
			{
				--logDepth;
			});

		if (!jsValue.isObject())
		{
			const bool showQuotes = isParentObject && jsValue.isString();

			if (showQuotes)
			{
				output += "\"";
			}
			output += convert::to_native::ToValue<std::string>(ctx, jsValue);
			if (showQuotes)
			{
				output += "\"";
			}
		}
		else
		{
			if (logDepth > kMaxLogDepth)
			{ // Don't parse object, if we reached the depth limit
				output += JS::InformalValueTypeName(jsValue);
				return output;
			}

			JS::RootedObject jsObject(ctx, &jsValue.toObject());

			if (JS_ObjectIsFunction(jsObject))
			{
				output += JS::InformalValueTypeName(jsValue);
			}
			else
			{
				for (const auto& curObject : curObjects)
				{
					if (jsObject.get() == curObject)
					{
						output += "<Circular>";
						return output;
					}
				}

				std::ignore = curObjects.emplaceBack(jsObject);

				auto autoPop = wil::scope_exit([&curObjects]
					{
						curObjects.popBack();
					});

				bool is;
				if (!JS::IsArrayObject(ctx, jsObject, &is))
				{
					throw JsException();
				}

				if (is)
				{
					output += ParseJsArray(ctx, jsObject, curObjects, logDepth);
				}
				else
				{
					output += ParseJsObject(ctx, jsObject, curObjects, logDepth);
				}
			}
		}

		return output;
	}

	void LogImpl(JSContext* ctx, unsigned argc, JS::Value* vp)
	{
		auto args = JS::CallArgsFromVp(argc, vp);
		const auto count = args.length();

		if (count > 0)
		{
			JS::RootedObjectVector curObjects(ctx);
			Strings parts;
			uint32_t logDepth{};

			for (const auto i : indices(count))
			{
				const auto part = ParseJsValue(ctx, args[i], &curObjects, logDepth, false);
				parts.emplace_back(part);
			}

			FB2K_console_formatter() << fmt::format("{}", fmt::join(parts, " ")).c_str();
		}

		args.rval().setUndefined();
	}

	DEFINE_JS_CLASS_OPS(Console::FinalizeJsObject)

	DEFINE_JS_CLASS_NO_PROPERTIES("Console")

	MJS_DEFINE_JS_FN_FROM_NATIVE(ClearBacklog, Console::ClearBacklog);
	MJS_DEFINE_JS_FN_FROM_NATIVE(GetLines, Console::GetLines);

	MJS_DEFINE_JS_FN(log, LogImpl)

	constexpr auto jsFunctions = std::to_array<JSFunctionSpec>(
		{
			JS_FN("ClearBacklog", ClearBacklog, 0, kDefaultPropsFlags),
			JS_FN("GetLines", GetLines, 1, kDefaultPropsFlags),
			JS_FN("log", log, 0, kDefaultPropsFlags),
			JS_FS_END,
		});
}

namespace mozjs
{
	const JSClass Console::JsClass = jsClass;
	const JSFunctionSpec* Console::JsFunctions = jsFunctions.data();
	const JSPropertySpec* Console::JsProperties = jsProperties.data();

	Console::Console(JSContext* ctx) : m_ctx(ctx) , m_ptr(fb2k::console_manager::get()) {}

	std::unique_ptr<Console> Console::CreateNative(JSContext* ctx)
	{
		return std::unique_ptr<Console>(new Console(ctx));
	}

	uint32_t Console::GetInternalSize()
	{
		return 0;
	}

	auto Console::ClearBacklog() -> void
	{
		m_ptr->clearBacklog();
	}

	auto Console::GetLines(bool with_timestamp) -> JSObject*
	{
		fb2k::arrayRef array;

		if (with_timestamp)
		{
			array = m_ptr->getLinesTimestamped();
		}
		else
		{
			array = m_ptr->getLines();
		}

		const auto& strings = array->typed<fb2k::string>();

		JS::RootedObject jsArray(m_ctx, JS::NewArrayObject(m_ctx, strings.size()));
		JsException::ExpectTrue(jsArray);

		JS::RootedValue jsValue(m_ctx);
		uint32_t i{};

		for (auto&& string : strings)
		{
			convert::to_js::ToValue(m_ctx, std::string(string->c_str()), &jsValue);

			if (!JS_SetElement(m_ctx, jsArray, i++, jsValue))
			{
				throw JsException();
			}
		}

		return jsArray;
	}
}
