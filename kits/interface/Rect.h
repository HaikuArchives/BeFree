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
 * File: Rect.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_RECT_H__
#define __ETK_RECT_H__

#include <math.h>
#include <interface/Point.h>

#ifdef __cplusplus /* Just for C++ */

class BRect
{
	public:
		float left;
		float top;
		float right;
		float bottom;

		BRect();
		BRect(const BRect &r);
		BRect(float l, float t, float r, float b);
		BRect(BPoint leftTop, BPoint rightBottom);

		BRect &operator=(const BRect &from);
		void Set(float l, float t, float r, float b);

		BPoint LeftTop() const;
		BPoint RightBottom() const;
		BPoint LeftBottom() const;
		BPoint RightTop() const;
		BPoint Center() const;

		void SetLeftTop(const BPoint pt);
		void SetRightBottom(const BPoint pt);
		void SetLeftBottom(const BPoint pt);
		void SetRightTop(const BPoint pt);

		void SetLeftTop(float x, float y);
		void SetRightBottom(float x, float y);
		void SetLeftBottom(float x, float y);
		void SetRightTop(float x, float y);

		void InsetBy(BPoint pt);
		void InsetBy(float dx, float dy);
		void OffsetBy(BPoint pt);
		void OffsetBy(float dx, float dy);
		void OffsetTo(BPoint pt);
		void OffsetTo(float x, float y);

		void Floor();
		void Ceil();
		void Round();

		BRect& InsetBySelf(BPoint pt);
		BRect& InsetBySelf(float dx, float dy);
		BRect InsetByCopy(BPoint pt) const;
		BRect InsetByCopy(float dx, float dy) const;
		BRect& OffsetBySelf(BPoint pt);
		BRect& OffsetBySelf(float dx, float dy);
		BRect OffsetByCopy(BPoint pt) const;
		BRect OffsetByCopy(float dx, float dy) const;
		BRect& OffsetToSelf(BPoint pt);
		BRect& OffsetToSelf(float x, float y);
		BRect OffsetToCopy(BPoint pt) const;
		BRect OffsetToCopy(float x, float y) const;

		BRect& FloorSelf();
		BRect FloorCopy() const;
		BRect& CeilSelf();
		BRect CeilCopy() const;
		BRect& RoundSelf();
		BRect RoundCopy() const;

		bool operator==(BRect r) const;
		bool operator!=(BRect r) const;

		BRect operator&(BRect r) const;
		BRect operator|(BRect r) const;

		BRect& operator&=(BRect r);
		BRect& operator|=(BRect r);

		bool IsValid() const;
		float Width() const;
		int32 IntegerWidth() const;
		float Height() const;
		int32 IntegerHeight() const;

		bool Intersects(BRect r) const;
		bool Intersects(float l, float t, float r, float b) const;

		bool Contains(BPoint pt) const;
		bool Contains(float x, float y) const;
		bool Contains(BRect r) const;
		bool Contains(float l, float t, float r, float b) const;

		void PrintToStream() const;
};


inline BPoint BRect::LeftTop() const
{
	return(BPoint(left, top));
}


inline BPoint BRect::RightBottom() const
{
	return(BPoint(right, bottom));
}


inline BPoint BRect::LeftBottom() const
{
	return(BPoint(left, bottom));
}


inline BPoint BRect::RightTop() const
{
	return(BPoint(right, top));
}


inline BPoint BRect::Center() const
{
	return(BPoint(left + (right - left) / 2, top + (bottom - top) / 2));
}


inline BRect::BRect()
{
	top = left = 0;
	bottom = right = -1;
}


inline BRect::BRect(float l, float t, float r, float b)
{
	left = l;
	top = t;
	right = r;
	bottom = b;
}


inline BRect::BRect(const BRect &r)
{
	left = r.left;
	top = r.top;
	right = r.right;
	bottom = r.bottom;
}


inline BRect::BRect(BPoint leftTop, BPoint rightBottom)
{
	left = leftTop.x;
	top = leftTop.y;
	right = rightBottom.x;
	bottom = rightBottom.y;
}


inline BRect& BRect::operator=(const BRect& from)
{
	left = from.left;
	top = from.top;
	right = from.right;
	bottom = from.bottom;
	return *this;
}


inline void BRect::Set(float l, float t, float r, float b)
{
	left = l;
	top = t;
	right = r;
	bottom = b;
}


inline bool BRect::IsValid() const
{
	return(left <= right && top <= bottom);
}


inline int32 BRect::IntegerWidth() const
{
	return((int32)ceil((double)(right - left)));
}


inline float BRect::Width() const
{
	return(right - left);
}


inline int32 BRect::IntegerHeight() const
{
	return((int32)ceil((double)(bottom - top)));
}


inline float BRect::Height() const
{
	return(bottom - top);
}

#endif /* __cplusplus */

#endif /* __ETK_RECT_H__ */

