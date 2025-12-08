#pragma once
#include "AlbumArtStatic.hpp"

#include <events/event_dispatcher.h>
#include <events/event_js_callback.h>

class GetAlbumArtAsync : public fb2k::threadEntry
{
public:
	GetAlbumArtAsync(HWND wnd, const metadb_handle_ptr& handle, size_t type_id, bool want_stub, bool only_embed)
		: m_wnd(wnd)
		, m_handle(handle)
		, m_type_id(type_id)
		, m_want_stub(want_stub)
		, m_only_embed(only_embed) {}

	void run() noexcept final
	{
		if (m_handle.is_empty())
			return;

		std::string path;
		auto data = AlbumArtStatic::get(m_handle, m_type_id, m_want_stub, m_only_embed, path);
		auto bitmap = AlbumArtStatic::to_bitmap(data);

		smp::EventDispatcher::Get().PutEvent(
			m_wnd,
			GenerateEvent_JsCallback(
				smp::EventId::kInternalGetAlbumArtDone,
				m_handle,
				m_type_id,
				std::move(bitmap),
				path
			)
		);
	}

private:
	HWND m_wnd;
	metadb_handle_ptr m_handle;
	bool m_want_stub{}, m_only_embed{};
	size_t m_type_id{};
};
