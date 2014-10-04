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
 * File: StorageDefs.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_STORAGE_DEFS_H__
#define __ETK_STORAGE_DEFS_H__

#include <support/SupportDefs.h>

typedef int32	e_dev_t;

/* open_mode */
enum {
	B_READ_ONLY		=	0x00,	/* read only */
	B_WRITE_ONLY		=	0x01,	/* write only */
	B_READ_WRITE		=	0x02,	/* read and write */
	B_FAIL_IF_EXISTS	=	0x04,	/* exclusive create */
	B_CREATE_FILE		=	0x08,	/* create the file */
	B_ERASE_FILE		=	0x10,	/* erase the file's data */
	B_OPEN_AT_END		=	0x20,	/* point to the end of the data */
};

/* access_mode */
enum {
	B_USER_READ		=	0x01,
	B_USER_WRITE		=	0x02,
	B_USER_EXEC		=	0x04,
	B_USER_ALL		=	0x07,
	B_GROUP_READ		=	0x08,
	B_GROUP_WRITE		=	0x10,
	B_GROUP_EXEC		=	0x20,
	B_GROUP_ALL		=	0x38,
	B_OTHERS_READ		=	0x40,
	B_OTHERS_WRITE		=	0x80,
	B_OTHERS_EXEC		=	0x0100,
	B_OTHERS_ALL		=	0x01c0,
};

/* node flavor */
enum {
	B_FILB_NODE		=	0x01,
	B_DIRECTORY_NODE	=	0x02,
	B_ANY_NODE		=	0xff,
};

#endif /* __ETK_STORAGE_DEFS_H__ */

