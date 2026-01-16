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
		pfc::string8 Eval(bool force = false);
		pfc::string8 EvalWithOpt(size_t optArgCount, bool force);
		pfc::string8 EvalWithMetadb(JsFbMetadbHandle* handle, bool want_full_info = false);
		pfc::string8 EvalWithMetadbWithOpt(size_t optArgCount, JsFbMetadbHandle* handle, bool want_full_info);
		JS::Value EvalWithMetadbs(JsFbMetadbHandleList* handles);

	private:
		JsFbTitleFormat(JSContext* cx, const std::string& expr);

	private:
		JSContext* m_ctx{};
		titleformat_object::ptr m_obj;
	};
}
