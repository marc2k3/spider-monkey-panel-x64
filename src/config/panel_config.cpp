#include "PCH.hpp"
#include "panel_config.h"

#include <utils/guid_helpers.h>

namespace fs = std::filesystem;

namespace
{
	enum class ScriptType : uint8_t
	{
		SimpleInMemory = 1,
		SimpleSample = 2,
		SimpleFile = 3,
		Package = 4
	};

	enum class LocationType : uint8_t
	{
		Full = 0,
		Component = 1,
		Profile = 2,
		Fb2k = 3
	};

	constexpr const char kPropJsonConfigVersion[] = "1";
	constexpr const char kPropJsonConfigId[] = "properties";

	constexpr const char kSettingsJsonConfigVersion[] = "1";
	constexpr const char kSettingsJsonConfigId[] = "settings";

	JSON SerializePropertiesToObject(const config::PanelProperties& properties)
	{
		auto jsonValues = JSON::object();

		for (const auto& [nameW, pValue] : properties.values)
		{
			const auto propertyName = smp::ToU8(nameW);
			const auto& serializedValue = *pValue;

			std::visit([&jsonValues, &propertyName](auto&& arg)
				{
					jsonValues.push_back({ propertyName, arg });
				}, serializedValue);
		}

		return JSON::object({
			{ "id", kPropJsonConfigId },
			{ "version", kPropJsonConfigVersion },
			{ "values", jsonValues }
		});
	}

	std::string SerializeProperties(const config::PanelProperties& properties)
	{
		return SerializePropertiesToObject(properties).dump(2);
	}

	config::PanelProperties DeserializePropertiesFromObject(const JSON& jsonMain)
	{
		config::PanelProperties properties;

		if (!jsonMain.is_object())
		{
			throw QwrException("Corrupted serialized properties: not a JSON object");
		}

		if (jsonMain.at("version").get<std::string>() != kPropJsonConfigVersion || jsonMain.at("id").get<std::string>() != kPropJsonConfigId)
		{
			throw QwrException("Corrupted serialized properties: version/id mismatch");
		}

		auto& jsonValues = jsonMain.at("values");

		if (!jsonValues.is_object())
		{
			throw QwrException("Corrupted serialized properties: values");
		}

		for (auto& [key, value] : jsonValues.items())
		{
			if (key.empty())
			{
				throw QwrException("Corrupted serialized properties: empty key");
			}

			config::SerializedJsValue serializedValue;

			if (value.is_boolean())
			{
				serializedValue = value.get<bool>();
			}
			else if (value.is_number_integer())
			{
				serializedValue = value.get<int32_t>();
			}
			else if (value.is_number_float())
			{
				serializedValue = value.get<double>();
			}
			else if (value.is_string())
			{
				serializedValue = value.get<std::string>();
			}
			else
			{
				continue;
			}

			properties.values.emplace(smp::ToWide(key), std::make_shared<config::SerializedJsValue>(serializedValue));
		}

		return properties;
	}

	config::PanelProperties DeserializeProperties(const std::string& str)
	{
		return DeserializePropertiesFromObject(JSON::parse(str, nullptr, false));
	}

	config::PanelSettings LoadSettings(stream_reader* reader, abort_callback& abort)
	{
		try
		{
			config::PanelSettings panelSettings;
			const auto jsonMain = JSON::parse(reader->read_string(abort).get_ptr());

			if (jsonMain.at("version").get<std::string>() != kSettingsJsonConfigVersion || jsonMain.at("id").get<std::string>() != kSettingsJsonConfigId)
			{
				throw QwrException("Corrupted serialized settings: version/id mismatch");
			}

			if (jsonMain.find("panelId") != jsonMain.end())
			{
				jsonMain.at("panelId").get_to(panelSettings.id);
			}

			const auto scriptType = jsonMain.at("scriptType").get<ScriptType>();
			const auto jsonPayload = jsonMain.at("payload");
			switch (scriptType)
			{
			case ScriptType::SimpleInMemory:
			{
				panelSettings.payload = config::PanelSettings_InMemory{ jsonPayload.at("script").get<std::string>() };
				break;
			}
			case ScriptType::SimpleFile:
			{
				const auto wpath = smp::ToWide(jsonPayload.at("path").get<std::string>());
				const auto fsPath = fs::path(wpath).lexically_normal();
				const auto fullPath = [&]
					{
						switch (jsonPayload.at("locationType").get<LocationType>())
						{
						case LocationType::Component:
						{
							return (smp::path::Component() / fsPath);
						}
						case LocationType::Fb2k:
						{
							return (smp::path::Foobar2000() / fsPath);
						}
						case LocationType::Profile:
						{
							return (smp::path::Profile() / fsPath);
						}
						case LocationType::Full:
						{
							return fsPath;
						}
						default:
							throw QwrException("Corrupted serialized settings: unknown file location type");
						}
					}();

				panelSettings.payload = config::PanelSettings_File{ fullPath.u8string() };
				break;
			}
			case ScriptType::SimpleSample:
			{
				panelSettings.payload = config::PanelSettings_Sample{ jsonPayload.at("sampleName").get<std::string>() };
				break;
			}
			case ScriptType::Package:
			{
				panelSettings.payload = config::PanelSettings_Package{
					jsonPayload.at("id").get<std::string>(),
					jsonPayload.at("name").get<std::string>(),
					jsonPayload.at("author").get<std::string>()
				};

				break;
			}
			default:
			{
				throw QwrException("Corrupted serialized settings: unknown script type");
			}
			}

			panelSettings.properties = DeserializePropertiesFromObject(jsonMain.at("properties"));
			return panelSettings;
		}
		catch (const JSON::exception& e)
		{
			throw QwrException(e.what());
		}
		catch (const fs::filesystem_error& e)
		{
			throw QwrException(e);
		}
		catch (const pfc::exception& e)
		{
			throw QwrException(e.what());
		}
	}

	void SaveSettings(stream_writer* writer, abort_callback& abort, const config::PanelSettings& settings)
	{
		try
		{
			auto jsonMain = JSON::object();
			jsonMain.push_back({ "id", kSettingsJsonConfigId });
			jsonMain.push_back({ "version", kSettingsJsonConfigVersion });
			jsonMain.push_back({ "panelId", settings.id });

			auto jsonPayload = JSON::object();
			const auto scriptType = std::visit([&jsonPayload](const auto& data)
				{
					using T = std::decay_t<decltype(data)>;
					if constexpr (std::is_same_v<T, config::PanelSettings_InMemory>)
					{
						jsonPayload.push_back({ "script", data.script });
						return ScriptType::SimpleInMemory;
					}
					else if constexpr (std::is_same_v<T, config::PanelSettings_File>)
					{
						const auto [path, locationType] = [wpath = smp::ToWide(data.path)]
							{
								try
								{
									auto fsPath = fs::path(wpath).lexically_normal();

									const auto isSubpath = [](const auto& path, const auto& base) {
										return (path.wstring().starts_with(base.lexically_normal().wstring()));
										};

									if (isSubpath(fsPath, smp::path::Component()))
									{
										return std::make_tuple(fs::relative(fsPath, smp::path::Component()).u8string(), LocationType::Component);
									}
									if (isSubpath(fsPath, smp::path::Profile()))
									{
										return std::make_tuple(fs::relative(fsPath, smp::path::Profile()).u8string(), LocationType::Profile);
									}
									if (isSubpath(fsPath, smp::path::Foobar2000()))
									{
										return std::make_tuple(fs::relative(fsPath, smp::path::Foobar2000()).u8string(), LocationType::Fb2k);
									}

									return std::make_tuple(fsPath.u8string(), LocationType::Full);
								}
								catch (const fs::filesystem_error& e)
								{
									throw QwrException(e);
								}
							}();

						jsonPayload.push_back({ "path", path });
						jsonPayload.push_back({ "locationType", locationType });
						return ScriptType::SimpleFile;
					}
					else if constexpr (std::is_same_v<T, config::PanelSettings_Sample>)
					{
						jsonPayload.push_back({ "sampleName", data.sampleName });
						return ScriptType::SimpleSample;
					}
					else if constexpr (std::is_same_v<T, config::PanelSettings_Package>)
					{
						jsonPayload.push_back({ "id", data.id });
						jsonPayload.push_back({ "name", data.name });
						jsonPayload.push_back({ "author", data.author });
						jsonPayload.push_back({ "version", data.version });
						return ScriptType::Package;
					}
					else
					{
						static_assert(smp::always_false_v<T>, "non-exhaustive visitor!");
					}
				}, settings.payload);

			jsonMain.push_back({ "scriptType", static_cast<uint8_t>(scriptType) });
			jsonMain.push_back({ "payload", jsonPayload });
			jsonMain.push_back({ "properties", SerializePropertiesToObject(settings.properties) });
			writer->write_string(jsonMain.dump(2), abort);
		}
		catch (const JSON::exception& e)
		{
			throw QwrException(e.what());
		}
		catch (const fs::filesystem_error& e)
		{
			throw QwrException(e);
		}
		catch (const pfc::exception& e)
		{
			throw QwrException(e.what());
		}
	}

	void SaveProperties(stream_writer* writer, abort_callback& abort, const config::PanelProperties& properties)
	{
		try
		{
			writer->write_string(SerializeProperties(properties), abort);
		}
		catch (const pfc::exception& e)
		{
			throw QwrException(e.what());
		}
	}
}

namespace config
{
	PanelProperties PanelProperties::FromJson(const std::string& jsonString)
	{
		return DeserializeProperties(jsonString);
	}

	std::string PanelProperties::ToJson() const
	{
		return SerializeProperties(*this);
	}

	void PanelProperties::Save(stream_writer* writer, abort_callback& abort) const
	{
		SaveProperties(writer, abort, *this);
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
		const auto guidStr = smp::GuidToStr(smp::GenerateGuid());

		payload = PanelSettings_InMemory{};
		id = smp::ToU8(guidStr);
	}

	PanelSettings PanelSettings::Load(stream_reader* reader, size_t size, abort_callback& abort)
	{
		if (size < sizeof(uint32_t))
			return {};

		try
		{
			reader->skip_object(sizeof(uint32_t), abort); // was "SettingsType"
			return LoadSettings(reader, abort);
		}
		catch (const pfc::exception& e)
		{
			throw QwrException(e.what());
		}
	}

	void PanelSettings::Save(stream_writer* writer, abort_callback& abort) const
	{
		writer->write_object_t(2u, abort); // was "SettingsType"
		SaveSettings(writer, abort, *this);
	}
}
