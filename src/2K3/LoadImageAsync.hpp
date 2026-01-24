#pragma once
#include "FileHelper.hpp"

#include <events/event_dispatcher.h>
#include <events/event_js_callback.h>

class LoadImageAsync : public fb2k::threadEntry
{
public:
	LoadImageAsync(HWND wnd, std::wstring_view path, uint32_t task_id)
		: m_wnd(wnd)
		, m_path(path)
		, m_task_id(task_id) {}

	void run() noexcept final
	{
		auto bitmap = FileHelper(m_path).load_image();

		smp::EventDispatcher::Get().PutEvent(
			m_wnd,
			GenerateEvent_JsCallback(
				smp::EventId::kInternalLoadImageDone,
				m_task_id,
				std::move(bitmap),
				m_path
			)
		);
	}

private:
	HWND m_wnd;
	std::wstring m_path;
	uint32_t m_task_id{};
};
