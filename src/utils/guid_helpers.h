#pragma once

namespace smp
{
	[[nodiscard]] GUID GenerateGuid();
	[[nodiscard]] std::wstring GuidToStr(const GUID& guid);
	[[nodiscard]] std::optional<GUID> StrToGuid(const std::wstring& str);
}
