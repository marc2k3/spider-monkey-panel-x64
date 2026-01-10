#pragma once

namespace mozjs
{
	class JsFbPlaybackQueueItem : public JsObjectBase<JsFbPlaybackQueueItem>
	{
	public:
		~JsFbPlaybackQueueItem() override = default;

		DEFINE_JS_INTERFACE_VARS

		static std::unique_ptr<JsFbPlaybackQueueItem> CreateNative(JSContext* cx, const t_playback_queue_item& playbackQueueItem);
		uint32_t GetInternalSize();

	public:
		JSObject* get_Handle();
		int32_t get_PlaylistIndex();
		int32_t get_PlaylistItemIndex();

	private:
		JsFbPlaybackQueueItem(JSContext* cx, const t_playback_queue_item& playbackQueueItem);

		bool ValidateIndexes() const;

	private:
		JSContext* pJsCtx_ = nullptr;
		t_playback_queue_item playbackQueueItem_;
	};
}
