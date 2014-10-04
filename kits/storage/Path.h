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
 * File: Path.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_PATH_H__
#define __ETK_PATH_H__

#include <support/SupportDefs.h>

#ifdef __cplusplus /* Just for C++ */

class BPath
{
	public:
		BPath();
		BPath(const char *dir, const char *leaf = NULL, bool normalize = false);
		BPath(const BPath &path);
		virtual ~BPath();

		status_t	SetTo(const char *dir, const char *leaf = NULL, bool normalize = false);
		status_t	Append(const char *path, bool normalize = false);
		void		Unset();

		const char	*Path() const;
		const char	*Leaf() const;

		status_t	GetParent(BPath *parent) const;

		bool		operator==(const BPath &path) const;
		bool		operator==(const char *path) const;
		bool		operator!=(const BPath &path) const;
		bool		operator!=(const char *path) const;
		BPath&		operator=(const BPath &path);
		BPath&		operator=(const char *path);

	private:
		char *fPath;
};

#endif /* __cplusplus */

#endif /* __ETK_PATH_H__ */

