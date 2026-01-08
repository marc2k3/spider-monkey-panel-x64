#include <PCH.hpp>
#include "error_popup.h"

namespace smp
{
	void ReportErrorWithPopup(const std::string& errorText)
	{
		FB2K_console_formatter() << SMP_UNDERSCORE_NAME << ":\n" << errorText;
		MessageBeep(MB_ICONASTERISK);

		// errors may happen on fb2k init before popup services are available
		fb2k::inMainThread([errorText]
			{
				popup_message::g_show(errorText.c_str(), SMP_UNDERSCORE_NAME);
			});
	}

	void ReportFSErrorWithPopup(const std::filesystem::filesystem_error& e)
	{
		const auto errorText = smp::FS_Error_ToU8(e);
		ReportErrorWithPopup(errorText);
	}
}
