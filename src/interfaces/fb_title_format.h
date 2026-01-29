#pragma once

namespace mozjs
{
	class JsFbMetadbHandle;
	class JsFbMetadbHandleList;

	class JsFbTitleFormat : public JsObjectBase<JsFbTitleFormat>
	{
	public:
		~JsFbTitleFormat() override = default;

		DEFINE_JS_INTERFACE_VARS_GLOBAL_PROTO

		static std::unique_ptr<JsFbTitleFormat> CreateNative(JSContext* cx, const std::string& expr);
		uint32_t GetInternalSize();

	public:
		titleformat_object::ptr GetTitleFormat();

	public: // ctor
		static JSObject* Constructor(JSContext* cx, const std::string& expr);

	public:
		std::wstring Eval(bool force = false);
		std::wstring EvalWithOpt(size_t optArgCount, bool force);
		std::wstring EvalWithMetadb(JsFbMetadbHandle* handle, bool want_full_info = false);
		std::wstring EvalWithMetadbWithOpt(size_t optArgCount, JsFbMetadbHandle* handle, bool want_full_info);
		JS::Value EvalWithMetadbs(JsFbMetadbHandleList* handles);

	private:
		JsFbTitleFormat(JSContext* cx, const std::string& expr);

	private:
		JSContext* m_ctx{};
		titleformat_object::ptr m_obj;
	};
}
