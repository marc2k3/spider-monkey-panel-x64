#pragma once
#include <config/panel_config.h>

namespace smp::config::json
{
	[[nodiscard]] PanelSettings LoadSettings(stream_reader* reader, abort_callback& abort);
	[[nodiscard]] PanelProperties LoadProperties(stream_reader* reader, abort_callback& abort);
	[[nodiscard]] PanelProperties DeserializeProperties(const std::string& str);
	[[nodiscard]] std::string SerializeProperties(const PanelProperties& properties);

	void SaveProperties(stream_writer* writer, abort_callback& abort, const PanelProperties& properties);
	void SaveSettings(stream_writer* writer, abort_callback& abort, const PanelSettings& settings);
}
