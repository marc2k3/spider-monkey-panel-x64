#include "PCH.hpp"
#include "error_helpers.h"

namespace smp
{
	std::string GdiErrorCodeToText(Gdiplus::Status errorCode)
	{
		switch (errorCode)
		{
		case Gdiplus::Ok:
			return "No error";
		case Gdiplus::GenericError:
			return "GenericError";
		case Gdiplus::InvalidParameter:
			return "InvalidParameter";
		case Gdiplus::OutOfMemory:
			return "OutOfMemory";
		case Gdiplus::ObjectBusy:
			return "ObjectBusy";
		case Gdiplus::InsufficientBuffer:
			return "InsufficientBuffer";
		case Gdiplus::NotImplemented:
			return "NotImplemented";
		case Gdiplus::Win32Error:
			return "Win32Error";
		case Gdiplus::WrongState:
			return "WrongState";
		case Gdiplus::Aborted:
			return "Aborted";
		case Gdiplus::FileNotFound:
			return "FileNotFound";
		case Gdiplus::ValueOverflow:
			return "ValueOverflow";
		case Gdiplus::AccessDenied:
			return "AccessDenied";
		case Gdiplus::UnknownImageFormat:
			return "UnknownImageFormat";
		case Gdiplus::FontFamilyNotFound:
			return "FontFamilyNotFound";
		case Gdiplus::FontStyleNotFound:
			return "FontStyleNotFound";
		case Gdiplus::NotTrueTypeFont:
			return "NotTrueTypeFont";
		case Gdiplus::UnsupportedGdiplusVersion:
			return "UnsupportedGdiplusVersion";
		case Gdiplus::GdiplusNotInitialized:
			return "GdiplusNotInitialized";
		case Gdiplus::PropertyNotFound:
			return "PropertyNotFound";
		case Gdiplus::PropertyNotSupported:
			return "PropertyNotSupported";
		default:
			return "UnknownErrorCode";
		}
	}

	void CheckGdi(Gdiplus::Status gdiStatus, std::string_view functionName)
	{
		if (gdiStatus > 0)
		{
			throw QwrException("GdiPlus error: {} failed with error ({:#x}): {}", functionName, static_cast<int>(gdiStatus), GdiErrorCodeToText(gdiStatus));
		}
	}

	void ReportActiveXError(HRESULT hresult, EXCEPINFO& exception, UINT& argerr)
	{
		switch (hresult)
		{
		case DISP_E_BADVARTYPE:
		{
			throw QwrException("ActiveXObject: Bad variable type `{}`", argerr);
		}
		case DISP_E_EXCEPTION:
		{
			auto autoCleaner = wil::scope_exit([&exception]
				{
					SysFreeString(exception.bstrSource);
					SysFreeString(exception.bstrDescription);
					SysFreeString(exception.bstrHelpFile);
				});

			HRESULT hr = S_OK;

			if (exception.wCode != 0)
			{
				hr = _com_error::WCodeToHRESULT(exception.wCode);
			}

			if (exception.bstrDescription)
			{
				throw QwrException(
					L"ActiveXObject:\n"
					L"  code: {:#x}\n"
					L"  description: {}\n"
					L"  source: {}",
					static_cast<uint32_t>(hr),
					exception.bstrDescription,
					exception.bstrSource ? exception.bstrSource : L"<none>");
			}
			else
			{
				smp::CheckHR(hr, "ActiveXObject call");
				throw QwrException("ActiveXObject: <no info> (malformed DISP_E_EXCEPTION)", argerr);
			}
		}
		case DISP_E_OVERFLOW:
		{
			throw QwrException("ActiveXObject: Can not convert variable `{}`", argerr);
		}
		case DISP_E_PARAMNOTFOUND:
		{
			throw QwrException("ActiveXObject: Parameter `{}` not found", argerr);
		}
		case DISP_E_TYPEMISMATCH:
		{
			throw QwrException("ActiveXObject: Parameter `{}` type mismatch", argerr);
		}
		case DISP_E_PARAMNOTOPTIONAL:
		{
			throw QwrException("ActiveXObject: Parameter `{}` is required", argerr);
		}
		default:
		{
			smp::CheckHR(hresult, "ActiveXObject");
		}
		}
	}
}
