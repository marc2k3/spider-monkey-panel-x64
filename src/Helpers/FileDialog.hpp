#pragma once

class FileDialog
{
public:
	static void open(HWND parent, std::string_view title, std::string_view types, const fb2k::fileDialogGetPath_t& path_func) noexcept
	{
		auto dialog = fb2k::fileDialog::get()->setupOpen();
		dialog->setParent(parent);
		dialog->setTitle(title.data());
		dialog->setFileTypes(types.data());
		dialog->runSimple(path_func);
	}

	static void save(HWND parent, std::string_view title, std::string_view types, std::string_view default_ext, const fb2k::fileDialogGetPath_t& path_func) noexcept
	{
		auto dialog = fb2k::fileDialog::get()->setupSave();
		dialog->setParent(parent);
		dialog->setTitle(title.data());
		dialog->setFileTypes(types.data());
		dialog->setDefaultExtension(default_ext.data());
		dialog->runSimple(path_func);
	}
};
