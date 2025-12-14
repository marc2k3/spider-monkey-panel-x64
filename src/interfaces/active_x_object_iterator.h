#pragma once

namespace mozjs
{
	class JsActiveXObject;

	class JsActiveXObject_Iterator : public JsObjectBase<JsActiveXObject_Iterator>
	{
	public:
		~JsActiveXObject_Iterator() override;

		DEFINE_JS_INTERFACE_VARS

		static std::unique_ptr<JsActiveXObject_Iterator> CreateNative(JSContext* cx, JsActiveXObject& activeXObject);
		uint32_t GetInternalSize();

	public:
		JSObject* Next();

	public:
		static bool IsIterable(const JsActiveXObject& activeXObject);

	private:
		// alias for IEnumVARIANTPtr: don't want to drag extra com headers
		using EnumVARIANTComPtr = _com_ptr_t<_com_IIID<IEnumVARIANT, &__uuidof(IEnumVARIANT)>>;

		JsActiveXObject_Iterator(JSContext* cx, EnumVARIANTComPtr pEnum);

		void LoadCurrentElement();

	private:
		JSContext* pJsCtx_ = nullptr;
		EnumVARIANTComPtr pEnum_ = nullptr;

		HeapHelper heapHelper_;
		std::optional<uint32_t> jsNextId_;

		_variant_t curElem_;
		bool isAtEnd_ = true;
	};
}
