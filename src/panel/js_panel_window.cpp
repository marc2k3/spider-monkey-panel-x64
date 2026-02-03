#include "PCH.hpp"
#include "js_panel_window.h"
#include "edit_script.h"
#include "modal_blocking_scope.h"

#include <Helpers/FontHelper.hpp>
#include <com_utils/track_drop_target.h>
#include <com_utils/com_destruction_handler.h>
#include <config/delayed_package_utils.h>
#include <config/package_utils.h>
#include <events/event_dispatcher.h>
#include <events/event_drag.h>
#include <events/event_js_callback.h>
#include <events/event_mouse.h>
#include <timeout/timeout_manager.h>
#include <ui/ui_properties.h>
#include <utils/image_helpers.h>
#include <utils/mainmenu_dynamic.h>

namespace smp
{
	js_panel_window::js_panel_window(PanelType instanceType) : panelType_(instanceType) {}

	js_panel_window::~js_panel_window() {}

	void js_panel_window::ui_colors_changed()
	{
		isDark_ = ui_config_manager::g_is_dark_mode();

		if (m_native_tooltip)
		{
			m_native_tooltip->Update();
		}

		wnd_.Invalidate();
		EventDispatcher::Get().PutEvent(wnd_, GenerateEvent_JsCallback(EventId::kUiColoursChanged));
	}

	void js_panel_window::ui_fonts_changed()
	{
		EventDispatcher::Get().PutEvent(wnd_, GenerateEvent_JsCallback(EventId::kUiFontChanged));
	}

	void js_panel_window::Fail(const std::string& errorText)
	{
		hasFailed_ = true;
		ReportErrorWithPopup(errorText);

		if (wnd_)
		{              // can be null during startup
			Repaint(); ///< repaint to display error message
		}
		UnloadScript(true);
	}

	const config::ParsedPanelSettings& js_panel_window::GetSettings() const
	{
		return settings_;
	}

	config::PanelProperties& js_panel_window::GetPanelProperties()
	{
		return properties_;
	}

	TimeoutManager& js_panel_window::GetTimeoutManager()
	{
		return *pTimeoutManager_;
	}

	void js_panel_window::ReloadScript()
	{
		if (pJsContainer_)
		{
			UnloadScript();

			if (ReloadSettings())
			{
				LoadScript();
			}
		}
	}

	void js_panel_window::LoadSettings(stream_reader* reader, size_t size, abort_callback& abort, bool reloadPanel)
	{
		const auto settings = [&]
			{
				try
				{
					return config::PanelSettings::Load(reader, size, abort);
				}
				catch (const QwrException& e)
				{
					ReportErrorWithPopup(
						fmt::format(
							"Can't load panel settings. Your panel will be completely reset!\nError: {}",
							e.what()
						)
					);

					return config::PanelSettings{};
				}
			}();

		if (!UpdateSettings(settings, reloadPanel))
		{
			ReportErrorWithPopup(fmt::format("Can't load panel settings. Your panel will be completely reset!"));
			UpdateSettings(config::PanelSettings{}, reloadPanel);
		}
	}

	bool js_panel_window::UpdateSettings(const config::PanelSettings& settings, bool reloadPanel)
	{
		try
		{
			settings_ = config::ParsedPanelSettings::Parse(settings);
		}
		catch (const QwrException& e)
		{
			Fail(e.what());
			return false;
		}

		properties_ = settings.properties;

		if (reloadPanel)
		{
			ReloadScript();
		}

		return true;
	}

	bool js_panel_window::SaveSettings(stream_writer* writer, abort_callback& abort) const
	{
		try
		{
			auto settings = settings_.GeneratePanelSettings();
			settings.properties = properties_;
			settings.Save(writer, abort);
			return true;
		}
		catch (const QwrException& e)
		{
			ReportErrorWithPopup(e.what());
			return false;
		}
	}

	bool js_panel_window::ReloadSettings()
	{
		try
		{
			settings_ = config::ParsedPanelSettings::Reparse(settings_);
			return true;
		}
		catch (const QwrException& e)
		{
			Fail(e.what());
			return false;
		}
	}

	bool js_panel_window::IsPanelIdOverridenByScript() const
	{
		return isPanelIdOverridenByScript_;
	}

	LRESULT js_panel_window::OnMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
	{
		// According to MSDN:
		////
		// If no filter is specified, messages are processed in the following order:
		// - Sent messages
		// - Posted messages
		// - Input (hardware) messages and system internal events
		// - Sent messages (again)
		// - WM_PAINT messages
		// - WM_TIMER messages
		////
		// Since we are constantly processing our own `event` messages, we need to take additional care
		// so as not to stall WM_PAINT and WM_TIMER messages.

		static uint32_t msgNestedCounter = 0;
		++msgNestedCounter;

		auto autoComObjectDeleter = wil::scope_exit([&]
			{
				// delete only on exit as to avoid delaying processing of the current message due to reentrancy
				--msgNestedCounter;
				if (!msgNestedCounter)
				{
					DeleteMarkedObjects();
				}
			});

		if (EventDispatcher::IsRequestEventMessage(msg))
		{
			EventDispatcher::Get().OnRequestEventMessageReceived(wnd_);
			if (auto retVal = ProcessEvent(); retVal.has_value())
				return *retVal;
		}
		else
		{
			if (auto retVal = ProcessSyncMessage(MSG{ hwnd, msg, wp, lp }); retVal.has_value())
				return *retVal;
		}

		return DefWindowProc(hwnd, msg, wp, lp);
	}

	void js_panel_window::ExecuteEvent_Basic(EventId id)
	{
		switch (id)
		{
		case EventId::kScriptEdit:
		{
			EditScript();
			break;
		}
		case EventId::kScriptReload:
		{
			ReloadScript();
			break;
		}
		case EventId::kScriptShowConfigure:
		{
			ShowConfigure(wnd_);
			break;
		}
		case EventId::kScriptShowConfigureLegacy:
		{
			switch (settings_.GetSourceType())
			{
			case config::ScriptSourceType::InMemory:
			{
				EditScript();
				break;
			}
			default:
			{
				ShowConfigure(wnd_);
				break;
			}
			}
			break;
		}
		case EventId::kScriptShowProperties:
		{
			ShowProperties(wnd_);
			break;
		}
		case EventId::kWndPaint:
		{
			if (isPaintInProgress_)
			{
				break;
			}

			isPaintInProgress_ = true;

			{
				auto paintdc = wil::BeginPaint(wnd_);
				OnPaint(paintdc.get());
			}

			isPaintInProgress_ = false;
			break;
		}
		case EventId::kWndResize:
		{
			if (pJsContainer_)
			{
				pJsContainer_->InvokeJsCallback("on_size", rect_.Width(), rect_.Height());
				Repaint();
			}
			break;
		}
		default:
			break;
		}
	}

	void js_panel_window::ExecuteEvent_JsTask(EventId id, Event_JsExecutor& task)
	{
		const auto execJs = [this](auto& jsTask) -> std::optional<bool>
			{
				if (pJsContainer_)
					return jsTask.JsExecute(*pJsContainer_);
				else
					return false;
			};

		switch (id)
		{
		case EventId::kMouseRightButtonUp:
		{
			const auto pEvent = task.AsMouseEvent();

			if (!(pEvent->IsShiftPressed() && pEvent->IsWinPressed()) && execJs(*pEvent).value_or(false))
			{
				break;
			}

			EventDispatcher::Get().PutEvent(
				wnd_,
				std::make_unique<Event_Mouse>(
					EventId::kMouseContextMenu,
					pEvent->GetX(),
					pEvent->GetY(),
					0,
					pEvent->GetModifiers()
				),
				EventPriority::kInput
			);
			break;
		}
		case EventId::kMouseContextMenu:
		{
			const auto pMouseEvent = task.AsMouseEvent();
			OnContextMenu(pMouseEvent->GetX(), pMouseEvent->GetY());
			break;
		}
		case EventId::kMouseDragEnter:
		{
			const auto pDragEvent = task.AsDragEvent();
			lastDragParams_.reset();

			if (pJsContainer_)
			{
				auto dragParams = pDragEvent->GetDragParams();
				const auto bRet = pJsContainer_->InvokeOnDragAction(
					"on_drag_enter",
					{ pDragEvent->GetX(), pDragEvent->GetY() },
					pDragEvent->GetMask(),
					dragParams
				);

				if (bRet)
				{
					lastDragParams_ = dragParams;
				}
			}

			pDragEvent->DisposeStoredData();

			break;
		}
		case EventId::kMouseDragLeave:
		{
			const auto pDragEvent = task.AsDragEvent();
			lastDragParams_.reset();
			pDragEvent->DisposeStoredData();

			if (pJsContainer_)
			{
				pJsContainer_->InvokeJsCallback("on_drag_leave");
			}
			break;
		}
		case EventId::kMouseDragOver:
		{
			const auto pDragEvent = task.AsDragEvent();

			if (pJsContainer_)
			{
				auto dragParams = pDragEvent->GetDragParams();
				const auto bRet = pJsContainer_->InvokeOnDragAction(
					"on_drag_over",
					{ pDragEvent->GetX(), pDragEvent->GetY() },
					pDragEvent->GetMask(),
					dragParams
				);

				if (bRet)
				{
					lastDragParams_ = dragParams;
				}
			}

			pDragEvent->DisposeStoredData();
			break;
		}
		case EventId::kMouseDragDrop:
		{
			const auto pDragEvent = task.AsDragEvent();

			if (pJsContainer_)
			{
				auto dragParams = pDragEvent->GetDragParams();
				const auto bRet = pJsContainer_->InvokeOnDragAction(
					"on_drag_drop",
					{ pDragEvent->GetX(), pDragEvent->GetY() },
					pDragEvent->GetMask(),
					dragParams
				);

				if (bRet)
				{
					TrackDropTarget::ProcessDropEvent(pDragEvent->GetStoredData(), dragParams);
				}
			}

			lastDragParams_.reset();
			pDragEvent->DisposeStoredData();
			break;
		}
		case EventId::kInputFocus:
		{
			selectionHolder_ = ui_selection_manager::get()->acquire();
			execJs(task);
			break;
		}
		default:
		{
			execJs(task);
		}
		}
	}

	bool js_panel_window::ExecuteEvent_JsCode(mozjs::JsAsyncTask& jsTask)
	{
		if (pJsContainer_)
		{
			return pJsContainer_->InvokeJsAsyncTask(jsTask);
		}

		return false;
	}

	void js_panel_window::OnProcessingEventStart()
	{
		++eventNestedCounter_;
	}

	void js_panel_window::OnProcessingEventFinish()
	{
		--eventNestedCounter_;

		if (!eventNestedCounter_)
		{ // Jobs (e.g. futures) should be drained only with empty JS stack and after the current task (as required by ES).
			// Also see https://developer.mozilla.org/en-US/docs/Web/JavaScript/EventLoop#Run-to-completion
			mozjs::JsContainer::RunJobs();
		}

		if (!eventNestedCounter_ || modal::IsModalBlocked())
		{
			EventDispatcher::Get().RequestNextEvent(wnd_);
		}
	}

	std::optional<LRESULT> js_panel_window::ProcessEvent()
	{
		OnProcessingEventStart();
		auto onEventProcessed = wil::scope_exit([&]
			{
				OnProcessingEventFinish();
			});

		if (const auto stalledMsgOpt = GetStalledMessage(); stalledMsgOpt)
		{
			// stalled messages always have a higher priority
			if (auto retVal = ProcessStalledMessage(*stalledMsgOpt); retVal.has_value())
			{
				return *retVal;
			}

			return DefWindowProc(wnd_, stalledMsgOpt->message, stalledMsgOpt->wParam, stalledMsgOpt->lParam);
		}
		else
		{
			if (eventNestedCounter_ == 1 || modal::IsModalBlocked())
			{
				EventDispatcher::Get().ProcessNextEvent(wnd_);
			}

			return std::nullopt;
		}
	}

	void js_panel_window::ProcessEventManually(Runnable& runnable)
	{
		OnProcessingEventStart();
		auto onEventProcessed = wil::scope_exit([&]
			{
				OnProcessingEventFinish();
			});

		runnable.Run();
	}

	std::optional<MSG> js_panel_window::GetStalledMessage()
	{
		MSG msg;
		bool hasMessage = PeekMessageW(&msg, wnd_, WM_TIMER, WM_TIMER, PM_REMOVE);

		if (!hasMessage)
		{
			return std::nullopt;
		}

		if (!hRepaintTimer_)
		{
			// means that WM_PAINT was invoked properly
			return std::nullopt;
		}

		KillTimer(wnd_, hRepaintTimer_);
		hRepaintTimer_ = NULL;
		return msg;
	}

	std::optional<LRESULT> js_panel_window::ProcessStalledMessage(const MSG& msg)
	{
		switch (msg.message)
		{
		case WM_TIMER:
		{
			wnd_.RedrawWindow(nullptr, nullptr, RDW_UPDATENOW);
			return 0;
		}
		default:
		{
			return std::nullopt;
		}
		}
	}

	std::optional<LRESULT> js_panel_window::ProcessSyncMessage(const MSG& msg)
	{
		if (auto retVal = ProcessCreationMessage(msg); retVal.has_value())
		{
			return *retVal;
		}

		if (auto retVal = ProcessWindowMessage(msg); retVal.has_value())
		{
			return *retVal;
		}

		if (IsInEnumRange(msg.message))
		{
			if (auto retVal = ProcessInternalSyncMessage(static_cast<InternalSyncMessage>(msg.message), msg.wParam, msg.lParam); retVal.has_value())
			{
				return *retVal;
			}
		}

		return std::nullopt;
	}

	std::optional<LRESULT> js_panel_window::ProcessCreationMessage(const MSG& msg)
	{
		switch (msg.message)
		{
		case WM_CREATE:
		{
			OnCreate(msg.hwnd);
			return 0;
		}
		case WM_DESTROY:
		{
			OnDestroy();
			return 0;
		}
		default:
		{
			return std::nullopt;
		}
		}
	}

	std::optional<LRESULT> js_panel_window::ProcessWindowMessage(const MSG& msg)
	{
		if (!pJsContainer_)
			return std::nullopt;

		switch (msg.message)
		{
		case WM_DISPLAYCHANGE:
		case WM_THEMECHANGED:
		{
			EventDispatcher::Get().PutEvent(wnd_, std::make_unique<Event_Basic>(EventId::kScriptReload), EventPriority::kControl);
			return 0;
		}
		case WM_ERASEBKGND:
		{
			return 1;
		}
		case WM_PAINT:
		{
			if (hRepaintTimer_)
			{
				KillTimer(wnd_, hRepaintTimer_);
				hRepaintTimer_ = NULL;
			}

			if (isPaintInProgress_)
			{
				return std::nullopt;
			}

			auto pEvent = std::make_unique<Event_Basic>(EventId::kWndPaint);
			pEvent->SetTarget(pTarget_);
			ProcessEventManually(*pEvent);

			return 0;
		}
		case WM_SIZE:
		{
			wnd_.GetClientRect(&rect_);

			if (!rect_.IsRectEmpty())
			{
				CreateDrawContext();

				auto pEvent = std::make_unique<Event_Basic>(EventId::kWndResize);
				pEvent->SetTarget(pTarget_);
				ProcessEventManually(*pEvent);
			}

			return 0;
		}
		case WM_GETMINMAXINFO:
		{ // This message will be called before WM_CREATE as well,
			// but we don't need to handle it before panel creation,
			// since default values suit us just fine
			auto pmmi = reinterpret_cast<LPMINMAXINFO>(msg.lParam);
			pmmi->ptMaxTrackSize = maxSize_;
			pmmi->ptMinTrackSize = minSize_;
			return 0;
		}
		case WM_GETDLGCODE:
		{
			return DlgCode();
		}
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		{
			static const std::unordered_map<int, EventId> kMsgToEventId{
				{ WM_LBUTTONDOWN, EventId::kMouseLeftButtonDown },
				{ WM_MBUTTONDOWN, EventId::kMouseMiddleButtonDown },
				{ WM_RBUTTONDOWN, EventId::kMouseRightButtonDown }
			};

			if (settings_.shouldGrabFocus)
			{
				wnd_.SetFocus();
			}

			SetCaptureMouseState(true);

			EventDispatcher::Get().PutEvent(
				wnd_,
				GenerateEvent_JsCallback(
					kMsgToEventId.at(msg.message),
					static_cast<int32_t>(GET_X_LPARAM(msg.lParam)),
					static_cast<int32_t>(GET_Y_LPARAM(msg.lParam)),
					static_cast<uint32_t>(msg.wParam)
				),
				EventPriority::kInput
			);

			return std::nullopt;
		}
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		{
			static const std::unordered_map<int, EventId> kMsgToEventId{
				{ WM_LBUTTONUP, EventId::kMouseLeftButtonUp },
				{ WM_MBUTTONUP, EventId::kMouseMiddleButtonUp }
			};

			if (isMouseCaptured_)
			{
				SetCaptureMouseState(false);
			}

			EventDispatcher::Get().PutEvent(
				wnd_,
				GenerateEvent_JsCallback(
					kMsgToEventId.at(msg.message),
					static_cast<int32_t>(GET_X_LPARAM(msg.lParam)),
					static_cast<int32_t>(GET_Y_LPARAM(msg.lParam)),
					static_cast<uint32_t>(msg.wParam)
				),
				EventPriority::kInput
			);

			return std::nullopt;
		}
		case WM_RBUTTONUP:
		{
			if (isMouseCaptured_)
			{
				SetCaptureMouseState(false);
			}

			EventDispatcher::Get().PutEvent(
				wnd_,
				std::make_unique<Event_Mouse>(
					EventId::kMouseRightButtonUp,
					static_cast<int32_t>(GET_X_LPARAM(msg.lParam)),
					static_cast<int32_t>(GET_Y_LPARAM(msg.lParam)),
					static_cast<uint32_t>(msg.wParam),
					GetHotkeyModifierFlags()
				),
				EventPriority::kInput
			);

			return 0;
		}
		case WM_LBUTTONDBLCLK:
		{
			EventDispatcher::Get().PutEvent(
				wnd_,
				GenerateEvent_JsCallback(
					EventId::kMouseLeftButtonDoubleClick,
					static_cast<int32_t>(GET_X_LPARAM(msg.lParam)),
					static_cast<int32_t>(GET_Y_LPARAM(msg.lParam)),
					static_cast<uint32_t>(msg.wParam)
				),
				EventPriority::kInput
			);

			return std::nullopt;
		}
		case WM_MBUTTONDBLCLK:
		{
			EventDispatcher::Get().PutEvent(
				wnd_,
				GenerateEvent_JsCallback(
					EventId::kMouseMiddleButtonDoubleClick,
					static_cast<int32_t>(GET_X_LPARAM(msg.lParam)),
					static_cast<int32_t>(GET_Y_LPARAM(msg.lParam)),
					static_cast<uint32_t>(msg.wParam)
				),
				EventPriority::kInput
			);

			return std::nullopt;
		}
		case WM_RBUTTONDBLCLK:
		{
			EventDispatcher::Get().PutEvent(
				wnd_,
				GenerateEvent_JsCallback(
					EventId::kMouseRightButtonDoubleClick,
					static_cast<int32_t>(GET_X_LPARAM(msg.lParam)),
					static_cast<int32_t>(GET_Y_LPARAM(msg.lParam)),
					static_cast<uint32_t>(msg.wParam)
				),
				EventPriority::kInput
			);

			return std::nullopt;
		}
		case WM_CONTEXTMENU:
		{
			// WM_CONTEXTMENU receives screen coordinates
			POINT p{ GET_X_LPARAM(msg.wParam), GET_Y_LPARAM(msg.lParam) };
			ScreenToClient(wnd_, &p);
			EventDispatcher::Get().PutEvent(
				wnd_,
				std::make_unique<Event_Mouse>(
					EventId::kMouseContextMenu,
					p.x,
					p.y,
					0,
					GetHotkeyModifierFlags()
				),
				EventPriority::kInput
			);

			return 1;
		}
		case WM_MOUSEMOVE:
		{
			if (!isMouseTracked_)
			{
				isMouseTracked_ = true;

				TRACKMOUSEEVENT tme{ sizeof(TRACKMOUSEEVENT), TME_LEAVE, wnd_, HOVER_DEFAULT };
				TrackMouseEvent(&tme);

				// Restore default cursor
				SetCursor(LoadCursor(nullptr, IDC_ARROW));
			}

			EventDispatcher::Get().PutEvent(
				wnd_,
				GenerateEvent_JsCallback(
					EventId::kMouseMove,
					static_cast<int32_t>(GET_X_LPARAM(msg.lParam)),
					static_cast<int32_t>(GET_Y_LPARAM(msg.lParam)),
					static_cast<uint32_t>(msg.wParam)
				),
				EventPriority::kInput
			);

			return std::nullopt;
		}
		case WM_MOUSELEAVE:
		{
			isMouseTracked_ = false;

			// Restore default cursor
			SetCursor(LoadCursor(nullptr, IDC_ARROW));

			EventDispatcher::Get().PutEvent(wnd_, GenerateEvent_JsCallback(EventId::kMouseLeave), EventPriority::kInput);
			return std::nullopt;
		}
		case WM_MOUSEWHEEL:
		{
			EventDispatcher::Get().PutEvent(
				wnd_,
				GenerateEvent_JsCallback(
					EventId::kMouseVerticalWheel,
					static_cast<int32_t>(GET_WHEEL_DELTA_WPARAM(msg.wParam) > 0 ? 1 : -1),
					static_cast<int32_t>(GET_WHEEL_DELTA_WPARAM(msg.wParam)),
					static_cast<int32_t>(WHEEL_DELTA)
				),
				EventPriority::kInput
			);

			return std::nullopt;
		}
		case WM_MOUSEHWHEEL:
		{
			EventDispatcher::Get().PutEvent(
				wnd_,
				GenerateEvent_JsCallback(
					EventId::kMouseHorizontalWheel,
					static_cast<int32_t>(GET_WHEEL_DELTA_WPARAM(msg.wParam) > 0 ? 1 : -1)
				),
				EventPriority::kInput
			);

			return std::nullopt;
		}
		case WM_SETCURSOR:
		{
			return 1;
		}
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			EventDispatcher::Get().PutEvent(
				wnd_,
				GenerateEvent_JsCallback(
					EventId::kKeyboardKeyDown,
					static_cast<uint32_t>(msg.wParam)
				),
				EventPriority::kInput
			);

			return 0;
		}
		case WM_SYSKEYUP:
		case WM_KEYUP:
		{
			EventDispatcher::Get().PutEvent(
				wnd_,
				GenerateEvent_JsCallback(
					EventId::kKeyboardKeyUp,
					static_cast<uint32_t>(msg.wParam)
				),
				EventPriority::kInput
			);

			return 0;
		}
		case WM_CHAR:
		{
			EventDispatcher::Get().PutEvent(
				wnd_,
				GenerateEvent_JsCallback(
					EventId::kKeyboardChar,
					static_cast<uint32_t>(msg.wParam)
				),
				EventPriority::kInput
			);

			return 0;
		}
		case WM_SETFOCUS:
		{
			// Note: selection holder is acquired during event processing
			EventDispatcher::Get().PutEvent(
				wnd_,
				GenerateEvent_JsCallback(
					EventId::kInputFocus,
					true
				),
				EventPriority::kInput
			);

			return std::nullopt;
		}
		case WM_KILLFOCUS:
		{
			selectionHolder_.release();
			EventDispatcher::Get().PutEvent(
				wnd_,
				GenerateEvent_JsCallback(
					EventId::kInputBlur,
					false
				),
				EventPriority::kInput
			);

			return std::nullopt;
		}
		default:
		{
			return std::nullopt;
		}
		}
	}

	std::optional<LRESULT> js_panel_window::ProcessInternalSyncMessage(InternalSyncMessage msg, WPARAM, LPARAM lp)
	{
		if (!pJsContainer_)
			return std::nullopt;

		switch (msg)
		{
		case InternalSyncMessage::legacy_notify_others:
		{
			// this event is sent via EventDispatcher, hence we don't need to set target manually
			ProcessEventManually(*reinterpret_cast<EventBase*>(lp));
			return 0;
		}
		case InternalSyncMessage::script_fail:
		{
			Fail(*reinterpret_cast<const std::string*>(lp));
			return 0;
		}
		case InternalSyncMessage::prepare_for_exit:
		{
			UnloadScript();
			return 0;
		}
		case InternalSyncMessage::ui_script_editor_saved:
		{
			ReloadScript();
			return 0;
		}
		case InternalSyncMessage::wnd_drag_drop:
		{
			if (isMouseCaptured_)
			{
				SetCaptureMouseState(false);
			}

			return 0;
		}
		case InternalSyncMessage::wnd_drag_enter:
		{
			isDraggingInside_ = true;
			return 0;
		}
		case InternalSyncMessage::wnd_drag_leave:
		{
			isDraggingInside_ = false;
			return 0;
		}
		case InternalSyncMessage::wnd_internal_drag_start:
		{
			hasInternalDrag_ = true;
			return 0;
		}
		case InternalSyncMessage::wnd_internal_drag_stop:
		{
			if (!isDraggingInside_)
			{
				isMouseTracked_ = false;
			}
			if (isMouseCaptured_)
			{
				SetCaptureMouseState(false);
			}

			hasInternalDrag_ = false;

			return 0;
		}
		default:
		{
			return std::nullopt;
		}
		}
	}

	void js_panel_window::EditScript()
	{
		switch (settings_.GetSourceType())
		{
		case config::ScriptSourceType::InMemory:
		case config::ScriptSourceType::File:
		case config::ScriptSourceType::Sample:
		{
			try
			{
				smp::EditScript(wnd_, settings_);
			}
			catch (const QwrException& e)
			{
				ReportErrorWithPopup(e.what());
			}
			break;
		}
		case config::ScriptSourceType::Package:
		{
			ShowConfigure(wnd_, CDialogConf::Tab::package);
			break;
		}
		default:
		{
			break;
		}
		}
	}

	void js_panel_window::ShowConfigure(HWND parent, CDialogConf::Tab tab)
	{
		if (modal_dialog_scope::can_create())
		{
			modal::ModalBlockingScope scope(parent, true);

			CDialogConf dlg(this, tab);
			dlg.DoModal(parent);
		}
	}

	void js_panel_window::ShowProperties(HWND parent)
	{
		if (modal_dialog_scope::can_create())
		{
			modal::ModalBlockingScope scope(parent, true);

			auto dlg = CDialogProperties(*this);
			dlg.DoModal(parent);
		}
	}

	void js_panel_window::GenerateContextMenu(HMENU hMenu, size_t id_base)
	{
		namespace fs = std::filesystem;

		try
		{
			CMenuHandle menu{ hMenu };

			menu.AppendMenuW(MF_STRING, id_base + 1uz, L"&Reload");
			menu.AppendMenuW(MF_SEPARATOR, UINT_PTR{}, LPCWSTR{});
			menu.AppendMenuW(MF_STRING, id_base + 2uz, L"&Open component folder");
			menu.AppendMenuW(MF_STRING, id_base + 3uz, L"&Open documentation");
			menu.AppendMenuW(MF_SEPARATOR, UINT_PTR{}, LPCWSTR{});

			if (settings_.GetSourceType() == config::ScriptSourceType::Package)
			{
				const auto scriptFiles = PackageUtils::GetScriptFiles(settings_);
				const auto scriptsDir = PackageUtils::GetScriptsDir(settings_);

				CMenu cSubMenu;
				cSubMenu.CreatePopupMenu();
				auto scriptIdx = id_base + 100uz;

				for (const auto& file: scriptFiles)
				{
					const auto relativePath = [&]
						{
							if (fs::path(file).filename() == "main.js")
							{
								return fs::path("main.js");
							}
							else
							{
								return fs::relative(file, scriptsDir);
							}
						}();

					cSubMenu.AppendMenuW(MF_STRING, ++scriptIdx, relativePath.c_str());
				}

				menu.AppendMenuW(MF_STRING, cSubMenu, L"&Edit panel script");
				cSubMenu.Detach(); ///< AppendMenu takes ownership
			}
			else
			{
				menu.AppendMenuW(MF_STRING, id_base + 5uz, L"&Edit panel script...");
			}

			menu.AppendMenuW(MF_STRING, id_base + 6uz, L"&Panel properties...");
			menu.AppendMenuW(MF_STRING, id_base + 7uz, L"&Configure panel...");
		}
		catch (const fs::filesystem_error& e)
		{
			ReportFSErrorWithPopup(e);
		}
		catch (const QwrException& e)
		{
			ReportErrorWithPopup(e.what());
		}
	}

	void js_panel_window::ExecuteContextMenu(size_t id, size_t id_base)
	{
		try
		{
			switch (id - id_base)
			{
			case 1uz:
			{
				EventDispatcher::Get().PutEvent(wnd_, std::make_unique<Event_Basic>(EventId::kScriptReload), EventPriority::kControl);
				break;
			}
			case 2uz:
			{
				ShellExecuteW(nullptr, L"open", path::Component().c_str(), nullptr, nullptr, SW_SHOW);
				break;
			}
			case 3uz:
			{
				ShellExecuteW(nullptr, L"open", path::JsDocsIndex().c_str(), nullptr, nullptr, SW_SHOW);
				break;
			}
			case 5uz:
			{
				EventDispatcher::Get().PutEvent(wnd_, std::make_unique<Event_Basic>(EventId::kScriptEdit), EventPriority::kControl);
				break;
			}
			case 6uz:
			{
				EventDispatcher::Get().PutEvent(wnd_, std::make_unique<Event_Basic>(EventId::kScriptShowProperties), EventPriority::kControl);
				break;
			}
			case 7uz:
			{
				EventDispatcher::Get().PutEvent(wnd_, std::make_unique<Event_Basic>(EventId::kScriptShowConfigure), EventPriority::kControl);
				break;
			}
			}

			if (id - id_base > 100uz)
			{
				const auto scriptFiles = PackageUtils::GetScriptFiles(settings_);
				const auto fileIdx = std::min(id - id_base - 100uz, scriptFiles.size()) - 1uz;

				EditPackageScript(wnd_, scriptFiles[fileIdx], settings_);
				ReloadScript();
			}
		}
		catch (const QwrException& e)
		{
			ReportErrorWithPopup(e.what());
		}
	}

	std::string js_panel_window::GetPanelId()
	{
		return settings_.panelId;
	}

	std::string js_panel_window::GetPanelDescription(bool includeVersionAndAuthor)
	{
		std::string ret;

		if (settings_.scriptName.empty())
		{
			ret += settings_.panelId;
		}
		else
		{
			ret += settings_.scriptName;
		}

		if (includeVersionAndAuthor)
		{
			if (!settings_.scriptVersion.empty())
			{
				ret += fmt::format(" v{}", settings_.scriptVersion);
			}
			if (!settings_.scriptAuthor.empty())
			{
				ret += fmt::format(" by {}", settings_.scriptAuthor);
			}
		}

		return ret;
	}

	HWND js_panel_window::GetHWND() const
	{
		return wnd_;
	}

	uint32_t& js_panel_window::DlgCode()
	{
		return dlgCode_;
	}

	js_panel_window::PanelType js_panel_window::GetPanelType() const
	{
		return panelType_;
	}

	void js_panel_window::SetSettings_ScriptInfo(const std::string& scriptName, const std::string& scriptAuthor, const std::string& scriptVersion)
	{
		settings_.scriptName = scriptName;
		settings_.scriptAuthor = scriptAuthor;
		settings_.scriptVersion = scriptVersion;
	}

	void js_panel_window::SetSettings_PanelName(const std::string& panelName)
	{
		settings_.panelId = panelName;
		isPanelIdOverridenByScript_ = true;
	}

	void js_panel_window::SetSettings_DragAndDropStatus(bool isEnabled)
	{
		settings_.enableDragDrop = isEnabled;

		SetDragAndDropStatus(settings_.enableDragDrop);
	}

	void js_panel_window::SetSettings_CaptureFocusStatus(bool isEnabled)
	{
		settings_.shouldGrabFocus = isEnabled;
	}

	void js_panel_window::ResetLastDragParams()
	{
		lastDragParams_.reset();
	}

	const std::optional<DragActionParams>& js_panel_window::GetLastDragParams() const
	{
		return lastDragParams_;
	}

	bool js_panel_window::HasInternalDrag() const
	{
		return hasInternalDrag_;
	}

	void js_panel_window::Repaint(bool force)
	{
		if (!force && !hRepaintTimer_)
		{ // paint message might be stalled if the message queue is not empty, we circumvent this via WM_TIMER
			hRepaintTimer_ = SetTimer(wnd_, NULL, USER_TIMER_MINIMUM, nullptr);
		}
		wnd_.RedrawWindow(nullptr, nullptr, RDW_INVALIDATE | (force ? RDW_UPDATENOW : 0));
	}

	void js_panel_window::RepaintRect(const CRect& rc, bool force)
	{
		if (!force && !hRepaintTimer_)
		{ // paint message might be stalled if the message queue is not empty, we circumvent this via WM_TIMER
			hRepaintTimer_ = SetTimer(wnd_, NULL, USER_TIMER_MINIMUM, nullptr);
		}
		wnd_.RedrawWindow(&rc, nullptr, RDW_INVALIDATE | (force ? RDW_UPDATENOW : 0));
	}

	bool js_panel_window::LoadScript()
	{
		auto timer = pfc::hires_timer::create_and_start();

		hasFailed_ = false;
		isPanelIdOverridenByScript_ = false;

		SetDragAndDropStatus(settings_.enableDragDrop);

		DynamicMainMenuManager::Get().RegisterPanel(wnd_, settings_.panelId);

		wnd_.SetWindowPos(nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

		ResetMinMax();
		NotifySizeLimitChanged();

		if (!pJsContainer_->Initialize())
			return false;

		pTimeoutManager_->SetLoadingStatus(true);

		if (settings_.script)
		{
			modal::WhitelistedScope scope; // Initial script execution must always be whitelisted
			if (!pJsContainer_->ExecuteScript(*settings_.script))
				return false;
		}
		else
		{
			if (settings_.GetSourceType() == config::ScriptSourceType::Package)
			{
				config::MarkPackageAsInUse(*settings_.packageId);
			}

			modal::WhitelistedScope scope; // Initial script execution must always be whitelisted
			if (!pJsContainer_->ExecuteScriptFile(*settings_.scriptPath))
				return false;
		}

		pTimeoutManager_->SetLoadingStatus(false);

		const auto ms = static_cast<uint32_t>(timer.query() * 1000);
		Component::log ("{} initialized in {} ms", GetPanelDescription(), ms);

		EventDispatcher::Get().PutEvent(wnd_, std::make_unique<Event_Basic>(EventId::kWndResize), EventPriority::kResize);
		return true;
	}

	void js_panel_window::UnloadScript(bool force)
	{
		if (!pJsContainer_)
		{ // possible during startup config load
			return;
		}

		if (!force)
		{ // should not go in JS again when forced to unload (e.g. in case of an error)
			pJsContainer_->InvokeJsCallback("on_script_unload");
		}

		DynamicMainMenuManager::Get().UnregisterPanel(wnd_);
		pJsContainer_->Finalize();
		pTimeoutManager_->StopAllTimeouts();
		selectionHolder_.release();

		SetDragAndDropStatus(false);
	}

	void js_panel_window::CreateDrawContext()
	{
		auto dc = wil::GetDC(wnd_);
		bmp_.reset(CreateCompatibleBitmap(dc.get(), rect_.Width(), rect_.Height()));
	}

	void js_panel_window::OnContextMenu(int x, int y)
	{
		if (modal::IsModalBlocked())
		{
			return;
		}

		modal::MessageBlockingScope scope;

		POINT p{ x, y };
		ClientToScreen(wnd_, &p);

		CMenu menu = CreatePopupMenu();
		constexpr uint32_t base_id = 0;
		GenerateContextMenu(menu, base_id);

		const uint32_t ret = menu.TrackPopupMenu(TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, p.x, p.y, wnd_, nullptr);
		ExecuteContextMenu(ret, base_id);
	}

	void js_panel_window::OnCreate(HWND hWnd)
	{
		wnd_ = hWnd;
		isDark_ = ui_config_manager::g_is_dark_mode();
		supportsTransparency_ = pfc::getWindowClassName(GetParent(wnd_)) == "ReBarWindow32";

		pTarget_ = std::make_shared<PanelTarget>(*this);
		EventDispatcher::Get().AddWindow(wnd_, pTarget_);

		pJsContainer_ = std::make_shared<mozjs::JsContainer>(*this);
		pTimeoutManager_ = std::make_unique<TimeoutManager>(pTarget_);

		LoadScript();
	}

	void js_panel_window::OnDestroy()
	{
		// Careful when changing invocation order here!

		UnloadScript();

		if (pTarget_)
		{
			pTarget_->UnlinkPanel();
		}

		if (pTimeoutManager_)
		{
			pTimeoutManager_->Finalize();
			pTimeoutManager_.reset();
		}

		EventDispatcher::Get().RemoveWindow(wnd_);

		if (hRepaintTimer_)
		{
			KillTimer(wnd_, hRepaintTimer_);
			hRepaintTimer_ = NULL;
		}

		pJsContainer_.reset();
		bmp_.reset();
		brush_.reset();
	}

	void js_panel_window::OnPaint(HDC paintdc)
	{
		if (!bmp_)
		{
			return;
		}

		auto memdc = wil::unique_hdc(CreateCompatibleDC(paintdc));
		auto select = wil::SelectObject(memdc.get(), bmp_.get());

		if (hasFailed_
			|| !pJsContainer_
			|| mozjs::JsContainer::JsStatus::EngineFailed == pJsContainer_->GetStatus()
			|| mozjs::JsContainer::JsStatus::Failed == pJsContainer_->GetStatus())
		{
			OnPaintErrorScreen(memdc.get());
		}
		else
		{
			if (supportsTransparency_)
			{
				uie::win32::paint_background_using_parent(wnd_, memdc.get(), false);
			}
			else
			{
				if (isDark_)
				{
					brush_.reset(CreateSolidBrush(RGB(32, 32, 32)));
				}
				else
				{
					const auto colour = GetSysColor(COLOR_WINDOW);
					brush_.reset(CreateSolidBrush(colour));
				}

				FillRect(memdc.get(), &rect_, brush_.get());
			}

			OnPaintJs(memdc.get());
		}

		BitBlt(paintdc, 0, 0, rect_.Width(), rect_.Height(), memdc.get(), 0, 0, SRCCOPY);
	}

	void js_panel_window::OnPaintErrorScreen(HDC memdc)
	{
		// Tahoma for WINE peasants
		auto font = FontHelper::get().create(L"Tahoma", 24, Gdiplus::FontStyleBold);
		auto select = wil::SelectObject(memdc, font.get());

		brush_.reset(CreateSolidBrush(RGB(225, 60, 45)));
		FillRect(memdc, &rect_, brush_.get());
		SetBkMode(memdc, TRANSPARENT);

		SetTextColor(memdc, RGB(255, 255, 255));
		DrawTextW(memdc, L"Spider Monkey Panel JavaScript error", -1, &rect_, DT_CENTER | DT_VCENTER | DT_NOPREFIX | DT_SINGLELINE);
	}

	void js_panel_window::OnPaintJs(HDC memdc)
	{
		Gdiplus::Graphics gr(memdc);

		// SetClip() may improve performance slightly
		gr.SetClip(Gdiplus::Rect(rect_.left, rect_.top, rect_.Width(), rect_.Height()));

		pJsContainer_->InvokeOnPaint(gr);
	}

	void js_panel_window::ResetMinMax()
	{
		maxSize_.SetPoint(INT_MAX, INT_MAX);
		minSize_.SetPoint(0, 0);
	}

	void js_panel_window::SetCaptureMouseState(bool shouldCapture)
	{
		if (shouldCapture)
		{
			SetCapture(wnd_);
		}
		else
		{
			ReleaseCapture();
		}

		isMouseCaptured_ = shouldCapture;
	}

	void js_panel_window::SetDragAndDropStatus(bool isEnabled)
	{
		isDraggingInside_ = false;
		hasInternalDrag_ = false;
		lastDragParams_.reset();

		if (isEnabled)
		{
			if (!dropTargetHandler_)
			{
				dropTargetHandler_ = new TrackDropTarget(*this);
				dropTargetHandler_->RegisterDragDrop();
			}
		}
		else
		{
			if (dropTargetHandler_)
			{
				dropTargetHandler_->RevokeDragDrop();
				dropTargetHandler_.reset();
			}
		}
	}
}
