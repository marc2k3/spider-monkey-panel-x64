#pragma once

namespace smp
{
	[[nodiscard]] std::unique_ptr<Gdiplus::Bitmap> LoadWithWIC(IStream* stream);
}
