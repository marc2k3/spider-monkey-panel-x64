#include "PCH.hpp"

#include <2K3/PlaylistLock.hpp>
#include <config/delayed_package_utils.h>
#include <events/event_dispatcher.h>
#include <utils/thread_pool.h>

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

	FB2K_SERVICE_FACTORY(InitStageCallback);
	FB2K_RUN_ON_QUIT(on_quit);
}
