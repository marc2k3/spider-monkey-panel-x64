#include "PCH.hpp"
#include "fb_playback_queue_item.h"
#include "fb_metadb_handle.h"

namespace
{
	using namespace mozjs;

	DEFINE_JS_CLASS_OPS(JsFbPlaybackQueueItem::FinalizeJsObject)

	DEFINE_JS_CLASS_NO_FUNCTIONS("FbPlaybackQueueItem")

	MJS_DEFINE_JS_FN_FROM_NATIVE(get_Handle, JsFbPlaybackQueueItem::get_Handle)
	MJS_DEFINE_JS_FN_FROM_NATIVE(get_PlaylistIndex, JsFbPlaybackQueueItem::get_PlaylistIndex)
	MJS_DEFINE_JS_FN_FROM_NATIVE(get_PlaylistItemIndex, JsFbPlaybackQueueItem::get_PlaylistItemIndex)

	constexpr auto jsProperties = std::to_array<JSPropertySpec>(
		{
			JS_PSG("Handle", get_Handle, kDefaultPropsFlags),
			JS_PSG("PlaylistIndex", get_PlaylistIndex, kDefaultPropsFlags),
			JS_PSG("PlaylistItemIndex", get_PlaylistItemIndex, kDefaultPropsFlags),
			JS_PS_END,
		});
}

namespace mozjs
{
	const JSClass JsFbPlaybackQueueItem::JsClass = jsClass;
	const JSFunctionSpec* JsFbPlaybackQueueItem::JsFunctions = jsFunctions.data();
	const JSPropertySpec* JsFbPlaybackQueueItem::JsProperties = jsProperties.data();
	const JsPrototypeId JsFbPlaybackQueueItem::PrototypeId = JsPrototypeId::FbPlaybackQueueItem;

	JsFbPlaybackQueueItem::JsFbPlaybackQueueItem(JSContext* cx, const t_playback_queue_item& playbackQueueItem)
		: pJsCtx_(cx)
		, playbackQueueItem_(playbackQueueItem) {}

	bool JsFbPlaybackQueueItem::ValidateIndexes() const
	{
		return playbackQueueItem_.m_playlist < fb2k::api::pm->get_playlist_count()
			&& playbackQueueItem_.m_item < fb2k::api::pm->playlist_get_item_count(playbackQueueItem_.m_playlist);
	}

	std::unique_ptr<JsFbPlaybackQueueItem> JsFbPlaybackQueueItem::CreateNative(JSContext* cx, const t_playback_queue_item& playbackQueueItem)
	{
		return std::unique_ptr<JsFbPlaybackQueueItem>(new JsFbPlaybackQueueItem(cx, playbackQueueItem));
	}

	uint32_t JsFbPlaybackQueueItem::GetInternalSize()
	{
		return sizeof(t_playback_queue_item);
	}

	JSObject* JsFbPlaybackQueueItem::get_Handle()
	{
		return JsFbMetadbHandle::CreateJs(pJsCtx_, playbackQueueItem_.m_handle);
	}

	int32_t JsFbPlaybackQueueItem::get_PlaylistIndex()
	{
		if (ValidateIndexes())
			return to_int(playbackQueueItem_.m_playlist);
		else
			return -1;
	}

	int32_t JsFbPlaybackQueueItem::get_PlaylistItemIndex()
	{
		if (ValidateIndexes())
			return to_int(playbackQueueItem_.m_item);
		else
			return -1;
	}
}
