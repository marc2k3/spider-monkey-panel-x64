#pragma once

namespace js::frontend
{
	struct CompilationStencil;
};

namespace JS
{
	using Stencil = js::frontend::CompilationStencil;
}

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
		struct CachedScriptStencil
		{
			RefPtr<JS::Stencil> scriptStencil;
			std::filesystem::file_time_type writeTime;
		};

		[[nodiscard]] RefPtr<JS::Stencil> GetCachedStencil(JSContext* pJsCtx, const std::filesystem::path& absolutePath, const JS::CompileOptions& compileOpts);

		std::unordered_map<std::string, CachedScriptStencil> m_script_cache;
	};
}
