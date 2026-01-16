#pragma once

namespace mozjs
{
	[[nodiscard]] JSObject* GetImagePromise(JSContext* ctx, HWND hWnd, const std::wstring& imagePath);
}
