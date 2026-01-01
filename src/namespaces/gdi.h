#pragma once

namespace mozjs
{
	class Gdi : public JsObjectBase<Gdi>
	{
	public:
		~Gdi() override = default;

		DEFINE_JS_NAMESPACE_VARS

		static std::unique_ptr<Gdi> CreateNative(JSContext* ctx);
		uint32_t GetInternalSize();

	public:
		JSObject* CreateImage(uint32_t w, uint32_t h);
		JSObject* Font(const std::wstring& fontName, uint32_t pxSize, uint32_t style = 0);
		JSObject* FontWithOpt(size_t optArgCount, const std::wstring& fontName, uint32_t pxSize, uint32_t style);
		JSObject* Image(const std::wstring& path);
		uint32_t LoadImageAsync(uint32_t /*window_id*/, const std::wstring& path);
		JSObject* LoadImageAsyncV2(uint32_t /*window_id*/, const std::wstring& path);

	private:
		Gdi(JSContext* ctx);

	private:
		JSContext* m_ctx{};
	};
}
