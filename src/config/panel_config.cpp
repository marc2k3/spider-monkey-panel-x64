#include <stdafx.h>
#include "panel_config.h"

#include <2K3/String.hpp>
#include <config/panel_config_json.h>
#include <utils/guid_helpers.h>

namespace smp::config
{
	PanelProperties PanelProperties::FromJson(const std::string& jsonString)
	{
		return smp::config::json::DeserializeProperties(jsonString);
	}

	std::string PanelProperties::ToJson() const
	{
		return smp::config::json::SerializeProperties(*this);
	}

	void PanelProperties::Save(stream_writer* writer, abort_callback& abort) const
	{
		smp::config::json::SaveProperties(writer, abort, *this);
	}

	std::string PanelSettings_InMemory::GetDefaultScript()
	{
		return get_resource_text(IDR_DEFAULT_SCRIPT);
	}

	PanelSettings::PanelSettings()
	{
		ResetToDefault();
	}

	void PanelSettings::ResetToDefault()
	{
		payload = PanelSettings_InMemory{};
		isPseudoTransparent = false;
		edgeStyle = EdgeStyle::NoEdge;
		id = [] {
			const auto guidStr = GuidToStr(GenerateGuid());
			return smp::ToU8(guidStr);
		}();
	}

	PanelSettings PanelSettings::Load(stream_reader* reader, size_t size, abort_callback& abort)
	{
		if (size < sizeof(uint32_t))
		{
			return {};
		}

		try
		{
			reader->skip_object(sizeof(uint32_t), abort); // was "SettingsType"
			return smp::config::json::LoadSettings(reader, abort);
		}
		catch (const pfc::exception& e)
		{
			throw QwrException(e.what());
		}
	}

	void PanelSettings::Save(stream_writer* writer, abort_callback& abort) const
	{
		writer->write_object_t(2u, abort); // was "SettingsType"
		smp::config::json::SaveSettings(writer, abort, *this);
	}
}
