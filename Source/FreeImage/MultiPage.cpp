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
#include <list>
#include <windows.h>

#include "../zlib/zlib.h"

#include "CacheFile.h"
#include "FreeImageIO.h"
#include "Plugin.h"
#include "Utilities.h"

// ----------------------------------------------------------

enum BlockType { BLOCK_CONTINUEUS, BLOCK_REFERENCE };

// ----------------------------------------------------------

struct Block {
	BlockType m_type;
};

struct BlockContinueus {
	BlockType m_type;
	int       m_start;
	int       m_end;

	BlockContinueus(int s, int e) :
	m_type(BLOCK_CONTINUEUS),
	m_start(s),
	m_end(e) {
	}	
};

struct BlockReference {
	BlockType m_type;
	int       m_reference;
	int       m_size;
	int       m_uncompressed_size;

	BlockReference(int r, int size, int unsize) :
	m_type(BLOCK_REFERENCE),
	m_reference(r),
	m_size(size),
	m_uncompressed_size(unsize) {
	}
};

// ----------------------------------------------------------

typedef std::list<Block *> BlockList;
typedef std::list<Block *>::iterator BlockListIterator;

// ----------------------------------------------------------

FI_STRUCT (MULTIBITMAPHEADER) {
	PluginNode *node;
	FREE_IMAGE_FORMAT fif;
	FreeImageIO *io;
	fi_handle handle;
	CacheFile *m_cachefile;
	std::map<FIBITMAP *, int> locked_pages;
	BOOL changed;
	int page_count;
	BlockList m_blocks;
	const char *m_filename;
	BOOL read_only;
};

// =====================================================================
// Interal Multipage functions
// =====================================================================

inline MULTIBITMAPHEADER *
FreeImage_GetMultiBitmapHeader(FIMULTIBITMAP *bitmap) {
	return (MULTIBITMAPHEADER *)bitmap->data;
}

static BlockListIterator DLL_CALLCONV
FreeImage_FindBlock(FIMULTIBITMAP *bitmap, int position) {
	if (bitmap) {
		MULTIBITMAPHEADER *header = FreeImage_GetMultiBitmapHeader(bitmap);

		// step 1: find the block that matches the given position

		int prev_count = 0;
		int count = 0;
		BlockListIterator i;
		Block *current_block = NULL;

		for (i = header->m_blocks.begin(); i != header->m_blocks.end(); ++i) {
			prev_count = count;

			switch((*i)->m_type) {
				case BLOCK_CONTINUEUS :
					count += ((BlockContinueus *)(*i))->m_end - ((BlockContinueus *)(*i))->m_start + 1;
					break;

				case BLOCK_REFERENCE :
					count++;
					break;
			}

			current_block = *i;

			if (count >= position)
				break;
		}

		// step 2: make sure we found the node. from here it gets a little complicated:
		// * if the block is there, just return it
		// * if the block is a series of blocks, split it in max 3 new blocks
		//   and return the splitted block

		if ((current_block) && (count >= position)) {
			switch(current_block->m_type) {
				case BLOCK_REFERENCE :
					return i;

				case BLOCK_CONTINUEUS :
				{
					BlockContinueus *block = (BlockContinueus *)current_block;

					if (block->m_start != block->m_end) {
						int item = block->m_start + (position - prev_count);

						if (item != block->m_start) {
							BlockContinueus *block_a = new BlockContinueus(block->m_start, item - 1);
							header->m_blocks.insert(i, (Block *)block_a);
						}

						BlockContinueus *block_b = new BlockContinueus(item, item);
						BlockListIterator block_target = header->m_blocks.insert(i, (Block *)block_b);

						if (item != block->m_end) {
							BlockContinueus *block_c = new BlockContinueus(item + 1, block->m_end);
							header->m_blocks.insert(i, (Block *)block_c);
						}

						header->m_blocks.remove((Block *)block);
						delete block;
						
						return block_target;
					}

					return i;
				}
			}
		}
	}

	return 0;
}

int DLL_CALLCONV
FreeImage_InternalGetPageCount(FIMULTIBITMAP *bitmap) {	
	if (bitmap) {
		if (((MULTIBITMAPHEADER *)bitmap->data)->handle) {
			MULTIBITMAPHEADER *header = FreeImage_GetMultiBitmapHeader(bitmap);

			header->io->seek_proc(header->handle, 0, SEEK_SET);

   			void *data = FreeImage_Open(header->node, header->io, header->handle, TRUE);

			int page_count = (header->node->m_plugin->pagecount_proc != NULL) ? header->node->m_plugin->pagecount_proc(header->io, header->handle, data) : 1;

			FreeImage_Close(header->node, header->io, header->handle, data);

			return page_count;
		}
	}

	return 0;
}

static int DLL_CALLCONV
FreeImage_CompressData(BYTE *target, int target_size, BYTE *data, int source_size) {
	unsigned long dest_len = target_size;

	compress(target, &dest_len, data, source_size);

	return dest_len;
}

static int DLL_CALLCONV
FreeImage_UncompressData(BYTE *target, int target_size, BYTE *data, int source_size) {
	unsigned long dest_len = target_size;

	uncompress(target, &dest_len, data, source_size);

	return dest_len;
}

// =====================================================================
// Multipage functions
// =====================================================================

FIMULTIBITMAP * DLL_CALLCONV
FreeImage_OpenMultiBitmap(FREE_IMAGE_FORMAT fif, const char *filename, BOOL create_new, BOOL read_only, BOOL keep_cache_in_memory) {
	// sanity check on the parameters

	if (create_new)
		read_only = false;

	// retrieve the plugin list to find the node belonging to this plugin

	PluginList *list = FreeImage_GetPluginList();

	if (list) {
		PluginNode *node = list->FindNodeFromFIF(fif);

		if (node) {
			FreeImageIO *io = new FreeImageIO;

			if (io) {
				SetDefaultIO(io);

				bool cont = true;

				FILE *handle = NULL;

				if (!create_new) {
					handle = fopen(filename, "rb");

					if (handle == NULL) {
						cont = false;
					}
				}

				if (cont) {
					FIMULTIBITMAP *bitmap = new FIMULTIBITMAP;

					if (bitmap) {
						MULTIBITMAPHEADER *header = new MULTIBITMAPHEADER;

						header->m_filename = filename;
						header->node = node;
						header->fif = fif;
						header->io = io;
						header->handle = handle;						
						header->changed = FALSE;						
						header->read_only = read_only;
						header->m_cachefile = NULL;

						if (header) {
							// store the MULTIBITMAPHEADER in the surrounding FIMULTIBITMAP structure

							bitmap->data = header;

							// cache the page count

							header->page_count = FreeImage_InternalGetPageCount(bitmap);

							// allocate a continueus block to describe the bitmap

							if (!create_new)
								header->m_blocks.push_back((Block *)new BlockContinueus(0, header->page_count - 1));

							// set up the cache

							if (!read_only) {
								char cache_name[256];
								ReplaceExtension(cache_name, filename, "ficache");

								CacheFile *cache_file = new CacheFile(cache_name, keep_cache_in_memory);

								if (cache_file->open()) {
									header->m_cachefile = cache_file;

									// return the multibitmap

									return bitmap;
								}

								delete cache_file;
								delete header;
							}

							return bitmap;
						}
						
						return NULL;
					}
				}
			}

			delete io;
		}
	}

	return NULL;
}

BOOL DLL_CALLCONV
FreeImage_CloseMultiBitmap(FIMULTIBITMAP *bitmap) {
	if (bitmap) {
		if (bitmap->data) {
			MULTIBITMAPHEADER *header = FreeImage_GetMultiBitmapHeader(bitmap);

			if (header->changed) {
				// open a temp file

				char spool_name[256];

				ReplaceExtension(spool_name, header->m_filename, "fispool");

				// open the spool file and the source file

				FILE *f = fopen(spool_name, "w+b");

				void *data = FreeImage_Open(header->node, header->io, (fi_handle)f, FALSE);
				void *data_read = NULL;

				if (header->handle) {
					header->io->seek_proc(header->handle, 0, SEEK_SET);

			   		data_read = FreeImage_Open(header->node, header->io, header->handle, TRUE);
				}

				// write all the pages to the temp file

				int count = 0;
				BOOL success = TRUE;

				for (BlockListIterator i = header->m_blocks.begin(); i != header->m_blocks.end(); ++i, count++) {
					if (success) {
						switch((*i)->m_type) {
							case BLOCK_CONTINUEUS :
							{
								BlockContinueus *block = (BlockContinueus *)(*i);

								for (int j = block->m_start; j <= block->m_end; ++j) {
									FIBITMAP *bitmap = header->node->m_plugin->load_proc(FreeImage_GetFreeImageStruct(), header->io, header->handle, j, 0, data_read);

									success = header->node->m_plugin->save_proc(FreeImage_GetFreeImageStruct(), header->io, bitmap, (fi_handle)f, count, 0, data);

									FreeImage_Unload(bitmap);
								}

								break;
							}

							case BLOCK_REFERENCE :
							{
								BlockReference *ref = (BlockReference *)(*i);

								FIBITMAP bitmap;
								bitmap.data = new BYTE[ref->m_uncompressed_size];

								// read the compressed data

								BYTE *compressed_data = new BYTE[ref->m_size + (0.1 * ref->m_size) + 12];

								header->m_cachefile->readFile((BYTE *)compressed_data, ref->m_reference, ref->m_size);

								// uncompress the data

								FreeImage_UncompressData((BYTE *)bitmap.data, ref->m_uncompressed_size, compressed_data, ref->m_size);

								// save the data

								success = header->node->m_plugin->save_proc(FreeImage_GetFreeImageStruct(), header->io, &bitmap, (fi_handle)f, count, 0, data);

								// get rid of the buffers

								delete [] compressed_data;							
								delete [] bitmap.data;

								break;
							}
						}
					} else {
						break;
					}
				}

				// close the files

				fclose(f);

				if (header->handle) {
					FreeImage_Close(header->node, header->io, header->handle, data_read);

					fclose((FILE *)header->handle);
				}	

				if (success) {
					remove(header->m_filename);

					rename(spool_name, header->m_filename);
				} else {
					remove(spool_name);
				}
			} else {
				if (header->handle) {
					fclose((FILE *)header->handle);
				}
			}

			// clear the blocks list

			for (BlockListIterator i = header->m_blocks.begin(); i != header->m_blocks.end(); ++i)
				delete *i;			

			// flush and dispose the cache

			if (header->m_cachefile) {
				header->m_cachefile->close();

				delete header->m_cachefile;
			}

			// delete the last open bitmaps

			while (!header->locked_pages.empty()) {
				FreeImage_Unload(header->locked_pages.begin()->first);

				header->locked_pages.erase(header->locked_pages.begin()->first);
			}

			// get rid of the IO structure

			delete header->io;

			// delete the FIMULTIBITMAPHEADER

			delete bitmap->data;
		}

		delete bitmap;

		return TRUE;
	}

	return FALSE;
}

int DLL_CALLCONV
FreeImage_GetPageCount(FIMULTIBITMAP *bitmap) {
	if (bitmap) {
		MULTIBITMAPHEADER *header = FreeImage_GetMultiBitmapHeader(bitmap);

		if (header->page_count == -1) {
			header->page_count = 0;

			for (BlockListIterator i = header->m_blocks.begin(); i != header->m_blocks.end(); ++i) {
				switch((*i)->m_type) {
					case BLOCK_CONTINUEUS :
						header->page_count += ((BlockContinueus *)(*i))->m_end - ((BlockContinueus *)(*i))->m_start + 1;
						break;

					case BLOCK_REFERENCE :
						header->page_count++;
						break;
				}
			}
		}

		return header->page_count;
	}

	return 0;
}

void DLL_CALLCONV
FreeImage_AppendPage(FIMULTIBITMAP *bitmap, FIBITMAP *data) {
	if ((bitmap) && (data)) {
		MULTIBITMAPHEADER *header = FreeImage_GetMultiBitmapHeader(bitmap);

		if ((!header->read_only) && (header->locked_pages.empty())) {
			int size = FreeImage_GetFreeImageHeaderSize() + FreeImage_GetDIBSize(data);

			// compress the bitmap data

			BYTE *compressed_data = new BYTE[size + (0.1 * size) + 12];
			int compressed_size = FreeImage_CompressData(compressed_data, size, (BYTE *)data->data, size);

			// write the compressed data to the cache

			int ref = header->m_cachefile->writeFile(compressed_data, compressed_size);

			BlockReference *block = new BlockReference(ref, compressed_size, size);

			// get rid of the compressed data

			delete [] compressed_data;

			// add the block

			header->m_blocks.push_back((Block *)block);
			header->changed = true;
			header->page_count = -1;
		}
	}
}

void DLL_CALLCONV
FreeImage_InsertPage(FIMULTIBITMAP *bitmap, int page, FIBITMAP *data) {
	if ((bitmap) && (data)) {
		if (page < FreeImage_GetPageCount(bitmap)) {
			MULTIBITMAPHEADER *header = FreeImage_GetMultiBitmapHeader(bitmap);

			if ((!header->read_only) && (header->locked_pages.empty())) {
				int size = FreeImage_GetFreeImageHeaderSize() + FreeImage_GetDIBSize(data);

				// compress the bitmap data

				BYTE *compressed_data = new BYTE[size + (0.1 * size) + 12];
				int compressed_size = FreeImage_CompressData(compressed_data, size, (BYTE *)data->data, size);

				// write the compressed data to the cache

				int ref = header->m_cachefile->writeFile(compressed_data, compressed_size);

				// add a block

				if (page > 0) {
					BlockListIterator block_source = FreeImage_FindBlock(bitmap, page - 1);

					BlockReference *block = new BlockReference(ref, compressed_size, size);

					header->m_blocks.insert(block_source, (Block *)block);
				} else {
					BlockReference *block = new BlockReference(ref, compressed_size, size);

					header->m_blocks.push_front((Block *)block);
				}

				// get rid of the compressed buffer

				delete [] compressed_data;

				header->changed = true;
				header->page_count = -1;
			}
		}
	}
}

void DLL_CALLCONV
FreeImage_DeletePage(FIMULTIBITMAP *bitmap, int page) {
	if (bitmap) {
		MULTIBITMAPHEADER *header = FreeImage_GetMultiBitmapHeader(bitmap);

		if ((!header->read_only) && (header->locked_pages.empty())) {
			if (FreeImage_GetPageCount(bitmap) > 1) {
				BlockListIterator i = FreeImage_FindBlock(bitmap, page);

				if (i != 0) {
					switch((*i)->m_type) {
						case BLOCK_CONTINUEUS :
							header->m_blocks.erase(i);
							break;

						case BLOCK_REFERENCE :
							header->m_cachefile->deleteFile(((BlockReference *)(*i))->m_reference);
							header->m_blocks.erase(i);
							break;
					}

					header->changed = true;
					header->page_count = -1;
				}
			}
		}
	}
}


FIBITMAP * DLL_CALLCONV
FreeImage_LockPage(FIMULTIBITMAP *bitmap, int page) {
	if (bitmap) {
		MULTIBITMAPHEADER *header = FreeImage_GetMultiBitmapHeader(bitmap);

		// only lock if the page wasn't locked before...

		for (std::map<FIBITMAP *, int>::iterator i = header->locked_pages.begin(); i != header->locked_pages.end(); ++i) {
			if (i->second == page) {
				return NULL;
			}
		}

		// open the bitmap

		header->io->seek_proc(header->handle, 0, SEEK_SET);

   		void *data = FreeImage_Open(header->node, header->io, header->handle, TRUE);

		// load the bitmap data

		if (data != NULL) {
			FIBITMAP *bitmap = (header->node->m_plugin->load_proc != NULL) ? header->node->m_plugin->load_proc(FreeImage_GetFreeImageStruct(), header->io, header->handle, page, 0, data) : NULL;

			// close the file

			FreeImage_Close(header->node, header->io, header->handle, data);

			// if there was still another bitmap open, get rid of it

			if (bitmap) {
				header->locked_pages[bitmap] = page;

				return bitmap;
			}	

			return NULL;
		}
	}

	return NULL;
}

void DLL_CALLCONV
FreeImage_UnlockPage(FIMULTIBITMAP *bitmap, FIBITMAP *page, BOOL changed) {
	if ((bitmap) && (page) && (changed)) {
		MULTIBITMAPHEADER *header = FreeImage_GetMultiBitmapHeader(bitmap);

		// find out if the page we try to unlock is actually locked...

		if (header->locked_pages.find(page) != header->locked_pages.end()) {
			// store the bitmap compressed in the cache for later writing

			if (!header->read_only) {
				header->changed = true;

				// cut loose the block from the rest

				BlockListIterator i = FreeImage_FindBlock(bitmap, header->locked_pages[page]);

				// compress the data

				int size = FreeImage_GetFreeImageHeaderSize() + FreeImage_GetDIBSize(page);

				BYTE *compressed_data = new BYTE[size + (0.1 * size) + 12];

				int compressed_size = FreeImage_CompressData(compressed_data, size, (BYTE *)page->data, size);

				// write the data to the cache

				switch ((*i)->m_type) {
					case BLOCK_CONTINUEUS :
					{
						int page = header->m_cachefile->writeFile(compressed_data, compressed_size);

						delete (*i);

						*i = (Block *)new BlockReference(page, compressed_size, size);

						break;
					}

					case BLOCK_REFERENCE :
					{
						BlockReference *reference = (BlockReference *)(*i);

						header->m_cachefile->deleteFile(reference->m_reference);

						delete (*i);

						int page = header->m_cachefile->writeFile(compressed_data, compressed_size);

						*i = (Block *)new BlockReference(page, compressed_size, size);

						break;
					}
				};

				// get rid of the compressed data

				delete [] compressed_data;
			}

			// reset the locked page so that another page can be locked

			FreeImage_Free(page);

			header->locked_pages.erase(page);
		}
	}
}

BOOL DLL_CALLCONV
FreeImage_MovePage(FIMULTIBITMAP *bitmap, int target, int source) {
	if (bitmap) {
		MULTIBITMAPHEADER *header = FreeImage_GetMultiBitmapHeader(bitmap);

		if ((!header->read_only) && (header->locked_pages.empty())) {
			if ((target != source) && ((target >= 0) && (target < FreeImage_GetPageCount(bitmap))) && ((source >= 0) && (source < FreeImage_GetPageCount(bitmap)))) {
				BlockListIterator block_source = FreeImage_FindBlock(bitmap, target);
				BlockListIterator block_target = FreeImage_FindBlock(bitmap, source);

				header->m_blocks.insert(block_target, *block_source);			
				header->m_blocks.erase(block_source);

				header->changed = true;

				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL DLL_CALLCONV
FreeImage_GetLockedPageNumbers(FIMULTIBITMAP *bitmap, int *pages, int *count) {
	if ((bitmap) && (pages) && (count)) {
		MULTIBITMAPHEADER *header = FreeImage_GetMultiBitmapHeader(bitmap);

		if (*count == 0) {
			*count = header->locked_pages.size();
		} else {
			int c = 0;

			for (std::map<FIBITMAP *, int>::iterator i = header->locked_pages.begin(); i != header->locked_pages.end(); ++i) {
				pages[c] = i->second;

				c++;

				if (c == *count)
					break;
			}
		}

		return TRUE;
	}

	return FALSE;
}
