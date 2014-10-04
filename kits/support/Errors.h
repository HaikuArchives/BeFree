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
 * File: Errors.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_ERRORS_H__
#define __ETK_ERRORS_H__

#include <support/SupportDefs.h>

/* Error baselines */
#define B_GENERAL_ERROR_BASE	B_MININT32
#define B_OS_ERROR_BASE		B_GENERAL_ERROR_BASE + 0x1000
#define B_APP_ERROR_BASE	B_GENERAL_ERROR_BASE + 0x2000
#define B_STORAGE_ERROR_BASE	B_GENERAL_ERROR_BASE + 0x3000

/* General Errors */
enum {
	B_NO_MEMORY = B_GENERAL_ERROR_BASE,
	B_IO_ERROR,
	B_PERMISSION_DENIED,
	B_BAD_INDEX,
	B_BAD_TYPE,
	B_BAD_VALUE,
	B_MISMATCHED_VALUES,
	B_NAMB_NOT_FOUND,
	B_NAME_IN_USE,
	B_TIMED_OUT,
	B_INTERRUPTED,
	B_WOULD_BLOCK,
	B_CANCELED,
	B_NO_INIT,
	B_BUSY,
	B_NOT_ALLOWED,

	B_ERROR = -1,
	B_OK = 1,
	B_NO_ERROR = 1
};

/* Kernel Kit Errors */
enum {
	B_BAD_SEM_ID = B_OS_ERROR_BASE,
	B_NO_MORE_SEMS,

	B_BAD_THREAD_ID = B_OS_ERROR_BASE + 0x100,
	B_NO_MORE_THREADS,
	B_BAD_THREAD_STATE,
	B_BAD_TEAM_ID,
	B_NO_MORE_TEAMS,

	B_BAD_PORT_ID = B_OS_ERROR_BASE + 0x200,
	B_NO_MORE_PORTS,

	B_BAD_IMAGE_ID = B_OS_ERROR_BASE + 0x300,
	B_BAD_ADDRESS,
	B_NOT_AN_EXECUTABLE,
	B_MISSING_LIBRARY,
	B_MISSING_SYMBOL,

	B_DEBUGGER_ALREADY_INSTALLED = B_OS_ERROR_BASE + 0x400
};

/* Application Kit Errors */
enum {
	B_BAD_REPLY = B_APP_ERROR_BASE,
	B_DUPLICATE_REPLY,
	B_MESSAGE_TO_SELF,
	B_BAD_HANDLER,
	B_ALREADY_RUNNING,
};

/* Storage Kit Errors */
enum {
	B_FILE_ERROR = B_STORAGE_ERROR_BASE,
	B_ENTRY_NOT_FOUND,
	B_LINK_LIMIT,
	B_NAME_TOO_LONG,
};

#endif /* __ETK_ERRORS_H__ */

