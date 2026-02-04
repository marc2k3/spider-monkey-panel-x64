#include "PCH.hpp"
#include "mainmenu_dynamic.h"

namespace
{
	class MainMenuNodeCommand_PanelCommand : public mainmenu_node_command
	{
	public:
		MainMenuNodeCommand_PanelCommand(
			HWND panelHwnd,
			const std::string& panelName,
			uint32_t commandId,
			const std::string& commandName,
			const std::string& commandDescription
		);

		void get_display(pfc::string_base& text, uint32_t& flags) final;
		void execute(service_ptr_t<service_base> callback) final;
		GUID get_guid() final;
		bool get_description(pfc::string_base& out) final;

	private:
		const HWND panelHwnd_;
		const std::string panelName_;
		const uint32_t commandId_;
		const std::string commandName_;
		const std::string commandDescription_;
	};

	class MainMenuNodeGroup_PanelCommands : public mainmenu_node_group
	{
	public:
		MainMenuNodeGroup_PanelCommands(
			HWND panelHwnd,
			const std::string& panelName,
			const std::unordered_map<uint32_t, smp::DynamicMainMenuManager::CommandData>& idToCommand
		);

		mainmenu_node::ptr get_child(size_t index) final;
		size_t get_children_count() final;
		void get_display(pfc::string_base& text, uint32_t& flags) final;

	private:
		const smp::DynamicMainMenuManager::PanelData panelData_;
		const std::string panelName_;
		std::vector<mainmenu_node::ptr> commandNodes_;
	};

	class MainMenuNodeGroup_Panels : public mainmenu_node_group
	{
	public:
		MainMenuNodeGroup_Panels();
		void get_display(pfc::string_base& text, uint32_t& flags) final;
		size_t get_children_count() final;
		mainmenu_node::ptr get_child(size_t index) final;

	private:
		std::vector<mainmenu_node::ptr> panelNodes_;
	};

	class MainMenuCommands_Panels : public mainmenu_commands_v2
	{
	public:
		// mainmenu_commands
		uint32_t get_command_count() final;
		GUID get_command(uint32_t p_index) final;
		void get_name(uint32_t p_index, pfc::string_base& p_out) final;
		bool get_description(uint32_t p_index, pfc::string_base& p_out) final;
		GUID get_parent() final;
		void execute(uint32_t p_index, service_ptr_t<service_base> p_callback) final;

		// mainmenu_commands_v2
		bool is_command_dynamic(uint32_t index) final;
		bool dynamic_execute(uint32_t index, const GUID& subID, service_ptr_t<service_base> callback) final;
		mainmenu_node::ptr dynamic_instantiate(uint32_t index) final;
	};
}

namespace
{
	MainMenuNodeCommand_PanelCommand::MainMenuNodeCommand_PanelCommand(
		HWND panelHwnd,
		const std::string& panelName,
		uint32_t commandId,
		const std::string& commandName,
		const std::string& commandDescription)
		: panelHwnd_(panelHwnd)
		, panelName_(panelName)
		, commandId_(commandId)
		, commandName_(commandName)
		, commandDescription_(commandDescription) {}

	void MainMenuNodeCommand_PanelCommand::get_display(pfc::string_base& text, uint32_t& flags)
	{
		text = commandName_.c_str();
		flags = 0u;
	}

	void MainMenuNodeCommand_PanelCommand::execute(service_ptr_t<service_base>)
	{
		smp::EventDispatcher::Get().PutEvent(panelHwnd_, smp::GenerateEvent_JsCallback(smp::EventId::kDynamicMainMenu, commandId_));
	}

	GUID MainMenuNodeCommand_PanelCommand::get_guid()
	{
		auto api = hasher_md5::get();
		hasher_md5_state state;
		api->initialize(state);
		// process termination character as well - it will act as a separator
		api->process(state, panelName_.data(), panelName_.size() + 1);
		api->process(state, &commandId_, sizeof(commandId_));

		return api->get_result_guid(state);
	}

	bool MainMenuNodeCommand_PanelCommand::get_description(pfc::string_base& out)
	{
		if (commandDescription_.empty())
		{
			return false;
		}

		out = commandDescription_.c_str();
		return true;
	}

	MainMenuNodeGroup_PanelCommands::MainMenuNodeGroup_PanelCommands(
		HWND panelHwnd,
		const std::string& panelName,
		const std::unordered_map<uint32_t, smp::DynamicMainMenuManager::CommandData>& idToCommand) : panelName_(panelName)
	{
		auto view = idToCommand | std::views::transform([](const auto& elem)
			{
				return std::make_pair(elem.second.name, elem.first);
			});

		// use map to sort commands by their name
		std::multimap commandNameToId = { std::from_range, view };

		for (const auto& [name, id] : commandNameToId)
		{
			const auto& command = idToCommand.at(id);
			commandNodes_.emplace_back(fb2k::service_new<MainMenuNodeCommand_PanelCommand>(panelHwnd, panelName_, id, command.name, command.description));
		}
	}

	void MainMenuNodeGroup_PanelCommands::get_display(pfc::string_base& text, uint32_t& flags)
	{
		text = panelName_.c_str();
		flags = mainmenu_commands::flag_defaulthidden | mainmenu_commands::sort_priority_base;
	}

	size_t MainMenuNodeGroup_PanelCommands::get_children_count()
	{
		return commandNodes_.size();
	}

	mainmenu_node::ptr MainMenuNodeGroup_PanelCommands::get_child(size_t index)
	{
		return commandNodes_.at(index);
	}

	MainMenuNodeGroup_Panels::MainMenuNodeGroup_Panels()
	{
		const auto& panels = smp::DynamicMainMenuManager::Get().GetAllCommandData();

		std::map<std::string, HWND> panel_name_map;

		for (auto&& panel : panels)
		{
			panel_name_map.emplace(panel.second.name, panel.first);
		}

		for (const auto& [name, hWnd] : panel_name_map)
		{
			const auto& panelData = panels.at(hWnd);

			if (!panelData.commands.empty())
			{
				panelNodes_.emplace_back(fb2k::service_new<MainMenuNodeGroup_PanelCommands>(hWnd, panelData.name, panelData.commands));
			}
		}
	}

	void MainMenuNodeGroup_Panels::get_display(pfc::string_base& text, uint32_t& flags)
	{
		text = "Script commands";
		flags = mainmenu_commands::flag_defaulthidden | mainmenu_commands::sort_priority_base;
	}

	size_t MainMenuNodeGroup_Panels::get_children_count()
	{
		return panelNodes_.size();
	}

	mainmenu_node::ptr MainMenuNodeGroup_Panels::get_child(size_t index)
	{
		return panelNodes_.at(index);
	}

	uint32_t MainMenuCommands_Panels::get_command_count()
	{
		return 1u;
	}

	GUID MainMenuCommands_Panels::get_command(uint32_t /*p_index*/)
	{
		return smp::guid::menu_script_commands;
	}

	void MainMenuCommands_Panels::get_name(uint32_t /*p_index*/, pfc::string_base& p_out)
	{
		p_out = "Script commands";
	}

	bool MainMenuCommands_Panels::get_description(uint32_t /*p_index*/, pfc::string_base& p_out)
	{
		p_out = "Commands provided by scripts.";
		return true;
	}

	GUID MainMenuCommands_Panels::get_parent()
	{
		return smp::guid::mainmenu_group_predefined;
	}

	void MainMenuCommands_Panels::execute(uint32_t /*p_index*/, service_ptr_t<service_base> /*p_callback*/)
	{
		// Should not get here, someone not aware of our dynamic status tried to invoke us?
	}

	bool MainMenuCommands_Panels::is_command_dynamic(uint32_t /*index*/)
	{
		return true;
	}

	mainmenu_node::ptr MainMenuCommands_Panels::dynamic_instantiate(uint32_t /*index*/)
	{
		return fb2k::service_new<MainMenuNodeGroup_Panels>();
	}

	bool MainMenuCommands_Panels::dynamic_execute(uint32_t index, const GUID& subID, service_ptr_t<service_base> callback)
	{
		return __super::dynamic_execute(index, subID, callback);
	}

	FB2K_SERVICE_FACTORY(MainMenuCommands_Panels);
}

namespace smp
{
	DynamicMainMenuManager& DynamicMainMenuManager::Get()
	{
		static DynamicMainMenuManager dmmm;
		return dmmm;
	}

	void DynamicMainMenuManager::RegisterPanel(HWND hWnd, const std::string& panelName)
	{
		panels_.try_emplace(hWnd, PanelData{ panelName });
	}

	void DynamicMainMenuManager::UnregisterPanel(HWND hWnd)
	{
		// don't check hWnd presence, since this might be called several times during error handling
		panels_.erase(hWnd);
	}

	void DynamicMainMenuManager::RegisterCommand(HWND hWnd, uint32_t id, const std::string& name, const std::string& description)
	{
		auto& panelData = panels_.at(hWnd);
		QwrException::ExpectTrue(!panelData.commands.contains(id), "Command with id `{}` was already registered", id);

		panelData.commands.try_emplace(id, name, description);
	}

	void DynamicMainMenuManager::UnregisterCommand(HWND hWnd, uint32_t id)
	{
		auto& panelData = panels_.at(hWnd);
		QwrException::ExpectTrue(panelData.commands.contains(id), "Unknown command id `{}`", id);

		panelData.commands.erase(id);
	}

	const std::unordered_map<HWND, DynamicMainMenuManager::PanelData>& DynamicMainMenuManager::GetAllCommandData() const
	{
		return panels_;
	}
}
