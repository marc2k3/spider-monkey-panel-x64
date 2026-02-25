#pragma once

namespace smp
{
	/// @details These messages are synchronous
	enum class InternalSyncMessage : uint32_t
	{
		first_message = WM_USER + 100,
		legacy_notify_others = first_message,
		prepare_for_exit,
		run_next_event,
		script_fail,
		ui_script_editor_saved,
		wnd_drag_drop,
		wnd_drag_enter,
		wnd_drag_leave,
		wnd_drag_over,
		wnd_internal_drag_start,
		wnd_internal_drag_stop,
		last_message = wnd_internal_drag_stop,
	};

	/// @brief Message definitions that are not handled by the main panel window
	enum class MiscMessage : uint32_t
	{
		heartbeat = std::to_underlying(InternalSyncMessage::last_message) + 1u,
		key_down
	};

	constexpr bool IsInEnumRange(uint32_t value)
	{
		return value >= std::to_underlying(InternalSyncMessage::first_message) && value <= std::to_underlying(InternalSyncMessage::last_message);
	}
}
