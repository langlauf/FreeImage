// ==========================================================
// DDS Loader
//
// Design and implementation by
// - Volker Gärtner (volkerg@gmx.at)
//
// This file is part of FreeImage 3
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
#include "Utilities.h"

// ----------------------------------------------------------
//   Definitions for the DDS format
// ----------------------------------------------------------

#ifdef WIN32
#pragma pack(push, 1)
#else
#pragma pack(1)
#endif

typedef struct tagDDPIXELFORMAT {
	DWORD dwSize;	// size of this structure (must be 32)
	DWORD dwFlags;	// see DDPF_*
	DWORD dwFourCC;
	DWORD dwRGBBitCount;	// Total number of bits for RGB formats
	DWORD dwRBitMask;
	DWORD dwGBitMask;
	DWORD dwBBitMask;
	DWORD dwRGBAlphaBitMask;
} DDPIXELFORMAT;

// DIRECTDRAW PIXELFORMAT FLAGS
enum {
	DDPF_ALPHAPIXELS = 0x00000001l,	// surface has alpha channel
	DDPF_ALPHA		 = 0x00000002l,	// alpha only
	DDPF_FOURCC		 = 0x00000004l,	// FOURCC available
	DDPF_RGB		 = 0x00000040l	// RGB(A) bitmap
};

typedef struct tagDDCAPS2 {
	DWORD dwCaps1;	// Zero or more of the DDSCAPS_* members
	DWORD dwCaps2;	// Zero or more of the DDSCAPS2_* members
	DWORD dwReserved[2];
} DDCAPS2;

// DIRECTDRAWSURFACE CAPABILITY FLAGS
enum {
	DDSCAPS_ALPHA	= 0x00000002l, // alpha only surface
	DDSCAPS_COMPLEX	= 0x00000008l, // complex surface structure
	DDSCAPS_TEXTURE	= 0x00001000l, // used as texture (should always be set)
	DDSCAPS_MIPMAP	= 0x00400000l  // Mipmap present
};

enum {
	DDSCAPS2_CUBEMAP			= 0x00000200L,
	DDSCAPS2_CUBEMAP_POSITIVEX	= 0x00000400L,
	DDSCAPS2_CUBEMAP_NEGATIVEX	= 0x00000800L,
	DDSCAPS2_CUBEMAP_POSITIVEY	= 0x00001000L,
	DDSCAPS2_CUBEMAP_NEGATIVEY	= 0x00002000L,
	DDSCAPS2_CUBEMAP_POSITIVEZ	= 0x00004000L,
	DDSCAPS2_CUBEMAP_NEGATIVEZ	= 0x00008000L,
	DDSCAPS2_VOLUME				= 0x00200000L
};

typedef struct tagDDSURFACEDESC2 {
	DWORD dwSize;	// size of this structure (must be 124)
	DWORD dwFlags;	// combination of the DDSS_* flags
	DWORD dwHeight;
	DWORD dwWidth;
	DWORD dwPitchOrLinearSize;
	DWORD dwDepth;	// Depth of a volume texture
	DWORD dwMipMapCount;
	DWORD dwReserved1[11];
	DDPIXELFORMAT ddpfPixelFormat;
	DDCAPS2 ddsCaps;
	DWORD dwReserved2;
} DDSURFACEDESC2;

enum {
	DDSD_CAPS			= 0x00000001l,
	DDSD_HEIGHT			= 0x00000002l,
	DDSD_WITH			= 0x00000004l,
	DDSD_PITCH			= 0x00000008l,
	DDSD_ALPHABITDEPTH  = 0x00000080l,
	DDSD_PIXELFORMAT	= 0x00001000l,
	DDSD_MIPMAPCOUNT	= 0x00020000l,
	DDSD_LINEARSIZE		= 0x00080000l,
	DDSD_DEPTH			= 0x00800000l
};

typedef struct tagDDSHEADER {
	DWORD dwMagic;			// FOURCC: "DDS "
	DDSURFACEDESC2 surfaceDesc;
} DDSHEADER;

#define MAKEFOURCC(ch0, ch1, ch2, ch3) \
	((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |   \
    ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))

#define FOURCC_DXT1	MAKEFOURCC('D','X','T','1')
#define FOURCC_DXT2	MAKEFOURCC('D','X','T','2')
#define FOURCC_DXT3	MAKEFOURCC('D','X','T','3')
#define FOURCC_DXT4	MAKEFOURCC('D','X','T','4')
#define FOURCC_DXT5	MAKEFOURCC('D','X','T','5')

// ----------------------------------------------------------
//   Structures used by DXT textures
// ----------------------------------------------------------

typedef struct tagColor8888 {
	BYTE b;
	BYTE g;
	BYTE r;
	BYTE a;
} Color8888;

typedef struct tagColor565 {
	WORD b : 5;
	WORD g : 6;
	WORD r : 5;
} Color565;

typedef struct tagDXTColBlock {
	Color565 colors[2];
	BYTE row[4];
} DXTColBlock;

typedef struct tagDXTAlphaBlockExplicit {
	WORD row[4];
} DXTAlphaBlockExplicit;

typedef struct tagDXTAlphaBlock3BitLinear {
	BYTE alpha[2];
	BYTE data[6];
} DXTAlphaBlock3BitLinear;

typedef struct tagDXT1Block
{
	DXTColBlock color;
} DXT1Block;

typedef struct tagDXT3Block {		// also used by dxt2
	DXTAlphaBlockExplicit alpha;
	DXTColBlock color;
} DXT3Block;

typedef struct tagDXT5Block {		// also used by dxt4
	DXTAlphaBlock3BitLinear alpha;
	DXTColBlock color;
} DXT5Block;

#ifdef WIN32
#	pragma pack(pop)
#else
#	pragma pack()
#endif

// ==========================================================

// Get the 4 possible colors for a block
//
static void 
GetBlockColors(DXTColBlock *block, Color8888 colors[4], bool isDXT1) {
	int i;
	for(i = 0; i < 2; i++)	{
		colors[i].a = 0xff;
		colors[i].r = block->colors[i].r << 3;
		colors[i].g = block->colors[i].g << 2;
		colors[i].b = block->colors[i].b << 3;
	}

	WORD *wCol = (WORD *)block->colors;
	if(wCol[0] > wCol[1] || !isDXT1) {
		// 4 color block
		for(i = 0; i < 2; ++i) {
			colors[i + 2].a = 0xff;
			colors[i + 2].r = (WORD (colors[0].r) * (2 - i) + WORD (colors[1].r) * (1 + i)) / 3;
			colors[i + 2].g = (WORD (colors[0].g) * (2 - i) + WORD (colors[1].g) * (1 + i)) / 3;
			colors[i + 2].b = (WORD (colors[0].b) * (2 - i) + WORD (colors[1].b) * (1 + i)) / 3;
		}
	}
	else {
		// 3 color block, number 4 is transparent
		colors[2].a = 0xff;
		colors[2].r = (WORD (colors[0].r) + WORD (colors[1].r)) / 2;
		colors[2].g = (WORD (colors[0].g) + WORD (colors[1].g)) / 2;
		colors[2].b = (WORD (colors[0].b) + WORD (colors[1].b)) / 2;

		colors[3].a = 0x00;
		colors[3].g = 0x00;
		colors[3].b = 0x00;
		colors[3].r = 0x00;
	}
}

static void 
DecodeColorBlock(BYTE *dstBits, DXTColBlock *block, int dstLine, Color8888 colors[4]) {
	for (int y = 0; y < 4; y++)	{
		Color8888 *dstColor = (Color8888 *)(dstBits + y * dstLine);
		unsigned row = block->row[y];
		for(int x = 0; x < 4; ++x) {
			unsigned bits = (row >> (x * 2)) & 3;
			*dstColor = colors[bits];
			dstColor++;
		}
	}
}

static void 
DecodeAlphaExplicit(BYTE *dstBits, DXTAlphaBlockExplicit &block, int dstLine) {
	for (int y = 0; y < 4; y++)	{
		Color8888 *dstColor = (Color8888 *)(dstBits + y * dstLine);
		unsigned row = block.row[y];
		for (int x = 0; x < 4; x++)	{
			unsigned bits = row & 0xf;
			dstColor->a = (bits * 0xff) / 0xf;
			dstColor++;
			row >>= 4;
		}
	}
}

static void 
DecodeAlpha3BitLinear(BYTE *dstBits, DXTAlphaBlock3BitLinear &block, int dstLine) {
	unsigned alphas[8];
	alphas[0] = block.alpha[0];
	alphas[1] = block.alpha[1];

	if (alphas[0] > alphas[1]) {
		// 8 alpha block
		for (int i = 0; i < 6; i++)	{
			alphas[i + 2] = ((6 - i) * alphas[0] + (1 + i) * alphas[1] + 3) / 7;
		}
	}
	else {
		// 6 alpha block
		for (int i = 0; i < 4; ++i) {
			alphas[i + 2] = ((4 - i) * alphas[0] + (1 + i) * alphas[1] + 2) / 5;
		}
		alphas[6] = 0;
		alphas[7] = 0xff;
	}

	for (int i = 0; i < 2; i++) {
		unsigned bits = unsigned(block.data[0 + i * 3]) | (unsigned(block.data[1 + i * 3]) << 8)
			| (unsigned(block.data[2 + i * 3]) << 16);
		for (int y = 0; y < 2; y++)	{
			Color8888 *dstColor = (Color8888 *)(dstBits + (i * 2 + y) * dstLine);
			for (int x = 0; x < 4; x++)	{
				dstColor->a = alphas[bits & 7];
				dstColor++;
				bits >>= 3;
			}
		}
	}
}

// ==========================================================
// Plugin Interface
// ==========================================================

static int s_format_id;

// ==========================================================
// Internal functions
// ==========================================================

static FIBITMAP *
LoadRGB(DDSURFACEDESC2 &desc, FreeImageIO *io, fi_handle handle, int page, int flags, void *data) {
	int width = (int)desc.dwWidth;
	int height = (int)desc.dwHeight;
	int bpp = (int)desc.ddpfPixelFormat.dwRGBBitCount;

	// allocate a new dib
	FIBITMAP *dib = FreeImage_Allocate (width, height, bpp, desc.ddpfPixelFormat.dwRBitMask,
		desc.ddpfPixelFormat.dwGBitMask, desc.ddpfPixelFormat.dwBBitMask);
	if (dib == NULL)
		return NULL;

	// read the file
	int line   = CalculateLine(width, bpp);
	int filePitch = (desc.dwFlags & DDSD_PITCH) ? (int)desc.dwPitchOrLinearSize : line;
	long delta = (long)filePitch - (long)line;
	for (int i = 0; i < height; i++) {
		io->read_proc (FreeImage_GetScanLine (dib, i), 1, line, handle);
		io->seek_proc (handle, delta, SEEK_CUR);
	}

	// enable transparency
	FreeImage_SetTransparent (dib, (desc.ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS) ? TRUE : FALSE);

	if (!(desc.ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS) && bpp == 32) {
		// no transparency: convert to 24-bit
		FIBITMAP *old = dib;
		dib = FreeImage_ConvertTo24Bits (old);
		FreeImage_Unload (old);
	}

	return dib;
}

static void 
LoadDXT1(FreeImageIO *io, fi_handle handle, int page, int flags, void *data, FIBITMAP *dib, BYTE *bits, int width, int height, int line) {
	DXT1Block *input_buffer = new DXT1Block[width / 4];
	for (int y = 0; y < height; y += 4)	{
		BYTE *dstBits = FreeImage_GetScanLine (dib, y);
		io->read_proc (input_buffer, sizeof (DXT1Block), width / 4, handle);
		DXT1Block *srcBlock = input_buffer;
		for (int x = 0; x < width; x += 4) {
			Color8888 colors[4];
			GetBlockColors (&srcBlock->color, colors, true);
			DecodeColorBlock (dstBits, &srcBlock->color, line, colors);
			dstBits += 4 * 4;
			srcBlock++;
		}
	}
	delete[] input_buffer;
}

static void 
LoadDXT3(FreeImageIO *io, fi_handle handle, int page, int flags, void *data, FIBITMAP *dib, BYTE *bits, int width, int height, int line) {
	DXT3Block *input_buffer = new DXT3Block[width / 4];
	for (int y = 0; y < height; y += 4) {
		BYTE *dstBits = FreeImage_GetScanLine (dib, y);
		io->read_proc (input_buffer, sizeof (DXT3Block), width / 4, handle);
		DXT3Block *srcBlock = input_buffer;
		for (int x = 0; x < width; x += 4) {
			Color8888 colors[4];
			GetBlockColors (&srcBlock->color, colors, false);
			DecodeColorBlock (dstBits, &srcBlock->color, line, colors);
			DecodeAlphaExplicit (dstBits, srcBlock->alpha, line);
			dstBits += 4 * 4;
			srcBlock++;
		}
	}
	delete[] input_buffer;
}

static void 
LoadDXT5(FreeImageIO *io, fi_handle handle, int page, int flags, void *data, FIBITMAP *dib, BYTE *bits, int width, int height, int line) {
	DXT5Block *input_buffer = new DXT5Block[width / 4];
	for (int y = 0; y < height; y += 4)	{
		BYTE *dstBits = FreeImage_GetScanLine (dib, y);
		io->read_proc (input_buffer, sizeof (DXT5Block), width / 4, handle);
		DXT5Block *srcBlock = input_buffer;
		for (int x = 0; x < width; x += 4) {
			Color8888 colors[4];
			GetBlockColors (&srcBlock->color, colors, true);
			DecodeColorBlock (dstBits, &srcBlock->color, line, colors);
			DecodeAlpha3BitLinear (dstBits, srcBlock->alpha, line);
			dstBits += 4 * 4;
			srcBlock++;
		}
	}
	delete[] input_buffer;
}

static FIBITMAP *
LoadDXT(int type, DDSURFACEDESC2 &desc, FreeImageIO *io, fi_handle handle, int page, int flags, void *data) {
	int width = (int)desc.dwWidth;
	int height = (int)desc.dwHeight;

	// allocate a 32-bit dib
	FIBITMAP *dib = FreeImage_Allocate (width, height, 32, 0x000000FF, 0x0000FF00, 0x00FF0000);
	if (dib == NULL)
		return NULL;

	int bpp = FreeImage_GetBPP (dib);
	int line = CalculateLine (width, bpp);
	BYTE *bits = FreeImage_GetBits (dib);

	// select the right decoder
	switch (type) {
		case 1:
			LoadDXT1 (io, handle, page, flags, data, dib, bits, width, height, line);
			break;
		case 3:
			LoadDXT3 (io, handle, page, flags, data, dib, bits, width, height, line);
			break;
		case 5:
			LoadDXT5 (io, handle, page, flags, data, dib, bits, width, height, line);
			break;
	}
	
	return dib;
}
// ==========================================================
// Plugin Implementation
// ==========================================================

static const char * DLL_CALLCONV
Format() {
	return "DDS";
}

static const char * DLL_CALLCONV
Description() {
	return "DirectDraw Surface";
}

static const char * DLL_CALLCONV
Extension() {
	return "dds";
}

static const char * DLL_CALLCONV
RegExpr() {
	return NULL;
}

static const char * DLL_CALLCONV
MimeType() {
	return "image/freeimage-dds";
}

static BOOL DLL_CALLCONV
Validate(FreeImageIO *io, fi_handle handle) {
	DDSHEADER header;
	io->read_proc(&header, 1, sizeof(header), handle);
	if (header.dwMagic != MAKEFOURCC ('D','D','S',' '))
		return FALSE;
	if (header.surfaceDesc.dwSize != sizeof (header.surfaceDesc) ||
		header.surfaceDesc.ddpfPixelFormat.dwSize != sizeof (header.surfaceDesc.ddpfPixelFormat))
		return FALSE;
	return TRUE;
}

static BOOL DLL_CALLCONV
SupportsExportDepth(int depth) {
	return FALSE;
}

static BOOL DLL_CALLCONV 
SupportsExportType(FREE_IMAGE_TYPE type) {
	return FALSE;
}

// ----------------------------------------------------------

static void * DLL_CALLCONV
Open(FreeImageIO *io, fi_handle handle, BOOL read) {
	return NULL;
}

static void DLL_CALLCONV
Close(FreeImageIO *io, fi_handle handle, void *data) {
}

// ----------------------------------------------------------

static int DLL_CALLCONV
PageCount(FreeImageIO *io, fi_handle handle, void *data) {
	return 0;
}

// ----------------------------------------------------------

static FIBITMAP * DLL_CALLCONV
Load(FreeImageIO *io, fi_handle handle, int page, int flags, void *data) {
	DDSHEADER header;
	FIBITMAP *dib = NULL;

	io->read_proc(&header, 1, sizeof(header), handle);

	if (header.surfaceDesc.ddpfPixelFormat.dwFlags & DDPF_RGB) {
		dib = LoadRGB (header.surfaceDesc, io, handle, page, flags, data);
	}
	else if (header.surfaceDesc.ddpfPixelFormat.dwFlags & DDPF_FOURCC) {
		switch (header.surfaceDesc.ddpfPixelFormat.dwFourCC) {
			case FOURCC_DXT1:
				dib = LoadDXT (1, header.surfaceDesc, io, handle, page, flags, data);
				break;
			case FOURCC_DXT3:
				dib = LoadDXT (3, header.surfaceDesc, io, handle, page, flags, data);
				break;
			case FOURCC_DXT5:
				dib = LoadDXT (5, header.surfaceDesc, io, handle, page, flags, data);
				break;
		}
	}

	return dib;
}

static BOOL DLL_CALLCONV
Save(FreeImageIO *io, FIBITMAP *dib, fi_handle handle, int page, int flags, void *data) {
	return FALSE;
}

// ==========================================================
//   Init
// ==========================================================

void DLL_CALLCONV
InitDDS(Plugin *plugin, int format_id) {
	s_format_id = format_id;

	plugin->format_proc = Format;
	plugin->description_proc = Description;
	plugin->extension_proc = Extension;
	plugin->regexpr_proc = RegExpr;
	plugin->open_proc = NULL; //Open; // not implemented
	plugin->close_proc = NULL; //Close;
	plugin->pagecount_proc = NULL; //PageCount;
	plugin->pagecapability_proc = NULL;
	plugin->load_proc = Load;
	plugin->save_proc = NULL;	//Save;	// not implemented (yet?)
	plugin->validate_proc = Validate;
	plugin->mime_proc = MimeType;
	plugin->supports_export_bpp_proc = SupportsExportDepth;
	plugin->supports_export_type_proc = SupportsExportType;
	plugin->supports_icc_profiles_proc = NULL;
}
