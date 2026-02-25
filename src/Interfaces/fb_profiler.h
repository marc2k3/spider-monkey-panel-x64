#pragma once

namespace mozjs
{
	class JsFbProfiler : public JsObjectBase<JsFbProfiler>
	{
	public:
		~JsFbProfiler() override = default;

		DEFINE_JS_INTERFACE_VARS_GLOBAL_PROTO

		static std::unique_ptr<JsFbProfiler> CreateNative(JSContext* cx, const std::string& name);
		uint32_t GetInternalSize();

	public: // ctor
		static JSObject* Constructor(JSContext* cx, const std::string& name = "");
		static JSObject* ConstructorWithOpt(JSContext* cx, size_t optArgCount, const std::string& name);

	public:
		void Print(const std::string& additionalMsg = "", bool printComponentInfo = true);
		void PrintWithOpt(size_t optArgCount, const std::string& additionalMsg, bool printComponentInfo);
		void Reset();

	public:
		uint32_t get_Time();

	private:
		JsFbProfiler(JSContext* cx, const std::string& name);

	private:
		[[maybe_unused]] JSContext* pJsCtx_ = nullptr;
		std::string name_;
		pfc::hires_timer timer_;
	};
}
