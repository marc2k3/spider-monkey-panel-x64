#pragma once

namespace Component
{
	static constexpr std::string_view name = "Spider Monkey Panel";
	static constexpr std::string_view underscore_name = "foo_spider_monkey_panel";
	static constexpr std::string_view version = "1.7.26.3.1";

	static const std::string name_with_version = fmt::format("{} v{}", name, version);
	static const std::string user_agent = fmt::format("{}/{}", name, version);

	template <typename... Args>
	static void log(fmt::string_view format, Args&&... args)
	{
		const auto msg = fmt::format(fmt::runtime(format), std::forward<Args>(args)...);
		FB2K_console_formatter() << name_with_version.data() << ": " << msg.c_str();
	}

	extern cfgDialogPosition dialog_position;
}
