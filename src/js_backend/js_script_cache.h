#pragma once

namespace mozjs
{
	class JsScriptCache
	{
	public:
		JsScriptCache();
		~JsScriptCache();

		/// @throw qwr::QwrException
		/// @throw smp::JsException
		[[nodiscard]] JSScript* GetCachedScript(JSContext* pJsCtx, const std::filesystem::path& absolutePath);

	private:
		[[nodiscard]] RefPtr<JS::Stencil> GetCachedStencil(JSContext* pJsCtx, const std::filesystem::path& absolutePath, const std::string& hackedPathId, const JS::CompileOptions& compileOpts);

	private:
		struct CachedScriptStencil
		{
			RefPtr<JS::Stencil> scriptStencil;
			std::filesystem::file_time_type writeTime;
		};

		std::unordered_map<std::string, CachedScriptStencil> scriptCache_;
	};
}
