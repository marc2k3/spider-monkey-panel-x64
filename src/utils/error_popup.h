#pragma once

namespace smp
{
	void ReportErrorWithPopup(const std::string& title, const std::string& errorText);
	void ReportFSErrorWithPopup(const std::filesystem::filesystem_error& e);
}
