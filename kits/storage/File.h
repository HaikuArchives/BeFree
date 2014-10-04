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
 * File: File.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_FILE_H__
#define __ETK_FILE_H__

#include <storage/StorageDefs.h>
#include <storage/Directory.h>

#ifdef __cplusplus /* Just for C++ */

class BFile
{
	public:
		BFile();
		BFile(const char *path, uint32 open_mode, uint32 access_mode = B_USER_READ |B_USER_WRITE);
		BFile(const BEntry *entry, uint32 open_mode, uint32 access_mode = B_USER_READ |B_USER_WRITE);
		BFile(const BDirectory *dir, const char *leaf, uint32 open_mode, uint32 access_mode = B_USER_READ |B_USER_WRITE);
		BFile(const BFile &from);
		virtual ~BFile();

		status_t	InitCheck() const;
		status_t	SetTo(const char *path, uint32 open_mode, uint32 access_mode = B_USER_READ |B_USER_WRITE);
		status_t	SetTo(const BEntry *entry, uint32 open_mode, uint32 access_mode = B_USER_READ |B_USER_WRITE);
		status_t	SetTo(const BDirectory *dir, const char *leaf, uint32 open_mode, uint32 access_mode = B_USER_READ |B_USER_WRITE);
		void		Unset();

		bool		IsReadable() const;
		bool		IsWritable() const;

		ssize_t		Read(void *buffer, size_t size);
		ssize_t		ReadAt(int64 pos, void *buffer, size_t size);
		ssize_t		Write(const void *buffer, size_t size);
		ssize_t		WriteAt(int64 pos, const void *buffer, size_t size);

		int64		Seek(int64 position, uint32 seek_mode);
		int64		Position() const;
		status_t	SetSize(int64 size);

		BFile&		operator=(const BFile &from);

	private:
		void *fFD;
		uint32 fMode;
};

#endif /* __cplusplus */

#endif /* __ETK_FILE_H__ */

