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
 * File: Region.h
 * Description: BRegion --- Combination of rectangles to describe region
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_REGION_H__
#define __ETK_REGION_H__

#include <support/List.h>
#include <interface/Rect.h>

#ifdef __cplusplus /* Just for C++ */

class BRegion
{
	public:
		BRegion();
		BRegion(const BRegion &region);
		BRegion(const BRect &rect);
		virtual ~BRegion();

		BRegion &operator=(const BRegion &from);

		BRegion operator&(BRect r) const;
		BRegion operator|(BRect r) const;

		BRegion& operator&=(BRect r);
		BRegion& operator|=(BRect r);

		BRegion operator&(const BRegion &region) const;
		BRegion operator|(const BRegion &region) const;

		BRegion& operator&=(const BRegion &region);
		BRegion& operator|=(const BRegion &region);

		BRect Frame() const;
		BRect RectAt(int32 index) const;
		int32 CountRects() const;

		void Set(BRect singleBound);
		void MakeEmpty();

		bool Include(BRect r);
		bool Include(const BRegion *region);

		bool Exclude(BRect r);
		bool Exclude(const BRegion *region);

		void OffsetBy(float dx, float dy);
		void OffsetBy(BPoint pt);
		BRegion& OffsetBySelf(float dx, float dy);
		BRegion& OffsetBySelf(BPoint pt);
		BRegion OffsetByCopy(float dx, float dy);
		BRegion OffsetByCopy(BPoint pt);

		void Scale(float scaling);
		BRegion& ScaleSelf(float scaling);
		BRegion ScaleCopy(float scaling);

		bool Intersects(BRect r) const;
		bool Intersects(float l, float t, float r, float b) const;
		bool Intersects(const BRegion *region) const;

		bool Contains(BPoint pt) const;
		bool Contains(float x, float y) const;
		bool Contains(BRect r) const;
		bool Contains(float l, float t, float r, float b) const;
		bool Contains(const BRegion *region) const;

		void PrintToStream() const;

	private:
		BList fRects;
		BRect fFrame;
};

#endif /* __cplusplus */

#endif /* __ETK_REGION_H__ */

