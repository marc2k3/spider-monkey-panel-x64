#pragma once

class ScintillaConfig
{
public:
	enum class Mode
	{
		PlainText,
		JavaScriptAuto,
		JavaScriptCustom,
	};

	using Data = std::vector<std::pair<std::string, std::string>>;
	using Map = std::map<std::string, std::string>;

	static Data cfg_string_to_data(std::string_view str);
	static Data get_default_data();
	static Data preset_to_data(int32_t id);
	static Mode get_mode();
	static int64_t get_zoom();
	static std::string data_to_string(const Data& data);
	static void set_mode(Mode mode);
	static void set_zoom(int64_t zoom);

	void export_to_file(std::wstring_view path);
	void import_from_file(std::wstring_view path);
	void init_data();
	void load_preset(int32_t id);
	void set_data();
	void set_data_item(size_t idx, std::string_view str);

	static constexpr std::array styles =
	{
		"style.default",
		"style.comment",
		"style.keyword",
		"style.identifier",
		"style.string",
		"style.number",
		"style.operator",
		"style.bracelight",
		"style.bracebad",
		"colour.caret.fore",
		"colour.selection.back",
	};

	Data m_data;
};

extern ScintillaConfig scintilla_config;
