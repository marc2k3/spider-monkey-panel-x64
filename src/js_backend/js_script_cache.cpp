#include "PCH.hpp"
#include "js_script_cache.h"

#include "cached_utf8_paths_hack.h"

using namespace smp;

namespace mozjs
{
	JsScriptCache::JsScriptCache() {}
	JsScriptCache::~JsScriptCache() {}

	JSScript* JsScriptCache::GetCachedScript(JSContext* pJsCtx, const std::filesystem::path& absolutePath)
	{
		// use ids instead of filepaths to work around https://github.com/TheQwertiest/foo_spider_monkey_panel/issues/1
		// and https://bugzilla.mozilla.org/show_bug.cgi?id=1492090
		const auto pathId = hack::CacheUtf8Path(absolutePath.u8string());

		JS::CompileOptions opts(pJsCtx);
		opts.setFileAndLine(pathId.c_str(), 1);

		auto pStencil = GetCachedStencil(pJsCtx, absolutePath, opts);
		JS::InstantiateOptions inst(opts);
		JS::RootedScript jsScript(pJsCtx, JS::InstantiateGlobalStencil(pJsCtx, inst, pStencil));
		JsException::ExpectTrue(jsScript);

		return jsScript;
	}

	RefPtr<JS::Stencil> JsScriptCache::GetCachedStencil(JSContext* pJsCtx, const std::filesystem::path& absolutePath, const JS::CompileOptions& compileOpts)
	{
		const auto cleanPath = absolutePath.lexically_normal();
		const auto lastWriteTime = [&absolutePath, &cleanPath]
			{
				try
				{
					return std::filesystem::last_write_time(absolutePath);
				}
				catch (const std::filesystem::filesystem_error& e)
				{
					throw QwrException("Failed to open file `{}`:\n  {}", cleanPath.u8string(), smp::FS_Error_ToU8(e));
				}
			}();

		if (auto it = m_script_cache.find(cleanPath.u8string()); m_script_cache.cend() != it)
		{
			if (it->second.writeTime == lastWriteTime)
			{
				return it->second.scriptStencil;
			}
		}

		const auto scriptCode = TextFile(cleanPath.native()).read();

		JS::SourceText<mozilla::Utf8Unit> source;
		if (!source.init(pJsCtx, scriptCode.c_str(), scriptCode.length(), JS::SourceOwnership::Borrowed))
		{
			throw JsException();
		}

		RefPtr<JS::Stencil> scriptStencil = JS::CompileGlobalScriptToStencil(pJsCtx, compileOpts, source);
		JsException::ExpectTrue(scriptStencil);

		return m_script_cache.insert_or_assign(cleanPath.u8string(), CachedScriptStencil{ scriptStencil, lastWriteTime }).first->second.scriptStencil;
	}
}
