#include "stdafx.h"
#include "FreeImage_ActiveX.h"
#include "Multipage.h"

/////////////////////////////////////////////////////////////////////////////
//
// CMultipage
//
/////////////////////////////////////////////////////////////////////////////

CMultipage::CMultipage()
{
}

CMultipage::~CMultipage()
{
}

STDMETHODIMP CMultipage::Test(BSTR msg, BSTR title)
{
	USES_CONVERSION;
	::MessageBox(NULL, W2A(msg), W2A(title), MB_OK|MB_TOPMOST|MB_SETFOREGROUND);

	return S_OK;
}

/*

Open
OpenEx
Close
Append(IImage*, BOOL bDoClone)
Insert(IImage*, BOOL bDoClone)
DeletePage
MovePage
GetPageCout
IImage* LockPage(id
UnlockPage(id)
*/

/*	
FreeImage_OpenMultiBitmap(FREE_IMAGE_FORMAT fif, const char *filename, BOOL create_new, BOOL read_only, BOOL keep_cache_in_memory FI_DEFAULT(FALSE));
FreeImage_CloseMultiBitmap(FIMULTIBITMAP *bitmap);
FreeImage_GetPageCount(FIMULTIBITMAP *bitmap);
FreeImage_AppendPage(FIMULTIBITMAP *bitmap, FIBITMAP *data);
FreeImage_InsertPage(FIMULTIBITMAP *bitmap, int page, FIBITMAP *data);
FreeImage_DeletePage(FIMULTIBITMAP *bitmap, int page);
FreeImage_MovePage(FIMULTIBITMAP *bitmap, int target, int source);
*/
