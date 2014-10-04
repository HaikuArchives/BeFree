/* --------------------------------------------------------------------------
 *
 * ETK++ --- The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2006, Anthony Lee, All Rights Reserved
 *
 * ETK++ library is a freeware; it may be used and distributed according to
 * the terms of The MIT License.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * File: FindDirectory.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_FIND_DIRECTORY_H__
#define __ETK_FIND_DIRECTORY_H__

#include <storage/Path.h>

typedef enum {
	B_BOOT_DIRECTORY = 0,
	B_APPS_DIRECTORY,
	B_BIN_DIRECTORY,
	B_LIB_DIRECTORY,
	B_ETC_DIRECTORY,
	B_ADDONS_DIRECTORY,
	B_TEMP_DIRECTORY,

	B_USER_DIRECTORY = 1000,
	B_USER_CONFIG_DIRECTORY,
	B_USER_BIN_DIRECTORY,
	B_USER_LIB_DIRECTORY,
	B_USER_ETC_DIRECTORY,
	B_USER_ADDONS_DIRECTORY,
} e_directory_which;

#ifdef __cplusplus /* Just for C++ */

status_t e_find_directory(e_directory_which which, BPath *path);

#endif /* __cplusplus */

#endif /* __ETK_FIND_DIRECTORY_H__ */

