#include "PCH.hpp"
#include "global_object.h"

#include "cached_utf8_paths_hack.h"

#include <config/package_utils.h>
#include <interfaces/active_x_object.h>
#include <interfaces/enumerator.h>
#include <interfaces/fb_metadb_handle_list.h>
#include <interfaces/fb_profiler.h>
#include <interfaces/fb_title_format.h>
#include <interfaces/gdi_bitmap.h>
#include <interfaces/gdi_font.h>
#include <namespaces/console.h>
#include <namespaces/fb.h>
#include <namespaces/gdi.h>
#include <namespaces/plman.h>
#include <namespaces/utils.h>
#include <namespaces/window.h>
#include <panel/js_panel_window.h>

namespace fs = std::filesystem;
using namespace mozjs;
using namespace smp;

namespace
{
	/// @throw QwrException
	[[noreturn]] void ThrowInvalidPathError(const fs::path& invalidPath)
	{
		throw QwrException("Path does not point to a valid file: {}", invalidPath.u8string());
	}

	/// @throw QwrException
	auto FindSuitableFileForInclude(const fs::path& path, const std::span<const fs::path>& searchPaths)
	{
		try
		{
			const auto verifyRegularFile = [&](const auto& pathToVerify)
				{
					if (fs::is_regular_file(pathToVerify))
					{
						return;
					}
					::ThrowInvalidPathError(pathToVerify);
				};

			const auto verifyFileExists = [&](const auto& pathToVerify)
				{
					if (fs::exists(pathToVerify))
					{
						return;
					}
					::ThrowInvalidPathError(pathToVerify);
				};

			if (path.is_absolute())
			{
				verifyFileExists(path);
				verifyRegularFile(path);

				return path.lexically_normal();
			}
			else
			{
				for (const auto& searchPath : searchPaths)
				{
					const auto curPath = searchPath / path;
					if (fs::exists(curPath))
					{
						verifyRegularFile(curPath);
						return curPath.lexically_normal();
					}
				}
				::ThrowInvalidPathError(path);
			}
		}
		catch (const fs::filesystem_error& e)
		{
			throw QwrException("Failed to open file `{}`:\n  {}", path.u8string(), smp::FS_Error_ToU8(e));
		}
	}

	void JsFinalizeOpLocal(JS::GCContext* /*gcCtx*/, JSObject* obj)
	{
		auto x = static_cast<JsGlobalObject*>(GetMaybePtrFromReservedSlot(obj, kReservedObjectSlot));
		if (x)
		{
			delete x;
			JS::SetReservedSlot(obj, kReservedObjectSlot, JS::UndefinedValue());

			auto pJsRealm = static_cast<JsRealmInner*>(JS::GetRealmPrivate(js::GetNonCCWObjectRealm(obj)));
			if (pJsRealm)
			{
				delete pJsRealm;
				JS::SetRealmPrivate(js::GetNonCCWObjectRealm(obj), nullptr);
			}
		}
	}

	JSClassOps jsOps = {
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		JsFinalizeOpLocal,
		nullptr,
		nullptr,
		nullptr // set in runtime to JS_GlobalObjectTraceHook
	};

	constexpr JSClass jsClass = {
		"Global",
		JSCLASS_GLOBAL_FLAGS_WITH_SLOTS(std::to_underlying(JsPrototypeId::PrototypeCount)) | JSCLASS_FOREGROUND_FINALIZE,
		&jsOps
	};

	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT_FULL(IncludeScript, "include", JsGlobalObject::IncludeScript, JsGlobalObject::IncludeScriptWithOpt, 1)
	MJS_DEFINE_JS_FN_FROM_NATIVE(clearInterval, JsGlobalObject::ClearInterval)
	MJS_DEFINE_JS_FN_FROM_NATIVE(clearTimeout, JsGlobalObject::ClearTimeout)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(setInterval, JsGlobalObject::SetInterval, JsGlobalObject::SetIntervalWithOpt, 1)
	MJS_DEFINE_JS_FN_FROM_NATIVE_WITH_OPT(setTimeout, JsGlobalObject::SetTimeout, JsGlobalObject::SetTimeoutWithOpt, 1)

	constexpr auto jsFunctions = std::to_array<JSFunctionSpec>(
		{
			JS_FN("clearInterval", clearInterval, 1, kDefaultPropsFlags),
			JS_FN("clearTimeout", clearTimeout, 1, kDefaultPropsFlags),
			JS_FN("include", IncludeScript, 1, kDefaultPropsFlags),
			JS_FN("setInterval", setInterval, 2, kDefaultPropsFlags),
			JS_FN("setTimeout", setTimeout, 2, kDefaultPropsFlags),
			JS_FS_END,
		});
}

namespace mozjs
{
	const JSClass& JsGlobalObject::JsClass = jsClass;

	JsGlobalObject::JsGlobalObject(JSContext* ctx, JsContainer& parentContainer, Window* pWindow)
		: m_ctx(ctx)
		, m_parent_container(parentContainer)
		, m_window(pWindow) {}

	JSObject* JsGlobalObject::CreateNative(JSContext* ctx, JsContainer& parentContainer)
	{
		if (!jsOps.trace)
		{ // JS_GlobalObjectTraceHook address is only accessible after mozjs is loaded.
			jsOps.trace = JS_GlobalObjectTraceHook;
		}

		JS::RealmCreationOptions creationOptions;
		creationOptions.setTrace(JsGlobalObject::Trace);
		JS::RealmOptions options(creationOptions, JS::RealmBehaviors{});
		JS::RootedObject jsObj(ctx, JS_NewGlobalObject(ctx, &jsClass, nullptr, JS::DontFireOnNewGlobalHook, options));

		if (!jsObj)
		{
			throw JsException();
		}

		JSAutoRealm ac(ctx, jsObj);
		JS::SetRealmPrivate(js::GetContextRealm(ctx), new JsRealmInner());

		if (!JS::InitRealmStandardClasses(ctx))
		{
			throw JsException();
		}

		CreateAndInstallObject<Console>(ctx, jsObj, "console");
		CreateAndInstallObject<Gdi>(ctx, jsObj, "gdi");
		CreateAndInstallObject<Plman>(ctx, jsObj, "plman");
		CreateAndInstallObject<Utils>(ctx, jsObj, "utils");
		CreateAndInstallObject<Fb>(ctx, jsObj, "fb");
		CreateAndInstallObject<Window>(ctx, jsObj, "window", parentContainer.GetParentPanel());

		if (!JS_DefineFunctions(ctx, jsObj, jsFunctions.data()))
		{
			throw JsException();
		}

		CreateAndInstallPrototype<JsActiveXObject>(ctx, JsPrototypeId::ActiveX);
		CreateAndInstallPrototype<JsGdiBitmap>(ctx, JsPrototypeId::GdiBitmap);
		CreateAndInstallPrototype<JsGdiFont>(ctx, JsPrototypeId::GdiFont);
		CreateAndInstallPrototype<JsEnumerator>(ctx, JsPrototypeId::Enumerator);
		CreateAndInstallPrototype<JsFbMetadbHandleList>(ctx, JsPrototypeId::FbMetadbHandleList);
		CreateAndInstallPrototype<JsFbProfiler>(ctx, JsPrototypeId::FbProfiler);
		CreateAndInstallPrototype<JsFbTitleFormat>(ctx, JsPrototypeId::FbTitleFormat);

		auto pWindow = GetNativeObjectProperty<Window>(ctx, jsObj, "window");
		auto pNative = std::unique_ptr<JsGlobalObject>(new JsGlobalObject(ctx, parentContainer, pWindow));
		pNative->m_heap_manager = GlobalHeapManager::Create(ctx);

		JS::SetReservedSlot(jsObj, kReservedObjectSlot, JS::PrivateValue(pNative.release()));
		JS_FireOnNewGlobalObject(ctx, jsObj);
		return jsObj;
	}

	JsGlobalObject* JsGlobalObject::ExtractNative(JSContext* ctx, JS::HandleObject jsObject)
	{
		return static_cast<mozjs::JsGlobalObject*>(GetInstanceFromReservedSlot(ctx, jsObject, &mozjs::JsGlobalObject::JsClass, nullptr));
	}

	void JsGlobalObject::Fail(const std::string& errorText)
	{
		m_parent_container.Fail(errorText);
	}

	GlobalHeapManager& JsGlobalObject::GetHeapManager() const
	{
		return *m_heap_manager;
	}

	void JsGlobalObject::PrepareForGc(JSContext* ctx, JS::HandleObject self)
	{
		auto pNativeGlobal = JsGlobalObject::ExtractNative(ctx, self);

		CleanupObjectProperty<Window>(ctx, self, "window");
		CleanupObjectProperty<Plman>(ctx, self, "plman");

		if (pNativeGlobal->m_heap_manager)
		{
			pNativeGlobal->m_heap_manager->PrepareForGc();
			pNativeGlobal->m_heap_manager.reset();
		}
	}

	HWND JsGlobalObject::GetPanelHwnd() const
	{
		return m_window->GetHwnd();
	}

	void JsGlobalObject::ClearInterval(uint32_t intervalId)
	{
		m_window->ClearInterval(intervalId);
	}

	void JsGlobalObject::ClearTimeout(uint32_t timeoutId)
	{
		m_window->ClearInterval(timeoutId);
	}

	void JsGlobalObject::IncludeScript(const std::wstring& path, JS::HandleValue options)
	{
		const auto allSearchPaths = [&]
			{
				std::vector<fs::path> paths;

				if (const auto currentPathOpt = hack::GetCurrentScriptPath(m_ctx); currentPathOpt)
				{
					paths.emplace_back(currentPathOpt->parent_path());
				}

				if (const auto& setting = m_parent_container.GetParentPanel().GetSettings(); setting.packageId)
				{
					paths.emplace_back(PackageUtils::GetScriptsDir(setting));
				}

				paths.emplace_back(smp::path::Component());

				return paths;
			}();

		const auto fsPath = ::FindSuitableFileForInclude(path, allSearchPaths);
		const auto parsedOptions = ParseIncludeOptions(options);

		if (!parsedOptions.alwaysEvaluate && m_included_files.contains(fsPath.native()))
		{
			return;
		}

		m_included_files.emplace(fsPath.native());

		JS::RootedScript jsScript(m_ctx, JsEngine::GetInstance().GetScriptCache().GetCachedScript(m_ctx, fsPath));
		JS::RootedValue dummyRval(m_ctx);

		if (!JS_ExecuteScript(m_ctx, jsScript, &dummyRval))
		{
			throw JsException();
		}
	}

	void JsGlobalObject::IncludeScriptWithOpt(size_t optArgCount, const std::wstring& path, JS::HandleValue options)
	{
		switch (optArgCount)
		{
		case 0:
			return IncludeScript(path, options);
		case 1:
			return IncludeScript(path);
		default:
			throw QwrException("Internal error: invalid number of optional arguments specified: {}", optArgCount);
		}
	}

	uint32_t JsGlobalObject::SetInterval(JS::HandleValue func, uint32_t delay, JS::HandleValueArray funcArgs)
	{
		return m_window->SetInterval(func, delay, funcArgs);
	}

	uint32_t JsGlobalObject::SetIntervalWithOpt(size_t optArgCount, JS::HandleValue func, uint32_t delay, JS::HandleValueArray funcArgs)
	{
		return m_window->SetIntervalWithOpt(optArgCount, func, delay, funcArgs);
	}

	uint32_t JsGlobalObject::SetTimeout(JS::HandleValue func, uint32_t delay, JS::HandleValueArray funcArgs)
	{
		return m_window->SetTimeout(func, delay, funcArgs);
	}

	uint32_t JsGlobalObject::SetTimeoutWithOpt(size_t optArgCount, JS::HandleValue func, uint32_t delay, JS::HandleValueArray funcArgs)
	{
		return m_window->SetTimeoutWithOpt(optArgCount, func, delay, funcArgs);
	}

	JsGlobalObject::IncludeOptions JsGlobalObject::ParseIncludeOptions(JS::HandleValue options)
	{
		IncludeOptions parsedOptions;

		if (!options.isNullOrUndefined())
		{
			QwrException::ExpectTrue(options.isObject(), "options argument is not an object");
			JS::RootedObject jsOptions(m_ctx, &options.toObject());

			parsedOptions.alwaysEvaluate = GetOptionalProperty<bool>(m_ctx, jsOptions, "always_evaluate").value_or(false);
		}

		return parsedOptions;
	}

	void JsGlobalObject::Trace(JSTracer* trc, JSObject* obj)
	{
		auto pNative = static_cast<JsGlobalObject*>(GetMaybePtrFromReservedSlot(obj, kReservedObjectSlot));

		if (pNative && pNative->m_heap_manager)
		{
			pNative->m_heap_manager->Trace(trc);
		}
	}
}
