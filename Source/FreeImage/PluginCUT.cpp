// ==========================================================
// CUT Loader
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

#include <stdlib.h>

#include "FreeImage.h"
#include "Utilities.h"

// ----------------------------------------------------------
//   Constants + headers
// ----------------------------------------------------------

#ifdef WIN32
#pragma pack(push, 1)
#else
#pragma pack(1)
#endif

typedef struct CUT_HEAD {
	short width;
	short height;
	int dummy;
} CUT_HEAD;

#ifdef WIN32
#pragma pack(pop)
#else
#pragma pack(4)
#endif

// ==========================================================
// Plugin Interface
// ==========================================================

static int s_format_id;

// ==========================================================
// Plugin Implementation
// ==========================================================

static const char * DLL_CALLCONV
Format() {
	return "CUT";
}

static const char * DLL_CALLCONV
Description() {
	return "Dr. Halo";
}

static const char * DLL_CALLCONV
Extension() {
	return "cut";
}

static const char * DLL_CALLCONV
RegExpr() {
	return NULL;
}

static BOOL DLL_CALLCONV
Validate(FreeImageIO *io, fi_handle handle) {
	return FALSE;
}

static BOOL DLL_CALLCONV
SupportsExportDepth(int depth) {
	return FALSE;
}

// ----------------------------------------------------------

static FIBITMAP * DLL_CALLCONV
Load(FreeImage *freeimage, FreeImageIO *io, fi_handle handle, int page, int flags, void *data) {
	if (handle != NULL) {
		CUT_HEAD header;
		FIBITMAP *dib;

		// read the cut header

		io->read_proc(&header, 1, sizeof(CUT_HEAD), handle);

		if ((header.width == 0) || (header.width > 65536) || (header.height == 0) || (header.height > 65536))
			return NULL;

		// allocate a new bitmap

		dib = freeimage->allocate_proc(header.width, header.height, 8);

		if (dib == NULL)
			return NULL;

		// stuff it with a palette

		RGBQUAD *palette = freeimage->get_palette_proc(dib);

		for (int j = 0; j < 256; ++j)
			palette[j].rgbBlue = palette[j].rgbGreen = palette[j].rgbRed = j;

		// unpack the RLE bitmap bits

		BYTE *bits = freeimage->get_scanline_proc(dib, header.height - 1);

		int i = 0, k = 0, count = 0, run = 0;
		int line = freeimage->get_line_proc(dib);
		int pitch = freeimage->get_pitch_proc(dib);
		int size = header.width * header.height;

		while (i < size) {
			io->read_proc(&count, 1, sizeof(BYTE), handle);

			if (count == 0) {
				k = 0;
				bits -= pitch;

				// paint shop pro adds two useless bytes here...

				io->read_proc(&count, 2, sizeof(BYTE), handle);

				continue;
			}

			if (count & 0x80) {
				count &= ~(0x80);

				io->read_proc(&run, 1, sizeof(BYTE), handle);

				memset(bits + k, run, count);
			} else {
				io->read_proc(&bits[k], count, sizeof(BYTE), handle);
			}

			k += count;
			i += count;
		}

		return dib;		
	}

	return NULL;
}

// ==========================================================
//   Init
// ==========================================================

void DLL_CALLCONV
InitCUT(Plugin *plugin, int format_id) {
	s_format_id = format_id;

	plugin->format_proc = Format;
	plugin->description_proc = Description;
	plugin->extension_proc = Extension;
	plugin->regexpr_proc = RegExpr;
	plugin->load_proc = Load;
	plugin->validate_proc = Validate;
	plugin->supports_export_bpp_proc = SupportsExportDepth;
}
