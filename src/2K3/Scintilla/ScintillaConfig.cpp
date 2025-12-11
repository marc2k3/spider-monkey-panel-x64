#include <stdafx.h>
#include "ScintillaConfig.hpp"

#include <2K3/TextFile.hpp>

ScintillaConfig scintilla_config;

#pragma region static
ScintillaConfig::Data ScintillaConfig::cfg_string_to_data(std::string_view str)
{
	Data data;
	Map map;

	for (auto&& line : split_string(str, CRLF))
	{
		const auto tmp = split_string(line, "=");

		if (tmp.size() == 2uz)
		{
			map.emplace(tmp[0uz], tmp[1uz]);
		}
	}

	for (auto&& name : styles)
	{
		std::string value;
		const auto it = map.find(name);

		if (it != map.end())
		{
			value = it->second;
		}

		data.emplace_back(name, value);
	}

	return data;
}

ScintillaConfig::Data ScintillaConfig::get_default_data()
{
	if (ui_config_manager::g_is_dark_mode())
		return preset_to_data(IDR_CFG_DARK_GRAY);

	return preset_to_data(IDR_CFG_DEFAULT);
}

ScintillaConfig::Data ScintillaConfig::preset_to_data(int32_t id)
{
	const auto str = get_resource_text(id);
	return cfg_string_to_data(str);
}

ScintillaConfig::Mode ScintillaConfig::get_mode()
{
	auto mode = static_cast<Mode>(fb2k::configStore::get()->getConfigInt("spider.monkey.scintilla.mode"));

	if (mode == Mode::PlainText)
	{
		mode = Mode::JavaScriptAuto;
		set_mode(mode);
	}

	return mode;
}

int64_t ScintillaConfig::get_zoom()
{
	return fb2k::configStore::get()->getConfigInt("spider.monkey.scintilla.zoom");
}

std::string ScintillaConfig::data_to_string(const Data& data)
{
	fmt::memory_buffer buffer;

	for (const auto& [name, value] : data)
	{
		fmt::format_to(std::back_inserter(buffer), "{}={}{}", name, value, CRLF);
	}

	return fmt::to_string(buffer);
}

void ScintillaConfig::set_mode(Mode mode)
{
	fb2k::configStore::get()->setConfigInt("spider.monkey.scintilla.mode", std::to_underlying(mode));
}

void ScintillaConfig::set_zoom(int64_t zoom)
{
	fb2k::configStore::get()->setConfigInt("spider.monkey.scintilla.zoom", zoom);
}
#pragma endregion

void ScintillaConfig::export_to_file(std::wstring_view path)
{
	const auto str = data_to_string(m_data);
	TextFile(path).write(str);
}

void ScintillaConfig::import_from_file(std::wstring_view path)
{
	const auto cfg = TextFile(path).read();
	m_data = cfg_string_to_data(cfg);
	set_data();
}

void ScintillaConfig::init_data()
{
	const std::string cfg = fb2k::configStore::get()->getConfigString("spider.monkey.scintilla.cfg")->c_str();

	if (cfg.empty())
	{
		m_data = get_default_data();
		set_data();
	}
	else
	{
		m_data = cfg_string_to_data(cfg);
	}
}

void ScintillaConfig::load_preset(int32_t id)
{
	m_data = preset_to_data(id);
	set_data();
}

void ScintillaConfig::set_data()
{
	const auto cfg = data_to_string(m_data);
	fb2k::configStore::get()->setConfigString("spider.monkey.scintilla.cfg", cfg.c_str());
}

void ScintillaConfig::set_data_item(size_t idx, std::string_view str)
{
	m_data[idx].second = str;
	set_data();
}
