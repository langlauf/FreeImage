// Multipage.h : Deklaration von CMultipage

#ifndef __MULTIPAGE_H_
#define __MULTIPAGE_H_

#include "resource.h"       // Hauptsymbole

/////////////////////////////////////////////////////////////////////////////
// CMultipage
class ATL_NO_VTABLE CMultipage : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CMultipage, &CLSID_Multipage>,
	public IConnectionPointContainerImpl<CMultipage>,
	public IDispatchImpl<IMultipage, &IID_IMultipage, &LIBID_FREEIMAGE_ACTIVEXLib>
{
public:
	CMultipage();
	virtual ~CMultipage();

DECLARE_REGISTRY_RESOURCEID(IDR_MULTIPAGE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CMultipage)
	COM_INTERFACE_ENTRY(IMultipage)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
END_COM_MAP()
BEGIN_CONNECTION_POINT_MAP(CMultipage)
END_CONNECTION_POINT_MAP()


// IMultipage
public:
	STDMETHOD(Test)(BSTR msg, BSTR title);

};

#endif //__MULTIPAGE_H_
