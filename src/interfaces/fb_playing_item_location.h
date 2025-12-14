#pragma once

namespace mozjs
{
	class JsFbPlayingItemLocation : public JsObjectBase<JsFbPlayingItemLocation>
	{
	public:
		~JsFbPlayingItemLocation() override = default;

		DEFINE_JS_INTERFACE_VARS

		static std::unique_ptr<JsFbPlayingItemLocation> CreateNative(JSContext* cx, bool is_valid, size_t playlistIndex, size_t playlistItemIndex);
		uint32_t GetInternalSize();

	public:
		bool get_IsValid();
		int32_t get_PlaylistIndex();
		int32_t get_PlaylistItemIndex();

	private:
		JsFbPlayingItemLocation(JSContext* cx, bool is_valid, size_t playlistIndex, size_t playlistItemIndex);

		JSContext* m_ctx{};
		bool m_is_valid{};
		int32_t m_playlistIndex = -1;
		int32_t m_playlistItemIndex = -1;
	};
}
