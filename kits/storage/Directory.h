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
 * File: Directory.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_DIRECTORY_H__
#define __ETK_DIRECTORY_H__

#include <storage/Entry.h>

#ifdef __cplusplus /* Just for C++ */

class BDirectory
{
	public:
		BDirectory();
		BDirectory(const char *path);
		virtual ~BDirectory();

		status_t	InitCheck() const;
		status_t	SetTo(const char *path);
		void		Unset();

		status_t	GetEntry(BEntry *entry) const;
		status_t	GetNextEntry(BEntry *entry, bool traverse = false);
		status_t	Rewind();
		int32		CountEntries();

		void		DoForEach(bool (*func)(const char *path));
		void		DoForEach(bool (*func)(const char *path, void *user_data), void *user_data);

	private:
		friend class BEntry;

		void *fDir;
		char *fName;
};

#endif /* __cplusplus */

#endif /* __ETK_DIRECTORY_H__ */

