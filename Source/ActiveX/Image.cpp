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

#include "stdafx.h"
#include "FreeImage_ActiveX.h"
#include "Image.h"

/////////////////////////////////////////////////////////////////////////////
//
// CImage
//
/////////////////////////////////////////////////////////////////////////////

	#ifdef _LOG_ME_
		const char *szLog = "G:\\Library\\graphics\\freeimage\\current\\253\\Source\\ActiveX\\test\\clone.txt";
	#endif

CImage::CImage()
{
//_CrtDbgBreak();
	m_pCurrentImage = NULL;

	#ifdef _LOG_ME_
		FILE *pFile = fopen(szLog, "at");
		if( pFile )
			fprintf(pFile, "creating CImage %p\n", this);
		fclose(pFile);
	#endif
}

CImage::~CImage()
{
	#ifdef _LOG_ME_
		FILE *pFile = fopen(szLog, "at");
		if( pFile )
			fprintf(pFile, "destroying CImage %p\n", this);
		fclose(pFile);
	#endif

	Close();
}

void CImage::ImageChanged()
{
}

bool CImage::ImageIsLoaded()
{
	return (m_pCurrentImage!=NULL);
}

STDMETHODIMP CImage::Test(BSTR msg, BSTR title)
{
	USES_CONVERSION;
	::MessageBox(NULL, W2A(msg), W2A(title), MB_OK|MB_TOPMOST|MB_SETFOREGROUND);

	return S_OK;
}

// -----------------------------------------------------------------------
//
// Opening and saving ...
//
// -----------------------------------------------------------------------

STDMETHODIMP CImage::Open(BSTR filename, short *pbSuccess)
{
	USES_CONVERSION;
	return OpenEx(FreeImage_GetFIFFromFilename(W2A(filename)), filename, 0, pbSuccess);
}

STDMETHODIMP CImage::OpenEx(short type, BSTR filename, short params, short *pbSuccess)
{
	USES_CONVERSION;

	Close();
	*pbSuccess		= 0;
	
	if( type == FIF_UNKNOWN )
		return E_FAIL;

	try
	{
		m_pCurrentImage	= FreeImage_Load((FREE_IMAGE_FORMAT)type, W2A(filename), params);
	}
	catch(...)
	{
		return E_FAIL;
	}

	ImageChanged();

	if( m_pCurrentImage )
		*pbSuccess	= 1;

	return S_OK;
}

STDMETHODIMP CImage::Save(BSTR filename, short *pbSuccess)
{
	USES_CONVERSION;
	return SaveEx(FreeImage_GetFIFFromFilename(W2A(filename)), filename, 0, pbSuccess);
}

STDMETHODIMP CImage::SaveEx(short type, BSTR filename, short params, short *pbSuccess)
{
	USES_CONVERSION;
	BOOL				bErg = FALSE;

//	#ifdef _DEBUG
//		_CrtDbgBreak();
//	#endif

	*pbSuccess		= 0;
	
	if( !ImageIsLoaded() || type==FIF_UNKNOWN )
		return E_FAIL;

	try
	{
		bErg = FreeImage_Save((FREE_IMAGE_FORMAT)type, m_pCurrentImage, W2A(filename), params);
	}
	catch(...)
	{
		return E_FAIL;
	}
	
	if( bErg )
		*pbSuccess	= 1;

	return S_OK;
}

STDMETHODIMP CImage::Close()
{
	if( m_pCurrentImage )
	{
		FreeImage_Unload(m_pCurrentImage);
		m_pCurrentImage = NULL;
		ImageChanged();
	}
	return S_OK;
}

// -----------------------------------------------------------------------
//
// image-infos ...
//
// -----------------------------------------------------------------------


STDMETHODIMP CImage::get_ResolutionX(double *pVal)
{
	BITMAPINFOHEADER	*pHeader	= (m_pCurrentImage ? FreeImage_GetInfoHeader(m_pCurrentImage) : NULL);
	LONG				resDPM;
	double				resDPI		= 0.0;
	
	if( pHeader )
	{
		resDPM = pHeader->biXPelsPerMeter;
		resDPI = resDPM * 0.0254;
	}

	*pVal = resDPI;
	return (pHeader ? S_OK : E_FAIL);
}

STDMETHODIMP CImage::put_ResolutionX(double newVal)
{
	BITMAPINFOHEADER	*pHeader	= (m_pCurrentImage ? FreeImage_GetInfoHeader(m_pCurrentImage) : NULL);
	double				resDPM;
	
	if( pHeader )
	{
		resDPM = newVal / 0.0254;
		pHeader->biXPelsPerMeter = (LONG) (resDPM + 0.5);
	}

	return (pHeader ? S_OK : E_FAIL);
}

STDMETHODIMP CImage::get_ResolutionY(double *pVal)
{
	BITMAPINFOHEADER	*pHeader	= (m_pCurrentImage ? FreeImage_GetInfoHeader(m_pCurrentImage) : NULL);
	LONG				resDPM;
	double				resDPI		= 0.0;
	
	if( pHeader )
	{
		resDPM = pHeader->biYPelsPerMeter;
		resDPI = resDPM * 0.0254;
	}

	*pVal = resDPI;
	return (pHeader ? S_OK : E_FAIL);
}

STDMETHODIMP CImage::put_ResolutionY(double newVal)
{
	BITMAPINFOHEADER	*pHeader	= (m_pCurrentImage ? FreeImage_GetInfoHeader(m_pCurrentImage) : NULL);
	double				resDPM;
	
	if( pHeader )
	{
		resDPM = newVal / 0.0254;
		pHeader->biYPelsPerMeter = (LONG) (resDPM + 0.5);;
	}

	return (pHeader ? S_OK : E_FAIL);
}

STDMETHODIMP CImage::get_Width(long *pVal)
{
	if( !ImageIsLoaded() )
		return E_FAIL;
	
	*pVal = FreeImage_GetWidth(m_pCurrentImage);
	return S_OK;
}

STDMETHODIMP CImage::get_Height(long *pVal)
{
	if( !ImageIsLoaded() )
		return E_FAIL;
	
	*pVal = FreeImage_GetHeight(m_pCurrentImage);
	return S_OK;
}

STDMETHODIMP CImage::get_BPP(long *pVal)
{
	if( !ImageIsLoaded() )
		return E_FAIL;
	
	*pVal = FreeImage_GetBPP(m_pCurrentImage);
	return S_OK;
}

STDMETHODIMP CImage::get_DIBSize(long *pVal)
{
	if( !ImageIsLoaded() )
		return E_FAIL;
	
	*pVal = FreeImage_GetDIBSize(m_pCurrentImage);
	return S_OK;
}

STDMETHODIMP CImage::get_RedMask(long *pVal)
{
	if( !ImageIsLoaded() )
		return E_FAIL;
	
	*pVal = FreeImage_GetRedMask(m_pCurrentImage);
	return S_OK;
}

STDMETHODIMP CImage::get_GreenMask(long *pVal)
{
	if( !ImageIsLoaded() )
		return E_FAIL;
	
	*pVal = FreeImage_GetGreenMask(m_pCurrentImage);
	return S_OK;
}

STDMETHODIMP CImage::get_BlueMask(long *pVal)
{
	if( !ImageIsLoaded() )
		return E_FAIL;
	
	*pVal = FreeImage_GetBlueMask(m_pCurrentImage);
	return S_OK;
}

STDMETHODIMP CImage::get_ColorsUsed(long *pVal)
{
	if( !ImageIsLoaded() )
		return E_FAIL;
	
	*pVal = FreeImage_GetColorsUsed(m_pCurrentImage);
	return S_OK;
}

STDMETHODIMP CImage::get_ColorType(short *pVal)
{
	if( !ImageIsLoaded() )
		return E_FAIL;
	
	*pVal = FreeImage_GetColorType(m_pCurrentImage);
	return S_OK;
}

STDMETHODIMP CImage::get_IsTransparent(short *pVal)
{
	if( !ImageIsLoaded() )
		return E_FAIL;
	
	*pVal = FreeImage_IsTransparent(m_pCurrentImage);
	return S_OK;
}

// -----------------------------------------------------------------------
//
// Modify image...
//
// -----------------------------------------------------------------------

STDMETHODIMP CImage::ChangeColors(short colors, short param, short *pVal)
{
	*pVal = 0;

	if( !ImageIsLoaded() )
		return E_FAIL;

	FIBITMAP	*pNew = NULL;

//	#ifdef _DEBUG
//		_CrtDbgBreak();
//	#endif

	if( colors == (short) FreeImage_GetBPP(m_pCurrentImage) )
	{
		// already ok
		*pVal = 2;
		return S_OK;
	}
	
	switch( colors )
	{
		case 1:		// TODO how can this be archieved ?
					break;

		case 8:		pNew = FreeImage_ConvertTo8Bits(m_pCurrentImage);
					break;

		case 256:	if( param == FIQ_WUQUANT ) 
						pNew = FreeImage_ColorQuantize(m_pCurrentImage, FIQ_WUQUANT);
					else if (param == FIQ_NNQUANT)
						pNew = FreeImage_ColorQuantize(m_pCurrentImage, FIQ_NNQUANT);
					break;

		case 16:	//_CrtDbgBreak();
					if( param == 555 || param == 0 )
						pNew = FreeImage_ConvertTo16Bits555(m_pCurrentImage);
					else if( param == 565 )
						pNew = FreeImage_ConvertTo16Bits565(m_pCurrentImage);
					break;
						
		case 24:	pNew = FreeImage_ConvertTo24Bits(m_pCurrentImage);
					break;

		case 32:	pNew = FreeImage_ConvertTo32Bits(m_pCurrentImage);
					break;
	}

	if( !pNew )
		return E_FAIL;

	FreeImage_Unload(m_pCurrentImage);
	m_pCurrentImage = pNew;
	ImageChanged();

	*pVal = 1;
	return S_OK;
}

STDMETHODIMP CImage::Clone(IImage **ppImage)
{
//_CrtDbgBreak();
	*ppImage = NULL;

	if( !ImageIsLoaded() )
		return E_FAIL;

	IUnknown *pUnk=NULL;

	// Get a COM object to wrap
	HRESULT hRes = CoCreateInstance(CLSID_Image, NULL, CLSCTX_INPROC_SERVER,
									IID_IImage, (void**) &pUnk);

	if (FAILED(hRes))		return E_FAIL;

	hRes = pUnk->QueryInterface(IID_IImage, (void **) ppImage);
	pUnk->Release();// because now two references are active
	if (FAILED(hRes))		return E_FAIL;

	FIBITMAP *pNew = FreeImage_Clone(m_pCurrentImage);
	if( !pNew )
	{
		(*ppImage)->Release();
		return E_FAIL;
	}

	(*ppImage)->FromHandle((LPUNKNOWN)pNew);

	return S_OK;
}

STDMETHODIMP CImage::FromHandle(LPUNKNOWN pHandle)
{
	Close();
	
	if( pHandle == NULL )
		return E_FAIL;

	m_pCurrentImage	= (FIBITMAP*) pHandle;
	ImageChanged();

	return S_OK;
}
