#pragma once

namespace mozjs
{
	class JsMeasureStringInfo : public JsObjectBase<JsMeasureStringInfo>
	{
	public:
		~JsMeasureStringInfo() override = default;

		DEFINE_JS_INTERFACE_VARS

		static std::unique_ptr<JsMeasureStringInfo> CreateNative(JSContext* cx, float x, float y, float w, float h, uint32_t l, uint32_t c);
		uint32_t GetInternalSize();

	public:
		[[nodiscard]] uint32_t get_Chars() const;
		[[nodiscard]] float get_Height() const;
		[[nodiscard]] uint32_t get_Lines() const;
		[[nodiscard]] float get_Width() const;
		[[nodiscard]] float get_X() const;
		[[nodiscard]] float get_Y() const;

	private:
		JsMeasureStringInfo(JSContext* cx, float x, float y, float w, float h, uint32_t l, uint32_t c);

	private:
		JSContext* pJsCtx_{};

		float x_{}, y_{}, w_{}, h_{};
		int32_t lines_{}, characters_{};
	};
}
