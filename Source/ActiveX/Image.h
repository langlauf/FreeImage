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

#if !defined(AFX_IMAGE_H__F74D6423_C0C8_42BE_9CB7_06D0B6DA9FF2__INCLUDED_)
#define AFX_IMAGE_H__F74D6423_C0C8_42BE_9CB7_06D0B6DA9FF2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"   // Main symbols

/////////////////////////////////////////////////////////////////////////////
//
// CImage
//
/////////////////////////////////////////////////////////////////////////////

class ATL_NO_VTABLE CImage : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CImage, &CLSID_Image>,
	public IConnectionPointContainerImpl<CImage>,
	public IDispatchImpl<IImage, &IID_IImage, &LIBID_FREEIMAGE_ACTIVEXLib>
{

public:
	CImage();
	virtual ~CImage();

	void	ImageChanged();

	DECLARE_REGISTRY_RESOURCEID(IDR_IMAGE)
	DECLARE_PROTECT_FINAL_CONSTRUCT()
	BEGIN_COM_MAP(CImage)
		COM_INTERFACE_ENTRY(IImage)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(IConnectionPointContainer)
	END_COM_MAP()
	BEGIN_CONNECTION_POINT_MAP(CImage)
	END_CONNECTION_POINT_MAP()

public:
	STDMETHOD(get_ResolutionX)(/*[out, retval]*/ double *pVal);
	STDMETHOD(put_ResolutionX)(/*[in]*/ double newVal);
	STDMETHOD(get_ResolutionY)(/*[out, retval]*/ double *pVal);
	STDMETHOD(put_ResolutionY)(/*[in]*/ double newVal);
	STDMETHOD(Close)();
	STDMETHOD(Save)(/*[in]*/ BSTR filename, /*[out, retval]*/ short *pbSuccess);
	STDMETHOD(Open)(/*[in]*/ BSTR filename, /*[out, retval]*/ short *pbSuccess);
	STDMETHOD(Test)(BSTR msg, BSTR title);
	STDMETHOD(OpenEx)(/*[in]*/ short type, /*[in]*/ BSTR filename, /*[in]*/ short params, /*[out, retval]*/ short *pbSuccess);
	STDMETHOD(SaveEx)(/*[in]*/ short type, /*[in]*/ BSTR filename, /*[in]*/ short params, /*[out, retval]*/ short *pbSuccess);
	STDMETHOD(get_Width)(/*[out, retval]*/ long *pVal);
	STDMETHOD(get_Height)(/*[out, retval]*/ long *pVal);
	STDMETHOD(get_BPP)(/*[out, retval]*/ long *pVal);
	STDMETHOD(get_DIBSize)(/*[out, retval]*/ long *pVal);
	STDMETHOD(get_RedMask)(/*[out, retval]*/ long *pVal);
	STDMETHOD(get_GreenMask)(/*[out, retval]*/ long *pVal);
	STDMETHOD(get_BlueMask)(/*[out, retval]*/ long *pVal);
	STDMETHOD(get_ColorsUsed)(/*[out, retval]*/ long *pVal);
	STDMETHOD(get_ColorType)(/*[out, retval]*/ short *pVal);
	STDMETHOD(get_IsTransparent)(/*[out, retval]*/ short *pVal);
	STDMETHOD(ChangeColors)(/*[in]*/ short colors, /*[in]*/ short param, /*[out, retval]*/ short *pVal);
	STDMETHOD(Clone)(/*[out, retval]*/ IImage **ppImage);
	STDMETHOD(FromHandle)(/*[in]*/ LPUNKNOWN pHandle);

protected:
	
	bool	ImageIsLoaded();
	
	FIBITMAP	*m_pCurrentImage;
};

#endif // !defined(AFX_IMAGE_H__F74D6423_C0C8_42BE_9CB7_06D0B6DA9FF2__INCLUDED_)
