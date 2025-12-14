#pragma once

namespace mozjs
{
	class JsFbAudioChunk : public JsObjectBase<JsFbAudioChunk>
	{
	public:
		~JsFbAudioChunk() override = default;

		DEFINE_JS_INTERFACE_VARS

		static std::unique_ptr<JsFbAudioChunk> CreateNative(JSContext* cx, const audio_chunk_impl& chunk);
		uint32_t GetInternalSize();

	public:
		JS::Value get_Data();
		uint32_t get_ChannelConfig();
		uint32_t get_ChannelCount();
		uint32_t get_SampleRate();
		size_t get_SampleCount();

	private:
		JsFbAudioChunk(JSContext* cx, const audio_chunk_impl& chunk);

	private:
		[[maybe_unused]] JSContext* pJsCtx_ = nullptr;
		audio_chunk_impl chunk_;
	};
}
