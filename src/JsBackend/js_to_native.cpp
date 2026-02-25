#include "PCH.hpp"
#include "js_to_native.h"

namespace mozjs::convert::to_native
{
	namespace internal
	{
		template <>
		bool ToSimpleValue(JSContext*, const JS::HandleValue& jsValue)
		{
			return JS::ToBoolean(jsValue);
		}

		template <>
		int32_t ToSimpleValue(JSContext* ctx, const JS::HandleValue& jsValue)
		{
			int32_t val{};

			if (JS::ToInt32(ctx, jsValue, &val))
			{
				return val;
			}

			throw JsException();
		}

		template <>
		uint8_t ToSimpleValue(JSContext* ctx, const JS::HandleValue& jsValue)
		{
			uint8_t val{};

			if (JS::ToUint8(ctx, jsValue, &val))
			{
				return val;
			}

			throw JsException();
		}

		template <>
		uint32_t ToSimpleValue(JSContext* ctx, const JS::HandleValue& jsValue)
		{
			uint32_t val{};

			if (JS::ToUint32(ctx, jsValue, &val))
			{
				return val;
			}

			throw JsException();
		}

		template <>
		uint64_t ToSimpleValue(JSContext* ctx, const JS::HandleValue& jsValue)
		{
			uint64_t val{};

			if (JS::ToUint64(ctx, jsValue, &val))
			{
				return val;
			}

			throw JsException();
		}

		template <>
		float ToSimpleValue(JSContext* ctx, const JS::HandleValue& jsValue)
		{
			double val{};

			if (JS::ToNumber(ctx, jsValue, &val))
			{
				return static_cast<float>(val);
			}

			throw JsException();
		}

		template <>
		double ToSimpleValue(JSContext* ctx, const JS::HandleValue& jsValue)
		{
			double val{};

			if (JS::ToNumber(ctx, jsValue, &val))
			{
				return val;
			}

			throw JsException();
		}

		template <>
		std::string ToSimpleValue(JSContext* ctx, const JS::HandleValue& jsValue)
		{
			JS::RootedString jsString(ctx, JS::ToString(ctx, jsValue));
			return ToValue<std::string>(ctx, jsString);
		}

		template <>
		std::wstring ToSimpleValue(JSContext* ctx, const JS::HandleValue& jsValue)
		{
			JS::RootedString jsString(ctx, JS::ToString(ctx, jsValue));
			return ToValue<std::wstring>(ctx, jsString);
		}

		template <>
		std::nullptr_t ToSimpleValue(JSContext*, const JS::HandleValue&)
		{
			return nullptr;
		}
	}

	template <>
	std::string ToValue(JSContext* ctx, const JS::HandleString& jsString)
	{
		return smp::ToU8(ToValue<std::wstring>(ctx, jsString));
	}

	template <>
	std::wstring ToValue(JSContext* ctx, const JS::HandleString& jsString)
	{
		std::wstring wStr(JS_GetStringLength(jsString), '\0');
		mozilla::Range<char16_t> wCharStr(reinterpret_cast<char16_t*>(wStr.data()), wStr.size());

		if (JS_CopyStringChars(ctx, wCharStr, jsString))
		{
			return wStr;
		}

		throw JsException();
	}
}
