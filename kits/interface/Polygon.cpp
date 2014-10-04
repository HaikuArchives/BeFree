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
 * File: Polygon.cpp
 *
 * --------------------------------------------------------------------------*/

#include <math.h>

#include "Polygon.h"

BPolygon::BPolygon(const BPoint *pts, int32 nPts)
		: fCount(0), fPts(NULL), fNeededToUpdateFrame(false)
{
	AddPoints(pts, nPts);
}


BPolygon::BPolygon()
		: fCount(0), fPts(NULL), fNeededToUpdateFrame(false)
{
}


BPolygon::BPolygon(const BPolygon *poly)
		: fCount(0), fPts(NULL), fNeededToUpdateFrame(false)
{
	if (poly != NULL) AddPoints(poly->fPts, poly->fCount);
}


BPolygon::~BPolygon()
{
	if (fPts) free(fPts);
}


BPolygon&
BPolygon::operator=(const BPolygon &poly)
{
	if (fPts) free(fPts);
	fCount = 0;
	fPts = NULL;
	fFrame = BRect();
	fNeededToUpdateFrame = false;
	AddPoints(poly.fPts, poly.fCount);

	return *this;
}


BRect
BPolygon::Frame() const
{
	return fFrame;
}


void
BPolygon::UpdateFrame()
{
	if (fNeededToUpdateFrame == false) return;

	fNeededToUpdateFrame = false;
	fFrame = BRect();
	BPoint *pts = fPts;

	for (int32 i = 0; i < fCount; i++, pts++) {
		if (i > 0) {
			fFrame.left = min_c(fFrame.left, pts->x);
			fFrame.right = max_c(fFrame.right, pts->x);
			fFrame.top = min_c(fFrame.top, pts->y);
			fFrame.bottom = max_c(fFrame.bottom, pts->y);
		} else {
			fFrame.left = fFrame.right = pts->x;
			fFrame.top = fFrame.bottom = pts->y;
		}
	}
}


bool
BPolygon::AddPoints(const BPoint *pts, int32 nPts, bool updateFrame)
{
	if (pts == NULL || nPts <= 0 ||B_MAXINT32 - nPts < fCount) return false;

	if (fCount < 0 || fPts == NULL) fCount = 0;

	BPoint *newPts = (BPoint*)realloc(fPts, sizeof(BPoint) * (size_t)(fCount + nPts));
	if (newPts == NULL) return false;
	fPts = newPts;

	if (updateFrame == false) {
		fNeededToUpdateFrame = true;
		memcpy(fPts + fCount, pts, sizeof(BPoint) * (size_t)nPts);
		fCount += nPts;
	} else {
		UpdateFrame();

		BPoint *destPts = fPts + fCount;
		int32 i = fCount;
		fCount += nPts;

		for (; i < fCount; i++, destPts++, pts++) {
			*destPts = *pts;
			if (i > 0) {
				fFrame.left = min_c(fFrame.left, pts->x);
				fFrame.right = max_c(fFrame.right, pts->x);
				fFrame.top = min_c(fFrame.top, pts->y);
				fFrame.bottom = max_c(fFrame.bottom, pts->y);
			} else {
				fFrame.left = fFrame.right = pts->x;
				fFrame.top = fFrame.bottom = pts->y;
			}
		}
	}

	return true;
}


void
BPolygon::RemovePoints(int32 fromIndex, int32 toIndex, bool updateFrame)
{
	if (fPts == NULL || fromIndex < 0 || fromIndex >= fCount || fromIndex > toIndex) return;

	if (toIndex < fCount - 1) memmove(fPts + fromIndex, fPts + toIndex + 1, sizeof(BPoint) * (size_t)(fCount - toIndex - 1));
	fCount -= (toIndex - fromIndex + 1);

	if (fCount == 0) {
		free(fPts);
		fPts = NULL;
	}
	fNeededToUpdateFrame = (fCount != 0);

	if (updateFrame) UpdateFrame();
}


const BPoint&
BPolygon::operator[](int32 index) const
{
	return(*(fPts + index));
}


int32
BPolygon::CountPoints() const
{
	return fCount;
}


bool
BPolygon::MapTo(BRect srcRect, BRect dstRect)
{
	if (fCount <= 0 || fPts == NULL) return false;
	if (!srcRect.IsValid() || !dstRect.IsValid()) return false;
	if (srcRect.Width() == 0.f || srcRect.Height() == 0.f) return false;

	float xScale = dstRect.Width() / srcRect.Width();
	float yScale = dstRect.Height() / srcRect.Height();

	fFrame = BRect();
	fNeededToUpdateFrame = false;

	BPoint *pts = fPts;
	for (int32 i = 0; i < fCount; i++, pts++) {
		pts->x = dstRect.left + (pts->x - srcRect.left) * xScale;
		pts->y = dstRect.top + (pts->y - srcRect.top) * yScale;

		if (i > 0) {
			fFrame.left = min_c(fFrame.left, pts->x);
			fFrame.right = max_c(fFrame.right, pts->x);
			fFrame.top = min_c(fFrame.top, pts->y);
			fFrame.bottom = max_c(fFrame.bottom, pts->y);
		} else {
			fFrame.left = fFrame.right = pts->x;
			fFrame.top = fFrame.bottom = pts->y;
		}
	}

	return true;
}


const BPoint*
BPolygon::Points() const
{
	return fPts;
}


void
BPolygon::PrintToStream() const
{
	const BPoint *pts = fPts;
	for (int32 i = 0; i < fCount; i++, pts++) {
		pts->PrintToStream();
		if (i < fCount - 1) ETK_OUTPUT(", ");
	}
}

