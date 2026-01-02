#pragma once
#include "active_x_object.h"

namespace mozjs
{
	class JsEnumerator : public JsObjectBase<JsEnumerator>
	{
	public:
		~JsEnumerator() override = default;

		DEFINE_JS_INTERFACE_VARS_GLOBAL_PROTO

		static std::unique_ptr<JsEnumerator> CreateNative(JSContext* cx, IUnknown* pUnknown);
		uint32_t GetInternalSize();

	public:
		static JSObject* Constructor(JSContext* cx, JsActiveXObject* pActiveXObject);

	public:
		bool AtEnd() const;
		JS::Value Item();
		void MoveFirst();
		void MoveNext();

	private:
		// alias for IEnumVARIANTPtr: don't want to drag extra com headers
		using EnumVARIANTComPtr = _com_ptr_t<_com_IIID<IEnumVARIANT, &__uuidof(IEnumVARIANT)>>;

		JsEnumerator(JSContext* cx, EnumVARIANTComPtr pEnum);

		void LoadCurrentElement();

	private:
		JSContext* pJsCtx_ = nullptr;
		EnumVARIANTComPtr pEnum_ = nullptr;
		_variant_t curElem_;
		bool isAtEnd_ = true;
	};
}
