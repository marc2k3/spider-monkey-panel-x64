#pragma once
#include <ComUtils/com_tools.h>

class CDialogHtml
	: public CAxDialogImpl<CDialogHtml>
	, public IServiceProviderImpl<CDialogHtml>
	, public IDispEventImpl<IDC_IE, CDialogHtml>
	, public IHTMLOMWindowServices
	, public IDocHostUIHandler
{
public:
	enum
	{
		IDD = IDD_DIALOG_HTML
	};

	COM_QI_BEGIN()
		COM_QI_ENTRY(IHTMLOMWindowServices)
		COM_QI_ENTRY(IServiceProvider)
		COM_QI_END()

#pragma warning(push)
#pragma warning(disable : 6388) // might not be '0'
	BEGIN_SERVICE_MAP(CDialogHtml)
#pragma warning(pop)
		SERVICE_ENTRY(SID_SHTMLOMWindowServices)
	END_SERVICE_MAP()

	BEGIN_MSG_MAP(CDialogHtml)
		CHAIN_MSG_MAP(CAxDialogImpl<CDialogHtml>)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_DESTROY(OnDestroyDialog)
		MSG_WM_SIZE(OnSize)
		MSG_WM_CLOSE(OnClose)
		COMMAND_RANGE_HANDLER_EX(IDOK, IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	BEGIN_SINK_MAP(CDialogHtml)
		SINK_ENTRY(IDC_IE, DISPID_BEFORENAVIGATE2, &CDialogHtml::OnBeforeNavigate2)
		SINK_ENTRY(IDC_IE, DISPID_TITLECHANGE, &CDialogHtml::OnTitleChange)
		SINK_ENTRY(IDC_IE, DISPID_WINDOWCLOSING, &CDialogHtml::OnWindowClosing)
	END_SINK_MAP()

public:
	class HostExternal : public JSDispatch<IHostExternal>
	{
	protected:
		HostExternal(_variant_t data) : data_(data) {}

		~HostExternal() = default;

	public:
		STDMETHODIMP get_dialogArguments(VARIANT* pData) final
		{
			if (pData)
			{
				return VariantCopy(pData, &data_);
			}
			else
			{
				return S_OK;
			}
		}

	private:
		_variant_t data_;
	};

	struct Options
	{
		uint32_t width = 250u;
		uint32_t height = 100u;
		int32_t x{};
		int32_t y{};
		bool isCentered = true;
		bool isContextMenuEnabled{};
		bool isFormSelectionEnabled{};
		bool isResizable{};
		bool isScrollEnabled{};
	};

	CDialogHtml(JSContext* ctx, const Options& options, const std::wstring& code_or_path, wil::com_ptr<HostExternal> host_external);

	LRESULT OnInitDialog(HWND hwndFocus, LPARAM lParam);
	LRESULT OnDestroyDialog();
	void OnSize(UINT nType, CSize size);
	void OnClose();
	void OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	void OnBeforeNavigate2(IDispatch* pDisp, VARIANT* URL, VARIANT* Flags, VARIANT* TargetFrameName, VARIANT* PostData, VARIANT* Headers, VARIANT_BOOL* Cancel);
	void OnTitleChange(BSTR title);
	void OnWindowClosing(VARIANT_BOOL bIsChildWindow, VARIANT_BOOL* Cancel);

	// IHTMLOMWindowServices
	STDMETHODIMP moveTo(LONG x, LONG y) final;
	STDMETHODIMP moveBy(LONG x, LONG y) final;
	STDMETHODIMP resizeTo(LONG x, LONG y) final;
	STDMETHODIMP resizeBy(LONG x, LONG y) final;

	// IDocHostUIHandler
	STDMETHODIMP ShowContextMenu(DWORD dwID, POINT* ppt, IUnknown* pcmdtReserved, IDispatch* pdispReserved) final;
	STDMETHODIMP GetHostInfo(DOCHOSTUIINFO* pInfo) final;
	STDMETHODIMP ShowUI(DWORD dwID, IOleInPlaceActiveObject* pActiveObject, IOleCommandTarget* pCommandTarget, IOleInPlaceFrame* pFrame, IOleInPlaceUIWindow* pDoc) final;
	STDMETHODIMP HideUI() final;
	STDMETHODIMP UpdateUI() final;
	STDMETHODIMP EnableModeless(BOOL fEnable) final;
	STDMETHODIMP OnDocWindowActivate(BOOL fActivate) final;
	STDMETHODIMP OnFrameWindowActivate(BOOL fActivate) final;
	STDMETHODIMP ResizeBorder(LPCRECT prcBorder, IOleInPlaceUIWindow* pUIWindow, BOOL fRameWindow) final;
	STDMETHODIMP TranslateAcceleratorW(LPMSG lpMsg, const GUID* pguidCmdGroup, DWORD nCmdID) final;
	STDMETHODIMP GetOptionKeyPath(LPOLESTR* pchKey, DWORD dw) final;
	STDMETHODIMP GetDropTarget(IDropTarget* pDropTarget,IDropTarget** ppDropTarget) final;
	STDMETHODIMP GetExternal(IDispatch** ppDispatch) final;
	STDMETHODIMP TranslateUrl(DWORD dwTranslate, LPWSTR pchURLIn, LPWSTR* ppchURLOut) final;
	STDMETHODIMP FilterDataObject(IDataObject* pDO, IDataObject** ppDORet) final;

	// IUnknown
	ULONG STDMETHODCALLTYPE AddRef() final;
	ULONG STDMETHODCALLTYPE Release() final;

private:
	void SetOptions();

	static void GetMsgProc(int code, WPARAM wParam, LPARAM lParam, HWND hParent, CDialogHtml* pParent);

	JSContext* m_ctx{};
	Options m_options;
	bool m_closing{};
	std::wstring m_code_or_path;
	uint32_t m_hook_id{};

	wil::com_ptr<HostExternal> m_host_external;
	IDocHostUIHandlerPtr m_host_ui_handler;
	IOleInPlaceActiveObjectPtr m_active_object;
};
