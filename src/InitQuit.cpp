#include "PCH.hpp"

#include <FB2K/PlaylistLock.hpp>
#include <Config/delayed_package_utils.h>
#include <Utils/thread_pool.h>

#include <Scintilla.h>

wil::com_ptr<ITypeLib> typelib_smp;
wil::com_ptr<IWICImagingFactory> imaging_factory_smp;

namespace
{
	CAppModule wtl_module;
	GdiplusScope scope;

	class InitStageCallback : public init_stage_callback
	{
		void on_init_stage(uint32_t stage) noexcept final
		{
			if (stage == init_stages::before_config_read)
			{
				config::ProcessDelayedPackages();
			}
			else if (stage == init_stages::before_ui_init)
			{
				const auto ins = core_api::get_my_instance();
				Scintilla_RegisterClasses(ins);

				imaging_factory_smp = wil::CoCreateInstance<IWICImagingFactory>(CLSID_WICImagingFactory);

				fb2k::api::before_ui_init(); // must be before PlaylistLock
				PlaylistLock::before_ui_init();

				std::ignore = wtl_module.Init(nullptr, ins);

				const auto path = wil::GetModuleFileNameW(ins);
				std::ignore = LoadTypeLibEx(path.get(), REGKIND_NONE, &typelib_smp);
			}
		}
	};

	void on_init() noexcept
	{
		fb2k::api::init();
	}

	void on_quit() noexcept
	{
		mozjs::JsEngine::GetInstance().PrepareForExit();
		smp::EventDispatcher::Get().NotifyAllAboutExit();
		QwrThreadPool::GetInstance().Finalize();
		Scintilla_ReleaseResources();
		fb2k::api::reset();
		imaging_factory_smp.reset();
		typelib_smp.reset();
		wtl_module.Term();
	}

	FB2K_SERVICE_FACTORY(InitStageCallback)
	FB2K_RUN_ON_INIT_QUIT(on_init, on_quit)
}
