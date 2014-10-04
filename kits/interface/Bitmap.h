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
 * File: Bitmap.h
 * Description: BBitmap --- a rectangular image for drawing
 * Warning: Unfinished.
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_BITMAP_H__
#define __ETK_BITMAP_H__

#include <interface/View.h>

#ifdef __cplusplus /* Just for C++ */

class BGraphicsDrawable;
class BPixmap;

class BBitmap : public BArchivable
{
	public:
		BBitmap(BRect bounds, bool acceptsViews = false);
		BBitmap(const BBitmap *bitmap, bool acceptsViews = false);
		BBitmap(const BPixmap *pixmap, bool acceptsViews = false);
		virtual ~BBitmap();

		status_t	InitCheck() const;
		bool		IsValid() const;

		BRect		Bounds() const;

		virtual	void	AddChild(BView *view);
		virtual	bool	RemoveChild(BView *view);
		int32		CountChildren() const;
		BView		*ChildAt(int32 index) const;
		BView		*FindView(const char *name) const;
		BView		*FindView(BPoint where) const;
		bool		Lock();
		void		Unlock();

	private:
		friend class BView;

		uint32 fRows;
		uint32 fColumns;

		BGraphicsDrawable *fPixmap;
		BWindow *fWindow;

		void InitSelf(BRect, bool);
};

#endif /* __cplusplus */

#endif /* __ETK_BITMAP_H__ */

