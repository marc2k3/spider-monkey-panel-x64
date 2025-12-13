#include <stdafx.h>
#include "js_object_helper.h"

namespace mozjs
{
	bool DummyGetter(JSContext*, unsigned, JS::Value* vp)
	{
		vp->setUndefined();
		return true;
	}

	const void* GetSmpProxyFamily()
	{
		// family must contain unique pointer, so the value does not really matter
		static const char kProxyFamilyVar = 'Q';
		return &kProxyFamilyVar;
	}

	SerializedJsValue SerializeJsValue(JSContext* cx, JS::HandleValue jsValue)
	{
		SerializedJsValue serializedValue;

		if (jsValue.isBoolean())
		{
			serializedValue = jsValue.toBoolean();
		}
		else if (jsValue.isInt32())
		{
			serializedValue = jsValue.toInt32();
		}
		else if (jsValue.isDouble())
		{
			serializedValue = jsValue.toDouble();
		}
		else if (jsValue.isString())
		{
			JS::RootedValue rVal(cx, jsValue);
			serializedValue = mozjs::convert::to_native::ToValue<std::string>(cx, rVal);
		}
		else
		{
			throw QwrException("Unsupported value type");
		}

		return serializedValue;
	}

	void DeserializeJsValue(JSContext* cx, const SerializedJsValue& serializedValue, JS::MutableHandleValue jsValue)
	{
		auto visitor = [cx, &jsValue](auto&& arg)
			{
				using T = std::decay_t<decltype(arg)>;

				if constexpr (std::is_same_v<T, bool>)
				{
					jsValue.setBoolean(arg);
				}
				else if constexpr (std::is_same_v<T, int32_t>)
				{
					jsValue.setInt32(arg);
				}
				else if constexpr (std::is_same_v<T, double>)
				{
					jsValue.setDouble(arg);
				}
				else if constexpr (std::is_same_v<T, std::string>)
				{
					mozjs::convert::to_js::ToValue(cx, arg, jsValue);
				}
				else
				{
					static_assert(smp::always_false_v<T>, "non-exhaustive visitor!");
				}
			};

		std::visit(visitor, serializedValue);
	}
}
