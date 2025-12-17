#include <stdafx.h>
#include "fb_properties.h"

using namespace smp;

namespace
{
	static config::SerializedJsValue SerializeJsValue(JSContext* cx, JS::HandleValue jsValue)
	{
		config::SerializedJsValue serializedValue;

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

	static void DeserializeJsValue(JSContext* cx, const config::SerializedJsValue& serializedValue, JS::MutableHandleValue jsValue)
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

namespace mozjs
{
	FbProperties::FbProperties(JSContext* cx, js_panel_window& parentPanel) : pJsCtx_(cx), parentPanel_(parentPanel) {}

	std::unique_ptr<FbProperties> FbProperties::Create(JSContext* cx, js_panel_window& parentPanel)
	{
		return std::unique_ptr<FbProperties>(new FbProperties(cx, parentPanel));
	}

	void FbProperties::Trace(JSTracer* trc)
	{
		for (auto& [name, heapElem] : properties_)
		{
			JS::TraceEdge(trc, &heapElem->value, "CustomHeap_Properties");
		}
	}

	void FbProperties::PrepareForGc()
	{
		properties_.clear();
	}

	JS::Value FbProperties::GetProperty(const std::wstring& propName, JS::HandleValue propDefaultValue)
	{
		bool hasProperty = false;
		if (properties_.contains(propName))
		{
			hasProperty = true;
		}
		else
		{
			auto& panelPropertyValues = parentPanel_.GetPanelProperties().values;

			auto it = panelPropertyValues.find(propName);
			if (it != panelPropertyValues.end())
			{
				hasProperty = true;

				JS::RootedValue jsProp(pJsCtx_);
				DeserializeJsValue(pJsCtx_, *it->second, &jsProp);
				properties_.emplace(propName, std::make_unique<HeapElement>(jsProp));
			}
		}

		if (!hasProperty)
		{
			if (propDefaultValue.isNullOrUndefined())
			{ // Not a error: user does not want to set default value
				return JS::NullValue();
			}

			SetProperty(propName, propDefaultValue);
		}

		return properties_[propName]->value.get();
	}

	void FbProperties::SetProperty(const std::wstring& propName, JS::HandleValue propValue)
	{
		auto& panelPropertyValues = parentPanel_.GetPanelProperties().values;

		if (propValue.isNullOrUndefined())
		{
			panelPropertyValues.erase(propName);
			properties_.erase(propName);
			return;
		}

		auto serializedValue = SerializeJsValue(pJsCtx_, propValue);

		properties_.insert_or_assign(propName, std::make_unique<HeapElement>(propValue));
		panelPropertyValues.insert_or_assign(propName, std::make_shared<smp::config::SerializedJsValue>(serializedValue));
	}
}
