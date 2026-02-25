#pragma once

namespace mozjs
{
	class Console : public JsObjectBase<Console>
	{
	public:
		~Console() = default;

		DEFINE_JS_NAMESPACE_VARS

		static std::unique_ptr<Console> CreateNative(JSContext* ctx);
		uint32_t GetInternalSize();

		auto ClearBacklog() -> void;
		auto GetLines(bool with_timestamp) -> JSObject*;

	private:
		Console(JSContext* ctx);

		JSContext* m_ctx{};
		fb2k::console_manager::ptr m_ptr;
	};
}
