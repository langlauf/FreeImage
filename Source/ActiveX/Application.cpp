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
#include "Application.h"


extern short g_ShowMessages;

/////////////////////////////////////////////////////////////////////////////
//
// CApplication
//
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CApplication::Test(BSTR msg, BSTR title)
{
	USES_CONVERSION;
	::MessageBox(NULL, W2A(msg), W2A(title), MB_OK|MB_TOPMOST|MB_SETFOREGROUND);

	return S_OK;
}

STDMETHODIMP CApplication::get_Version(BSTR *pVal)
{
	*pVal = A2BSTR(FreeImage_GetVersion());
	return S_OK;
}

STDMETHODIMP CApplication::get_Copyright(BSTR *pVal)
{
	*pVal = A2BSTR(FreeImage_GetCopyrightMessage());
	return S_OK;
}

STDMETHODIMP CApplication::IsValidImage(BSTR filename, short *pbIsValid)
{
	USES_CONVERSION;
	*pbIsValid = (FreeImage_GetFIFFromFilename(W2A(filename)) != FIF_UNKNOWN);

	return S_OK;
}

STDMETHODIMP CApplication::get_ShowMessages(short *pVal)
{
	*pVal = g_ShowMessages;
	return S_OK;
}

STDMETHODIMP CApplication::put_ShowMessages(short newVal)
{
	g_ShowMessages = newVal;
	return S_OK;
}

STDMETHODIMP CApplication::GetImageType(short flags, BSTR filename, short *pVal)
{
	USES_CONVERSION;
	FREE_IMAGE_FORMAT	fmt		= FIF_UNKNOWN;
	HRESULT				res		= S_OK;

	switch(flags)
	{
		case 0:		// only looking at extension
					fmt = FreeImage_GetFIFFromFilename(W2A(filename));
					break;
		
		case 1:		// opening file and looking in it whats it about
					fmt = FreeImage_GetFileType(W2A(filename), 100);		
					break;
		
		default:	res = E_FAIL;
					break;
	}

	*pVal = fmt;

	return res;
}

STDMETHODIMP CApplication::get_SupportsReading(short type, short *pVal)
{
	HRESULT		res			= S_OK;
	BOOL		bSupport	= FALSE;
	
	if( !FreeImage_IsPluginEnabled((FREE_IMAGE_FORMAT)type) )
	{
		res = E_FAIL;
	}
	else
	{
		bSupport = FreeImage_FIFSupportsReading((FREE_IMAGE_FORMAT)type);
	}

	*pVal = bSupport;
	return res;
}

STDMETHODIMP CApplication::get_SupportsWriting(short type, short *pVal)
{
	HRESULT		res			= S_OK;
	BOOL		bSupport	= FALSE;
	
	if( !FreeImage_IsPluginEnabled((FREE_IMAGE_FORMAT)type) )
	{
		res = E_FAIL;
	}
	else
	{
		bSupport = FreeImage_FIFSupportsWriting((FREE_IMAGE_FORMAT)type);
	}

	*pVal = bSupport;
	return res;
}

