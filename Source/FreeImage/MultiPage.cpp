// ==========================================================
// Multi-Page functions
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

#pragma warning (disable : 4786)

#include <malloc.h>

#include "FreeImageIO.h"
#include "Plugin.h"

// ----------------------------------------------------------

FI_STRUCT (MULTIBITMAPHEADER) {
	PluginNode *node;
	FREE_IMAGE_FORMAT fif;
	FreeImageIO *io;
	fi_handle handle;
	long tell;
	BOOL use_internal_io;
	unsigned current_page;
	FIBITMAP *current_bitmap;
	BOOL changed;
	int page_count;
};

// =====================================================================
// Interal Multipage functions
// =====================================================================

inline MULTIBITMAPHEADER *
FreeImage_GetMultiBitmapHeader(FIMULTIBITMAP *bitmap) {
	return (MULTIBITMAPHEADER *)bitmap->data;
}

// =====================================================================
// Multipage functions
// =====================================================================

FIMULTIBITMAP * DLL_CALLCONV
FreeImage_OpenMultiBitmapFromHandle(FREE_IMAGE_FORMAT fif, FreeImageIO *io, fi_handle handle) {
	PluginList *list = FreeImage_GetPluginList();

	if ((list) && (io)) {
		PluginNode *node = list->FindNodeFromFIF(fif);

		if (node) {
			FIMULTIBITMAP *bitmap = (FIMULTIBITMAP *)malloc(sizeof(FIMULTIBITMAP));

			if (bitmap != NULL) {
				// fill in the multibitmap data

				MULTIBITMAPHEADER *header = (MULTIBITMAPHEADER *)malloc(sizeof(MULTIBITMAPHEADER));

				header->node = node;
				header->fif = fif;
				header->io = io;
				header->handle = handle;
				header->tell = io->tell_proc(handle);
				header->use_internal_io = FALSE;
				header->changed = TRUE;
				header->current_bitmap = NULL;

				bitmap->data = header;

				// select the first page in the bitmap

				FreeImage_GetPageCount(bitmap);
				FreeImage_SelectPage(bitmap, 0);

				// return the multibitmap

				return bitmap;
			}
		}
	}

	return NULL;
}

FIMULTIBITMAP * DLL_CALLCONV
FreeImage_OpenMultiBitmap(FREE_IMAGE_FORMAT fif, const char *filename, BOOL create_new) {
	FreeImageIO *io = new FreeImageIO;
	SetDefaultIO(io);
	
	FILE *handle = fopen(filename, "r+b");

	if (handle) {
		FIMULTIBITMAP *bitmap = FreeImage_OpenMultiBitmapFromHandle(fif, io, (fi_handle)handle);	

		FreeImage_GetMultiBitmapHeader(bitmap)->use_internal_io = TRUE;

		return bitmap;
	}


	return NULL;
}


BOOL DLL_CALLCONV
FreeImage_CloseMultiBitmap(FIMULTIBITMAP *bitmap) {
	if (bitmap) {
		if (bitmap->data) {
			MULTIBITMAPHEADER *header = FreeImage_GetMultiBitmapHeader(bitmap);

			if (header->use_internal_io) {
				fclose((FILE *)header->handle);

				delete header->io;
			}

			if (header->current_bitmap) {
				FreeImage_Unload(header->current_bitmap);
			}

			free(bitmap->data);
		}

		free(bitmap);

		return TRUE;
	}

	return FALSE;
}

int DLL_CALLCONV
FreeImage_GetPageCount(FIMULTIBITMAP *bitmap) {	
	if (bitmap) {
		MULTIBITMAPHEADER *header = FreeImage_GetMultiBitmapHeader(bitmap);

		if (header->changed) {
			PluginList *list = FreeImage_GetPluginList();

			if (list) {
				header->changed = FALSE;

				header->io->seek_proc(header->handle, header->tell, SEEK_SET);

   				void *data = FreeImage_Open(header->node, header->io, header->handle, TRUE);

				header->page_count = (header->node->m_plugin->pagecount_proc != NULL) ? header->node->m_plugin->pagecount_proc(header->io, header->handle, data) : 1;

				FreeImage_Close(header->node, header->io, header->handle, data);
			}
		}

		return header->page_count;
	}

	return 0;
}

BOOL DLL_CALLCONV
FreeImage_SelectPage(FIMULTIBITMAP *bitmap, int page) {
	PluginList *list = FreeImage_GetPluginList();

	if ((bitmap) && (list)) {
		MULTIBITMAPHEADER *header = FreeImage_GetMultiBitmapHeader(bitmap);

		header->io->seek_proc(header->handle, header->tell, SEEK_SET);

   		void *data = FreeImage_Open(header->node, header->io, header->handle, TRUE);

		FIBITMAP *bitmap = (header->node->m_plugin->load_proc != NULL) ? header->node->m_plugin->load_proc(FreeImage_GetFreeImageStruct(), header->io, header->handle, page, 0, data) : NULL;

		FreeImage_Close(header->node, header->io, header->handle, data);

		if (bitmap) {
			FreeImage_Unload(header->current_bitmap);

			header->current_page = page;
			header->current_bitmap = bitmap;
		}

		return bitmap != NULL;
	}

	return FALSE;
}

int DLL_CALLCONV
FreeImage_GetCurrentPageNumber(FIMULTIBITMAP *bitmap) {
	if (bitmap) {
		MULTIBITMAPHEADER *header = FreeImage_GetMultiBitmapHeader(bitmap);
		
		return header->current_page;
	}

	return -1;
}

FIBITMAP * DLL_CALLCONV
FreeImage_GetCurrentPage(FIMULTIBITMAP *bitmap) {
	if (bitmap) {
		MULTIBITMAPHEADER *header = FreeImage_GetMultiBitmapHeader(bitmap);
		
		return header->current_bitmap;
	}

	return NULL;
}
