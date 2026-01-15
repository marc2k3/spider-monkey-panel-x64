#include "PCH.hpp"
#include "fb_playlist_recycler.h"
#include "fb_metadb_handle_list.h"

namespace
{
	using namespace mozjs;

	DEFINE_JS_CLASS_OPS(JsFbPlaylistRecycler::FinalizeJsObject)

	DEFINE_JS_CLASS("FbPlaylistRecycler")

	MJS_DEFINE_JS_FN_FROM_NATIVE(GetContent, JsFbPlaylistRecycler::GetContent)
	MJS_DEFINE_JS_FN_FROM_NATIVE(GetName, JsFbPlaylistRecycler::GetName)
	MJS_DEFINE_JS_FN_FROM_NATIVE(Purge, JsFbPlaylistRecycler::Purge)
	MJS_DEFINE_JS_FN_FROM_NATIVE(Restore, JsFbPlaylistRecycler::Restore)

	constexpr auto jsFunctions = std::to_array<JSFunctionSpec>(
		{
			JS_FN("GetContent", GetContent, 1, kDefaultPropsFlags),
			JS_FN("GetName", GetName, 1, kDefaultPropsFlags),
			JS_FN("Purge", Purge, 1, kDefaultPropsFlags),
			JS_FN("Restore", Restore, 1, kDefaultPropsFlags),
			JS_FS_END,
		});

	MJS_DEFINE_JS_FN_FROM_NATIVE(get_Count, JsFbPlaylistRecycler::get_Count)

	constexpr auto jsProperties = std::to_array<JSPropertySpec>(
		{
			JS_PSG("Count", get_Count, kDefaultPropsFlags),
			JS_PS_END,
		});
}

namespace mozjs
{
	const JSClass JsFbPlaylistRecycler::JsClass = jsClass;
	const JSFunctionSpec* JsFbPlaylistRecycler::JsFunctions = jsFunctions.data();
	const JSPropertySpec* JsFbPlaylistRecycler::JsProperties = jsProperties.data();

	JsFbPlaylistRecycler::JsFbPlaylistRecycler(JSContext* cx) : pJsCtx_(cx) {}

	std::unique_ptr<JsFbPlaylistRecycler> JsFbPlaylistRecycler::CreateNative(JSContext* cx)
	{
		return std::unique_ptr<JsFbPlaylistRecycler>(new JsFbPlaylistRecycler(cx));
	}

	uint32_t JsFbPlaylistRecycler::GetInternalSize()
	{
		return 0;
	}

	JSObject* JsFbPlaylistRecycler::GetContent(uint32_t index)
	{
		const auto count = fb2k::api::pm->recycler_get_count();
		QwrException::ExpectTrue(index < count, "Index is out of bounds");

		metadb_handle_list handles;
		fb2k::api::pm->recycler_get_content(index, handles);

		return JsFbMetadbHandleList::CreateJs(pJsCtx_, handles);
	}

	pfc::string8 JsFbPlaylistRecycler::GetName(uint32_t index)
	{
		const auto count = fb2k::api::pm->recycler_get_count();
		QwrException::ExpectTrue(index < count, "Index is out of bounds");

		pfc::string8 name;
		fb2k::api::pm->recycler_get_name(index, name);
		return name;
	}

	void JsFbPlaylistRecycler::Purge(JS::HandleValue affectedItems)
	{
		pfc::bit_array_bittable affected(fb2k::api::pm->recycler_get_count());
		convert::to_native::ProcessArray<uint32_t>(pJsCtx_, affectedItems, [&affected](uint32_t index) { affected.set(index, true); });
		fb2k::api::pm->recycler_purge(affected);
	}

	void JsFbPlaylistRecycler::Restore(uint32_t index)
	{
		const auto count = fb2k::api::pm->recycler_get_count();
		QwrException::ExpectTrue(index < count, "Index is out of bounds");

		fb2k::api::pm->recycler_restore(index);
	}

	uint32_t JsFbPlaylistRecycler::get_Count()
	{
		return to_uint(fb2k::api::pm->recycler_get_count());
	}
}
