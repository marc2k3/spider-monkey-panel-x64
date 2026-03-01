#pragma once

namespace config
{
	using SerializedJsValue = std::variant<bool, int32_t, double, std::string>;

	struct PanelProperties
	{
		using PropertyMap = std::map<std::wstring, std::shared_ptr<SerializedJsValue>, CmpW>;
		PropertyMap values;

		/// @throw QwrException
		[[nodiscard]] static PanelProperties FromJson(const std::string& jsonString);

		/// @throw QwrException
		[[nodiscard]] std::string ToJson() const;

		/// @throw QwrException
		void Save(stream_writer* writer, abort_callback& abort) const;
	};

	struct PanelSettings_InMemory
	{
		[[nodiscard]] static std::string GetDefaultScript();

		std::string script = GetDefaultScript();
		bool shouldGrabFocus = true;
		bool enableDragDrop = false;
	};

	struct PanelSettings_File
	{
		std::string path;
	};

	struct PanelSettings_Sample
	{
		std::string sampleName;
	};

	struct PanelSettings_Package
	{
		std::string id;      ///< unique package id
		std::string name;    ///< used for logging only
		std::string author;  ///< used for logging only
		std::string version; ///< used for logging only
	};

	struct PanelSettings
	{
		PanelSettings();

		void ResetToDefault();

		/// @throw QwrException
		[[nodiscard]] static PanelSettings Load(stream_reader* reader, size_t size, abort_callback& abort);

		/// @throw QwrException
		void Save(stream_writer* writer, abort_callback& abort) const;

		std::string id;
		PanelProperties properties;

		using ScriptVariant = std::variant<PanelSettings_InMemory, PanelSettings_File, PanelSettings_Sample, PanelSettings_Package>;
		ScriptVariant payload;
	};
}
