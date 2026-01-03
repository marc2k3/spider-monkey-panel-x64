#pragma once

#define SMP_NAME              "Spider Monkey Panel"
#define SMP_UNDERSCORE_NAME   "foo_spider_monkey_panel"
#define SMP_WINDOW_CLASS_NAME SMP_UNDERSCORE_NAME "_class"
#define SMP_DLL_NAME          SMP_UNDERSCORE_NAME ".dll"

#define SMP_VERSION "1.7.26.1.2"
#define SMP_NAME_WITH_VERSION SMP_NAME " v" SMP_VERSION
#define SMP_USER_AGENT SMP_DLL_NAME "/" SMP_VERSION

namespace Component
{
	template <typename... Args>
	static void log(fmt::string_view format, Args&&... args)
	{
		const auto msg = fmt::format(fmt::runtime(format), std::forward<Args>(args)...);
		FB2K_console_formatter() << SMP_NAME_WITH_VERSION << ": " << msg.c_str();
	}
}
