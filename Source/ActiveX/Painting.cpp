#include "stdafx.h"
#include "FreeImage_ActiveX.h"
#include "Painting.h"

// IPersistStream

//
//	Function:
//		Load (IStream *pStream)
//
//	Parameters:
//		pStream 		IStream pointer to the stream from which the object should be loaded. 
//
//	Return Values:
//		S_OK			The object was successfully loaded. 
//		E_OUTOFMEMORY	The object was not loaded due to a lack of memory. 
//		E_FAIL			The object was not loaded due to some reason other than a lack of memory. 
//
//	Comments:
//		Initializes an object from the stream where it was previously saved. On exit, the seek
//		pointer should be left in the same position as it was on exit from Save, immediately past
//		the end of the data.
//
//	NOTE: If the object has already been initialized with InitNew, then this method must return
//		  E_UNEXPECTED
//
STDMETHODIMP CPainting::Load (IStream *pStream)
{
	DWORD nActual;

	// Check if the passed interface pointer is valid or not.

	if (NULL == pStream)
		return (E_POINTER);

	// zoom
	pStream->Read (&m_Zoom, sizeof (double), &nActual);

	// filename
	// NOTE: Make sure that you set the m_str member variable to NULL. This has to be dome to avoid
	//		 assertion inside the ATL code.
	//
	//		 Code from ATLBASE.H file...................
	//		 **********************************************************
	//		 HRESULT ReadFromStream(IStream* pStream)
	//		 {
	//			ATLASSERT(pStream != NULL);
	//			ATLASSERT(m_str == NULL); // should be empty
	//			..
	//		 *********************************************************	
	//
	m_currentFilename.m_str = NULL;
	m_currentFilename.ReadFromStream (pStream);
	ImageChanged();

	return (S_OK);
}

//
//	Function:
//		Save(IStream *pStream, BOOL fClearDirty)
//
//	Parameters:
//		pStream 		IStream pointer to the stream into which the object should be saved.  
//		fClearDirty		Indicates whether to clear the dirty flag after the save is complete. If
//						TRUE, the flag should be cleared. If FALSE, the flag should be left
//						unchanged.
//
//	Return Values:
//		S_OK			The object was successfully saved to the stream.
//		STG_E_CANTSAVE 	The object could not save itself to the stream. This error could indicate,
//						for example, that the object contains another object that is not
//						serializable to a stream or that an ISequentialStream::Write call returned
//						STG_E_CANTSAVE. 
//		STG_E_MEDIUMFULL The object could not be saved because there is no space left on the
//						 storage device. 
//		E_FAIL			Error
//
//	Comments:
//		Saves an object into the specified stream and indicates whether the object should reset
//		its dirty flag.
//

STDMETHODIMP CPainting::Save(IStream *pStream, BOOL fClearDirty)
{
	DWORD nActual;

	if (pStream == NULL) {
		ATLTRACE (_T ("Null stream pointer!\n"));
		return (E_POINTER);
	}
	
	// zoom
	pStream->Write (&m_Zoom, sizeof (double), &nActual);

	// filename
	m_currentFilename.WriteToStream (pStream);

	return (S_OK);
}

//
//	Function:
//		GetSizeMax (ULARGE_INTEGER *pcbSize)
//
//	Parameters:
//		pcbSize   		Pointer to size of stream needed to save object .
//
//	Return Values:
//		S_OK			The size was successfully returned.  
//		E_FAIL			The size was not successfully returned. 
//
//	Comments:
//		This method returns the size needed to save an object. You can call this method to
//		determine the size and set the necessary buffers before calling the
//		IPersistStream::Save method
//
//	NOTE: The followinf MACRO from OBJBASE.H is used to set the size in ULARGE_INTEGER
//			#define ULISet32(li, v) ((li).HighPart = 0, (li).LowPart = (v))
//

STDMETHODIMP CPainting::GetSizeMax (ULARGE_INTEGER *pcbSize)
{
	HRESULT hRes = E_POINTER;
	ULONG ulsize = 0;

	// Make sure that pointer is not null.

	if (pcbSize == NULL) {
		return (hRes);
	}

	// Add the size of each property.
	ulsize += sizeof (m_Zoom);
	ulsize += sizeof (OLECHAR) * m_currentFilename.Length ();
	// Set the size of control.

	ULISet32 (*pcbSize, ulsize);

	return (S_OK);
}

// IPersistStorage

//
//	Function:
//		Load (IStorage* pStorage)
//
//	Parameters:
//		pStorage 		Pointer to existing storage for the object
//
//	Return Values:
//		S_OK			The object was successfully loaded.
//		CO_E_ALREADYINITIALIZED  The object has already been initialized by a previous call to the
//								 IPersistStorage::Load method or the IPersistStorage::InitNew
//								 method. 
//		E_OUTOFMEMORY	The object was not loaded due to a lack of memory. 
//		E_FAIL			The object was not loaded due to some reason other than a lack of memory. 
//
//	Comments:
//		This method initializes an object from an existing storage. The object is placed in the
//		loaded state if this method is called by the container application. If called by the
//		default handler, this method places the object in the running state.
//
//	NOTE:
//		The implementation should perform the following steps to load an object: 
//		1) Open the object's streams in the storage object, and read the necessary data into the
//		object's internal data structures.
//		2) Clear the object's dirty flag. 
//		3) Call the IPersistStorage::AddRef method and cache the passed in storage pointer. 
//		4) Keep open and cache the pointers to any streams or storages that the object will need to
//		   save itself to this storage. 
//		5) Perform any other default initialization required for the object. 
//		
//		Steps 3 and 4 are particularly important for ensuring that the object can save itself in
//		low memory situations. Holding onto pointers to the storage and stream interfaces
//		guarantees that a save operation to this storage will not fail due to insufficient memory.

STDMETHODIMP CPainting::Load (IStorage* pStorage)
{
	DWORD nActual;
	HRESULT hRes = E_UNEXPECTED;
	CComQIPtr <IStream, &IID_IStream> pPropStream;
	CComQIPtr <IStream, &IID_IStream> pDataStream;
	CComQIPtr <IStream, &IID_IStream> pSummaryStream;
	CComQIPtr <IStorage, &IID_IStorage> pSubStorage;

	if (NULL == pStorage) {
		return (E_FAIL);
	}

	// Open stream.
	
	hRes = pStorage->OpenStream (L"Freeimage ActiveX Control", NULL, STGM_READWRITE |
		STGM_SHARE_EXCLUSIVE, NULL, &pPropStream);
	if (FAILED (hRes)) {
		return (E_FAIL);
	}
	
	// open the  sub storage.

	hRes = pStorage->OpenStorage (L"Summary Info",NULL, STGM_READWRITE | STGM_TRANSACTED |
		STGM_SHARE_EXCLUSIVE, NULL, 0, &pSubStorage);
	if (FAILED (hRes)) {
		return (E_FAIL);
	}

	// Create a stream to contain summary information about the control.

	hRes = pStorage->OpenStream (L"Summary Stream", NULL, STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
		NULL, &pSummaryStream);
	if (FAILED (hRes)) {
		ATLTRACE (_T ("Failed to create summary stream in sub storage."));
		return (E_FAIL);
	}

	// Write some onformation inthis stream.

	pSummaryStream->Read (&m_bstrAuthor, sizeof (m_bstrAuthor), &nActual);
	pSummaryStream->Read (&m_bstrEmail, sizeof (m_bstrEmail), &nActual);
	pSummaryStream->Read (&m_bstrDesc, sizeof (m_bstrDesc), &nActual);

	// Create a new stream in substorage to write control parameters..

	hRes = pSubStorage->OpenStream (L"Control Parameters", NULL, STGM_READWRITE |
		STGM_SHARE_EXCLUSIVE, NULL, &pDataStream);
	if (FAILED (hRes)) {
		return (E_FAIL);
	}

	// Delegate the stream save to the Save function call for IStream..

	if (FAILED (Load (pDataStream))) {
		ATLTRACE (_T ("Failed to save the stream!\n"));
		return (E_FAIL);
	}

	// Clear the dirty flag.

	SetDirty (FALSE);

	// Keep open storage pointer around for use in later incremental or low-memory calls of save.

	m_pIStorage = pStorage;
	m_pIStorage->AddRef ();

	return (S_OK);
}

//
//	Function:
//		Save(IStorage* pStorage, BOOL fSameAsLoad)
//
//	Parameters:
//		pStorage 		IStream pointer to storage object.  
//		fSameAsLoad		Indicates whether the specified storage object is the current one
//
//	Return Values:
//		S_OK			The object was successfully saved to the stream.
//		STG_E_MEDIUMFULL The object could not be saved because there is no space left on the
//						 storage device. 
//		E_FAIL			The object could not be saved due to errors other than a lack of disk space
//
//	Comments:
//		This method saves an object, and any nested objects it contains, into the specified
//		storage. It also places the object into NoScribble mode.
//

STDMETHODIMP CPainting::Save(IStorage* pStorage, BOOL fSameAsLoad)
{
	DWORD nActual;
	HRESULT hRes = E_UNEXPECTED;
	CComQIPtr <IStream, &IID_IStream> pPropStream;
	CComQIPtr <IStream, &IID_IStream> pDataStream;
	CComQIPtr <IStream, &IID_IStream> pSummaryStream;
	CComQIPtr <IStorage, &IID_IStorage> pSubStorage;

	// Create stream.

	if (NULL == pStorage) {
		return (E_FAIL);
	}
	
	// Create a stream to save the information about control. Make sure that the names with which
	// the streams or substorages are created are the same which are used to load the object.

	hRes = pStorage->CreateStream (L"Freeimage ActiveX Control", STGM_CREATE | STGM_WRITE |
		STGM_SHARE_EXCLUSIVE, 0, 0, &pPropStream);
	if (FAILED (hRes)) {
		return (E_FAIL);
	}
	
	// Create a sub storage.

	hRes = pStorage->CreateStorage (L"Summary Info", STGM_CREATE | STGM_WRITE |
		STGM_SHARE_EXCLUSIVE, 0, 0, &pSubStorage);
	if (FAILED (hRes)) {
		return (E_FAIL);
	}

	// Create a stream to contain summary information about the control.

	hRes = pStorage->CreateStream (L"Summary Stream", STGM_CREATE | STGM_WRITE |
		STGM_SHARE_EXCLUSIVE, 0, 0, &pSummaryStream);
	if (FAILED (hRes)) {
		ATLTRACE (_T ("Failed to create summary stream in sub storage."));
		return (E_FAIL);
	}

	// Write some onformation inthis stream.

	pSummaryStream->Write (&m_bstrAuthor, sizeof (m_bstrAuthor), &nActual);
	pSummaryStream->Write (&m_bstrEmail, sizeof (m_bstrEmail), &nActual);
	pSummaryStream->Write (&m_bstrDesc, sizeof (m_bstrDesc), &nActual);

	// Create a new stream in substorage to write control parameters..

	hRes = pSubStorage->CreateStream (L"Control Parameters", STGM_WRITE | STGM_WRITE |
	STGM_SHARE_EXCLUSIVE, 0, 0, &pDataStream);
	if (FAILED (hRes)) {
		return (E_FAIL);
	}

	// Delegate the stream save to the Save function call for IStream..

	if (FAILED (Save (pDataStream, true))) {
		ATLTRACE (_T ("Failed to save the stream!\n"));
		return (E_FAIL);
	}

	return (S_OK);
}

// IPeristPropertBag implementation.
//
//	Function:
//		Load (IPropertyBag *pPropBag, IErrorLog* pErrorLog)
//
//	Parameters:
//		pPropBag 		Pointer to caller's property bag
//		pErrorLog		Pointer to error log
//
//	Return Values:
//		S_OK			The object successfully initialized itself.
//		E_OUTOFMEMORY	The object was not loaded due to a lack of memory.
//		E_POINTER		The address in pPropBag is not valid (such as NULL) and therefore the
//						object cannot initialize itself. 
//		E_FAIL			The object was unable to retrieve a critical property that is necessary
//						for the object's successful operation. The object was therefore unable to
//						initialize itself completely. 
//
//	Comments:
//		Called by the container to load the control's properties. This method instructs the object
//		to initialize itself using the properties available in the property bag, notifying the
//		provided error log object when errors occur. All property storage must take place within
//		this method call as the object cannot hold the IPropertyBag pointer.
//		E_NOTIMPL is not a valid return code as any object implementing this interface must support
//		the entire functionality of the interface.
//
//	NOTE: The ATL_PROPMAP_ENTRY structure used by ATL is a s follows.
//			struct ATL_PROPMAP_ENTRY
//			{
//				LPCOLESTR szDesc;
//				DISPID dispid;
//				const CLSID* pclsidPropPage;
//				const IID* piidDispatch;
//				DWORD dwOffsetData;
//				DWORD dwSizeData;
//				VARTYPE vt;
//			}
//

STDMETHODIMP CPainting::Load (IPropertyBag *pPropBag, IErrorLog* pErrorLog)
{
	HRESULT hRes = E_UNEXPECTED;

	if (pPropBag == NULL) {
		return (E_POINTER);
	}

	// ATL implementation uses property map to check which properties need to be loaded from or
	// saved to property bag. The data is written to propertybag as a VARIANT. The default
	// implementation supports only the integer type data. If the vt memeber of ATL_PROPMAP_ENTRY
	// is of any other type, ATL ignores that map entry. Therfore we need to implement persistence
	// of non-variant data by generic functions which convert the data to and fro from VARIANT
	// type, so that we can persist that data. To make this happen, set the non-variant data type
	// to onw whichis not supported bt ATL.
	
	//	NOTE: For more information see "Professional ATL COM Programming, Wrox Press Ltd" by
	//		  Dr. Richard Grimes.
	//

	ATL_PROPMAP_ENTRY *pMap = GetPropertyMap ();
	ATLASSERT (pMap != NULL);

	for (int i = 0; pMap[i].pclsidPropPage != NULL; ++i) {
		if ((pMap[i].dwSizeData == 0) || (pMap[i].szDesc == NULL)) {
			continue;
		}

		// check variant data type.

		switch (pMap[i].vt) {
			case VTEX_FIXED_SIZE_BSTR_BLOB:
				CComVariant var;
				hRes = pPropBag->Read (pMap[i].szDesc, &var, pErrorLog);
				if (FAILED (hRes)) {
					return (hRes);
				}

				LPBYTE pData = reinterpret_cast<BYTE*>(pMap[i].dwOffsetData +
					reinterpret_cast<DWORD>(this));
				int size = pMap[i].dwSizeData;
				LPCWSTR ptr = var.bstrVal;
				Extract (&ptr, pData, size);
				break;
		}
	}

	return (IPersistPropertyBagImpl<CPainting>::Load (pPropBag, pErrorLog));
}

//
//	Function:
//		Save (IPropertyBag *pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties)
//
//	Parameters:
//		pPropBag 			Pointer to caller's property bag
//		fClearDirty			Specifies whether to clear the dirty flag
//		fSaveAllProperties	Specifies whether to save all properties or just those that have
//							changed
//
//	Return Values:
//		S_OK			The object successfully initialized itself.
//		E_POINTER		The address in pPropBag is not valid (such as NULL) and therefore the
//						object cannot initialize itself. 
//		E_FAIL			There was a problem saving one of the properties. The object can choose to
//						fail only if a necessary property could not be saved, meaning that the
//						object can assume default property values if a given property is not seen
//						through IPersistPropertyBag::Load at some later time.  
//		STG_E_MEDIUMFULL The object was not saved because of a lack of space on the disk. 
//
//	Comments:
//		This method instructs the object to save its properties to the specified property bag,
//		optionally clearing the object's dirty flag. The caller can request that the object save
//		all properties or that the object save only those that are known to have changed.
//		E_NOTIMPL is not a valid return code as any object implementing this interface must support
//		the entire functionality of the interface.
//

STDMETHODIMP CPainting::Save (IPropertyBag *pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties)
{
	HRESULT hRes = E_UNEXPECTED;

	if (pPropBag == NULL) {
		return (E_FAIL);
	}

	// See the discussion in Load function body.

	ATL_PROPMAP_ENTRY *pMap = GetPropertyMap ();
	ATLASSERT (pMap != NULL);

	for (int i = 0; pMap[i].pclsidPropPage != NULL; ++i) {
		if ((pMap[i].dwSizeData == 0) || (pMap[i].szDesc == NULL)) {
			continue;
		}

		// check variant data type.

		switch (pMap[i].vt) {
			case VTEX_FIXED_SIZE_BSTR_BLOB:
				LPBYTE pData = reinterpret_cast<BYTE*>(pMap[i].dwOffsetData +
					reinterpret_cast<DWORD>(this));
				CComBSTR bstr;
				Insert (&bstr, pData, pMap[i].dwSizeData);
				CComVariant var(bstr);
				hRes = pPropBag->Write (pMap[i].szDesc, &var);
				if (FAILED (hRes)) {
					return (hRes);
				}
				break;
		}
	}

	return (IPersistPropertyBagImpl<CPainting>::Save (pPropBag, fClearDirty, fSaveAllProperties));
}

//
//	Function:
//		Insert (BSTR *pStream, void *pData, DWORD size)
//
//	Parameters:
//		pStream		Pointer to BSTR
//		pData		Pointer to buffer
//		size		Size of buffer in byte
//
//	Returns Values:
//
//	Comments:
//		Function converts each byte in the buffer to a two-character readable hexadecimal string,
//		and adds onto the end of the BSTR (CComBSTR class reallocates as neccessary)
//
//	NOTE: For more information see "Professional ATL COM Programming, Wrox Press Ltd" by
//		  Dr. Richard Grimes.
//

void CPainting::Insert (BSTR *pStream, void *pData, DWORD size)
{
	CComBSTR bstr;
	bstr.Attach (*pStream);

	LPBYTE ptr = static_cast<LPBYTE>(pData);
	TCHAR strTemp[3] = {0};

	for (DWORD j = 0; j < size; ++j) {
		BYTE ch = *ptr;
		strTemp[1] = (ch & 0xf) > 9 ? '7'+ (ch & 0xf) : '0' + (ch & 0xf);
		ch = ch >> 4;

		strTemp[0] = (ch & 0xf) > 9 ? '7'+ (ch & 0xf) : '0' + (ch & 0xf);
		ptr++;

		bstr += strTemp;
	}

	*pStream = bstr.Detach ();
}

//
//	Function:
//		Extract (LPCWSTR *pStream, void *pData, DWORD size)
//
//	Parameters:
//		pStream		Pointer to LPWSTR
//		pData		Pointer to buffer
//		size		Size of buffer in bytes
//
//	Return Values:
//
//	Comments;
//		It takes LPWSTR pointer and extracts size two-byte hex strings, converting each to a byte
//		and then placing them in the buffer. The position in the input stream in updated to
//		reflect how many characters have been used.
//
//	NOTE: For more information see "Professional ATL COM Programming, Wrox Press Ltd" by
//		  Dr. Richard Grimes.
//

void CPainting::Extract (LPCWSTR *pStream, void *pData, DWORD size)
{
	LPCWSTR str = *pStream;
	LPBYTE ptr = static_cast<LPBYTE>(pData);

	for (DWORD j = 0; j < size; ++j) {
		BYTE ch = (str[0] >= L'A') ? str[0] - L'7' : str[0] - L'0';
		ch *= 16;
		ch += (str[1] >= L'A') ? str[1] - L'7' : str[1] - L'0';
		*ptr = ch;
		ptr++;
		str += 2;
	}

	*pStream = str;
}


/////////////////////////////////////////////////////////////////////////////
// CPainting

CPainting::CPainting()
{
	m_pCurrentImage			= NULL;
	m_Zoom					= 1.0;

	m_bstrAuthor = _T ("Markus Loibl");
	m_bstrEmail = _T ("markus.loibl@epost.de");
	m_bstrDesc = _T ("Freeimage ActiveX-interface");
}

CPainting::~CPainting()
{
	Close(TRUE);
}

void CPainting::Close(BOOL bEmptyFilename)
{
	if( m_pCurrentImage )
	{
		USES_CONVERSION;

		::FreeImage_Unload(m_pCurrentImage);
		m_pCurrentImage = NULL;
		
		if( bEmptyFilename )
		{
			m_currentFilename.Empty();
			ImageChanged();
		}
	}
}

void CPainting::ImageChanged(void)
{
	Close(FALSE);

	try
	{
		USES_CONVERSION;
		LPTSTR	szFileName;

		if( m_currentFilename.Length() )
				szFileName = OLE2T(m_currentFilename.m_str);
		else	szFileName = NULL;

		if( szFileName && *szFileName )	
			m_pCurrentImage = ::FreeImage_Load(FreeImage_GetFIFFromFilename(szFileName), szFileName);
	}
	catch(...)
	{
		Close(FALSE);
	}
	
	FireViewChange();
}

HRESULT CPainting::OnDrawAdvanced(ATL_DRAWINFO& di)
{
//	return CComControlBase::OnDrawAdvanced(di);

   BOOL bDeleteDC = FALSE;
   if (di.hicTargetDev == NULL)
   {
      di.hicTargetDev = AtlCreateTargetDC(di.hdcDraw, di.ptd);
      bDeleteDC = (di.hicTargetDev != di.hdcDraw);
   }
   RECTL rectBoundsDP = *di.prcBounds;

// Check for a metafile...
   BOOL bMetafile = GetDeviceCaps(di.hdcDraw, TECHNOLOGY) == DT_METAFILE;

// Changed "if" clause to check both the metafile flag and
// bounding window RECT passed to IViewObject::Draw...
   if ((!bMetafile) && (di.prcWBounds == NULL))
   {
      ::LPtoDP(di.hicTargetDev, (LPPOINT)&rectBoundsDP, 2);
      SaveDC(di.hdcDraw);
      SetMapMode(di.hdcDraw, MM_TEXT);
      SetWindowOrgEx(di.hdcDraw, 0, 0, NULL);
      SetViewportOrgEx(di.hdcDraw, 0, 0, NULL);
      di.bOptimize = TRUE; //since you save the DC you can do this
   }

   di.prcBounds = &rectBoundsDP;
   GetZoomInfo(di);

   HRESULT hRes = OnDraw(di);
   if (bDeleteDC)
      ::DeleteDC(di.hicTargetDev);
   if ((!bMetafile) && (di.prcWBounds == NULL))
      RestoreDC(di.hdcDraw, -1);
   return hRes;
}

HRESULT CPainting::OnDraw(ATL_DRAWINFO& di)
{
	RECT& rc = *(RECT*)di.prcBounds;
	Rectangle(di.hdcDraw, rc.left, rc.top, rc.right, rc.bottom);

	BOOL bMetafile = GetDeviceCaps(di.hdcDraw, TECHNOLOGY) == DT_METAFILE;

	if( !m_pCurrentImage )
	{
		USES_CONVERSION;
		LPTSTR	szFileName;

		if( m_currentFilename.Length() )
				szFileName = OLE2T(m_currentFilename.m_str);
		else	szFileName = NULL;
		
		if( !szFileName || !*szFileName )
			szFileName = "no file";

		TextOut(di.hdcDraw, 
			(rc.left + rc.right) / 2, 
			(rc.top + rc.bottom) / 2, 
			szFileName, 
			lstrlen(szFileName));
			return S_OK;
	}
	else
		return OnDrawImage(bMetafile, di);
}

HRESULT CPainting::OnDrawImage(BOOL bMetafile, ATL_DRAWINFO& di)
{
//	if( bMetafile )
//		::MessageBox(NULL, "metafile", "title", MB_OK|MB_TOPMOST|MB_SETFOREGROUND);
 
	SetMapMode(di.hdcDraw, MM_HIMETRIC);

	::SetStretchBltMode(di.hdcDraw, COLORONCOLOR);

	double		dpiX   = FreeImage_GetDotsPerMeterX(m_pCurrentImage),
				dpiY   = FreeImage_GetDotsPerMeterY(m_pCurrentImage),
				width  = FreeImage_GetWidth(m_pCurrentImage),
				height = FreeImage_GetHeight(m_pCurrentImage),
				sizeX,	// 1/100 mm == HIMETRIC
				sizeY;
	BYTE		*pData	= FreeImage_GetBits(m_pCurrentImage);

	// Resolution od screen or printer
	// screen: 72 or 96
	if( dpiX==0.0 )	dpiX = 100.0 / 2.54 * (double)GetDeviceCaps(di.hdcDraw, LOGPIXELSX);
	if( dpiY==0.0 )	dpiY = 100.0 / 2.54 * (double)GetDeviceCaps(di.hdcDraw, LOGPIXELSY);

	sizeX = m_Zoom * 100.0 * 1000.0 * width  / dpiX;
	sizeY = m_Zoom * 100.0 * 1000.0 * height / dpiY;

	int			xDst,
				yDst,
				dxDst,
				dyDst,
				xSrc,
				ySrc,
				dxSrc,
				dySrc;
	double		factorBegin,
				factorSize,
				fTemp;
	RECT		rectPicture;

	// lets see if we can see this within the clipping area
	// if not, nothing to do!
	rectPicture.left	=  0;
	rectPicture.right	=  (long) sizeX;
	rectPicture.top		=  0;
	rectPicture.bottom	=  (long) -sizeY;
	//TraceRect("rectPicture  ", rectPicture);

	//
	// dst: Metric coordinates in the view
	//

	xDst = rectPicture.left;
	yDst = rectPicture.top;

	dxDst = rectPicture.right  - rectPicture.left;
	dyDst = rectPicture.bottom - rectPicture.top;
	
	//
	// src: pixel  coordinates from the raster
	//

	// x-dim
	factorBegin = ((double)  xDst) / ((double) sizeX);
	factorSize  = ((double) dxDst) / ((double) sizeX);
	// now calc the corresponding pixels
	fTemp = factorBegin * width;
	xSrc  = (int) fTemp;// + 0.5;
	xSrc %= (LONG) width;

	fTemp = factorSize * width;
	dxSrc = (int) fTemp;// + 0.5;
	
	// y-dim
	rectPicture.top    += (long) sizeY;
	rectPicture.bottom += (long) sizeY;
	fTemp = rectPicture.bottom;
	factorBegin = 1.0 - ((double) -fTemp) / ((double)sizeY);
	factorSize  =       ((double) -dyDst) / ((double)sizeY);
	// now calc the corresponding pixels
	fTemp = factorBegin * height;
	ySrc  = (int) fTemp;// + 0.5;
	ySrc %= (LONG) height;

	fTemp = factorSize * height;
	dySrc = (int) fTemp;// + 0.5;

	//TRACE("m_Zoom = %f\n", m_Zoom);
	if( fabs(m_Zoom-1.0) <= 0.00002 )
	{
		// no stretching
		::SetDIBitsToDevice(
						di.hdcDraw,		// handle to DC
						xDst,	yDst,	// x-y-coord of destination upper-left corner
						dxSrc,	dySrc,	// width-height of source rectangle
						xSrc,	ySrc,	// x-y-coord of source upper-left corner
						0,//uStartScan,// first scan line in array
						FreeImage_GetHeight(m_pCurrentImage),	// number of scan lines
						FreeImage_GetBits(m_pCurrentImage),	// array of DIB bits
						FreeImage_GetInfo(m_pCurrentImage),	// bitmap information
						DIB_RGB_COLORS);				// RGB or palette indexes
	}
	else
	{
		// do stretching
		::StretchDIBits(di.hdcDraw,
					xDst,	yDst,	// x-y-coord of destination upper-left corner
					dxDst,	dyDst,	// width-height of destination rectangle
					xSrc,	ySrc,	// x-y-coord of source upper-left corner
					dxSrc,	dySrc,	// width-height of source rectangle
					pData,			// bitmap bits
					FreeImage_GetInfo(m_pCurrentImage),// bitmap data
					DIB_RGB_COLORS,	// usage options
					SRCCOPY);		// raster operation code
	}

	return S_OK;
}

STDMETHODIMP CPainting::get_SRC(BSTR *pVal)
{
	*pVal = m_currentFilename.Copy();
	return S_OK;
}

STDMETHODIMP CPainting::put_SRC(BSTR newVal)
{
	m_currentFilename = newVal;
	ImageChanged();

	return S_OK;
}

STDMETHODIMP CPainting::get_Zoom(double *pVal)
{
	*pVal = m_Zoom;
	return S_OK;
}

STDMETHODIMP CPainting::put_Zoom(double newVal)
{
	if( m_Zoom != newVal ) 
	{
		m_Zoom = newVal;
		FireViewChange();
	}
	return S_OK;
}
