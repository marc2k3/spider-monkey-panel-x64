#pragma once
#include "parsed_panel_config.h"

class PackageUtils
{
public:
	[[nodiscard]] static WStrings GetFiles(const config::ParsedPanelSettings& settings);
	[[nodiscard]] static WStrings GetScriptFiles(const config::ParsedPanelSettings& settings);
	[[nodiscard]] static const std::filesystem::path& GetRelativePathToMainFile();
	[[nodiscard]] static config::ParsedPanelSettings GetNewSettings(const std::string& name);
	[[nodiscard]] static config::ParsedPanelSettings GetSettingsFromPath(const std::filesystem::path& packagePath);
	[[nodiscard]] static std::filesystem::path GetAssetsDir(const config::ParsedPanelSettings& settings);
	[[nodiscard]] static std::filesystem::path GetPath(const config::ParsedPanelSettings& settings);
	[[nodiscard]] static std::filesystem::path GetScriptsDir(const config::ParsedPanelSettings& settings);
	[[nodiscard]] static std::filesystem::path GetStorageDir(const config::ParsedPanelSettings& settings);
	[[nodiscard]] static std::optional<std::filesystem::path> Find(const std::string& packageId);
	void static FillSettingsFromPath(const std::filesystem::path& packagePath, config::ParsedPanelSettings& settings);
	void static MaybeSaveData(const config::ParsedPanelSettings& settings);
};
