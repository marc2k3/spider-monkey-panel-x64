#pragma once

namespace mozjs
{
	[[nodiscard]] JSObject* GetImagePromise(JSContext* cx, HWND hWnd, const std::wstring& imagePath);
}
