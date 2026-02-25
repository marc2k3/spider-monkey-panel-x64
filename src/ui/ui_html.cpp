#include "PCH.hpp"
#include "ui_html.h"

#include <ComUtils/dispatch_ptr.h>
#include <JsBackend/js_to_native.h>
#include <utils/hook_handler.h>

using namespace mozjs;

CDialogHtml::CDialogHtml(JSContext* ctx, const Options& options, const std::wstring& code_or_path, wil::com_ptr<HostExternal> host_external)
	: m_ctx(ctx)
	, m_options(options)
	, m_code_or_path(code_or_path)
	, m_host_external(std::move(host_external)) {}

LRESULT CDialogHtml::OnInitDialog(HWND, LPARAM)
{
	SetIcon(ui_control::get()->get_main_icon());
	SetOptions();

	auto autoExit = wil::scope_exit([&]
		{
			EndDialog(-1);
		});

	auto hIE = GetDlgItem(IDC_IE).m_hWnd;

	try
	{
		CAxWindow wndIE = hIE;
		IObjectWithSitePtr pOWS;
		IDispatchPtr pDocDispatch;
		IWebBrowserPtr pBrowser;
		IHTMLDocument2Ptr pDocument;
		_variant_t v;

		THROW_IF_FAILED(wndIE.QueryHost(IID_IObjectWithSite, reinterpret_cast<void**>(&pOWS)));
		THROW_IF_FAILED(pOWS->SetSite(static_cast<IServiceProvider*>(this)));
		THROW_IF_FAILED(wndIE.QueryControl(&pBrowser));
		THROW_IF_FAILED(pBrowser->Navigate(_bstr_t(L"about:blank"), &v, &v, &v, &v));
		THROW_IF_FAILED(pBrowser->get_Document(&pDocDispatch));

		pDocument = pDocDispatch;

		// Request default handler from MSHTML client site
		IOleObjectPtr pOleObject(pDocument);
		IOleClientSitePtr pClientSite;
		THROW_IF_FAILED(pOleObject->GetClientSite(&pClientSite));

		m_host_ui_handler = pClientSite;

		// Set the new custom IDocHostUIHandler
		ICustomDocPtr pCustomDoc(pDocument);
		THROW_IF_FAILED(pCustomDoc->SetUIHandler(this));

		m_active_object = pBrowser;

		if (m_code_or_path.starts_with(L"file://"))
		{
			auto path = _bstr_t(m_code_or_path.c_str());
			THROW_IF_FAILED(pBrowser->Navigate(path, &v, &v, &v, &v));
		}
		else
		{
			SAFEARRAY* pSaStrings = SafeArrayCreateVector(VT_VARIANT, 0, 1);
			VARIANT* pSaVar{};

			auto autoPsa = wil::scope_exit([pSaStrings]
				{
					SafeArrayDestroy(pSaStrings);
				});

			THROW_IF_FAILED(pDocument->put_designMode(_bstr_t(L"on")));
			THROW_IF_FAILED(SafeArrayAccessData(pSaStrings, reinterpret_cast<void**>(&pSaVar)));

			auto code = _bstr_t(m_code_or_path.c_str());
			pSaVar->vt = VT_BSTR;
			pSaVar->bstrVal = code.Detach();

			THROW_IF_FAILED(SafeArrayUnaccessData(pSaStrings));
			THROW_IF_FAILED(pDocument->write(pSaStrings));
			THROW_IF_FAILED(pDocument->put_designMode(_bstr_t(L"off")));
			THROW_IF_FAILED(pDocument->close());
		}

		wndIE.SetFocus();
	}
	catch (...)
	{
		mozjs::ExceptionToJsError(m_ctx);
		return -1;
	}

	m_hook_id = QwrHookHandler::GetInstance().RegisterHook([hIE, this](int code, WPARAM wParam, LPARAM lParam)
		{
			GetMsgProc(code, wParam, lParam, hIE, this);
		});

	autoExit.release();
	return FALSE;
}

LRESULT CDialogHtml::OnDestroyDialog()
{
	if (m_hook_id)
	{
		QwrHookHandler::GetInstance().UnregisterHook(m_hook_id);
		m_hook_id = 0;
	}

	return 0;
}

void CDialogHtml::OnSize(UINT nType, CSize size)
{
	switch (nType)
	{
	case SIZE_MAXIMIZED:
	case SIZE_RESTORED:
	{
		CAxWindow wndIE = static_cast<HWND>(GetDlgItem(IDC_IE));
		wndIE.ResizeClient(size.cx, size.cy);
		break;
	}
	default:
		break;
	}
}

void CDialogHtml::OnClose()
{
	m_closing = true;
	OnCloseCmd(0, IDCANCEL, nullptr);
}

void CDialogHtml::OnCloseCmd(WORD, WORD wID, HWND)
{
	if (!m_closing)
	{ 
		// e.g. pressed RETURN
		return;
	}

	EndDialog(wID);
}

void CDialogHtml::OnBeforeNavigate2(IDispatch*, VARIANT* URL, VARIANT*, VARIANT*, VARIANT*, VARIANT*, VARIANT_BOOL* Cancel)
{
	if (!Cancel || !URL)
	{
		return;
	}

	*Cancel = VARIANT_FALSE;

	try
	{
		_bstr_t url_b(*URL);
		for (const auto& urlPrefix : std::to_array<std::wstring>({ L"http://", L"https://" }))
		{
			if (url_b.length() > urlPrefix.length() && !wmemcmp(url_b.GetBSTR(), urlPrefix.c_str(), urlPrefix.length()))
			{
				if (Cancel)
				{
					*Cancel = VARIANT_TRUE;
					return;
				}
			}
		}
	}
	catch (const _com_error&) {}
}

void CDialogHtml::OnTitleChange(BSTR title)
{
	SetWindowTextW(title);
}

void CDialogHtml::OnWindowClosing(VARIANT_BOOL, VARIANT_BOOL* Cancel)
{
	EndDialog(IDOK);

	if (Cancel)
	{
		*Cancel = VARIANT_TRUE;
	}
}

STDMETHODIMP CDialogHtml::moveTo(LONG x, LONG y)
{
	if (RECT rect; GetWindowRect(&rect))
	{
		MoveWindow(x, y, (rect.right - rect.left), (rect.bottom - rect.top));
	}

	return S_OK;
}

STDMETHODIMP CDialogHtml::moveBy(LONG x, LONG y)
{
	if (RECT rect; GetWindowRect(&rect))
	{
		MoveWindow(rect.left + x, rect.top + y, (rect.right - rect.left), (rect.bottom - rect.top));
	}

	return S_OK;
}

STDMETHODIMP CDialogHtml::resizeTo(LONG x, LONG y)
{
	if (RECT windowRect, clientRect; GetWindowRect(&windowRect) && GetClientRect(&clientRect))
	{
		const LONG clientW = x - ((windowRect.right - windowRect.left) - clientRect.right);
		const LONG clientH = y - ((windowRect.bottom - windowRect.top) - clientRect.bottom);
		ResizeClient(clientW, clientH);
	}

	return S_OK;
}

STDMETHODIMP CDialogHtml::resizeBy(LONG x, LONG y)
{
	if (RECT clientRect; GetClientRect(&clientRect))
	{
		const LONG clientW = x + clientRect.right;
		const LONG clientH = y + clientRect.bottom;
		ResizeClient(clientW, clientH);
	}

	return S_OK;
}

STDMETHODIMP CDialogHtml::ShowContextMenu(DWORD dwID, POINT* ppt, IUnknown* pcmdtReserved, IDispatch* pdispReserved)
{
	if (dwID == CONTEXT_MENU_TEXTSELECT || dwID == CONTEXT_MENU_CONTROL)
	{
		// always show context menu for text editors
		return S_FALSE;
	}

	if (m_host_ui_handler && m_options.isContextMenuEnabled)
	{
		return m_host_ui_handler->ShowContextMenu(dwID, ppt, pcmdtReserved, pdispReserved);
	}

	return S_OK;
}

STDMETHODIMP CDialogHtml::GetHostInfo(DOCHOSTUIINFO* pInfo)
{
	if (m_host_ui_handler)
	{
		RETURN_IF_FAILED(m_host_ui_handler->GetHostInfo(pInfo));

		if (pInfo)
		{
			if (!m_options.isFormSelectionEnabled)
			{
				pInfo->dwFlags |= DOCHOSTUIFLAG_DIALOG;
			}
			if (!m_options.isScrollEnabled)
			{
				pInfo->dwFlags |= DOCHOSTUIFLAG_SCROLL_NO;
			}
		}
	}

	return S_OK;
}

STDMETHODIMP CDialogHtml::ShowUI(DWORD dwID, IOleInPlaceActiveObject* pActiveObject, IOleCommandTarget* pCommandTarget, IOleInPlaceFrame* pFrame, IOleInPlaceUIWindow* pDoc)
{
	if (m_host_ui_handler)
	{
		return m_host_ui_handler->ShowUI(dwID, pActiveObject, pCommandTarget, pFrame, pDoc);
	}

	return S_OK;
}

STDMETHODIMP CDialogHtml::HideUI()
{
	if (m_host_ui_handler)
	{
		return m_host_ui_handler->HideUI();
	}

	return S_OK;
}

STDMETHODIMP CDialogHtml::UpdateUI()
{
	if (m_host_ui_handler)
	{
		return m_host_ui_handler->UpdateUI();
	}

	return S_OK;
}

STDMETHODIMP CDialogHtml::EnableModeless(BOOL fEnable)
{
	if (m_host_ui_handler)
	{
		return m_host_ui_handler->EnableModeless(fEnable);
	}

	return S_OK;
}

STDMETHODIMP CDialogHtml::OnDocWindowActivate(BOOL fActivate)
{
	if (m_host_ui_handler)
	{
		return m_host_ui_handler->OnDocWindowActivate(fActivate);
	}

	return S_OK;
}

STDMETHODIMP CDialogHtml::OnFrameWindowActivate(BOOL fActivate)
{
	if (m_host_ui_handler)
	{
		return m_host_ui_handler->OnFrameWindowActivate(fActivate);
	}

	return S_OK;
}

STDMETHODIMP CDialogHtml::ResizeBorder(LPCRECT prcBorder, IOleInPlaceUIWindow* pUIWindow, BOOL fRameWindow)
{
	if (m_host_ui_handler)
	{
		return m_host_ui_handler->ResizeBorder(prcBorder, pUIWindow, fRameWindow);
	}

	return S_OK;
}

STDMETHODIMP CDialogHtml::TranslateAcceleratorW(LPMSG lpMsg, const GUID* pguidCmdGroup, DWORD nCmdID)
{
	if (!m_host_ui_handler)
	{
		return S_OK;
	}

	auto isSupportedHotKey = [](UINT wm, int vk) -> bool
		{
			if (wm != WM_KEYDOWN && wm != WM_KEYUP && wm != WM_SYSKEYDOWN && wm != WM_SYSKEYUP)
			{
				return false;
			}

			const bool isCtrlPressed = HIBYTE(GetKeyState(VK_CONTROL));
			const bool isShiftPressed = HIBYTE(GetKeyState(VK_SHIFT));
			const bool isAltPressed = HIBYTE(GetKeyState(VK_MENU));

			constexpr std::array allowedCtrlKeys{
				0x41, // A
				0x43, // C
				0x56, // V
				0x58, // X
				0x59, // Y
				0x5A  // Z
			};

			return isCtrlPressed && !isShiftPressed && !isAltPressed && allowedCtrlKeys.end() != std::ranges::find(allowedCtrlKeys, vk);
		};

	if (isSupportedHotKey(lpMsg->message, static_cast<int>(lpMsg->wParam)))
	{
		return m_host_ui_handler->TranslateAccelerator(lpMsg, pguidCmdGroup, nCmdID);
	}
	else
	{
		if (WM_KEYDOWN == lpMsg->message && VK_ESCAPE == lpMsg->wParam)
		{
			// Restore default dialog behaviour
			m_closing = true;
		}
		return S_FALSE;
	}
}

STDMETHODIMP CDialogHtml::GetOptionKeyPath(LPOLESTR* pchKey, DWORD dw)
{
	if (m_host_ui_handler)
	{
		return m_host_ui_handler->GetOptionKeyPath(pchKey, dw);
	}

	return S_OK;
}

STDMETHODIMP CDialogHtml::GetDropTarget(IDropTarget* pDropTarget, IDropTarget** ppDropTarget)
{
	if (m_host_ui_handler)
	{
		return m_host_ui_handler->GetDropTarget(pDropTarget, ppDropTarget);
	}

	return S_OK;
}

STDMETHODIMP CDialogHtml::GetExternal(IDispatch** ppDispatch)
{
	RETURN_HR_IF_NULL(E_POINTER, ppDispatch);

	if (m_host_ui_handler)
	{
		if (m_host_external)
		{
			m_host_external->AddRef();
			*ppDispatch = m_host_external.get();
			return S_OK;
		}

		return m_host_ui_handler->GetExternal(ppDispatch);
	}

	return S_OK;
}

STDMETHODIMP CDialogHtml::TranslateUrl(DWORD dwTranslate, LPWSTR pchURLIn, LPWSTR* ppchURLOut)
{
	if (m_host_ui_handler)
	{
		return m_host_ui_handler->TranslateUrl(dwTranslate, pchURLIn, ppchURLOut);
	}

	return S_OK;
}

STDMETHODIMP CDialogHtml::FilterDataObject(IDataObject* pDO, IDataObject** ppDORet)
{
	if (m_host_ui_handler)
	{
		return m_host_ui_handler->FilterDataObject(pDO, ppDORet);
	}

	return S_OK;
}

ULONG STDMETHODCALLTYPE CDialogHtml::AddRef()
{
	return 0;
}

ULONG STDMETHODCALLTYPE CDialogHtml::Release()
{
	return 0;
}

void CDialogHtml::SetOptions()
{
	if (!m_options.isResizable)
	{
		ModifyStyle(WS_THICKFRAME, WS_BORDER, SWP_FRAMECHANGED);
	}

	resizeTo(m_options.width, m_options.height);

	if (m_options.isCentered)
	{
		CenterWindow();
	}

	if (m_options.x > 0 || m_options.y > 0)
	{
		moveTo(m_options.x, m_options.y);
	}
}

void CDialogHtml::GetMsgProc(int, WPARAM, LPARAM lParam, HWND hParent, CDialogHtml* pParent)
{
	if (auto pMsg = reinterpret_cast<LPMSG>(lParam); pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST)
	{
		// Only react to keypress events
		for (HWND tmpHwnd = pMsg->hwnd; tmpHwnd && (::GetWindowLong(tmpHwnd, GWL_STYLE) & WS_CHILD); tmpHwnd = ::GetParent(tmpHwnd))
		{
			if (tmpHwnd == hParent)
			{
				CDialogHtml* pThis = pParent;
				if (pThis && pThis->m_active_object && S_OK == pThis->m_active_object->TranslateAccelerator(pMsg))
				{
					pMsg->message = WM_NULL;
				}
				break;
			}
		}
	}
}
