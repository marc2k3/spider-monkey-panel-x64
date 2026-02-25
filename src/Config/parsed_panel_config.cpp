#include "PCH.hpp"
#include "parsed_panel_config.h"
#include "package_utils.h"

namespace fs = std::filesystem;

namespace
{
	void Parse_InMemory(const config::PanelSettings_InMemory& settings, config::ParsedPanelSettings& parsedSettings)
	{
		parsedSettings.enableDragDrop = settings.enableDragDrop;
		parsedSettings.shouldGrabFocus = settings.shouldGrabFocus;
		parsedSettings.script = settings.script;
	}

	void Parse_File(const config::PanelSettings_File& settings, config::ParsedPanelSettings& parsedSettings)
	{
		parsedSettings.scriptPath = smp::ToWide(settings.path);
	}

	void Parse_Sample(const config::PanelSettings_Sample& settings, config::ParsedPanelSettings& parsedSettings)
	{
		try
		{
			parsedSettings.scriptPath = (smp::path::ScriptSamples() / settings.sampleName);
			parsedSettings.isSample = true;
		}
		catch (const fs::filesystem_error& e)
		{
			throw QwrException(e);
		}
	}

	void Parse_Package(const config::PanelSettings_Package& settings, config::ParsedPanelSettings& parsedSettings)
	{
		QwrException::ExpectTrue(!settings.id.empty(), "Corrupted settings (package): `id` is empty");

		try
		{
			const auto packageDirRet = PackageUtils::Find(settings.id);
			const auto valueOrEmpty = [](const std::string& str) -> std::string
				{
					return (str.empty() ? "<empty>" : str);
				};

			QwrException::ExpectTrue(
				packageDirRet.has_value(),
				"Can't find the required package: `{} ({} by {})`",
				settings.id,
				valueOrEmpty(settings.name),
				valueOrEmpty(settings.author)
			);

			PackageUtils::FillSettingsFromPath(*packageDirRet, parsedSettings);
			QwrException::ExpectTrue(settings.id == parsedSettings.packageId, "Corrupted package: `id` is mismatched with parent folder");
		}
		catch (const fs::filesystem_error& e)
		{
			throw QwrException(e);
		}
		catch (const JSON::exception& e)
		{
			throw QwrException("Corrupted `package.json`: {}", e.what());
		}
	}

	void Reparse_Package(config::ParsedPanelSettings& parsedSettings)
	{
		const auto packageId = *parsedSettings.packageId;

		try
		{
			const auto packageDirRet = PackageUtils::Find(packageId);
			const auto valueOrEmpty = [](const std::string& str) -> std::string
				{
					return (str.empty() ? "<empty>" : str);
				};

			QwrException::ExpectTrue(
				packageDirRet.has_value(),
				"Can't find the required package: `{} ({} by {})`",
				packageId,
				valueOrEmpty(parsedSettings.scriptName),
				valueOrEmpty(parsedSettings.scriptAuthor)
			);

			PackageUtils::FillSettingsFromPath(*packageDirRet, parsedSettings);
			QwrException::ExpectTrue(packageId == parsedSettings.packageId, "Corrupted package: `id` is mismatched with parent folder");
		}
		catch (const fs::filesystem_error& e)
		{
			throw QwrException(e);
		}
		catch (const JSON::exception& e)
		{
			throw QwrException("Corrupted `package.json`: {}", e.what());
		}
	}

	config::PanelSettings_Package GetPayload_Package(const config::ParsedPanelSettings& parsedSettings)
	{
		config::PanelSettings_Package payload;
		payload.id = *parsedSettings.packageId;
		payload.author = parsedSettings.scriptAuthor;
		payload.version = parsedSettings.scriptVersion;
		payload.name = parsedSettings.scriptName;

		return payload;
	}

	config::PanelSettings_Sample GetPayload_Sample(const config::ParsedPanelSettings& parsedSettings)
	{
		config::PanelSettings_Sample payload;
		payload.sampleName = fs::relative(*parsedSettings.scriptPath, smp::path::ScriptSamples()).u8string();
		return payload;
	}

	config::PanelSettings_File GetPayload_File(const config::ParsedPanelSettings& parsedSettings)
	{
		config::PanelSettings_File payload;
		payload.path = parsedSettings.scriptPath->u8string();
		return payload;
	}

	config::PanelSettings_InMemory GetPayload_InMemory(const config::ParsedPanelSettings& parsedSettings)
	{
		config::PanelSettings_InMemory payload;
		payload.script = *parsedSettings.script;
		payload.enableDragDrop = parsedSettings.enableDragDrop;
		payload.shouldGrabFocus = parsedSettings.shouldGrabFocus;
		return payload;
	}
}

namespace config
{
	ParsedPanelSettings ParsedPanelSettings::GetDefault()
	{
		return Parse(PanelSettings{});
	}

	ParsedPanelSettings ParsedPanelSettings::Parse(const PanelSettings& settings)
	{
		ParsedPanelSettings parsedSettings;
		parsedSettings.panelId = settings.id;

		std::visit([&parsedSettings](const auto& data)
			{
				using T = std::decay_t<decltype(data)>;
				if constexpr (std::is_same_v<T, PanelSettings_InMemory>)
				{
					Parse_InMemory(data, parsedSettings);
				}
				else if constexpr (std::is_same_v<T, PanelSettings_File>)
				{
					Parse_File(data, parsedSettings);
				}
				else if constexpr (std::is_same_v<T, PanelSettings_Sample>)
				{
					Parse_Sample(data, parsedSettings);
				}
				else if constexpr (std::is_same_v<T, PanelSettings_Package>)
				{
					Parse_Package(data, parsedSettings);
				}
				else
				{
					static_assert(smp::always_false_v<T>, "non-exhaustive visitor!");
				}
			}, settings.payload);

		return parsedSettings;
	}

	config::ParsedPanelSettings ParsedPanelSettings::Reparse(const ParsedPanelSettings& parsedSettings)
	{
		auto reparsedSettings = parsedSettings;

		if (parsedSettings.packageId)
		{
			Reparse_Package(reparsedSettings);
		}
		else
		{
			// these are set dynamically in script
			reparsedSettings.scriptName.clear();
			reparsedSettings.scriptVersion.clear();
			reparsedSettings.scriptAuthor.clear();
			reparsedSettings.enableDragDrop = false;
			reparsedSettings.shouldGrabFocus = false;
		}

		return reparsedSettings;
	}

	PanelSettings ParsedPanelSettings::GeneratePanelSettings() const
	{
		PanelSettings settings;

		settings.id = panelId;
		settings.payload = [&] -> decltype(settings.payload)
			{
				switch (GetSourceType())
				{
				case ScriptSourceType::Package:
					return GetPayload_Package(*this);
				case ScriptSourceType::Sample:
					return GetPayload_Sample(*this);
				case ScriptSourceType::File:
					return GetPayload_File(*this);
				case ScriptSourceType::InMemory:
					return GetPayload_InMemory(*this);
				default:
					return PanelSettings_InMemory{};
				}
			}();

		return settings;
	}

	ScriptSourceType ParsedPanelSettings::GetSourceType() const
	{
		if (packageId)
		{
			return ScriptSourceType::Package;
		}
		else if (isSample)
		{
			return ScriptSourceType::Sample;
		}
		else if (scriptPath)
		{
			return ScriptSourceType::File;
		}
		else
		{
			return ScriptSourceType::InMemory;
		}
	}
}
