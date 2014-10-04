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
 * File: Point.cpp
 *
 * --------------------------------------------------------------------------*/

#include <math.h>

#include "GraphicsDefs.h"
#include "Point.h"
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


BPoint::BPoint()
{
	x = 0;
	y = 0;
}


BPoint::BPoint(float X, float Y)
{
	x = X;
	y = Y;
}


BPoint::BPoint(const BPoint& pt)
{
	x = pt.x;
	y = pt.y;
}


BPoint&
BPoint::operator=(const BPoint& from)
{
	x = from.x;
	y = from.y;

	return *this;
}


void
BPoint::Set(float X, float Y)
{
	x = X;
	y = Y;
}


BPoint
BPoint::operator+(const BPoint &plus) const
{
	return(BPoint(this->x + plus.x, this->y + plus.y));
}


BPoint
BPoint::operator-(const BPoint &minus) const
{
	return(BPoint(this->x - minus.x, this->y - minus.y));
}


BPoint&
BPoint::operator+=(const BPoint &plus)
{
	x += plus.x;
	y += plus.y;

	return *this;
}


BPoint&
BPoint::operator-=(const BPoint &minus)
{
	x -= minus.x;
	y -= minus.y;

	return *this;
}


bool
BPoint::operator!=(const BPoint &pt) const
{
	return(x != pt.x || y != pt.y);
}


bool
BPoint::operator==(const BPoint &pt) const
{
	return(x == pt.x && y == pt.y);
}

void
BPoint::ConstrainTo(BRect rect)
{
	if (!(x >= rect.left && x <= rect.right)) {
		float left_dist = (float)fabs((double)(rect.left - x));
		float right_dist = (float)fabs((double)(rect.right - x));

		float min_dist = min_c(left_dist, right_dist);

		if (min_dist == left_dist)
			x = rect.left;
		else
			x = rect.right;
	}

	if (!(y >= rect.top && y <= rect.bottom)) {
		float top_dist = (float)fabs((double)(rect.top - y));
		float bottom_dist = (float)fabs((double)(rect.bottom - y));

		float min_dist = min_c(top_dist, bottom_dist);

		if (min_dist == top_dist)
			y = rect.top;
		else
			y = rect.bottom;
	}
}


void
BPoint::Floor()
{
	x = (float)floor((double)x);
	y = (float)floor((double)y);
}


BPoint&
BPoint::FloorSelf()
{
	x = (float)floor((double)x);
	y = (float)floor((double)y);
	return *this;
}


BPoint
BPoint::FloorCopy() const
{
	float _x = (float)floor((double)x);
	float _y = (float)floor((double)y);
	return(BPoint(_x, _y));
}


void
BPoint::Ceil()
{
	x = (float)ceil((double)x);
	y = (float)ceil((double)y);
}


BPoint&
BPoint::CeilSelf()
{
	x = (float)ceil((double)x);
	y = (float)ceil((double)y);
	return *this;
}


BPoint
BPoint::CeilCopy() const
{
	float _x = (float)ceil((double)x);
	float _y = (float)ceil((double)y);
	return(BPoint(_x, _y));
}


void
BPoint::Round()
{
	x = (float)round((double)x);
	y = (float)round((double)y);
}


BPoint&
BPoint::RoundSelf()
{
	x = (float)round((double)x);
	y = (float)round((double)y);
	return *this;
}


BPoint
BPoint::RoundCopy() const
{
	float _x = (float)round((double)x);
	float _y = (float)round((double)y);
	return(BPoint(_x, _y));
}


void
BPoint::PrintToStream() const
{
	ETK_OUTPUT("BPoint(%g, %g)", x, y);
}

