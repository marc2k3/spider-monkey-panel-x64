#pragma once
#include "prototype_ids.h"

namespace mozjs
{
	inline void* GetMaybePtrFromReservedSlot(JSObject* obj, uint32_t slot)
	{
		JS::Value v = JS::GetReservedSlot(obj, slot);
		return v.isUndefined() ? nullptr : v.toPrivate();
	}

	inline void* GetInstanceFromReservedSlot(JSContext* ctx, JS::Handle<JSObject*> obj, const JSClass* cla, JS::CallArgs* args)
	{
		if (JS_InstanceOf(ctx, obj, cla, args))
		{
			return GetMaybePtrFromReservedSlot(obj, 0u);
		}

		return nullptr;
	}

	/// @brief Create a prototype for the specified object
	///        and store it in the current global object.
	///        Created prototype is not accessible from JS.
	template <typename JsObjectType>
	void CreateAndSavePrototype(JSContext* ctx, JsPrototypeId protoId)
	{
		auto globalObject = JS::RootedObject(ctx, JS::CurrentGlobalOrNull(ctx));
		const auto slotIdx = JSCLASS_GLOBAL_SLOT_COUNT + std::to_underlying(protoId);
		auto jsProto = JS::RootedObject(ctx, JsObjectType::CreateProto(ctx));
		auto protoVal = JS::ObjectValue(*jsProto);
		JS::SetReservedSlot(globalObject, slotIdx, protoVal);
	}

	/// @brief Create a prototype for the specified object
	///        and store it in the current global object.
	///        Created prototype is accessible from JS.
	template <typename JsObjectType>
	void CreateAndInstallPrototype(JSContext* ctx, JsPrototypeId protoId)
	{
		auto globalObject = JS::RootedObject(ctx, JS::CurrentGlobalOrNull(ctx));
		const auto slotIdx = JSCLASS_GLOBAL_SLOT_COUNT + std::to_underlying(protoId);
		auto jsProto = JS::RootedObject(ctx, JsObjectType::InstallProto(ctx, globalObject));
		auto protoVal = JS::ObjectValue(*jsProto);
		JS::SetReservedSlot(globalObject, slotIdx, protoVal);
	}

	/// @brief Get the prototype for the specified object from the current global object.
	template <typename JsObjectType>
	JSObject* GetPrototype(JSContext* ctx, JsPrototypeId protoId)
	{
		auto globalObject = JS::RootedObject(ctx, JS::CurrentGlobalOrNull(ctx));
		const auto slotIdx = JSCLASS_GLOBAL_SLOT_COUNT + std::to_underlying(protoId);
		auto protoVal = JS::GetReservedSlot(globalObject, slotIdx);

		QwrException::ExpectTrue(
			protoVal.isObject(),
			"Internal error: Slot {}({}) does not contain a prototype",
			std::to_underlying(protoId),
			slotIdx
		);

		return &protoVal.toObject();
	}

	/// @brief Get the prototype for the specified object from the current global object.
	///        And create the prototype, if it's missing.
	template <typename JsObjectType>
	JSObject* GetOrCreatePrototype(JSContext* ctx, JsPrototypeId protoId)
	{
		auto globalObject = JS::RootedObject(ctx, JS::CurrentGlobalOrNull(ctx));
		const auto slotIdx = JSCLASS_GLOBAL_SLOT_COUNT + std::to_underlying(protoId);

		{ // Try fetching prototype
			auto protoVal = JS::GetReservedSlot(globalObject, slotIdx);

			if (protoVal.isObject())
			{
				return &protoVal.toObject();
			}
		}

		CreateAndSavePrototype<JsObjectType>(ctx, protoId);

		auto protoVal = JS::GetReservedSlot(globalObject, slotIdx);
		return &protoVal.toObject();
	}
}
