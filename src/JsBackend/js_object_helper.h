#pragma once

namespace mozjs
{
	constexpr uint32_t DefaultClassFlagsWithSlots(uint32_t additionalSlotCount)
	{
		// one slot is always reserved for native object
		return JSCLASS_HAS_RESERVED_SLOTS(1u + additionalSlotCount) | JSCLASS_BACKGROUND_FINALIZE;
	}

	inline constexpr uint32_t kReservedObjectSlot = 0u;
	inline constexpr uint32_t kDefaultClassFlags = DefaultClassFlagsWithSlots(0u);
	inline constexpr uint8_t kDefaultPropsFlags = JSPROP_ENUMERATE | JSPROP_PERMANENT;

	/// @details Used to define write-only property with JS_PSGS
	bool DummyGetter(JSContext* ctx, unsigned argc, JS::Value* vp);

	const void* GetSmpProxyFamily();

	template <typename JsObjectType, typename... ArgsType>
	void CreateAndInstallObject(JSContext* ctx, JS::HandleObject parentObject, const std::string& propertyName, ArgsType&&... args)
	{
		JS::RootedObject objectToInstall(ctx, JsObjectType::CreateJs(ctx, args...));

		if (!JS_DefineProperty(ctx, parentObject, propertyName.c_str(), objectToInstall, kDefaultPropsFlags))
		{
			throw JsException();
		}
	}
}
