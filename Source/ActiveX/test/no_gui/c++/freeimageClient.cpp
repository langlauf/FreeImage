#include "stdafx.h"
#include "freeimageClient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CWinApp theApp;

using namespace std;


BOOL TestFreeimageComServer(void)
{
	IUnknown	*pUnk	= NULL;
	HRESULT		hRes	= 0;
	IImage		*pImage	= NULL;
	
	hRes = CoCreateInstance(CLSID_Image, NULL, CLSCTX_INPROC_SERVER,
		IID_IUnknown, (void**)&pUnk);
	if (FAILED(hRes))
		return FALSE;

	hRes = pUnk->QueryInterface(IID_IImage, (LPVOID*)&pImage);
	pUnk->Release();
	if (FAILED(hRes))
		return FALSE;

	pImage->Test(L"hello from within the freeimage-ActiveX-server called from C++", L"My title");

	pImage->Release();

	return TRUE;
}

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	BOOL erg;
	
	if (FAILED(CoInitialize(NULL)))
		return -1;

	erg = TestFreeimageComServer();

	CoUninitialize();
	
	return erg;
}


