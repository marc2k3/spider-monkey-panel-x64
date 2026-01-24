#pragma once

namespace mozjs
{
	class JsFbMetadbHandleList;

	class JsFbMetadbHandleList_Iterator : public JsObjectBase<JsFbMetadbHandleList_Iterator>
	{
	public:
		~JsFbMetadbHandleList_Iterator() override;

		DEFINE_JS_INTERFACE_VARS

		static std::unique_ptr<JsFbMetadbHandleList_Iterator> CreateNative(JSContext* cx, JsFbMetadbHandleList& handleList);
		uint32_t GetInternalSize();

	public:
		JSObject* Next();

	private:
		JsFbMetadbHandleList_Iterator(JSContext* cx, JsFbMetadbHandleList& handleList);

	private:
		JSContext* pJsCtx_ = nullptr;
		JsFbMetadbHandleList& handleList_;

		HeapHelper heapHelper_;
		std::optional<uint32_t> jsNextId_;

		uint32_t curPosition_ = 0;
	};
}
