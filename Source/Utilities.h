// ==========================================================
// Utility functions
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

#ifndef UTILITIES_H
#define UTILITIES_H

#ifndef _INC_MATH
#include <math.h>
#endif

#ifndef _INC_STDLIB
#include <stdlib.h> 
#endif

#ifndef _INC_MEMORY
#include <memory.h>
#endif

#ifndef _INC_STDIO
#include <stdio.h>
#endif

#ifndef _INC_STRING
#include <string.h>
#endif

// ==========================================================
//   Utility functions
// ==========================================================

inline int
round(double x) {
	return (int)floor(x + 0.5);
}

inline unsigned char
HINIBBLE (unsigned char byte) {
	return byte & 240;
}

inline unsigned char
LOWNIBBLE (unsigned char byte) {
	return byte & 15;
}

inline int
CalculateUsedBits(int bits) {
	int bit_count = 0;
	unsigned bit = 1;

	for (unsigned i = 0; i < 32; i++) {
		if ((bits & bit) == bit) {
			bit_count++;
		}

		bit <<= 1;
	}

	return bit_count;
}

inline int
CalculateLine(int width, int bitdepth) {
	return ((width * bitdepth) + 7) / 8;
}

inline int
CalculatePitch(int line) {
	return line + 3 & ~3;
}

inline int
CalculateUsedPaletteEntries(int bit_count) {
	if ((bit_count >= 1) && (bit_count <= 8))
		return 1 << bit_count;

	return 0;
}

inline unsigned char *
CalculateScanLine(unsigned char *bits, unsigned pitch, int scanline) {
	return (bits + (pitch * scanline));
}

inline void
ReplaceExtension(char *result, const char *filename, const char *extension) {
	for (int i = strlen(filename) - 1; i > 0; --i) {
		if (filename[i] == '.') {
			memcpy(result, filename, i);
			result[i] = '.';
			memcpy(result + i + 1, extension, strlen(extension) + 1);
			return;
		}
	}

	memcpy(result, filename, strlen(filename));
	result[strlen(filename)] = '.';
	memcpy(result + strlen(filename) + 1, extension, strlen(extension) + 1);
}

#endif
