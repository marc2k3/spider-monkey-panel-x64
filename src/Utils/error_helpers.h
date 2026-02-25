#pragma once

namespace smp
{
	[[nodiscard]] std::string GdiErrorCodeToText(Gdiplus::Status errorCode);

	/// @throw QwrException
	void CheckGdi(Gdiplus::Status gdiStatus, std::string_view functionName);

	/// @throw QwrException
	template <typename T>
	void CheckGdiPlusObject(const std::unique_ptr<T>& obj)
	{
		if (!obj)
			throw QwrException("Failed to create GdiPlus object");

		const auto status = obj->GetLastStatus();

		if (Gdiplus::Ok != status)
		{
			throw QwrException("Failed to create GdiPlus object ({:#x}): {}", static_cast<int>(status), GdiErrorCodeToText(status));
		}
	}

	void ReportActiveXError(HRESULT hresult, EXCEPINFO& exception, UINT& argerr);
}
