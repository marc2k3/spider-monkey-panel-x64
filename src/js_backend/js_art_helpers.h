#pragma once

namespace mozjs
{
	/// @throw JsException
	[[nodiscard]] JSObject* GetAlbumArtPromise(
		JSContext* ctx,
		HWND hWnd,
		const metadb_handle_ptr& handle,
		uint32_t art_id,
		bool want_stub,
		bool only_embed
	);
}
