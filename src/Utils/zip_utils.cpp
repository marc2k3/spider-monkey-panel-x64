#include "PCH.hpp"
#include "zip_utils.h"

namespace fs = std::filesystem;

namespace
{
	template <typename... Args>
	void CheckMZip(mz_bool mzBool, mz_zip_archive* mzZip, std::string_view functionName, std::string_view introMessageFmt = {}, Args&&... introMessageFmtArgs)
	{
		if (!mzBool)
		{
			const auto introMessage = fmt::format(fmt::runtime(introMessageFmt), std::forward<Args>(introMessageFmtArgs)...);
			throw QwrException("{}{} failed with error {:#x}: {}", introMessage, functionName, static_cast<int>(mzZip->m_last_error), mz_zip_get_error_string(mzZip->m_last_error));
		}
	}
}

namespace smp
{
	ZipPacker::ZipPacker(const fs::path& zipFile) : zipFile_(zipFile)
	{
		const auto zRet = mz_zip_writer_init_file(pZip_, zipFile_.u8string().c_str(), 0);
		CheckMZip(zRet, pZip_, "mz_zip_writer_init_file");
	}

	ZipPacker::~ZipPacker()
	{
		const auto hasFailed = pZip_->m_last_error != 0 || pZip_->m_zip_mode != MZ_ZIP_MODE_INVALID;

		if (hasFailed)
		{
			if (pZip_->m_zip_mode == MZ_ZIP_MODE_WRITING || pZip_->m_zip_mode == MZ_ZIP_MODE_WRITING_HAS_BEEN_FINALIZED)
			{
				mz_zip_writer_end(pZip_);
			}

			std::error_code ec;
			fs::remove(zipFile_, ec);
		}

		delete pZip_;
	}

	void ZipPacker::AddFile(const fs::path& srcFile, const std::string& destFileName)
	{
		auto zRet = mz_zip_writer_add_file(pZip_, destFileName.c_str(), srcFile.u8string().c_str(), "", 0, MZ_BEST_COMPRESSION);
		CheckMZip(zRet, pZip_, "mz_zip_writer_init_file", "Failed to add file to archive: `{}`\n  ", srcFile.filename().u8string());
	}

	void ZipPacker::AddFolder(const fs::path& srcFolder)
	{
		try
		{
			for (const auto& it: fs::recursive_directory_iterator(srcFolder))
			{
				if (it.is_regular_file())
				{
					auto dstFilePath = fs::relative(it.path(), srcFolder).u8string();
					AddFile(it.path(), dstFilePath);
				}
			}
		}
		catch (const fs::filesystem_error& e)
		{
			throw QwrException(e);
		}
	}

	void ZipPacker::Finish()
	{
		auto zRet = mz_zip_writer_finalize_archive(pZip_);
		CheckMZip(zRet, pZip_, "mz_zip_writer_finalize_archive");

		zRet = mz_zip_writer_end(pZip_);
		CheckMZip(zRet, pZip_, "mz_zip_writer_end");
	}

	void UnpackZip(const fs::path& zipFile, const fs::path& dstFolder)
	{
		try
		{
			mz_zip_archive mzZip{};
			auto zRet = mz_zip_reader_init_file(&mzZip, zipFile.u8string().c_str(), 0);
			CheckMZip(zRet, &mzZip, "mz_zip_reader_init_file", "Failed to open archive: `{}`\n  ", zipFile.filename().u8string());
			const auto fileCount = mz_zip_reader_get_num_files(&mzZip);

			auto autoZip = wil::scope_exit([&]
				{
					mz_zip_reader_end(&mzZip);
				});

			for (const auto i : indices(fileCount))
			{
				mz_zip_archive_file_stat zFileStat;
				zRet = mz_zip_reader_file_stat(&mzZip, i, &zFileStat);
				CheckMZip(zRet, &mzZip, "mz_zip_reader_file_stat");
				const auto fileName = std::string(zFileStat.m_filename, strlen(zFileStat.m_filename));
				const auto curPath = dstFolder / smp::ToWide(fileName);

				if (zFileStat.m_is_directory)
				{
					if (!fs::is_directory(curPath))
					{
						fs::create_directories(curPath);
					}
				}
				else
				{
					if (!fs::is_directory(curPath.parent_path()))
					{
						fs::create_directories(curPath.parent_path());
					}
					zRet = mz_zip_reader_extract_to_file(&mzZip, i, curPath.u8string().c_str(), 0);
					CheckMZip(zRet, &mzZip, "mz_zip_reader_extract_to_file");
				}
			}
		}
		catch (const fs::filesystem_error& e)
		{
			throw QwrException(e);
		}
	}
}
