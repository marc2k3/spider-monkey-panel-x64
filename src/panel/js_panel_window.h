#pragma once
#include "drag_action_params.h"
#include "user_message.h"

#include <config/parsed_panel_config.h>
#include <JsBackend/Events/event.h>
#include <interfaces/fb_tooltip.h>
#include <UI/ui_conf/ui_conf.h>

class IDropTargetImpl;

namespace smp
{
	class TimeoutManager;

	class js_panel_window : public ui_config_callback_impl
	{
	public:
		enum class PanelType
		{
			CUI = 0,
			DUI = 1
		};

		js_panel_window(PanelType instanceType);
		virtual ~js_panel_window();

		// ui_config_callback_impl
		void ui_colors_changed() override;
		void ui_fonts_changed() override;

		void ReloadScript();
		void LoadSettings(stream_reader* reader, size_t size, abort_callback& abort, bool reloadPanel = true);
		bool UpdateSettings(const config::PanelSettings& settings, bool reloadPanel = true);
		bool SaveSettings(stream_writer* writer, abort_callback& abort) const;

		bool IsPanelIdOverridenByScript() const;

		void Fail(const std::string& errorText);

		void Repaint(bool force = false);
		void RepaintRect(const CRect& rc, bool force = false);

		[[nodiscard]] std::string GetPanelId();
		[[nodiscard]] std::string GetPanelDescription(bool includeVersionAndAuthor = true);
		[[nodiscard]] HWND GetHWND() const;
		[[nodiscard]] const config::ParsedPanelSettings& GetSettings() const;
		[[nodiscard]] config::PanelProperties& GetPanelProperties();
		// TODO: move to a better place
		[[nodiscard]] TimeoutManager& GetTimeoutManager();

		[[nodiscard]] uint32_t& DlgCode();
		[[nodiscard]] PanelType GetPanelType() const;

		virtual DWORD GetColour(uint32_t type) = 0;
		virtual LOGFONT GetFont(uint32_t type) = 0;
		virtual void NotifySizeLimitChanged() = 0;

		void SetSettings_ScriptInfo(const std::string& scriptName, const std::string& scriptAuthor, const std::string& scriptVersion);
		void SetSettings_PanelName(const std::string& panelName);
		/// @throw QwrException
		void SetSettings_DragAndDropStatus(bool isEnabled);
		void SetSettings_CaptureFocusStatus(bool isEnabled);

		void ResetLastDragParams();
		[[nodiscard]] const std::optional<DragActionParams>& GetLastDragParams() const;
		[[nodiscard]] bool HasInternalDrag() const;

		CPoint maxSize_;
		CPoint minSize_;
		CRect rect_;
		mozjs::JsFbTooltip* m_native_tooltip{};
		bool isDark_{};
		bool supportsTransparency_{};

	protected:
		LRESULT OnMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

		void EditScript();
		void ShowConfigure(HWND parent, CDialogConf::Tab tab = CDialogConf::Tab::def);
		void ShowProperties(HWND parent);

		void GenerateContextMenu(HMENU hMenu, size_t id_base);
		void ExecuteContextMenu(size_t id, size_t id_base);

	private:
		bool ReloadSettings();
		bool LoadScript();
		void UnloadScript(bool force = false);

		void CreateDrawContext();

		void SetCaptureMouseState(bool shouldCapture);
		void SetDragAndDropStatus(bool isEnabled);

	public: // event handling
		void ExecuteEvent_JsTask(EventId id, Event_JsExecutor& task);
		bool ExecuteEvent_JsCode(mozjs::JsAsyncTask& task);
		void ExecuteEvent_Basic(EventId id);

	private: // callback handling
		void OnProcessingEventStart();
		void OnProcessingEventFinish();
		std::optional<LRESULT> ProcessEvent();
		void ProcessEventManually(Runnable& runnable);

		std::optional<MSG> GetStalledMessage();
		std::optional<LRESULT> ProcessStalledMessage(const MSG& msg);
		std::optional<LRESULT> ProcessSyncMessage(const MSG& msg);
		std::optional<LRESULT> ProcessCreationMessage(const MSG& msg);
		std::optional<LRESULT> ProcessWindowMessage(const MSG& msg);
		std::optional<LRESULT> ProcessInternalSyncMessage(InternalSyncMessage msg, WPARAM wp, LPARAM lp);

		// Internal callbacks
		void OnContextMenu(int x, int y);
		void OnCreate(HWND hWnd);
		void OnDestroy();
		void ResetMinMax();

		// JS callbacks
		void OnPaint(HDC paintdc);
		void OnPaintErrorScreen(HDC memdc);
		void OnPaintJs(HDC memdc);

	private:
		const PanelType panelType_;
		config::ParsedPanelSettings settings_ = config::ParsedPanelSettings::GetDefault();
		config::PanelProperties properties_;

		std::shared_ptr<mozjs::JsContainer> pJsContainer_;
		std::shared_ptr<PanelTarget> pTarget_;
		std::unique_ptr<TimeoutManager> pTimeoutManager_;

		std::optional<DragActionParams> lastDragParams_;
		ui_selection_holder::ptr selectionHolder_;
		wil::com_ptr<IDropTargetImpl> dropTargetHandler_;

		CWindow wnd_;
		wil::unique_hbitmap bmp_;
		wil::unique_hbrush brush_;

		bool hasFailed_{};
		bool isPaintInProgress_{};
		bool isMouseTracked_{};
		bool isMouseCaptured_{};
		bool hasInternalDrag_{};
		bool isDraggingInside_{};
		bool isPanelIdOverridenByScript_{};

		uint32_t dlgCode_{};
		uint32_t eventNestedCounter_{};
		uintptr_t hRepaintTimer_{};
	};
}
