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
 * File: Rect.cpp
 *
 * --------------------------------------------------------------------------*/

#include "Rect.h"

#ifndef HAVE_ROUND
inline double round(double value)
{
	double iValue = 0;
	double fValue = modf(value, &iValue);

	if (fValue >= 0.5) iValue += 1;
	else if (fValue <= -0.5) iValue -= 1;

	return iValue;
}
#else
#define round(a) round(a)
#endif // HAVE_ROUND

bool
BRect::Contains(BPoint pt) const
{
	return(IsValid() ? pt.x >= left && pt.x <= right && pt.y >= top && pt.y <= bottom : false);
}


bool
BRect::Contains(float x, float y) const
{
	return(IsValid() ? x >= left && x <= right && y >= top && y <= bottom : false);
}


bool
BRect::Contains(BRect r) const
{
	if (r.IsValid() == false) return false;
	return(IsValid() ? r.left >= left && r.right <= right && r.top >= top && r.bottom <= bottom : false);
}


bool
BRect::Contains(float l, float t, float r, float b) const
{
	if (!(l <= r && t <= b)) return false;
	return(IsValid() ? l >= left && r <= right && t >= top && b <= bottom : false);
}


bool
BRect::operator==(BRect r) const
{
	return(r.left == left && r.right == right && r.top == top && r.bottom == bottom);
}


bool
BRect::operator!=(BRect r) const
{
	return(r.left != left || r.right != right || r.top != top || r.bottom != bottom);
}


inline BRect rect_intersection(BRect r1, BRect r2)
{
	if (r1.IsValid() == false || r2.IsValid() == false) return BRect();

	BRect r3;
	r3.left = max_c(r1.left, r2.left);
	r3.top = max_c(r1.top, r2.top);
	r3.right = min_c(r1.right, r2.right);
	r3.bottom = min_c(r1.bottom, r2.bottom);

	return r3;
}


BRect BRect::operator&(BRect r) const // intersection
{
	if (!IsValid() || !r.IsValid()) return BRect();
	return rect_intersection(*this, r);
}


BRect& BRect::operator&=(BRect r) // intersection
{
	if (!IsValid() || !r.IsValid())
		*this = BRect();
	else
		*this = rect_intersection(*this, r);

	return *this;
}


BRect BRect::operator|(BRect r) const // union
{
	if (!IsValid()) return r;
	if (!r.IsValid()) return *this;
	return(BRect(min_c(left, r.left), min_c(top, r.top), max_c(right, r.right), max_c(bottom, r.bottom)));
}


BRect& BRect::operator|=(BRect r) // union
{
	if (!IsValid()) {
		*this = r;
	} else if (r.IsValid()) {
		*this = BRect(min_c(left, r.left), min_c(top, r.top), max_c(right, r.right), max_c(bottom, r.bottom));
	}

	return *this;
}


bool
BRect::Intersects(BRect r) const
{
	return Intersects(r.left, r.top, r.right, r.bottom);
}


bool
BRect::Intersects(float l, float t, float r, float b) const
{
	if (!IsValid() || !(l <= r && t <= b)) return false;
	if (max_c(left, l) > min_c(right, r)) return false;
	if (max_c(top, t) > min_c(bottom, b)) return false;

	return true;
}


void
BRect::SetLeftTop(const BPoint pt)
{
	left = pt.x;
	top = pt.y;
}


void
BRect::SetRightBottom(const BPoint pt)
{
	right = pt.x;
	bottom = pt.y;
}


void
BRect::SetLeftBottom(const BPoint pt)
{
	left = pt.x;
	bottom = pt.y;
}


void
BRect::SetRightTop(const BPoint pt)
{
	right = pt.x;
	top = pt.y;
}


void
BRect::SetLeftTop(float x, float y)
{
	left = x;
	top = y;
}


void
BRect::SetRightBottom(float x, float y)
{
	right = x;
	bottom = y;
}


void
BRect::SetLeftBottom(float x, float y)
{
	left = x;
	bottom = y;
}


void
BRect::SetRightTop(float x, float y)
{
	right = x;
	top = y;
}


void
BRect::InsetBy(BPoint pt)
{
	left += pt.x;
	right -= pt.x;
	top += pt.y;
	bottom -= pt.y;
}


void
BRect::InsetBy(float dx, float dy)
{
	left += dx;
	right -= dx;
	top += dy;
	bottom -= dy;
}


void
BRect::OffsetBy(BPoint pt)
{
	left += pt.x;
	right += pt.x;
	top += pt.y;
	bottom += pt.y;
}


void
BRect::OffsetBy(float dx, float dy)
{
	left += dx;
	right += dx;
	top += dy;
	bottom += dy;
}


void
BRect::OffsetTo(BPoint pt)
{
	float width = right - left;
	float height = bottom - top;

	left = pt.x;
	right = left + width;

	top = pt.y;
	bottom = top + height;
}


void
BRect::OffsetTo(float x, float y)
{
	float width = right - left;
	float height = bottom - top;

	left = x;
	right = left + width;

	top = y;
	bottom = top + height;
}


BRect&
BRect::InsetBySelf(BPoint pt)
{
	InsetBy(pt);
	return *this;
}


BRect&
BRect::InsetBySelf(float dx, float dy)
{
	InsetBy(dx, dy);
	return *this;
}


BRect
BRect::InsetByCopy(BPoint pt) const
{
	return(BRect(left + pt.x, top + pt.y, right - pt.x, bottom - pt.y));
}


BRect
BRect::InsetByCopy(float dx, float dy) const
{
	return(BRect(left + dx, top + dy, right - dx, bottom - dy));
}


BRect&
BRect::OffsetBySelf(BPoint pt)
{
	OffsetBy(pt);
	return *this;
}


BRect&
BRect::OffsetBySelf(float dx, float dy)
{
	OffsetBy(dx, dy);
	return *this;
}


BRect
BRect::OffsetByCopy(BPoint pt) const
{
	return(BRect(left + pt.x, top + pt.y, right + pt.x, bottom + pt.y));
}


BRect
BRect::OffsetByCopy(float dx, float dy) const
{
	return(BRect(left + dx, top + dy, right + dx, bottom + dy));
}


BRect&
BRect::OffsetToSelf(BPoint pt)
{
	OffsetTo(pt);
	return *this;
}


BRect&
BRect::OffsetToSelf(float x, float y)
{
	OffsetTo(x, y);
	return *this;
}


BRect
BRect::OffsetToCopy(BPoint pt) const
{
	float width = Width();
	float height = Height();

	return(BRect(pt.x, pt.y, pt.x + width, pt.y + height));
}


BRect
BRect::OffsetToCopy(float x, float y) const
{
	float width = Width();
	float height = Height();

	return(BRect(x, y, x + width, y + height));
}


void
BRect::Floor()
{
	left = (float)floor((double)left);
	right = (float)floor((double)right);
	top = (float)floor((double)top);
	bottom = (float)floor((double)bottom);
}


BRect&
BRect::FloorSelf()
{
	Floor();
	return *this;
}


BRect
BRect::FloorCopy() const
{
	float _left = (float)floor((double)left);
	float _right = (float)floor((double)right);
	float _top = (float)floor((double)top);
	float _bottom = (float)floor((double)bottom);

	return(BRect(_left, _top, _right, _bottom));
}


void
BRect::Ceil()
{
	left = (float)ceil((double)left);
	right = (float)ceil((double)right);
	top = (float)ceil((double)top);
	bottom = (float)ceil((double)bottom);
}


BRect&
BRect::CeilSelf()
{
	Ceil();
	return *this;
}


BRect
BRect::CeilCopy() const
{
	float _left = (float)ceil((double)left);
	float _right = (float)ceil((double)right);
	float _top = (float)ceil((double)top);
	float _bottom = (float)ceil((double)bottom);

	return(BRect(_left, _top, _right, _bottom));
}


void
BRect::Round()
{
	left = (float)round((double)left);
	right = (float)round((double)right);
	top = (float)round((double)top);
	bottom = (float)round((double)bottom);
}


BRect&
BRect::RoundSelf()
{
	Round();
	return *this;
}


BRect
BRect::RoundCopy() const
{
	float _left = (float)round((double)left);
	float _right = (float)round((double)right);
	float _top = (float)round((double)top);
	float _bottom = (float)round((double)bottom);

	return(BRect(_left, _top, _right, _bottom));
}


void
BRect::PrintToStream() const
{
	ETK_OUTPUT("BRect(%g, %g, %g, %g)", left, top, right, bottom);
}

