// ==========================================================
// Multi-Page functions
//
// Design and implementation by
// - Floris van den Berg (flvdberg@wxs.nl)
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

#ifndef CACHEFILE_H
#define CACHEFILE_H

// ----------------------------------------------------------

#ifndef _LIST_
#include <list>
#endif

#ifndef _MAP_
#include <map>
#endif

#include "FreeImage.h"

// ----------------------------------------------------------

static const int CACHE_SIZE = 32;
static const int BLOCK_SIZE = 64 * 1024;

// ----------------------------------------------------------

#pragma pack(push, 1)
struct Block {
     long next;
     BYTE data[BLOCK_SIZE - 4];
};

struct Cache {
     int nr;
     Block *data;
};
#pragma pack(pop)

// ----------------------------------------------------------

class CacheFile {
	typedef std::list<Cache *> PageCache;
	typedef std::list<Cache *>::iterator PageCacheIt;
	typedef std::map<int, PageCacheIt> PageMap;
	typedef std::map<int, PageCacheIt>::iterator PageMapIt;

public :
	CacheFile(const char *filename, BOOL keep_in_memory);
	~CacheFile();

	bool open();
	void close();
	bool readFile(BYTE *data, int nr, int size);
	int writeFile(BYTE *data, int size);
	void deleteFile(int nr);

private :
	int allocateBlock();
	Block *lockBlock(int nr);
	bool unlockBlock(int nr);
	bool deleteBlock(int nr);

private :
	FILE *m_file;
	char *m_filename;
	std::list<int> m_free_pages;
	PageCache m_page_cache;
	PageMap m_page_map;
	int m_page_count;
	Block *m_current_block;
	BOOL m_keep_in_memory;
};

#endif // CACHEFILE_H
