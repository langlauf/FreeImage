// ==========================================================
// Deprecated functions
//
// Design and implementation by
// - Floris van den Berg (flvdberg@wxs.nl)
// - Hervé Drolon (drolon@infonie.fr)
//
// This file is part of FreeImage 2
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

#include "FreeImage.h"

#include "../DeprecationManager/DeprecationMgr.h"

// ----------------------------------------------------------

#define LOAD(what, registered_type) \
FIBITMAP * DLL_CALLCONV \
FreeImage_Load##what##(const char *filename, int flags) { \
	DEPRECATE("FreeImage_Load" #what "()", "FreeImage_Load()") \
	return FreeImage_Load(FreeImage_GetFIFFromFormat(#registered_type), filename, flags); \
} \
\
FIBITMAP * DLL_CALLCONV \
FreeImage_Load##what##FromHandle(FreeImageIO *io, fi_handle handle, int flags) { \
	DEPRECATE("FreeImage_Load" #what "FromHandle()", "FreeImage_LoadFromHandle()") \
	return FreeImage_LoadFromHandle(FreeImage_GetFIFFromFormat(#registered_type), io, handle, flags); \
}

#define SAVE(what, registered_type) \
BOOL DLL_CALLCONV \
FreeImage_Save##what##(FIBITMAP *dib, const char *filename, int flags) { \
	DEPRECATE("FreeImage_Save" #what "()", "FreeImage_Save()") \
	return FreeImage_Save(FreeImage_GetFIFFromFormat(#registered_type), dib, filename, flags); \
} \
\
BOOL DLL_CALLCONV \
FreeImage_Save##what##ToHandle(FIBITMAP *dib, FreeImageIO *io, fi_handle handle, int flags) { \
	DEPRECATE("FreeImage_Save" #what "ToHandle()", "FreeImage_SaveToHandle()") \
	return FreeImage_SaveToHandle(FreeImage_GetFIFFromFormat(#registered_type), dib, io, handle, flags); \
}

// ----------------------------------------------------------
// ALIASES FOR BACKWARDS COMPATIBILITY ----------------------
// ----------------------------------------------------------

LOAD(BMP, BMP)
LOAD(CUT, CUT)
LOAD(ICO, ICO)
LOAD(IFF, IFF)
LOAD(JPEG, JPEG)
LOAD(KOALA, KOALA)
LOAD(LBM, IFF)
LOAD(MNG, MNG)
LOAD(PCD, PCD)
LOAD(PCX, PCX)
LOAD(PNG, PNG)
LOAD(PNM, PPM)
LOAD(PSD, PSD)
LOAD(RAS, RAS)
LOAD(TARGA, TARGA)
LOAD(TIFF, TIFF)
LOAD(WBMP, WBMP)

SAVE(BMP, BMP)
SAVE(JPEG, JPEG)
SAVE(PNG, PNG)
SAVE(PNM, PPM)
SAVE(TIFF, TIFF)
SAVE(WBMP, WBMP)

// ----------------------------------------------------------

const char * DLL_CALLCONV
FreeImage_GetFileTypeFromFormat(FREE_IMAGE_FORMAT fif) {
	DEPRECATE("FreeImage_GetFileTypeFromFormat()", "FreeImage_GetFormatFromFIF()")
	return FreeImage_GetFormatFromFIF(fif);
}

FREE_IMAGE_FORMAT DLL_CALLCONV
FreeImage_GetFileTypeFromExt(const char *filename) {
	DEPRECATE("FreeImage_GetFileTypeFromExt()", "FreeImage_GetFIFFromFilename()")
	return FreeImage_GetFIFFromFilename(filename);
}

BYTE * DLL_CALLCONV
FreeImage_GetBitsRowCol(FIBITMAP *dib, int col, int row) {
	DEPRECATE("FreeImage_GetBitsRowCol()", "FreeImage_GetScanLine()") 
	return (dib) ? FreeImage_GetBits(dib) + ((FreeImage_GetHeight(dib) - row - 1) * FreeImage_GetPitch(dib)) + (col * FreeImage_GetBPP(dib) / 8) : NULL;
}

void DLL_CALLCONV 
FreeImage_Free(FIBITMAP *dib) { 
	DEPRECATE("FreeImage_Free()", "FreeImage_Unload()")
	FreeImage_Unload(dib);
}

