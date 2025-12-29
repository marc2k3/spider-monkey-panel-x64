#pragma once

namespace smp
{
	using Size = std::tuple<uint32_t, uint32_t>;

	[[nodiscard]] Size GetResizedImageSize(const Size& currentDimension, const Size& maxDimensions) noexcept;
	[[nodiscard]] std::unique_ptr<Gdiplus::Bitmap> LoadWithWIC(IStream* stream);
}
