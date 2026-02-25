#include "PCH.hpp"
#include "edit_script.h"

#include <utils/edit_text.h>

namespace fs = std::filesystem;

namespace smp
{
	void EditScript(HWND hParent, config::ParsedPanelSettings& settings)
	{
		try
		{
			switch (settings.GetSourceType())
			{
			case config::ScriptSourceType::Sample:
			{
				const auto status = popup_message_v3::get()->messageBox(
					hParent,
					"Are you sure?\n\n"
					"You are trying to edit a sample script.\n"
					"Any changes performed to the script will be applied to every panel that are using this sample.\n"
					"These changes will also be lost when updating the component.",
					"Editing script",
					MB_YESNO | MB_ICONWARNING);

				if (status != IDYES)
				{
					break;
				}

				const auto filePath = *settings.scriptPath;
				QwrException::ExpectTrue(fs::exists(filePath), "Sample script is missing: {}", filePath.u8string());

				EditTextFile(hParent, filePath, true, true);
				break;
			}
			case config::ScriptSourceType::File:
			{
				const auto filePath = *settings.scriptPath;
				QwrException::ExpectTrue(fs::exists(filePath), "Script is missing: {}", filePath.u8string());

				EditTextFile(hParent, filePath, true, true);
				break;
			}
			case config::ScriptSourceType::InMemory:
			{
				EditText(hParent, *settings.script, true);
				break;
			}
			case config::ScriptSourceType::Package:
			default:
			{
				break;
			}
			}
		}
		catch (const fs::filesystem_error& e)
		{
			throw QwrException(e);
		}
	}

	void EditPackageScript(HWND hParent, const std::filesystem::path& script, const config::ParsedPanelSettings& settings)
	{
		try
		{
			if (settings.isSample)
			{
				const auto status = popup_message_v3::get()->messageBox(
					hParent,
					"Are you sure?\n\n"
					"You are trying to edit a sample script.\n"
					"Any changes performed to the script will be applied to every panel that are using this sample.\n"
					"These changes will also be lost when updating the component.",
					"Editing script",
					MB_YESNO | MB_ICONWARNING);

				if (status != IDYES)
				{
					return;
				}
			}

			QwrException::ExpectTrue(fs::exists(script), "Script is missing: {}", script.u8string());
			EditTextFile(hParent, script, true, true);
		}
		catch (const fs::filesystem_error& e)
		{
			throw QwrException(e);
		}
	}
}
