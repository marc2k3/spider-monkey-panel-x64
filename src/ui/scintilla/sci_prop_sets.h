#pragma once

namespace smp::config::sci
{

struct ScintillaProp
{
	std::string key;
	std::string defaultval;
	std::string val;
};

using ScintillaPropList = std::vector<ScintillaProp>;

class ScintillaPropsCfg : public cfg_var_legacy::cfg_var
{
public:
	struct DefaultPropValue
	{
		const char* key;
		const char* defaultval;
	};

public:
	ScintillaPropsCfg(const GUID& p_guid);

	// cfg_var
	void get_data_raw(stream_writer* p_stream, abort_callback& p_abort) override;
	void set_data_raw(stream_reader* p_stream, t_size p_sizehint, abort_callback& p_abort) override;

	void reset();
	void export_to_file(fb2k::stringRef path);
	void import_from_file(fb2k::stringRef path);

	ScintillaPropList m_data;

private:
	struct StriCmpAscii
	{
		bool operator()(const std::string& a, const std::string& b) const;
	};

	using ScintillaPropValues = std::map<std::string, std::string, StriCmpAscii>;

private:
	void init_data(std::span<const DefaultPropValue> p_default);
	void merge_data(const ScintillaPropValues& data_map);
};

extern ScintillaPropsCfg props;

} // namespace smp::config::sci
