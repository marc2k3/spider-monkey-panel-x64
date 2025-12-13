#include <stdafx.h>
#include "component_paths.h"

namespace smp::path
{
	std::filesystem::path Component()
	{
		const auto path = wil::GetModuleFileNameW(core_api::get_my_instance());
		return std::filesystem::path(path.get()).parent_path();
	}

	std::filesystem::path Foobar2000()
	{
		const auto path = wil::GetModuleFileNameW();
		return std::filesystem::path(path.get()).parent_path();
	}

	std::filesystem::path Profile()
	{
		const auto path = filesystem::g_get_native_path(core_api::get_profile_path());
		return smp::ToWide(path);
	}

	std::filesystem::path JsDocsIndex()
	{
		return Component() / L"docs/html/index.html";
	}

	std::filesystem::path ScriptSamples()
	{
		return Component() / "samples";
	}

	std::filesystem::path Packages_Sample()
	{
		return ScriptSamples() / "packages";
	}

	std::filesystem::path Packages_Profile()
	{
		return Profile() / SMP_UNDERSCORE_NAME / "packages";
	}

	std::filesystem::path Packages_Foobar2000()
	{
		return Foobar2000() / SMP_UNDERSCORE_NAME / "packages";
	}

	std::filesystem::path Packages_Storage()
	{
		return Profile() / SMP_UNDERSCORE_NAME / "package_data";
	}

	std::filesystem::path TempFolder()
	{
		return Profile() / SMP_UNDERSCORE_NAME / "tmp";
	}

	std::filesystem::path TempFolder_PackageUnpack()
	{
		return TempFolder() / "unpacked_package";
	}

	std::filesystem::path TempFolder_PackageBackups()
	{
		return TempFolder() / "package_backups";
	}

	std::filesystem::path TempFolder_PackagesToInstall()
	{
		return TempFolder() / "packages_to_install";
	}

	std::filesystem::path TempFolder_PackagesToRemove()
	{
		return TempFolder() / "packages_to_remove";
	}

	std::filesystem::path TempFolder_PackagesInUse()
	{
		return TempFolder() / "packages_in_use";
	}
}
