#pragma once

namespace mozjs
{
	template <typename T>
	std::optional<T> GetOptionalProperty(JSContext* ctx, JS::HandleObject jsObject, const std::string& propName)
	{
		bool hasProp;
		if (!JS_HasProperty(ctx, jsObject, propName.c_str(), &hasProp))
		{
			throw JsException();
		}

		if (!hasProp)
		{
			return std::nullopt;
		}

		JS::RootedValue jsValue(ctx);
		if (!JS_GetProperty(ctx, jsObject, propName.c_str(), &jsValue))
		{
			throw JsException();
		}

		return convert::to_native::ToValue<T>(ctx, jsValue);
	};

	template <typename T>
	void AddProperty(JSContext* ctx, JS::HandleObject jsObject, const std::string& propName, const T& propValue)
	{
		if constexpr (std::is_same_v<T, JS::RootedValue>)
		{
			if (!JS_DefineProperty(ctx, jsObject, propName.c_str(), propValue, kDefaultPropsFlags))
			{
				throw JsException();
			}
		}
		else
		{
			JS::RootedValue jsProperty(ctx);
			convert::to_js::ToValue(ctx, propValue, &jsProperty);

			if (!JS_DefineProperty(ctx, jsObject, propName.c_str(), jsProperty, kDefaultPropsFlags))
			{
				throw JsException();
			}
		}
	};

	template <typename T>
	void SetProperty(JSContext* ctx, JS::HandleObject jsObject, const std::string& propName, const T& propValue)
	{
		JS::RootedValue jsProperty(ctx);
		convert::to_js::ToValue(ctx, propValue, &jsProperty);

		if (!JS_SetProperty(ctx, jsObject, propName.c_str(), jsProperty))
		{
			throw JsException();
		}
	};
}
