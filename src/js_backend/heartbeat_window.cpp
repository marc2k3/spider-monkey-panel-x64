#include "PCH.hpp"
#include "heartbeat_window.h"

#include <panel/user_message.h>

namespace smp
{
	HeartbeatWindow::HeartbeatWindow(HWND wnd) : m_wnd(wnd) {};

	HeartbeatWindow ::~HeartbeatWindow()
	{
		DestroyWindow(m_wnd);
	};

	std::unique_ptr<HeartbeatWindow> HeartbeatWindow::Create()
	{
		static const wchar_t* class_name = L"DUMMY_CLASS";
		WNDCLASSEX wx = { 0 };
		wx.cbSize = sizeof(WNDCLASSEX);
		wx.lpfnWndProc = WndProc;
		wx.lpszClassName = class_name;

		ATOM atom = RegisterClassEx(&wx);
		smp::CheckWinApi(!!atom, "RegisterClassEx");

		HWND hWnd = CreateWindowEx(0, MAKEINTATOM(atom), nullptr, 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, nullptr, nullptr);
		smp::CheckWinApi(hWnd, "CreateWindowEx");

		return std::unique_ptr<HeartbeatWindow>(new HeartbeatWindow(hWnd));
	}

	HWND HeartbeatWindow::GetHwnd() const
	{
		return m_wnd;
	}

	LRESULT CALLBACK HeartbeatWindow::WndProc(HWND wnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
		case WM_CREATE:
		case WM_DESTROY:
		{
			return 0;
		}
		case std::to_underlying(MiscMessage::heartbeat):
		{
			mozjs::JsEngine::GetInstance().OnHeartbeat();
			return 0;
		}
		default:
		{
			return DefWindowProcW(wnd, message, wParam, lParam);
		}
		}
	}
}
