#pragma once

namespace mozjs::convert::to_native
{
	namespace internal
	{
		template <class T>
		inline constexpr bool IsJsSimpleConvertableImplV = std::disjunction_v<std::is_fundamental<T>, std::is_same<std::string, T>, std::is_same<std::wstring, T>>;

		template <class T>
		inline constexpr bool IsJsSimpleConvertableV = IsJsSimpleConvertableImplV<std::remove_cv_t<T>>;

		template <typename T>
		T ToSimpleValue(JSContext* ctx, const JS::HandleObject& jsObject)
		{
			auto pNative = std::remove_pointer_t<T>::ExtractNative(ctx, jsObject);
			QwrException::ExpectTrue(pNative, "Object is not of valid type");

			return pNative;
		}

		template <typename T>
		T ToSimpleValue(JSContext* ctx, const JS::HandleValue& jsValue)
		{
			static_assert(smp::always_false_v<T>, "Unsupported type");
		}

		template <>
		bool ToSimpleValue<bool>(JSContext* ctx, const JS::HandleValue& jsValue);

		template <>
		int32_t ToSimpleValue<int32_t>(JSContext* ctx, const JS::HandleValue& jsValue);

		template <>
		uint8_t ToSimpleValue<uint8_t>(JSContext* ctx, const JS::HandleValue& jsValue);

		template <>
		uint32_t ToSimpleValue<uint32_t>(JSContext* ctx, const JS::HandleValue& jsValue);

		/// @details Returns only approximate uint64_t value, use with care!
		template <>
		uint64_t ToSimpleValue<uint64_t>(JSContext* ctx, const JS::HandleValue& jsValue);

		template <>
		float ToSimpleValue<float>(JSContext* ctx, const JS::HandleValue& jsValue);

		template <>
		double ToSimpleValue<double>(JSContext* ctx, const JS::HandleValue& jsValue);

		template <>
		std::string ToSimpleValue<std::string>(JSContext* ctx, const JS::HandleValue& jsValue);

		template <>
		std::wstring ToSimpleValue<std::wstring>(JSContext* ctx, const JS::HandleValue& jsValue);

		template <>
		std::nullptr_t ToSimpleValue<std::nullptr_t>(JSContext* ctx, const JS::HandleValue& jsValue);

		template <typename T>
		std::vector<T> ToVector(JSContext* ctx, JS::HandleObject jsObject)
		{
			std::vector<T> nativeValues;
			ProcessArray<T>(ctx, jsObject, [&nativeValues](T&& nativeValue) { nativeValues.push_back(std::forward<T>(nativeValue)); });

			return nativeValues;
		}

		template <typename T>
		std::vector<T> ToVector(JSContext* ctx, JS::HandleValue jsValue)
		{
			JS::RootedObject jsObject(ctx, jsValue.toObjectOrNull());
			QwrException::ExpectTrue(jsObject, "Value is not a JS object");

			return ToVector<T>(ctx, jsObject);
		}

		template <typename T, typename F>
		void ProcessArray(JSContext* ctx, JS::HandleObject jsObject, F&& workerFunc)
		{
			bool is{};
			if (!JS::IsArrayObject(ctx, jsObject, &is) || !is)
				throw QwrException("Not a valid JS array");

			uint32_t arraySize{};
			if (!JS::GetArrayLength(ctx, jsObject, &arraySize) || arraySize == 0)
				return;

			JS::RootedValue arrayElement(ctx);
			for (const auto i : indices(arraySize))
			{
				const bool ok = JS_GetElement(ctx, jsObject, i, &arrayElement);
				QwrException::ExpectTrue(ok, "Internal error: JS_GetElement failed");

				if constexpr (std::is_same_v<T, std::string>)
				{
					QwrException::ExpectTrue(arrayElement.isString(), "arrayElement is not a string");
				}

				workerFunc(ToValue<T>(ctx, arrayElement));
			}
		}
	} // namespace internal

	template <typename T>
	T ToValue(JSContext* ctx, JS::HandleValue jsValue)
	{
		if constexpr (internal::IsJsSimpleConvertableV<T>)
		{
			return internal::ToSimpleValue<T>(ctx, jsValue);
		}
		else if constexpr (std::is_pointer_v<T>)
		{
			if (!jsValue.isObjectOrNull())
			{
				throw QwrException("Value is not a JS object");
			}

			if (jsValue.isNull())
			{
				// Not an error: null might be a valid argument
				return T{};
			}

			JS::RootedObject jsObject(ctx, &jsValue.toObject());
			return internal::ToSimpleValue<T>(ctx, jsObject);
		}
		else if constexpr (smp::is_specialization_of_v<T, std::vector>)
		{
			return internal::ToVector<typename T::value_type>(ctx, jsValue);
		}
		else
		{
			static_assert(smp::always_false_v<T>, "Unsupported type");
		}
	}

	template <typename T>
	T ToValue(JSContext* ctx, const JS::HandleString& jsString)
	{
		static_assert(0, "Unsupported type");
	}

	template <>
	std::string ToValue(JSContext* ctx, const JS::HandleString& jsString);

	template <>
	std::wstring ToValue(JSContext* ctx, const JS::HandleString& jsString);

	template <typename T, typename F>
	void ProcessArray(JSContext* ctx, JS::HandleValue jsValue, F&& workerFunc)
	{
		if (jsValue.isObject())
		{
			auto jsObject = JS::RootedObject(ctx, jsValue.toObjectOrNull());
			if (jsObject)
			{
				internal::ProcessArray<T>(ctx, jsObject, std::forward<F>(workerFunc));
				return;
			}
		}

		throw QwrException("Not a valid JS array");
	}
}
