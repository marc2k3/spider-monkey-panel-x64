#include "PCH.hpp"
#include "package_utils.h"

#include <Helpers/DirectoryIterator.hpp>
#include <utils/guid_helpers.h>

namespace fs = std::filesystem;

namespace
{
	void Parse_PackageFromPath(const fs::path& packageDir, config::ParsedPanelSettings& parsedSettings)
	{
		try
		{
			QwrException::ExpectTrue(fs::exists(packageDir), "Can't find the required package: `{}`", packageDir.u8string());

			const auto packageJsonFile = packageDir / "package.json";
			QwrException::ExpectTrue(fs::exists(packageJsonFile), "Corrupted package: can't find `package.json`");

			parsedSettings.scriptPath = (packageDir / PackageUtils::GetRelativePathToMainFile());

			const auto str = TextFile(packageJsonFile.native()).read();
			const auto jsonMain = JSON::parse(str);
			QwrException::ExpectTrue(jsonMain.is_object(), "Corrupted `package.json`: not a JSON object");

			parsedSettings.packageId = jsonMain.at("id").get<std::string>();
			parsedSettings.scriptName = jsonMain.at("name").get<std::string>();
			parsedSettings.scriptAuthor = jsonMain.at("author").get<std::string>();
			parsedSettings.scriptVersion = jsonMain.at("version").get<std::string>();
			parsedSettings.scriptDescription = jsonMain.value("description", std::string());
			parsedSettings.enableDragDrop = jsonMain.value("enableDragDrop", false);
			parsedSettings.shouldGrabFocus = jsonMain.value("shouldGrabFocus", true);
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

	void Save_PackageData(const config::ParsedPanelSettings& parsedSettings)
	{
		QwrException::ExpectTrue(!parsedSettings.scriptPath->empty(), "Corrupted settings: `scriptPath` is empty");

		try
		{
			QwrException::ExpectTrue(!parsedSettings.packageId->empty(), "Corrupted settings: `id` is empty");

			auto jsonMain = JSON::object();
			jsonMain.push_back({ "id", *parsedSettings.packageId });
			jsonMain.push_back({ "name", parsedSettings.scriptName });
			jsonMain.push_back({ "author", parsedSettings.scriptAuthor });
			jsonMain.push_back({ "version", parsedSettings.scriptVersion });
			jsonMain.push_back({ "description", parsedSettings.scriptDescription });
			jsonMain.push_back({ "enableDragDrop", parsedSettings.enableDragDrop });
			jsonMain.push_back({ "shouldGrabFocus", parsedSettings.shouldGrabFocus });

			const auto packageDirRet = PackageUtils::Find(*parsedSettings.packageId);
			const auto packagePath = [&]
				{
					if (packageDirRet)
					{
						return *packageDirRet;
					}
					else
					{
						return PackageUtils::GetPath(parsedSettings);
					}
				}();

			if (!fs::exists(packagePath))
			{
				fs::create_directories(packagePath);
			}

			const auto packageJsonFile = packagePath / L"package.json";
			TextFile(packageJsonFile.native()).write(jsonMain.dump(2));

			const auto mainScriptPath = packagePath / PackageUtils::GetRelativePathToMainFile();
			if (!fs::exists(mainScriptPath))
			{
				TextFile(mainScriptPath.native()).write(config::PanelSettings_InMemory::GetDefaultScript());
			}
		}
		catch (const fs::filesystem_error& e)
		{
			throw QwrException(e);
		}
		catch (const JSON::exception& e)
		{
			throw QwrException("Corrupted settings: {}", e.what());
		}
	}
}

WStrings PackageUtils::GetFiles(const config::ParsedPanelSettings& settings)
{
	auto files = GetScriptFiles(settings);

	const auto assetsDir = GetAssetsDir(settings);
	auto assetFiles = DirectoryIterator(assetsDir).list_files(true);
	files.append_range(assetFiles);

	return files;
}

WStrings PackageUtils::GetScriptFiles(const config::ParsedPanelSettings& settings)
{
	const auto mainScript = *settings.scriptPath;
	const auto scriptsDir = GetScriptsDir(settings);

	WStrings scripts;
	scripts.emplace_back(mainScript);

	for (auto&& script : DirectoryIterator(scriptsDir).list_files(true))
	{
		if (fs::path(script).extension() == ".js" && script != mainScript)
			scripts.emplace_back(script);
	}

	return scripts;
}

const fs::path& PackageUtils::GetRelativePathToMainFile()
{
	static const fs::path main{ "main.js" };
	return main;
}

config::ParsedPanelSettings PackageUtils::GetNewSettings(const std::string& name)
{
	config::ParsedPanelSettings settings;

	try
	{
		fs::path packagePath;
		std::string id;
		do
		{
			const auto guidStr = smp::GuidToStr(smp::GenerateGuid());
			id = smp::ToU8(guidStr);
			packagePath = smp::path::Packages_Profile() / id;
		} while (fs::exists(packagePath));

		settings.packageId = id;
		settings.scriptName = name;
		settings.scriptPath = (packagePath / GetRelativePathToMainFile());
	}
	catch (const fs::filesystem_error& e)
	{
		throw QwrException(e);
	}

	return settings;
}

config::ParsedPanelSettings PackageUtils::GetSettingsFromPath(const fs::path& packagePath)
{
	config::ParsedPanelSettings settings{};
	Parse_PackageFromPath(packagePath, settings);
	return settings;
}

std::optional<fs::path> PackageUtils::Find(const std::string& packageId)
{
	std::error_code ec;
	const auto targetPath = smp::path::Packages_Profile() / packageId;

	if (fs::is_directory(targetPath, ec))
		return targetPath;

	return std::nullopt;
}

fs::path PackageUtils::GetAssetsDir(const config::ParsedPanelSettings& settings)
{
	return GetPath(settings) / "assets";
}

fs::path PackageUtils::GetPath(const config::ParsedPanelSettings& settings)
{
	return settings.scriptPath->parent_path();
}

fs::path PackageUtils::GetScriptsDir(const config::ParsedPanelSettings& settings)
{
	return GetPath(settings) / "scripts";
}

fs::path PackageUtils::GetStorageDir(const config::ParsedPanelSettings& settings)
{
	return smp::path::Packages_Storage() / *settings.packageId;
}

void PackageUtils::FillSettingsFromPath(const fs::path& packagePath, config::ParsedPanelSettings& settings)
{
	Parse_PackageFromPath(packagePath, settings);
}

void PackageUtils::MaybeSaveData(const config::ParsedPanelSettings& settings)
{
	if (settings.GetSourceType() == config::ScriptSourceType::Package)
	{
		Save_PackageData(settings);
	}
}
