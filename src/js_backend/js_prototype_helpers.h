#pragma once
#include "prototype_ids.h"

namespace mozjs
{
	/// @brief Create a prototype for the specified object
	///        and store it in the current global object.
	///        Created prototype is not accessible from JS.
	template <typename JsObjectType>
	void CreateAndSavePrototype(JSContext* cx, JsPrototypeId protoId)
	{
		auto globalObject = JS::RootedObject(cx, JS::CurrentGlobalOrNull(cx));
		const auto slotIdx = JSCLASS_GLOBAL_SLOT_COUNT + std::to_underlying(protoId);
		auto jsProto = JS::RootedObject(cx, JsObjectType::CreateProto(cx));
		auto protoVal = JS::ObjectValue(*jsProto);
		JS::SetReservedSlot(globalObject, slotIdx, protoVal);
	}

	/// @brief Create a prototype for the specified object
	///        and store it in the current global object.
	///        Created prototype is accessible from JS.
	template <typename JsObjectType>
	void CreateAndInstallPrototype(JSContext* cx, JsPrototypeId protoId)
	{
		auto globalObject = JS::RootedObject(cx, JS::CurrentGlobalOrNull(cx));
		const auto slotIdx = JSCLASS_GLOBAL_SLOT_COUNT + std::to_underlying(protoId);
		auto jsProto = JS::RootedObject(cx, JsObjectType::InstallProto(cx, globalObject));
		auto protoVal = JS::ObjectValue(*jsProto);
		JS::SetReservedSlot(globalObject, slotIdx, protoVal);
	}

	/// @brief Get the prototype for the specified object from the current global object.
	template <typename JsObjectType>
	JSObject* GetPrototype(JSContext* cx, JsPrototypeId protoId)
	{
		auto globalObject = JS::RootedObject(cx, JS::CurrentGlobalOrNull(cx));
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
	JSObject* GetOrCreatePrototype(JSContext* cx, JsPrototypeId protoId)
	{
		auto globalObject = JS::RootedObject(cx, JS::CurrentGlobalOrNull(cx));
		const auto slotIdx = JSCLASS_GLOBAL_SLOT_COUNT + std::to_underlying(protoId);

		{ // Try fetching prototype
			auto protoVal = JS::GetReservedSlot(globalObject, slotIdx);

			if (protoVal.isObject())
			{
				return &protoVal.toObject();
			}
		}

		CreateAndSavePrototype<JsObjectType>(cx, protoId);

		auto protoVal = JS::GetReservedSlot(globalObject, slotIdx);
		return &protoVal.toObject();
	}
}
