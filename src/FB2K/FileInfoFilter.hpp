#pragma once

class FileInfoFilter : public file_info_filter
{
public:
	struct Tag
	{
		std::string name;
		Strings values;
	};

	using Tags = std::vector<Tag>;

	FileInfoFilter(const Tags& tags) : m_tags(tags) {}

	bool apply_filter(metadb_handle_ptr, t_filestats, file_info& info) noexcept final
	{
		for (const auto& [name, values] : m_tags)
		{
			info.meta_remove_field(name.c_str());

			for (auto&& value : values)
			{
				info.meta_add(name.c_str(), value.c_str());
			}
		}

		return true;
	}

private:
	Tags m_tags;
};
