#pragma once

namespace mozjs::convert::to_js
{
	template <typename T>
	void ToValue(JSContext* ctx, const std::reference_wrapper<T>& inValue, JS::MutableHandleValue wrappedValue)
	{
		ToValue(ctx, inValue.get(), wrappedValue);
	}

	template <typename T>
	void ToValue(JSContext* ctx, JS::Handle<T> inValue, JS::MutableHandleValue wrappedValue)
	{
		static_assert(0, "Unsupported type");
	}

	template <>
	void ToValue(JSContext* ctx, JS::HandleObject inValue, JS::MutableHandleValue wrappedValue);

	template <>
	void ToValue(JSContext* ctx, JS::HandleValue inValue, JS::MutableHandleValue wrappedValue);

	template <typename T>
	void ToValue(JSContext* ctx, const T& inValue, JS::MutableHandleValue wrappedValue)
	{
		static_assert(0, "Unsupported type");
	}

	template <>
	void ToValue(JSContext* ctx, const bool& inValue, JS::MutableHandleValue wrappedValue);

	template <>
	void ToValue(JSContext* ctx, const uint8_t& inValue, JS::MutableHandleValue wrappedValue);

	template <>
	void ToValue(JSContext* ctx, const int32_t& inValue, JS::MutableHandleValue wrappedValue);

	template <>
	void ToValue(JSContext* ctx, const uint32_t& inValue, JS::MutableHandleValue wrappedValue);

	/// @details Returns only approximate uint64_t value, use with care!
	template <>
	void ToValue(JSContext* ctx, const uint64_t& inValue, JS::MutableHandleValue wrappedValue);

	template <>
	void ToValue(JSContext* ctx, const double& inValue, JS::MutableHandleValue wrappedValue);

	template <>
	void ToValue(JSContext* ctx, const float& inValue, JS::MutableHandleValue wrappedValue);

	template <>
	void ToValue(JSContext* ctx, const std::string& inValue, JS::MutableHandleValue wrappedValue);

	template <>
	void ToValue(JSContext* ctx, const std::wstring& inValue, JS::MutableHandleValue wrappedValue);

	template <>
	void ToValue(JSContext* ctx, const std::nullptr_t& inValue, JS::MutableHandleValue wrappedValue);

	template <>
	void ToValue(JSContext* ctx, const metadb_handle_ptr& inValue, JS::MutableHandleValue wrappedValue);

	template <>
	void ToValue(JSContext* ctx, const metadb_handle_list& inValue, JS::MutableHandleValue wrappedValue);

	template <>
	void ToValue(JSContext* ctx, const metadb_handle_list_cref& inValue, JS::MutableHandleValue wrappedValue);

	template <>
	void ToValue(JSContext* ctx, const t_playback_queue_item& inValue, JS::MutableHandleValue wrappedValue);

	template <typename T>
	void ToValue(JSContext* ctx, std::unique_ptr<T> inValue, JS::MutableHandleValue wrappedValue)
	{
		static_assert(0, "Unsupported type");
	}

	template <>
	void ToValue(JSContext* ctx, std::unique_ptr<Gdiplus::Bitmap> inValue, JS::MutableHandleValue wrappedValue);

	template <typename T>
	void ToValue(JSContext* ctx, std::shared_ptr<T> inValue, JS::MutableHandleValue wrappedValue)
	{
		if (!inValue)
		{
			wrappedValue.setNull();
			return;
		}

		ToValue(ctx, *inValue, wrappedValue);
	}

	template <typename T, typename F>
	void ToArrayValue(JSContext* ctx, const T& inContainer, F&& accessorFunc, JS::MutableHandleValue wrappedValue)
	{
		JS::RootedObject jsArray(ctx, JS::NewArrayObject(ctx, inContainer.size()));
		JsException::ExpectTrue(jsArray);

		JS::RootedValue jsValue(ctx);
		const auto size = sizeu(inContainer);

		for (const auto i : indices(size))
		{
			ToValue(ctx, accessorFunc(inContainer, i), &jsValue);
			if (!JS_SetElement(ctx, jsArray, i, jsValue))
			{
				throw JsException();
			}
		}

		wrappedValue.set(JS::ObjectValue(*jsArray));
	}

	template <typename T>
	void ToArrayValue(JSContext* ctx, const T& inContainer, JS::MutableHandleValue wrappedValue)
	{
		JS::RootedObject jsArray(ctx, JS::NewArrayObject(ctx, inContainer.size()));
		JsException::ExpectTrue(jsArray);

		JS::RootedValue jsValue(ctx);
		for (const auto& [i, elem]: std::views::enumerate(inContainer))
		{
			ToValue(ctx, elem, &jsValue);

			if (!JS_SetElement(ctx, jsArray, static_cast<uint32_t>(i), jsValue))
			{
				throw JsException();
			}
		}

		wrappedValue.set(JS::ObjectValue(*jsArray));
	}
}
