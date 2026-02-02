#include "PCH.hpp"
#include "js_panel_window.h"

#include <utils/colour_helpers.h>

namespace
{
	class js_panel_window_cui : public smp::js_panel_window, public uie::container_uie_window_v3
	{
#pragma region js_panel_window
	protected:
		DWORD GetColour(uint32_t type) final
		{
			const auto type_enum = static_cast<cui::colours::colour_identifier_t>(type);
			const auto colour = cui::colours::helper().get_colour(type_enum);
			return smp::ColorrefToArgb(colour);
		}

		LOGFONT GetFont(uint32_t type) final
		{
			LOGFONT lf{};
			const auto type_enum = static_cast<cui::fonts::font_type_t>(type);
			fb2k::std_api_get<cui::fonts::manager>()->get_font(type_enum, lf);
			return lf;
		}

		void NotifySizeLimitChanged() final
		{
			get_host()->on_size_limit_change(GetHWND(), uie::size_limit_all);
		}
#pragma endregion

	public:
		js_panel_window_cui() : js_panel_window(PanelType::CUI) {}

		LRESULT on_message(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) final
		{
			const auto global_key = (msg == WM_SYSKEYDOWN || msg == WM_KEYDOWN) && uie::window::g_process_keydown_keyboard_shortcuts(wp);

			if (global_key)
				return 0;
			else
				return OnMessage(hwnd, msg, wp, lp);
		}

		bool have_config_popup() const final
		{
			return true;
		}

		bool is_available(const uie::window_host_ptr&) const final
		{
			return true;
		}

		bool show_config_popup(HWND parent) final
		{
			ShowConfigure(parent);
			return true;
		}

		const GUID& get_extension_guid() const final
		{
			return smp::guid::window_cui;
		}

		unsigned get_type() const final
		{
			return uie::type_toolbar | uie::type_panel;
		}

		void get_category(pfc::string_base& out) const final
		{
			out = "Panels";
		}

		void get_config(stream_writer* writer, abort_callback& abort) const final
		{
			SaveSettings(writer, abort);
		}

		void get_name(pfc::string_base& out) const final
		{
			out = Component::name.data();
		}

		void set_config(stream_reader* reader, size_t size, abort_callback& abort) final
		{
			LoadSettings(reader, size, abort, false);
		}

		uie::container_window_v3_config get_window_config() final
		{
			return { L"smp_x64_class_cui", false, CS_DBLCLKS };
		}
	};

	static const auto g_window_cui = uie::window_factory<js_panel_window_cui>();
}
