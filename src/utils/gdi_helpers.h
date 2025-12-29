#pragma once

namespace smp
{
	/// @details Resets last status!
	template <typename T>
	[[nodiscard]] bool IsGdiPlusObjectValid(const T* obj)
	{
		return (obj && Gdiplus::Ok == obj->GetLastStatus());
	}
}
