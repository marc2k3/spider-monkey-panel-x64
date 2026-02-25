#pragma once

namespace smp
{
	class HeartbeatWindow
	{
	public:
		~HeartbeatWindow();
		HeartbeatWindow(HeartbeatWindow&) = delete;
		HeartbeatWindow& operator=(HeartbeatWindow&) = delete;

		/// @throw QwrException
		[[nodiscard]] static std::unique_ptr<HeartbeatWindow> Create();

		[[nodiscard]] HWND GetHwnd() const;

	private:
		HeartbeatWindow(HWND wnd);

		static LRESULT CALLBACK WndProc(HWND wnd, UINT message, WPARAM wParam, LPARAM lParam);

	private:
		HWND m_wnd{};
	};
}
