#pragma once

// Copied from https://github.com/reupen/ui_helpers
// Copyright (c) Reupen Shah 2003-2017
// All rights reserved.
// See THIRD_PARTY_NOTICES.md for full license text.

namespace uih
{
	bool create_drag_image(
		HWND wnd,
		std::wstring_view text,
		Gdiplus::Bitmap* bitmap,
		SHDRAGIMAGE& dragImage);
}
