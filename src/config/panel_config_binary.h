#pragma once

#include <config/panel_config.h>

namespace smp::config::binary
{
	[[nodiscard]] PanelSettings LoadSettings(stream_reader* reader, abort_callback& abort);
	[[nodiscard]] PanelProperties LoadProperties(stream_reader* reader, abort_callback& abort);
}
