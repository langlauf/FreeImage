// ==========================================================
// Simplified load functions
//
// Design and implementation by
// - Floris van den Berg (flvdberg@wxs.nl)
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
