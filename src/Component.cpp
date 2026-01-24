#include "PCH.hpp"
#include "Component.hpp"

namespace Component
{
	pfc::string8 about_text()
	{
		const auto date = pfc::string(__DATE__).replace("  ", " ");
		const auto msvc = fmt::to_string(_MSC_FULL_VER);

		return fmt::format(
			"{}\n"
			"Copyright (c) 2018 - 2022 TheQwertiest\n"
			"Copyright (c) 2025 - 2026 marc2003\n\n"
			"Based on JScript Panel by marc2003\n"
			"Based on WSH Panel Mod by T.P. Wang\n\n"
			"Build: {}, {}\n\n"
			"Spider Monkey: {}.{}\n"
			"foobar2000 SDK: {}\n"
			"Columns UI SDK: {}\n"
			"MSVC: {}.{}.{}",
			name_with_version,
			__TIME__,
			date.get_ptr(),
			MOZJS_MAJOR_VERSION,
			MOZJS_MINOR_VERSION,
			FOOBAR2000_SDK_VERSION,
			UI_EXTENSION_VERSION,
			msvc.substr(0, 2),
			msvc.substr(2, 2),
			msvc.substr(4)
		).c_str();
	}

	DECLARE_COMPONENT_VERSION(name.data(), version.data(), about_text());
	VALIDATE_COMPONENT_FILENAME("foo_spider_monkey_panel.dll");
}
