#pragma once

namespace mozjs
{
	class JsGdiBitmap : public JsObjectBase<JsGdiBitmap>
	{
	public:
		~JsGdiBitmap() override = default;

		DEFINE_JS_INTERFACE_VARS_GLOBAL_PROTO

		static std::unique_ptr<JsGdiBitmap> CreateNative(JSContext* ctx, std::unique_ptr<Gdiplus::Bitmap> bitmap);
		uint32_t GetInternalSize();

	public:
		[[nodiscard]] Gdiplus::Bitmap* GdiBitmap() const;

	public:
		static JSObject* Constructor(JSContext* ctx, JsGdiBitmap* other);

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
		void StackBlur(uint8_t radius);

	public:
		uint32_t get_Height();
		uint32_t get_Width();

	private:
		JsGdiBitmap(JSContext* ctx, std::unique_ptr<Gdiplus::Bitmap> gdiBitmap);

		std::unique_ptr<Gdiplus::Bitmap> ApplyAttributes(const Gdiplus::ImageAttributes& ia);

	private:
		JSContext* m_ctx{};
		std::unique_ptr<Gdiplus::Bitmap> m_bitmap;
	};
}
