#pragma once
#include "js_object_helper.h"
#include "js_prototype_helpers.h"
#include "js_realm_inner.h"

namespace mozjs
{
	/*
		Every object must define the following traits:

		// Indicates that object is created from JS prototype.
		// If true, object must also define `HasGlobalProto` and `PrototypeId`.
		static constexpr bool HasProto;

		// Indicates that object is wrapped in proxy.
		// If true, object must also define `JsProxy`.
		static constexpr bool HasProxy;

		// Indicates that object needs to perform actions on create JS object to finalize it's construction.
		// If true, object must also define `PostCreate`.
		static constexpr bool HasPostCreate;
	*/

	/*
		Traits that object might need to define (see above):

		// Indicates that object has a global JS constructor.
		// If true, object must also define `JsConstructor` and `HasStaticFunctions`.
		static constexpr bool HasGlobalProto;

		// Indicates that object contains static methods.
		// If true, object must also define `JsStaticFunctions`.
		static constexpr bool HasStaticFunctions;
	*/

	/*
		Every object must define and initialize the following properties:

		// Object's JS class
		// Note: it MUST contain `FinalizeJsObject` from this class!
		const JSClass JsClass;

		// List of object's JS methods.
		const JSFunctionSpec* JsFunctions;

		// List of object's JS properties
		const JSPropertySpec* JsProperties;
	*/

	/*
		Properties that object might need to define (see above):

		// Unique id for the object's JS prototype
		const JsPrototypeId PrototypeId;

		// Pointer to the object's JS constructor
		const JSNative JsConstructor;

		// Reference to the object's JS proxy
		const js::BaseProxyHandler& JsProxy;

		// List of object's static JS methods.
		const JSFunctionSpec* JsStaticFunctions;
	*/

	/*
		Every object must define and initialize the following methods:

		// Creates object T.
		// Must always return a valid object.
		// Throw JsException or QwrException on error.
		static std::unique_ptr<T> CreateNative(JSContext* ctx, Args... args);

		// Returns the size of properties of T, that can't be calculated by sizeof(T).
		// E.g. if T has property `std::unique_ptr<BigStruct> bigStruct_`, then
		// `GetInternalSize` must return sizeof(bigStruct_).
		uint32_t GetInternalSize();
	*/

	/*
		Methods that object might need to define (see above):

		// Finalizes the JS object that contains T.
		// Called before JS object is wrapped in proxy (if `HasProxy` is true).
		// Throw JsException or QwrException on error.
		static void PostCreate(JSContext* ctx, JS::HandleObject self);
	*/

#define DEFINE_JS_CLASS_OPS(finalize_object) \
	JSClassOps jsOps = { \
		nullptr, \
		nullptr, \
		nullptr, \
		nullptr, \
		nullptr, \
		nullptr, \
		finalize_object, \
		nullptr, \
		nullptr, \
		nullptr, \
	}; \

#define DEFINE_JS_CLASS(what) \
	constexpr JSClass jsClass = { \
		what, \
		kDefaultClassFlags, \
		&jsOps \
	}; \

#define DEFINE_JS_CLASS_NO_FUNCTIONS(what) \
	DEFINE_JS_CLASS(what) \
	\
	constexpr auto jsFunctions = std::to_array<JSFunctionSpec>( \
		{ \
			JS_FS_END, \
		}); \

#define DEFINE_JS_CLASS_NO_PROPERTIES(what) \
	DEFINE_JS_CLASS(what) \
	\
	constexpr auto jsProperties = std::to_array<JSPropertySpec>( \
		{ \
			JS_PS_END, \
		}); \

#define DEFINE_JS_NAMESPACE_VARS \
	static constexpr bool HasProto = false; \
	static constexpr bool HasProxy = false; \
	static constexpr bool HasPostCreate = false; \
	static const JSClass JsClass; \
	static const JSFunctionSpec* JsFunctions; \
	static const JSPropertySpec* JsProperties; \

#define DEFINE_JS_INTERFACE_VARS \
	static constexpr bool HasProto = true; \
	static constexpr bool HasGlobalProto = false; \
	static constexpr bool HasProxy = false; \
	static constexpr bool HasPostCreate = false; \
	static const JSClass JsClass; \
	static const JSFunctionSpec* JsFunctions; \
	static const JSPropertySpec* JsProperties; \
	static const JsPrototypeId PrototypeId; \

#define DEFINE_JS_INTERFACE_VARS_GLOBAL_PROTO \
	static constexpr bool HasProto = true; \
	static constexpr bool HasGlobalProto = true; \
	static constexpr bool HasStaticFunctions = false; \
	static constexpr bool HasProxy = false; \
	static constexpr bool HasPostCreate = false; \
	static const JSClass JsClass; \
	static const JSFunctionSpec* JsFunctions; \
	static const JSPropertySpec* JsProperties; \
	static const JsPrototypeId PrototypeId; \
	static const JSNative JsConstructor; \

	template <typename T>
	class JsObjectBase
	{
	public:
		using Self = JsObjectBase<T>;

		JsObjectBase() = default;
		JsObjectBase(const JsObjectBase&) = delete;
		JsObjectBase& operator=(const JsObjectBase&) = delete;
		virtual ~JsObjectBase() = default;

	public:
		[[nodiscard]] static JSObject* CreateProto(JSContext* ctx)
		{
			JS::RootedObject jsObject(ctx, JS_NewPlainObject(ctx));

			if (!jsObject)
			{
				throw JsException();
			}

			if (!JS_DefineFunctions(ctx, jsObject, T::JsFunctions) || !JS_DefineProperties(ctx, jsObject, T::JsProperties))
			{
				throw JsException();
			}

			return jsObject;
		}

		[[nodiscard]] static JSObject* InstallProto(JSContext* ctx, JS::HandleObject parentObject)
		{
			const JSFunctionSpec* staticFns = []
				{
					if constexpr (T::HasStaticFunctions)
						return T::JsStaticFunctions;
					else
						return nullptr;
				}();

			auto pJsProto = JS_InitClass(
				ctx,
				parentObject,
				nullptr,
				&T::JsClass,
				T::JsConstructor,
				0,
				T::JsProperties,
				T::JsFunctions,
				nullptr,
				staticFns
			);

			if (!pJsProto)
			{
				throw JsException();
			}
			return pJsProto;
		}

		template <typename... ArgTypes>
		[[nodiscard]] static JSObject* CreateJs(JSContext* ctx, ArgTypes&&... args)
		{
			JS::RootedObject jsProto(ctx);
			if constexpr (T::HasProto)
			{
				jsProto = GetProto(ctx);
			}

			JS::RootedObject jsObject(ctx, CreateJsObject_Base(ctx, jsProto));
			std::unique_ptr<T> nativeObject = T::CreateNative(ctx, std::forward<ArgTypes>(args)...);
			nativeObject->m_native_object_size = sizeof(T) + nativeObject->GetInternalSize();
			return CreateJsObject_Final(ctx, jsProto, jsObject, std::move(nativeObject));
		}

		[[nodiscard]] static JSObject* CreateJsFromNative(JSContext* ctx, std::unique_ptr<T> nativeObject)
		{
			JS::RootedObject jsProto(ctx);
			if constexpr (T::HasProto)
			{
				jsProto = GetProto(ctx);
			}

			JS::RootedObject jsObject(ctx, CreateJsObject_Base(ctx, jsProto));
			nativeObject->m_native_object_size = sizeof(T);

			return CreateJsObject_Final(ctx, jsProto, jsObject, std::move(nativeObject));
		}

		static void FinalizeJsObject(JS::GCContext* /*gcCtx*/, JSObject* pSelf)
		{
			auto pNative = Self::ExtractNativeUnchecked(pSelf);

			if (pNative)
			{
				auto pJsRealm = static_cast<JsRealmInner*>(JS::GetRealmPrivate(js::GetNonCCWObjectRealm(pSelf)));
				if (pJsRealm)
				{
					pJsRealm->OnHeapDeallocate(pNative->Self::m_native_object_size);
				}

				delete pNative;
				JS::SetReservedSlot(pSelf, kReservedObjectSlot, JS::UndefinedValue());
			}
		}

		[[nodiscard]] static T* ExtractNative(JSContext* ctx, JS::HandleObject jsObjectHandle)
		{
			JS::RootedObject jsObject(ctx, jsObjectHandle);

			if (js::IsWrapper(jsObject))
			{
				jsObject = js::UncheckedUnwrap(jsObject);
			}

			if (js::IsProxy(jsObject) && js::GetProxyHandler(jsObject)->family() == GetSmpProxyFamily())
			{
				jsObject = js::GetProxyTargetObject(jsObject);
			}

			if (auto pNative = ExtractNativeExact(ctx, jsObject))
			{
				return pNative;
			}

			return nullptr;
		}

		[[nodiscard]] static T* ExtractNative(JSContext* ctx, JS::HandleValue jsValue)
		{
			if (jsValue.isObject())
			{
				JS::RootedObject jsObject(ctx, &jsValue.toObject());
				return Self::ExtractNative(ctx, jsObject);
			}

			return nullptr;
		}

		[[nodiscard]] static T* ExtractNativeUnchecked(JSObject* jsObject)
		{
			return Self::ExtractNativeFromVoid(GetMaybePtrFromReservedSlot(jsObject, kReservedObjectSlot));
		}

		uint32_t m_native_object_size{};

	private:
		[[nodiscard]] static T* ExtractNativeExact(JSContext* ctx, JS::HandleObject jsObject)
		{
			auto pVoid = GetInstanceFromReservedSlot(ctx, jsObject, &T::JsClass, nullptr);
			return Self::ExtractNativeFromVoid(pVoid);
		}

		[[nodiscard]] static T* ExtractNativeFromVoid(void* pVoid)
		{
			return static_cast<T*>(pVoid);
		}

		template <typename U = T, std::enable_if_t<U::HasProto, int> = 0>
		[[nodiscard]] static JSObject* GetProto(JSContext* ctx)
		{
			JS::RootedObject jsProto(ctx);
			if constexpr (T::HasGlobalProto)
			{
				jsProto = GetPrototype<T>(ctx, T::PrototypeId);
			}
			else
			{
				jsProto = GetOrCreatePrototype<T>(ctx, T::PrototypeId);
			}

			return jsProto;
		}

		[[nodiscard]] static JSObject* CreateJsObject_Base(JSContext* ctx, [[maybe_unused]] JS::HandleObject jsProto)
		{
			JS::RootedObject jsObject(ctx);
			if constexpr (T::HasProto)
			{
				jsObject.set(JS_NewObjectWithGivenProto(ctx, &T::JsClass, jsProto));
				if (!jsObject)
				{
					throw JsException();
				}
			}
			else
			{
				jsObject.set(JS_NewObject(ctx, &T::JsClass));
				if (!jsObject)
				{
					throw JsException();
				}

				if (!JS_DefineFunctions(ctx, jsObject, T::JsFunctions) || !JS_DefineProperties(ctx, jsObject, T::JsProperties))
				{
					throw JsException();
				}
			}

			return jsObject;
		}

		[[nodiscard]] static JSObject* CreateJsObject_Final(
			JSContext* ctx,
			[[maybe_unused]] JS::HandleObject jsProto,
			JS::HandleObject jsBaseObject,
			std::unique_ptr<T> premadeNative)
		{
			auto pJsRealm = static_cast<JsRealmInner*>(JS::GetRealmPrivate(js::GetContextRealm(ctx)));
			pJsRealm->OnHeapAllocate(premadeNative->m_native_object_size);

			JS::SetReservedSlot(jsBaseObject, kReservedObjectSlot, JS::PrivateValue(premadeNative.release()));

			if constexpr (T::HasPostCreate)
			{
				T::PostCreate(ctx, jsBaseObject);
			}

			if constexpr (T::HasProxy)
			{
				JS::RootedValue jsBaseValue(ctx, JS::ObjectValue(*jsBaseObject));
				JS::RootedObject jsProxyObject(ctx, js::NewProxyObject(ctx, &T::JsProxy, jsBaseValue, jsProto));
				if (!jsProxyObject)
				{
					throw JsException();
				}
				return jsProxyObject;
			}
			else
			{
				return jsBaseObject;
			}
		}
	};
}
