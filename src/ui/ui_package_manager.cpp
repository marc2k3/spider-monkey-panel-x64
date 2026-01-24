#include "PCH.hpp"
#include "ui_package_manager.h"
#include "ui_input_box.h"

#include <2K3/FileDialog.hpp>
#include <config/package_utils.h>
#include <utils/zip_utils.h>

namespace fs = std::filesystem;

CDialogPackageManager::CDialogPackageManager(const std::string& currentPackageId)
	: focusedPackageId_(currentPackageId)
	, ddx_({ smp::CreateUiDdx<smp::UiDdx_ListBox>(focusedPackageIdx_, IDC_LIST_PACKAGES) }) {}

std::optional<config::ParsedPanelSettings> CDialogPackageManager::GetPackage() const
{
	if (focusedPackageIdx_ >= 0)
		return packages_[focusedPackageIdx_].parsedSettings;

	return std::nullopt;
}

LRESULT CDialogPackageManager::OnInitDialog(HWND, LPARAM)
{
	for (auto& ddx : ddx_)
	{
		ddx->SetHwnd(m_hWnd);
	}

	packagesListBox_ = GetDlgItem(IDC_LIST_PACKAGES);
	m_edit_package = GetDlgItem(IDC_PACKAGE_INFO);
	m_edit_package.SetWindowLongPtrW(GWL_EXSTYLE, 0L);
	pPackagesListBoxDrop_ = new FileDropTarget(packagesListBox_, m_hWnd);
	pPackagesListBoxDrop_->RegisterDragDrop();

	SetWindowTextW(L"Script package manager");

	CenterWindow();
	::SetFocus(packagesListBox_);

	LoadPackages();
	UpdateListBoxFromData();
	DoFullDdxToUi();
	m_hooks.AddDialogWithControls(m_hWnd);

	return FALSE;
}

void CDialogPackageManager::OnDestroy()
{
	pPackagesListBoxDrop_->RevokeDragDrop();
	pPackagesListBoxDrop_.reset();
}

void CDialogPackageManager::OnDdxUiChange(UINT /*uNotifyCode*/, int nID, CWindow /*wndCtl*/)
{
	auto it = std::ranges::find_if(ddx_, [nID](auto& ddx)
		{
			return ddx->IsMatchingId(nID);
		});

	if (ddx_.end() != it)
	{
		(*it)->ReadFromUi();
	}

	if (nID == IDC_LIST_PACKAGES)
	{
		UpdatedUiPackageInfo();
		UpdateUiButtons();
	}
}

void CDialogPackageManager::OnNewPackage(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wndCtl*/)
{
	auto dlg = CInputBox("Enter new package name", "Creating new package");

	if (dlg.DoModal(m_hWnd) != IDOK)
		return;

	const auto curName = dlg.GetValue();

	if (curName.empty())
		return;

	try
	{
		const auto newSettings = PackageUtils::GetNewSettings(curName);
		PackageUtils::MaybeSaveData(newSettings);

		packages_.emplace_back(GeneratePackageData(newSettings));
		focusedPackageId_ = *newSettings.packageId;

		UpdateListBoxFromData();
		DoFullDdxToUi();
	}
	catch (const QwrException& e)
	{
		smp::ReportErrorWithPopup(e.what());
	}
}

void CDialogPackageManager::OnDeletePackage(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wndCtl*/)
{
	if (packages_.empty())
		return;

	const auto status = popup_message_v3::get()->messageBox(
		m_hWnd,
		"Are you sure you want to delete the package?",
		"Deleting package",
		MB_YESNO
	);

	if (status != IDYES)
		return;

	try
	{
		const auto packageId = packages_[focusedPackageIdx_].id;
		const auto packagePathOpt = PackageUtils::Find(packages_[focusedPackageIdx_].id);

		if (packagePathOpt)
		{
			if (config::IsPackageInUse(packageId))
			{
				config::MarkPackageAsToBeRemoved(packageId);

				auto it = std::ranges::find_if(packages_, [&](const auto& elem)
					{
						return (packageId == elem.id);
					});

				if (it != packages_.cend())
				{
					focusedPackageId_ = packageId;
					it->status = config::PackageDelayStatus::ToBeRemoved;
				}

				if (ConfirmRebootOnPackageInUse())
				{
					Restart();
				}

				UpdateListBoxFromData();
				DoFullDdxToUi();
				return;
			}

			fs::remove_all(*packagePathOpt);
		}
	}
	catch (const fs::filesystem_error& e)
	{
		smp::ReportFSErrorWithPopup(e);
	}
	catch (const QwrException& e)
	{
		smp::ReportErrorWithPopup(e.what());
	}

	packages_.erase(packages_.cbegin() + focusedPackageIdx_);

	if (packages_.empty())
		focusedPackageIdx_ = -1;
	else
		focusedPackageIdx_ = std::max(0, focusedPackageIdx_ - 1);

	if (focusedPackageIdx_ == -1)
		focusedPackageId_.clear();
	else
		focusedPackageId_ = packages_[focusedPackageIdx_].id;

	UpdateListBoxFromData();
	DoFullDdxToUi();
}

void CDialogPackageManager::OnImportPackage(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wndCtl*/)
{
	auto path_func = [this](fb2k::stringRef path)
		{
			const auto wpath = nativeW(path);
			const auto isRestartNeeded = ImportPackage(wpath);

			if (isRestartNeeded && ConfirmRebootOnPackageInUse())
			{
				Restart();
			}
		};

	FileDialog::open(m_hWnd, "Import package", "Zip archives|*.zip", path_func);
}

void CDialogPackageManager::OnExportPackage(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wndCtl*/)
{
	auto path_func = [this](fb2k::stringRef path)
		{
			const auto wpath = nativeW(path);
			const auto& currentPackageData = packages_[focusedPackageIdx_];

			try
			{
				auto zp = smp::ZipPacker(wpath);
				zp.AddFolder(PackageUtils::GetPath(*currentPackageData.parsedSettings));
				zp.Finish();
			}
			catch (const fs::filesystem_error& e)
			{
				smp::ReportFSErrorWithPopup(e);
			}
			catch (const QwrException& e)
			{
				smp::ReportErrorWithPopup(e.what());
			}
		};

	FileDialog::save(m_hWnd, "Save package as", "Zip archives|*.zip", "zip", path_func);
}

void CDialogPackageManager::OnOpenFolder(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wndCtl*/)
{
	try
	{
		ShellExecuteW(
			nullptr,
			L"explore",
			PackageUtils::GetPath(*packages_[focusedPackageIdx_].parsedSettings).c_str(),
			nullptr,
			nullptr,
			SW_SHOWNORMAL
		);
	}
	catch (const fs::filesystem_error& e)
	{
		smp::ReportFSErrorWithPopup(e);
	}
	catch (const QwrException& e)
	{
		smp::ReportErrorWithPopup(e.what());
	}
}

LRESULT CDialogPackageManager::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/)
{
	if (wID != IDOK)
	{
		focusedPackageIdx_ = -1;
	}

	EndDialog(wID);
	return 0;
}

LRESULT CDialogPackageManager::OnDropFiles(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam)
{
	bool isRestartNeeded = false;

	const auto result = pPackagesListBoxDrop_->ProcessMessage(
		packagesListBox_,
		wParam,
		lParam,
		[&](const auto& path) { isRestartNeeded |= ImportPackage(path); });

	if (isRestartNeeded && ConfirmRebootOnPackageInUse())
	{
		Restart();
	}

	return result;
}

void CDialogPackageManager::DoFullDdxToUi()
{
	if (!this->m_hWnd)
	{
		return;
	}

	for (auto& ddx : ddx_)
	{
		ddx->WriteToUi();
	}

	UpdatedUiPackageInfo();
	UpdateUiButtons();
}

void CDialogPackageManager::UpdateUiButtons()
{
	if (focusedPackageIdx_ < 0 || static_cast<size_t>(focusedPackageIdx_) >= packages_.size())
	{
		GetDlgItem(IDOK).EnableWindow(false);
		GetDlgItem(IDC_BUTTON_DELETE_PACKAGE).EnableWindow(false);
		GetDlgItem(IDC_BUTTON_EXPORT_PACKAGE).EnableWindow(false);
		GetDlgItem(IDC_BUTTON_OPEN_FOLDER).EnableWindow(false);
		return;
	}

	const auto& currentPackageData = packages_[focusedPackageIdx_];
	const BOOL enable = currentPackageData.parsedSettings ? TRUE : FALSE;

	GetDlgItem(IDOK).EnableWindow(enable);
	GetDlgItem(IDC_BUTTON_DELETE_PACKAGE).EnableWindow(currentPackageData.status != config::PackageDelayStatus::ToBeRemoved);
	GetDlgItem(IDC_BUTTON_EXPORT_PACKAGE).EnableWindow(enable);
	GetDlgItem(IDC_BUTTON_OPEN_FOLDER).EnableWindow(enable);
}

void CDialogPackageManager::LoadPackages()
{
	packages_.clear();

	Strings packageIds;
	std::error_code ec;
	const auto packageDir = smp::path::Packages_Profile();

	if (fs::is_directory(packageDir, ec))
	{
		for (const auto& dirIt : fs::directory_iterator(packageDir))
		{
			const auto packageJson = dirIt.path() / L"package.json";

			if (fs::is_regular_file(packageJson, ec))
			{
				packageIds.emplace_back(dirIt.path().filename().u8string());
			}
		}
	}

	std::vector<PackageData> parsedPackages;

	for (const auto& packageId : packageIds)
	{
		try
		{
			const auto packagePathOpt = PackageUtils::Find(packageId);
			QwrException::ExpectTrue(packagePathOpt.has_value(), "Could not find package with id: {}", packageId);

			const auto settings = PackageUtils::GetSettingsFromPath(*packagePathOpt);
			parsedPackages.emplace_back(GeneratePackageData(settings));
		}
		catch (const QwrException& e)
		{
			const auto packageData = PackageData{
				smp::ToWide(fmt::format("{} (ERROR)", packageId)),
				packageId,
				std::nullopt,
				smp::ToWide(fmt::format("Package parsing failed:\r\n{}", e.what()))
			};

			parsedPackages.emplace_back(packageData);
		}

		parsedPackages.back().status = config::GetPackageDelayStatus(packageId);
	}

	packages_ = parsedPackages;
}

void CDialogPackageManager::SortPackages()
{
	std::ranges::sort(packages_, [](const auto& a, const auto& b)
		{
			return StrCmpLogicalW(a.displayedName.c_str(), b.displayedName.c_str()) < 0;
		});
}

void CDialogPackageManager::UpdateListBoxFromData()
{
	SortPackages();

	const auto it = std::ranges::find_if(packages_, [&](const auto& package)
		{
			return focusedPackageId_ == package.id;
		});

	if (it == packages_.cend())
	{
		if (packages_.empty())
		{
			focusedPackageIdx_ = -1;
			focusedPackageId_.clear();
		}
		else
		{
			focusedPackageIdx_ = 0;
			focusedPackageId_ = packages_[0].id;
		}
	}
	else
	{
		focusedPackageIdx_ = static_cast<int>(std::ranges::distance(packages_.cbegin(), it));
	}

	packagesListBox_.ResetContent();

	for (const auto& package : packages_)
	{
		const auto prefix = [&] -> std::wstring
			{
				switch (package.status)
				{
				case config::PackageDelayStatus::ToBeRemoved:
					return L"(will be removed) ";
				case config::PackageDelayStatus::ToBeUpdated:
					return L"(will be updated) ";
				default:
					return L"";
				}
			}();

		packagesListBox_.AddString((prefix + package.displayedName).c_str());
	}
}

void CDialogPackageManager::UpdatedUiPackageInfo()
{
	m_edit_package.SetWindowTextW(L"");

	if (focusedPackageIdx_ < 0)
	{
		return;
	}

	const auto& packageData = packages_[focusedPackageIdx_];

	if (packageData.parsedSettings)
	{
		const auto& parsedSettings = *packageData.parsedSettings;

		const auto appendText = [&](std::string_view field, std::string_view value)
			{
				if (value.empty())
					return;

				const auto str = fmt::format("{}: {}\r\n", field, value);
				m_edit_package.AppendText(smp::ToWide(str).data());
			};

		appendText("Name", parsedSettings.scriptName);
		appendText("Version", parsedSettings.scriptVersion);
		appendText("Author", parsedSettings.scriptAuthor);

		if (parsedSettings.scriptDescription.length())
		{
			const auto str = fmt::format("\r\n{}", parsedSettings.scriptDescription);
			m_edit_package.AppendText(smp::ToWide(str).data());
		}
	}
	else
	{
		const auto str = fmt::format(L"Error:\r\n{}", packages_[focusedPackageIdx_].errorText);
		m_edit_package.AppendText(str.data());
	}
}

CDialogPackageManager::PackageData CDialogPackageManager::GeneratePackageData(const config::ParsedPanelSettings& parsedSettings)
{
	std::string displayedName;

	if (parsedSettings.scriptAuthor.empty())
		displayedName = parsedSettings.scriptName;
	else
		displayedName = fmt::format("{} (by {})", parsedSettings.scriptName, parsedSettings.scriptAuthor);

	return PackageData{
		smp::ToWide(displayedName),
		*parsedSettings.packageId,
		parsedSettings,
		L"",
		config::PackageDelayStatus::NotDelayed
	};
}

bool CDialogPackageManager::ImportPackage(const std::filesystem::path& path)
{
	try
	{
		const auto tmpPath = smp::path::TempFolder_PackageUnpack();
		fs::remove_all(tmpPath);
		fs::create_directories(tmpPath);

		auto autoTmp = wil::scope_exit([&]
			{
				std::error_code ec;
				fs::remove_all(tmpPath, ec);
			});

		smp::UnpackZip(path, tmpPath);

		auto newSettings = PackageUtils::GetSettingsFromPath(tmpPath);
		const auto& packageId = *newSettings.packageId;

		if (const auto oldPackagePathOpt = PackageUtils::Find(packageId); oldPackagePathOpt)
		{
			if (!ConfirmPackageOverwrite(*oldPackagePathOpt, newSettings))
			{
				return false;
			}

			if (config::IsPackageInUse(packageId))
			{
				config::MarkPackageAsToBeInstalled(packageId, tmpPath);

				auto it = std::ranges::find_if(packages_, [&](const auto& elem)
					{
						return (packageId == elem.id);
					});

				if (it != packages_.cend())
				{
					focusedPackageId_ = packageId;
					it->status = config::PackageDelayStatus::ToBeUpdated;
				}

				UpdateListBoxFromData();
				DoFullDdxToUi();
				return true;
			}

			fs::remove_all(*oldPackagePathOpt);
		}

		const auto newPackagePath = smp::path::Packages_Profile() / packageId;
		fs::create_directories(newPackagePath);
		fs::copy(tmpPath, newPackagePath, fs::copy_options::recursive);

		newSettings.scriptPath = newPackagePath / PackageUtils::GetRelativePathToMainFile();

		auto it = std::ranges::find_if(packages_, [&](const auto& elem)
			{
				return (packageId == elem.id);
			});

		if (it != packages_.cend())
		{
			*it = GeneratePackageData(newSettings);
		}
		else
		{
			packages_.emplace_back(GeneratePackageData(newSettings));
		}

		focusedPackageId_ = packageId;
		UpdateListBoxFromData();
		DoFullDdxToUi();
	}
	catch (const fs::filesystem_error& e)
	{
		smp::ReportFSErrorWithPopup(e);
	}
	catch (const QwrException& e)
	{
		smp::ReportErrorWithPopup(e.what());
	}

	return false;
}

bool CDialogPackageManager::ConfirmPackageOverwrite(const std::filesystem::path& oldPackagePath, const config::ParsedPanelSettings& newSettings)
{
	try
	{
		const auto oldSettings = PackageUtils::GetSettingsFromPath(oldPackagePath);

		{
			const auto msg = fmt::format("Another version of this package is present:\nold: '{}' vs new: '{}'\n\nDo you want to update?",
				oldSettings.scriptVersion.empty() ? "<none>" : oldSettings.scriptVersion,
				newSettings.scriptVersion.empty() ? "<none>" : newSettings.scriptVersion
			);

			const auto status = popup_message_v3::get()->messageBox(
				m_hWnd,
				msg.c_str(),
				"Importing package",
				MB_YESNO
			);

			if (status != IDYES)
			{
				return false;
			}
		}

		if (oldSettings.scriptName != newSettings.scriptName)
		{
			const auto msg = fmt::format(
				"Currently installed package has a different name from the new one:\n"
				"old: '{}' vs new: '{}'\n\n"
				"Do you want to continue?",
				oldSettings.scriptName.empty() ? "<none>" : oldSettings.scriptName,
				newSettings.scriptName.empty() ? "<none>" : newSettings.scriptName
			);

			const auto status = popup_message_v3::get()->messageBox(
				m_hWnd,
				msg.c_str(),
				"Importing package",
				MB_YESNO | MB_ICONWARNING
			);

			if (status != IDYES)
			{
				return false;
			}
		}
	}
	catch (const QwrException&)
	{
		// old package might be broken and unparseable,
		// but we still need to confirm
		const auto status = popup_message_v3::get()->messageBox(
			m_hWnd,
			"Another version of this package is present.\nDo you want to update?",
			"Importing package",
			MB_YESNO
		);

		if (status != IDYES)
		{
			return false;
		}
	}

	return true;
}

bool CDialogPackageManager::ConfirmRebootOnPackageInUse()
{
	const auto status = popup_message_v3::get()->messageBox(
		m_hWnd,
		"The package is currently in use. Changes will be applied on the next foobar2000 start.\nDo you want to restart foobar2000 now?",
		"Changing package",
		MB_YESNO
	);

	return status == IDYES;
}

void CDialogPackageManager::Restart()
{
	fb2k::inMainThread([]
		{
			standard_commands::main_restart();
		});
}
