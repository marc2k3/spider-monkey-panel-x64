#pragma once

namespace mozjs
{
	class JsGdiBitmap : public JsObjectBase<JsGdiBitmap>
	{
	public:
		~JsGdiBitmap() override = default;

		DEFINE_JS_INTERFACE_VARS_GLOBAL_PROTO

		static std::unique_ptr<JsGdiBitmap> CreateNative(JSContext* cx, std::unique_ptr<Gdiplus::Bitmap> gdiBitmap);
		uint32_t GetInternalSize();

	public:
		[[nodiscard]] Gdiplus::Bitmap* GdiBitmap() const;

	public:
		static JSObject* Constructor(JSContext* cx, JsGdiBitmap* other);

	public:
		JSObject* ApplyAlpha(uint8_t alpha);
		void ApplyMask(JsGdiBitmap* mask);
		JSObject* Clone(float x, float y, float w, float h);
		JSObject* CreateRawBitmap();
		JS::Value GetColourScheme(uint32_t count);
		std::string GetColourSchemeJSON(uint32_t count);
		JSObject* GetGraphics();
		JSObject* InvertColours();
		void ReleaseGraphics(JsGdiGraphics* graphics);
		JSObject* Resize(uint32_t w, uint32_t h, uint32_t interpolationMode = 0);
		JSObject* ResizeWithOpt(size_t optArgCount, uint32_t w, uint32_t h, uint32_t interpolationMode);
		void RotateFlip(uint32_t mode);
		bool SaveAs(const std::wstring& path, const std::wstring& format = L"image/png");
		bool SaveAsWithOpt(size_t optArgCount, const std::wstring& path, const std::wstring& format);
		void StackBlur(uint32_t radius);

	public:
		std::uint32_t get_Height();
		std::uint32_t get_Width();

	private:
		JsGdiBitmap(JSContext* cx, std::unique_ptr<Gdiplus::Bitmap> gdiBitmap);

	private:
		JSContext* pJsCtx_ = nullptr;

		std::unique_ptr<Gdiplus::Bitmap> pGdi_;
	};
}
