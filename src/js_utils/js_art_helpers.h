#pragma once

namespace mozjs::art
{

/// @throw smp::JsException
[[nodiscard]] JSObject* GetAlbumArtPromise(
	JSContext* cx,
	HWND hWnd,
	const metadb_handle_ptr& handle,
	uint32_t art_id,
	bool want_stub,
	bool only_embed
);

} // namespace mozjs::art
