//
// ==========================================================
// FreeImage ActiveX-Interface
//
// Design and implementation by
// - Markus Loibl (markus.loibl@epost.de)
//
// This file is part of FreeImage 2.6.0
//
// COVERED CODE IS PROVIDED UNDER THIS LICENSE ON AN "AS IS" BASIS, WITHOUT WARRANTY
// OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, WITHOUT LIMITATION, WARRANTIES
// THAT THE COVERED CODE IS FREE OF DEFECTS, MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE
// OR NON-INFRINGING. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE COVERED
// CODE IS WITH YOU. SHOULD ANY COVERED CODE PROVE DEFECTIVE IN ANY RESPECT, YOU (NOT
// THE INITIAL DEVELOPER OR ANY OTHER CONTRIBUTOR) ASSUME THE COST OF ANY NECESSARY
// SERVICING, REPAIR OR CORRECTION. THIS DISCLAIMER OF WARRANTY CONSTITUTES AN ESSENTIAL
// PART OF THIS LICENSE. NO USE OF ANY COVERED CODE IS AUTHORIZED HEREUNDER EXCEPT UNDER
// THIS DISCLAIMER.
//
// Use at your own risk!
// ==========================================================
//
#ifndef __FREEIMAGE_X_APPLICATION_H__
#define __FREEIMAGE_X_APPLICATION_H__

#include "resource.h"       // Hauptsymbole

/////////////////////////////////////////////////////////////////////////////
//
// CApplication
//
/////////////////////////////////////////////////////////////////////////////

class ATL_NO_VTABLE CApplication : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CApplication, &CLSID_Application>,
	public IConnectionPointContainerImpl<CApplication>,
	public IDispatchImpl<IApplication, &IID_IApplication, &LIBID_FREEIMAGE_ACTIVEXLib>
{

public:
	CApplication()
	{
	}

	DECLARE_REGISTRY_RESOURCEID(IDR_APPLICATION)
	DECLARE_PROTECT_FINAL_CONSTRUCT()
	BEGIN_COM_MAP(CApplication)
		COM_INTERFACE_ENTRY(IApplication)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(IConnectionPointContainer)
	END_COM_MAP()
	BEGIN_CONNECTION_POINT_MAP(CApplication)
	END_CONNECTION_POINT_MAP()

public:
	STDMETHOD(get_Copyright)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(get_Version)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(Test)(BSTR msg, BSTR title);
	STDMETHOD(IsValidImage)(/*[in]*/ BSTR filename, /*[out, retval]*/ short *pbIsValid);
	STDMETHOD(get_ShowMessages)(/*[out, retval]*/ short *pVal);
	STDMETHOD(put_ShowMessages)(/*[in]*/ short newVal);
	STDMETHOD(GetImageType)(/*[in]*/ short flags, /*[in]*/ BSTR filename, /*[out, retval]*/ short *pVal);
	STDMETHOD(get_SupportsReading)(/*[in]*/ short type, /*[out, retval]*/ short *pVal);
	STDMETHOD(get_SupportsWriting)(/*[in]*/ short type, /*[out, retval]*/ short *pVal);
};

#endif //__FREEIMAGE_X_APPLICATION_H__

