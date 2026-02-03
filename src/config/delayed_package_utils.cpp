#include "PCH.hpp"
#include "delayed_package_utils.h"
#include "package_utils.h"

namespace fs = std::filesystem;

namespace
{
	void ForceRemoveDirContents(const fs::path& dir) noexcept
	{
		std::error_code ec;

		if (!fs::exists(dir, ec))
			return;

		for (const auto& it: fs::recursive_directory_iterator(dir))
		{
			fs::permissions(it, fs::perms::owner_write, fs::perm_options::add, ec);
		}

		for (const auto& it: fs::recursive_directory_iterator(dir))
		{
			if (!fs::is_directory(it, ec))
			{
				fs::remove(it, ec);
			}
		}
	}

	void ForceRemoveDir(const fs::path& dir) noexcept
	{
		std::error_code ec;

		if (!fs::exists(dir, ec))
			return;

		fs::remove_all(dir, ec);

		if (ec.value() != 0)
		{
			ForceRemoveDirContents(dir);
			fs::remove_all(dir, ec);
		}
	}

	/// @throw std::filesystem::filesystem_error
	/// @throw QwrException
	void CheckPackageBackups()
	{
		const auto dir = smp::path::TempFolder_PackageBackups();

		if (!fs::exists(dir) || !fs::is_directory(dir))
		{
			fs::remove_all(dir);
			return;
		}

		Strings backups;
		for (const auto& backup: fs::directory_iterator(dir))
		{
			if (!fs::is_directory(backup))
			{
				continue;
			}

			backups.emplace_back(backup.path().u8string());
		}

		if (!backups.empty())
		{ // in case user still haven't restored his package
			throw QwrException(
				"The following backups still exist:\n"
				"{}\n\n"
				"If you have completed package recovery process, remove them and restart foobar2000 to continue delayed package processing.",
				fmt::join(backups, ",\n"));
		}

		fs::remove_all(dir);
	}

	/// @throw std::filesystem::filesystem_error
	/// @throw QwrException
	void UpdatePackages()
	{
		const auto packagesToProcessDir = smp::path::TempFolder_PackagesToInstall();
		if (!fs::exists(packagesToProcessDir) || !fs::is_directory(packagesToProcessDir))
		{
			fs::remove_all(packagesToProcessDir);
			return;
		}

		const auto packagesDir = smp::path::Packages_Profile();
		const auto packageBackupsDir = smp::path::TempFolder_PackageBackups();

		for (const auto& newPackageDir: fs::directory_iterator(packagesToProcessDir))
		{
			if (!fs::is_directory(packagesToProcessDir))
			{
				continue;
			}

			auto autoTmp = wil::scope_exit([&]
				{
					ForceRemoveDir(newPackageDir);
				});

			const auto packageId = newPackageDir.path().filename().u8string();
			const auto packageToUpdateDir = packagesDir / packageId;
			const auto packageBackupDir = packageBackupsDir / packageId;

			// Save old version

			fs::create_directories(packageBackupDir.parent_path());

			try
			{
				fs::rename(packageToUpdateDir, packageBackupDir);
			}
			catch (const fs::filesystem_error& e)
			{
				smp::ReportErrorWithPopup(
					fmt::format(
						"Failed to update package `{}`:\n{}",
						packageId,
						smp::FS_Error_ToU8(e)
					)
				);

				continue;
			}

			try
			{
				// Try to update
				fs::remove_all(packageToUpdateDir);
				fs::create_directories(packageToUpdateDir.parent_path());
				fs::rename(newPackageDir, packageToUpdateDir);
				config::ClearPackageDelayStatus(packageId);
				ForceRemoveDir(packageBackupDir);
			}
			catch (const fs::filesystem_error&)
			{
				// Enter in recovery process
				ForceRemoveDirContents(packageToUpdateDir);
				fs::create_directories(packageToUpdateDir);

				const auto restorationScript = get_resource_text(IDR_RECOVERY_PACKAGE_SCRIPT);
				const auto restorationJson = get_resource_text(IDR_RECOVERY_PACKAGE_JSON);

				auto j = JSON::parse(restorationJson);
				j["id"] = packageId;

				const auto main_file = packageToUpdateDir / PackageUtils::GetRelativePathToMainFile();
				const auto package_json = packageToUpdateDir / "package.json";

				TextFile(main_file.native()).write(restorationScript);
				TextFile(package_json.native()).write(j.dump(2));

				smp::ReportErrorWithPopup(
					fmt::format(
						"Critical error encountered when updating package `{}`!\n\n"
						"The panel was replaced with recovery package.\n"
						"Follow the instructions to restore your old package.",
						packageId
					)
				);

				throw;
			}
		}

		fs::remove_all(packagesToProcessDir);
	}

	/// @throw std::filesystem::filesystem_error
	void RemovePackages()
	{
		const auto packagesToProcessDir = smp::path::TempFolder_PackagesToRemove();
		if (!fs::exists(packagesToProcessDir) || !fs::is_directory(packagesToProcessDir))
		{
			fs::remove_all(packagesToProcessDir);
			return;
		}

		const auto packagesDir = smp::path::Packages_Profile();

		for (const auto& packageContent: fs::directory_iterator(packagesToProcessDir))
		{
			const auto packageId = packageContent.path().filename().u8string();
			fs::remove_all(packagesDir / packageId);

			config::ClearPackageDelayStatus(packageId);
		}

		fs::remove_all(packagesToProcessDir);
	}
}

namespace config
{
	bool IsPackageInUse(const std::string& packageId) noexcept
	{
		std::error_code ec;
		return fs::exists(smp::path::TempFolder_PackagesInUse() / packageId, ec);
	}

	PackageDelayStatus GetPackageDelayStatus(const std::string& packageId)
	{
		try
		{
			if (fs::exists(smp::path::TempFolder_PackagesToRemove() / packageId))
			{
				return PackageDelayStatus::ToBeRemoved;
			}
			else if (const auto packagePath = smp::path::TempFolder_PackagesToInstall() / packageId; fs::exists(packagePath) && fs::is_directory(packagePath))
			{
				return PackageDelayStatus::ToBeUpdated;
			}
			else
			{
				return PackageDelayStatus::NotDelayed;
			}
		}
		catch (const fs::filesystem_error& e)
		{
			throw QwrException(e);
		}
	}

	void ClearPackageDelayStatus(const std::string& packageId)
	{
		try
		{
			for (const auto& path: { smp::path::TempFolder_PackagesToRemove() / packageId, smp::path::TempFolder_PackagesToInstall() / packageId })
			{
				fs::remove_all(path);
			}
		}
		catch (const fs::filesystem_error& e)
		{
			throw QwrException(e);
		}
	}

	void MarkPackageAsToBeRemoved(const std::string& packageId)
	{
		ClearPackageDelayStatus(packageId);
		try
		{
			const auto path = smp::path::TempFolder_PackagesToRemove() / packageId;

			fs::create_directories(path.parent_path());
			std::ofstream f(path);
			f.close();
		}
		catch (const fs::filesystem_error& e)
		{
			throw QwrException(e);
		}
	}

	void MarkPackageAsToBeInstalled(const std::string& packageId, const std::filesystem::path& packageContent)
	{
		ClearPackageDelayStatus(packageId);
		try
		{
			const auto path = smp::path::TempFolder_PackagesToInstall() / packageId;

			fs::create_directories(path);
			fs::copy(packageContent, path, fs::copy_options::recursive);
		}
		catch (const fs::filesystem_error& e)
		{
			throw QwrException(e);
		}
	}

	void MarkPackageAsInUse(const std::string& packageId)
	{
		try
		{
			const auto path = smp::path::TempFolder_PackagesInUse() / packageId;

			fs::create_directories(path.parent_path());
			std::ofstream f(path);
			f.close();
		}
		catch (const fs::filesystem_error& e)
		{
			throw QwrException(e);
		}
	}

	void ProcessDelayedPackages()
	{
		try
		{
			fs::remove_all(smp::path::TempFolder_PackagesInUse());
			fs::remove_all(smp::path::TempFolder_PackageUnpack());
			::RemovePackages();
			::CheckPackageBackups();
			::UpdatePackages();
		}
		catch (const fs::filesystem_error& e)
		{
			smp::ReportErrorWithPopup(
				fmt::format(
					"Failed to process delayed packages:\n{}",
					smp::FS_Error_ToU8(e)
				)
			);
		}
	}
}
