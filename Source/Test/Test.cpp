// ==========================================================
// FreeImage 2 Test Script
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

//#define QT
//#define API
#define MULTI

#include <assert.h>
#include <stdio.h>
#include <io.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <algorithm>
#include <functional>
#include <vector>

using namespace std;

#ifdef QT
#include <qapplication.h>
#include <qmainwindow.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qstring.h>
#include "FreeImageQt.h"
#endif

#ifdef API
#include "FreeImage.h"
#endif

#ifdef MULTI
#include "FreeImage.h"
#endif

// ----------------------------------------------------------

/*
FIBITMAP *
FreeImage_ExtractTransparency(FIBITMAP *dib) {
	if (FreeImage_GetBPP(dib) == 32) {
		FIBITMAP *new_dib = FreeImage_Allocate(FreeImage_GetWidth(dib), FreeImage_GetHeight(dib), 8);
		
		// make a grayscale palette

		RGBQUAD *quad = FreeImage_GetPalette(new_dib);

		for (int i = 0; i < 256; ++i) {
			quad[i].rgbRed =
			quad[i].rgbGreen =
			quad[i].rgbBlue = i;
		}

		// copy the alpha channel

		for (unsigned y = 0; y < FreeImage_GetHeight(dib); ++y) {
			BYTE    *t = FreeImage_GetScanLine(new_dib, y);
			RGBQUAD *s = (RGBQUAD *)FreeImage_GetScanLine(dib, y);

			for (unsigned x = 0; x < FreeImage_GetWidth(dib); ++x) {
				t[x] = s[x].rgbReserved;
			}
		}

		return new_dib;
	}

	return NULL;
}
*/

// ----------------------------------------------------------

static char s_error_message[256];

void
MessageFunc(FREE_IMAGE_FORMAT fif, const char *message) {
	strcpy(s_error_message, message);
}

// ----------------------------------------------------------

#ifdef MULTI
int __cdecl
TestMulti(int argc, char *argv[]) {
	FreeImage_Initialise();

	FIMULTIBITMAP *bitmap = FreeImage_OpenMultiBitmap(FIF_TIFF, "Kopie van money-256.tif", FALSE, FALSE);

	int count = FreeImage_GetPageCount(bitmap);

	FIBITMAP *clone = FreeImage_LoadBMP("e:\\test images\\MK50TEMP.BMP");
	FreeImage_AppendPage(bitmap, clone);
	FreeImage_Unload(clone);

	FreeImage_CloseMultiBitmap(bitmap);

	FreeImage_DeInitialise();

	return 0;
}

#endif

// ----------------------------------------------------------

#ifdef API

struct Extensions {
	char *ext;
	void *function;
};

static Extensions s_extensions[] = {
	"bmp", FreeImage_SaveBMP,
	"jpg", FreeImage_SaveJPEG,
	"png", FreeImage_SavePNG,
	"tif", FreeImage_SaveTIFF,
	"wbmp", FreeImage_SaveWBMP,
	"ppm", FreeImage_SavePNM,
};

int __cdecl
TestAPI(int argc, char *argv[]) {
	FreeImage_Initialise();
	FreeImage_SetOutputMessage(MessageFunc);

	FIBITMAP *dib = NULL;
	int id = 1;

	// print version strings

	printf(FreeImage_GetVersion());
	printf(FreeImage_GetCopyrightMessage());

	// open the log file

	FILE *log_file = fopen("log_file.txt", "w");

	// batch convert all supported bitmaps

	_finddata_t finddata;

	long handle;

	if ((handle = _findfirst("e:\\test images\\*.cut", &finddata)) != -1) {
		do {
			dib = NULL;

			// grab the extension

			char ext[4];
			ext[3] = 0;
			strncpy(ext, finddata.name + strlen(finddata.name) - 3, 3);

			// make a path to a directory

			char directory[128];
			strcpy(directory, "e:\\test images\\");
			strcat(directory, finddata.name);

			// open the file

			int before = GetTickCount();

			printf("loading %s... ", finddata.name);


			//FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(directory);
			FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(directory, 16);

			switch(fif) {
				case FIF_BMP :
					dib = FreeImage_LoadBMP(directory, BMP_DEFAULT);
					break;

				case FIF_CUT :
					dib = FreeImage_LoadCUT(directory, CUT_DEFAULT);
					break;

				case FIF_ICO :
					dib = FreeImage_LoadICO(directory, ICO_DEFAULT);
					break;

				case FIF_LBM :
					dib = FreeImage_LoadLBM(directory, LBM_DEFAULT);
					break;

				case FIF_JPEG :
					dib = FreeImage_LoadJPEG(directory, JPEG_DEFAULT);
					break;

				case FIF_KOALA :
					dib = FreeImage_LoadKOALA(directory, KOALA_DEFAULT);
					break;

				case FIF_MNG :
				case FIF_JNG :
					dib = FreeImage_LoadMNG(directory, PCD_DEFAULT);
					break;

				case FIF_PCD :
					dib = FreeImage_LoadPCD(directory, PCD_DEFAULT);
					break;

				case FIF_PCX :
					dib = FreeImage_LoadPCX(directory, PCX_DEFAULT);
					break;

				case FIF_PNG :
					dib = FreeImage_LoadPNG(directory, PNG_DEFAULT);
					break;

				case FIF_PBM :
					dib = FreeImage_LoadPNM(directory, PNM_DEFAULT);
					break;

				case FIF_PGM :
					dib = FreeImage_LoadPNM(directory, PNM_DEFAULT);
					break;

				case FIF_PPM :
					dib = FreeImage_LoadPNM(directory, PNM_DEFAULT);
					break;

				case FIF_PSD :
					dib = FreeImage_LoadPSD(directory, PSD_DEFAULT);
					break;

				case FIF_RAS :
					dib = FreeImage_LoadRAS(directory, RAS_DEFAULT);
					break;

				case FIF_TARGA :
					dib = FreeImage_LoadTARGA(directory, TARGA_DEFAULT);
					break;

				case FIF_TIFF :
					dib = FreeImage_LoadTIFF(directory, TIFF_DEFAULT);
					break;

				default :
					if ((fif == FreeImage_GetFIFFromFormat("GIF")) || (fif == FreeImage_GetFIFFromFormat("JBIG"))) {
						dib = FreeImage_Load(fif, directory, 0);
					} else {
						strcpy(s_error_message, "file extension not bound to plugin");
					}

					break;
			};

			if (dib != NULL) {
				printf("done in %d ms\n", GetTickCount() - before);

				char unique[128];

				FreeImage_SetTransparent(dib, FALSE);

				for (int i = 0; i < sizeof(s_extensions) / sizeof(s_extensions[0]); ++i) {
					FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(s_extensions[i].ext);

					if ((FreeImage_FIFSupportsWriting(fif)) && (FreeImage_FIFSupportsExportBPP(fif, FreeImage_GetBPP(dib)))) {
						FIBITMAP *c = FreeImage_Clone(dib);

						itoa(id, unique, 10);
						strcat(unique, ".");
						strcat(unique, s_extensions[i].ext);
						static_cast<void *(DLL_CALLCONV *)(FIBITMAP *dib, const char *filename, int flags)>(s_extensions[i].function)(c, unique, BMP_DEFAULT);

						FreeImage_Unload(c);
					}
				}

				FreeImage_Free(dib);

				fwrite(unique, strlen(unique), 1, log_file);
				fwrite(" >> ", 4, 1, log_file);
				fwrite(directory, strlen(directory), 1, log_file);
				fwrite("\n", 1, 1, log_file);

				id++;
			} else {
				printf("failed\n");
				printf("reason: %s\n", s_error_message);
			}
		} while (_findnext(handle, &finddata) == 0);

		_findclose(handle);
	}

	fclose(log_file);

	FreeImage_DeInitialise();

	return 0;
}
#endif

// ----------------------------------------------------------

#ifdef QT
int __cdecl
TestQt(int argc, char *argv[]) {
	FIQT_Register(false);

	QApplication application(argc, argv);

	QMainWindow window(NULL);

	_finddata_t finddata;

	int count = 0;

	long handle;

	int i = FIQT_GetFIFCount();

	printf("supported bitmaps: %d\n", i);
	
	for (int j = 0; j < i; ++j)
		printf("bitmap type %d (%s): %s (%s)\n", j, FIQT_GetFormatFromFIF((FREE_IMAGE_FORMAT)j), FIQT_GetFIFDescription((FREE_IMAGE_FORMAT)j), FIQT_GetFIFExtensionList((FREE_IMAGE_FORMAT)j));

	if ((handle = _findfirst("d:\\test images\\*.jpg", &finddata)) != -1) {
		do {
			QPixmap pixmap(QObject::tr("d:\\test images\\%1").arg(finddata.name));

			if (FIQT_FIFSupportsWriting(FIQT_GetFIFFromFormat("PNG")))
				pixmap.save(QObject::tr("%1.png").arg(count++), "PNG");
		} while (_findnext(handle, &finddata) == 0);

		_findclose(handle);
	}

	return 0;
}
#endif
// ----------------------------------------------------------

int __cdecl
main(int argc, char *argv[]) {
#ifdef API
	for (int j = FreeImage_GetFIFCount() - 1; j >= 0; --j)
		printf("bitmap type %d (%s): %s (%s)\n", j, FreeImage_GetFormatFromFIF((FREE_IMAGE_FORMAT)j), FreeImage_GetFIFDescription((FREE_IMAGE_FORMAT)j), FreeImage_GetFIFExtensionList((FREE_IMAGE_FORMAT)j));

	return TestAPI(argc, argv);
#endif
#ifdef QT
	return TestQt(argc, argv); 
#endif
#ifdef MULTI
	return TestMulti(argc, argv);
#endif

	return 0;
}
