// ==========================================================
// FreeImage implementation
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

#ifdef WIN32
#include <windows.h>
#endif

#include <assert.h>
#include <stdlib.h>

#include "FreeImage.h"

//----------------------------------------------------------------------

static const char *s_version = "2.5.2";
static const char *s_copyright = "This program uses FreeImage, a free, open source image library supporting all common bitmap formats. Get your free copy from http://www.6ixsoft.com.";

//----------------------------------------------------------------------

#ifdef WIN32
#ifndef _LIB
BOOL APIENTRY
DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH :
			FreeImage_Initialise(FALSE);
			break;

		case DLL_PROCESS_DETACH :
			FreeImage_DeInitialise();
			break;

		case DLL_THREAD_ATTACH :
		case DLL_THREAD_DETACH :
			break;
    }

    return TRUE;
}
#endif
#endif

//----------------------------------------------------------------------

const char * DLL_CALLCONV
FreeImage_GetVersion() {
	return s_version;
}

const char * DLL_CALLCONV
FreeImage_GetCopyrightMessage() {
	return s_copyright;
}

//----------------------------------------------------------------------

FreeImage_OutputMessageFunction FreeImage_OutputMessageProc = NULL;

void DLL_CALLCONV
FreeImage_SetOutputMessage(FreeImage_OutputMessageFunction omf) {
	FreeImage_OutputMessageProc = omf;
}

void DLL_CALLCONV
FreeImage_OutputMessage(int fif, const char *fmt, ...) {
	if (FreeImage_OutputMessageProc != NULL) {
		// 256 bytes should be more than enough for a short message

		char message[256];
		memset(message, 0, 256);

		// initialize the optional parameter list

		va_list arg;
		va_start(arg, fmt);

		// parse the format string and put the result in 'message'

		for (int i = 0, j = 0, str_length = strlen(fmt); i < str_length; ++i) {
			if (fmt[i] == '%') {
				if (i + 1 < str_length) {
					switch(tolower(fmt[i + 1])) {
						case '%' :
							message[j++] = '%';
							break;

						case 'o' : // octal numbers
						{
							char tmp[16];

							_itoa(va_arg(arg, int), tmp, 8);

							strcat(message, tmp);

							j += strlen(tmp);

							++i;

							break;
						}

						case 'i' : // decimal numbers
						case 'd' :
						{
							char tmp[16];

							_itoa(va_arg(arg, int), tmp, 10);

							strcat(message, tmp);

							j += strlen(tmp);

							++i;

							break;
						}

						case 'x' : // hexadecimal numbers
						{
							char tmp[16];

							_itoa(va_arg(arg, int), tmp, 16);

							strcat(message, tmp);

							j += strlen(tmp);

							++i;

							break;
						}

						case 's' : // strings
						{
							char *tmp = va_arg(arg, char*);

							strcat(message, tmp);

							j += strlen(tmp);

							++i;

							break;
						}
					};
				} else {
					message[j++] = fmt[i];
				}
			} else {
				message[j++] = fmt[i];
			};
		}

		// deinitialize the optional parameter list

		va_end(arg);

		// output the message to the user program

		FreeImage_OutputMessageProc((FREE_IMAGE_FORMAT)fif, message);
	}
}
