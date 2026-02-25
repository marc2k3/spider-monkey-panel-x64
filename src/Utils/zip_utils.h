#pragma once
#include <miniz/miniz.h>

namespace smp
{
	class ZipPacker
	{
	public:
		[[nodiscard]] ZipPacker(const std::filesystem::path& zipFile);
		~ZipPacker();

		void AddFile(const std::filesystem::path& srcFile, const std::string& destFileName);
		void AddFolder(const std::filesystem::path& srcFolder);
		void Finish();

	private:
		mz_zip_archive* pZip_ = new mz_zip_archive{};
		std::filesystem::path zipFile_;
	};

	void UnpackZip(const std::filesystem::path& zipFile, const std::filesystem::path& dstFolder);
}
