// Painting.h : Deklaration von CPainting

#ifndef __PAINTING_H_
#define __PAINTING_H_

#include "resource.h"       // Hauptsymbole
#include <atlctl.h>

enum {VTEX_FIXED_SIZE_BSTR_BLOB = 999};


/////////////////////////////////////////////////////////////////////////////
// CPainting
class ATL_NO_VTABLE CPainting : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CStockPropImpl<CPainting, IPainting, &IID_IPainting, &LIBID_FREEIMAGE_ACTIVEXLib>,
	public CComControl<CPainting>,
	public IOleControlImpl<CPainting>,
	public IOleObjectImpl<CPainting>,
	public IOleInPlaceActiveObjectImpl<CPainting>,
	public IViewObjectExImpl<CPainting>,
	public IOleInPlaceObjectWindowlessImpl<CPainting>,
	public ISpecifyPropertyPagesImpl<CPainting>,
	public IQuickActivateImpl<CPainting>,
	public IDataObjectImpl<CPainting>,
	public IProvideClassInfo2Impl<&CLSID_Painting, NULL, &LIBID_FREEIMAGE_ACTIVEXLib>,
	public CComCoClass<CPainting, &CLSID_Painting>,
	public IObjectWithSiteImpl<CPainting>,
	// Persistence
	public IPersistStreamInitImpl<CPainting>,
	public IPersistStorageImpl<CPainting>,
	public IPersistPropertyBagImpl<CPainting>
//	public IPerPropertyBrowsingImpl<CPainting>,
//	public IOleLinkImpl<CPainting>,
//	public IRunnableObjectImpl<CPainting>
{
public:
	CPainting();
	virtual ~CPainting();

DECLARE_REGISTRY_RESOURCEID(IDR_PAINTING)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CPainting)
	COM_INTERFACE_ENTRY(IPainting)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IViewObjectEx)
	COM_INTERFACE_ENTRY(IViewObject2)
	COM_INTERFACE_ENTRY(IViewObject)
	COM_INTERFACE_ENTRY(IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY(IOleInPlaceObject)
	COM_INTERFACE_ENTRY2(IOleWindow, IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY(IOleInPlaceActiveObject)
	COM_INTERFACE_ENTRY(IOleControl)
	COM_INTERFACE_ENTRY(IOleObject)
	COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
	COM_INTERFACE_ENTRY(IQuickActivate)
	COM_INTERFACE_ENTRY(IDataObject)
	COM_INTERFACE_ENTRY(IProvideClassInfo)
	COM_INTERFACE_ENTRY(IProvideClassInfo2)
	COM_INTERFACE_ENTRY(IObjectWithSite)
	// Persistence
	COM_INTERFACE_ENTRY(IPersistStreamInit)
	COM_INTERFACE_ENTRY2(IPersist, IPersistStreamInit)
	COM_INTERFACE_ENTRY(IPersistStorage)
	COM_INTERFACE_ENTRY(IPersistPropertyBag)
//	COM_INTERFACE_ENTRY_IID(IID_IPersist, IPersistPropertyBag)
	// whats that
//	COM_INTERFACE_ENTRY(IRunnableObject)
//	COM_INTERFACE_ENTRY(IOleLink)
//	COM_INTERFACE_ENTRY(IPerPropertyBrowsing)
END_COM_MAP()

BEGIN_PROP_MAP(CPainting)
	PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
	PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
	PROP_ENTRY("AutoSize", DISPID_AUTOSIZE, CLSID_NULL)
	PROP_ENTRY("Enabled", DISPID_ENABLED, CLSID_NULL)
	PROP_ENTRY("Valid", DISPID_VALID, CLSID_NULL)
	PROP_ENTRY("SRC",  dispid_SRC, CLSID_NULL)
	PROP_ENTRY("Zoom", dispid_ZOOM, CLSID_NULL)
END_PROP_MAP()


BEGIN_MSG_MAP(CPainting)
	CHAIN_MSG_MAP(CComControl<CPainting>)
	DEFAULT_REFLECTION_HANDLER()
END_MSG_MAP()

//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	void	Close(BOOL bEmptyFilename);
	void	ImageChanged();

// IViewObjectEx
	DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

// IPersistStreamImpl
	STDMETHOD(Load)(IStream *pStream);
	STDMETHOD(Save)(IStream *pStream, BOOL fClearDirty);
	STDMETHOD(GetSizeMax)(ULARGE_INTEGER* pcbSize);

// IPersistStorageImpl
	STDMETHOD(Load)(IStorage *Storage); 
	STDMETHOD(Save)(IStorage *pStorage, BOOL fSameAsLoad);

// IPersistPropertBag
	STDMETHOD(Load)(IPropertyBag *pPropBag, IErrorLog* pErrorLog);
	STDMETHOD(Save)(IPropertyBag *pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties);

// IPainting
public:
	STDMETHOD(get_Zoom)(/*[out, retval]*/ double *pVal);
	STDMETHOD(put_Zoom)(/*[in]*/ double newVal);
	STDMETHOD(get_SRC)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_SRC)(/*[in]*/ BSTR newVal);

	virtual HRESULT OnDrawAdvanced(ATL_DRAWINFO& di);
	HRESULT OnDraw(ATL_DRAWINFO& di);
	HRESULT OnDrawImage(BOOL bMetafile, ATL_DRAWINFO& di);

	BOOL m_bEnabled;
//	CComPtr<IPictureDisp> m_pPicture;
	BOOL m_bValid;

	FIBITMAP	*m_pCurrentImage;
	CComBSTR	m_currentFilename;
	double		m_Zoom;
	CComBSTR	m_bstrAuthor;
	CComBSTR	m_bstrEmail;
	CComBSTR	m_bstrDesc;
	IStorage	*m_pIStorage;
	void Insert (BSTR *stream, void *pData, DWORD size);
	void Extract (LPCWSTR *stream, void *pData, DWORD size);
};

#endif //__PAINTING_H_
