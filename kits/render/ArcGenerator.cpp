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
 * File: ArcGenerator.cpp
 * Description: BArcGenerator --- Pixel generator for zero-width-arc-drawing
 *
 * --------------------------------------------------------------------------*/

#include <math.h>
#include <string.h>

#include "ArcGenerator.h"


_LOCAL bool get_arc_12(BPoint &radius, BPoint &pStart, BPoint &pEnd, int32 &x, int32 &y, BPoint &radius2, float &deltaNext)
{
	if (radius.x <= 0 || radius.y <= 0 || pStart.x > pEnd.x || pStart.y > 0 || pEnd.y > 0) return false;

	if (radius2.x < 0 || radius2.y < 0) { // first
		radius2.x = radius.x * radius.x;
		radius2.y = radius.y * radius.y;

		BPoint start = pStart.FloorCopy();
		start += BPoint(0.5f, 0.5f);
		BPoint end = pEnd.FloorCopy();
		end += BPoint(0.5f, 0.5f);

		if (start.x >= 0.5f) { // region 1
			if (radius2.y * start.x < -radius2.x * start.y) { // y' < 1
				deltaNext = radius2.y * (start.x + 1.f) * (start.x + 1.f) +
				            radius2.x * (start.y + 0.5f) * (start.y + 0.5f) - radius2.x * radius2.y;
			} else { // y' >= 1
				deltaNext = radius2.y * (start.x + 0.5f) * (start.x + 0.5f) +
				            radius2.x * (start.y + 1.f) * (start.y + 1.f) - radius2.x * radius2.y;
			}
		} else { // region 2
			if (radius2.y * start.x <= radius2.x * start.y) { // y' <= -1
				deltaNext = radius2.y * (start.x + 0.5f) * (start.x + 0.5f) +
				            radius2.x * (start.y - 1.f) * (start.y - 1.f) - radius2.x * radius2.y;
			} else { // y' > -1
				deltaNext = radius2.y * (start.x + 1.f) * (start.x + 1.f) +
				            radius2.x * (start.y - 0.5f) * (start.y - 0.5f) - radius2.x * radius2.y;
			}
		}

		x = (int32)pStart.FloorCopy().x;
		y = (int32)pStart.FloorCopy().y;

		return true;
	}

	float X = (float)x + 0.5f;
	float Y = (float)y + 0.5f;

	if (x >= 0) { // region 1
		if (radius2.y * X < -radius2.x * Y) { // y' < 1
			if (deltaNext >= 0) {
				deltaNext += 2.f * radius2.y * X + 3.f * radius2.y +
				             2.f * radius2.x * Y + 2.f * radius2.x;
				x++;
				y++;
			} else {
				deltaNext += 2.f * radius2.y * X + 3.f * radius2.y;
				x++;
			}

			X = (float)x + 0.5f;
			Y = (float)y + 0.5f;

			if (radius2.y * X >= -radius2.x * Y) { // y' >= 1
				deltaNext = radius2.y * (X + 0.5f) * (X + 0.5f) +
				            radius2.x * (Y + 1.f) * (Y + 1.f) - radius2.x * radius2.y;
			}
		} else { // y' >= 1
			if (deltaNext < 0) {
				deltaNext += 2.f * radius2.x * Y + 3.f * radius2.x +
				             2.f * radius2.y * X + 2.f * radius2.y;
				x++;
				y++;
			} else {
				deltaNext += 2.f * radius2.x * Y + 3.f * radius2.x;
				y++;
			}

			X = (float)x + 0.5f;
			Y = (float)y + 0.5f;
		}
	} else { // region 2
		if (radius2.y * X <= radius2.x * Y) { // y' <= -1
			if (deltaNext >= 0) {
				deltaNext += 2.f * radius2.y * X + 2.f * radius2.y -
				             2.f * radius2.x * Y + 3.f * radius2.x;
				x++;
				y--;
			} else {
				deltaNext += -2.f * radius2.x * Y + 3.f * radius2.x;
				y--;
			}

			X = (float)x + 0.5f;
			Y = (float)y + 0.5f;

			if (radius2.y * X > radius2.x * Y) { // y' > -1
				deltaNext = radius2.y * (X + 1.f) * (X + 1.f) +
				            radius2.x * (Y - 0.5f) * (Y - 0.5f) - radius2.x * radius2.y;
			}
		} else { // y' > -1
			if (deltaNext < 0) {
				deltaNext += 2.f * radius2.y * X + 3.f * radius2.y -
				             2.f * radius2.x * Y + 2.f * radius2.x;
				x++;
				y--;
			} else {
				deltaNext += 2.f * radius2.y * X + 3.f * radius2.y;
				x++;
			}

			X = (float)x + 0.5f;
			Y = (float)y + 0.5f;
		}

		if (x == 0) { // region 1
			deltaNext = radius2.y * (X + 1.f) * (X + 1.f) +
			            radius2.x * (Y + 0.5f) * (Y + 0.5f) - radius2.x * radius2.y;
		}
	}

	if (X > pEnd.x || Y > max_c(pStart.y, pEnd.y)) return false;

	return true;
}


BArcGenerator::BArcGenerator(BPoint center, float xRadius, float yRadius, BPoint start, BPoint end)
		: fStep(0)
{
	fCenter = center;
	fRadius.x = (xRadius < 0 ? -xRadius : xRadius);
	fRadius.y = (yRadius < 0 ? -yRadius : yRadius);

	fStart = start - center;
	fEnd = end - center;
}


bool
BArcGenerator::Start(int32 &x, int32 &y, int32 &step, int32 &pixels, bool &both, bool isLoopX, float pixel_size)
{
	fIsLoopX = isLoopX;

	if (!fIsLoopX) {
		ETK_WARNING("[RENDER]: %s --- LoopY not supported yet!", __PRETTY_FUNCTION__);
		return false;
	}

	if (fStart.y < 0) {
		_fStart.x = ((fEnd.x < fStart.x && fEnd.y < 0) ? fEnd.x : -fRadius.x);
		_fStart.y = (_fStart.x == fEnd.x ? (fEnd.y > 0 ? -fEnd.y : fEnd.y) : 0);
		_fEnd.x = ((fEnd.x > fStart.x && fEnd.y < 0) ? fRadius.x : max_c(fStart.x, fEnd.x));
		_fEnd.y = (_fEnd.x == fRadius.x ? 0 :
		           (fStart.x > fEnd.x ? (fStart.y > 0 ? -fStart.y : fStart.y) : (fEnd.y > 0 ? -fEnd.y : fEnd.y)));
	} else {
		_fStart.x = ((fEnd.x < fStart.x && fEnd.y > 0) ? -fRadius.x : min_c(fStart.x, fEnd.x));
		_fStart.y = (_fStart.x == -fRadius.x ? 0 :
		             (fStart.x < fEnd.x ? (fStart.y > 0 ? -fStart.y : fStart.y) : (fEnd.y > 0 ? -fEnd.y : fEnd.y)));
		_fEnd.x = ((fEnd.x > fStart.x && fEnd.y > 0) ? fEnd.x : fRadius.x);
		_fEnd.y = (_fEnd.x == fEnd.x ? (fEnd.y > 0 ? -fEnd.y : fEnd.y) : 0);
	}

	if (pixel_size != 1) {
		_fRadius.x = fRadius.x / pixel_size;
		_fRadius.y = fRadius.y / pixel_size;

		_fStart.x = _fStart.x / pixel_size;
		_fStart.y = _fStart.y / pixel_size;

		_fEnd.x = _fEnd.x / pixel_size;
		_fEnd.y = _fEnd.y / pixel_size;
	} else {
		_fRadius.x = fRadius.x;
		_fRadius.y = fRadius.y;
	}

	_fCenterX = (int32)floor((double)(fCenter.x / pixel_size));
	_fCenterY = (int32)floor((double)(fCenter.y / pixel_size));
	_fRadiusX = (int32)floor((double)_fRadius.x);
	_fStartX = (int32)floor((double)(fStart.x / pixel_size));
	_fStartY = (int32)floor((double)(fStart.y / pixel_size));
	_fEndX = (int32)floor((double)(fEnd.x / pixel_size));
	_fEndY = (int32)floor((double)(fEnd.y / pixel_size));

	fStep = (int32)(_fEnd.FloorCopy().x - _fStart.FloorCopy().x);
	fRadius2.Set(-1, -1);

	bool havePixels = false;
	int32 oldX = 0, oldY = 0, lastY = 0;

	while (get_arc_12(_fRadius, _fStart, _fEnd, _X, _Y, fRadius2, fDeltaNext)) {
		if (!havePixels) {
			oldX = _X;
			lastY = oldY = _Y;
			havePixels = true;
		} else if (oldX != _X) break;
		else lastY = _Y;
	}

	if (!havePixels) {
		fStep = 0;
		return false;
	}

	bool same_side = true;

	if (oldX == _fRadiusX || oldX == -_fRadiusX) {
		both = !(oldY == lastY);
	} else if (oldX == _fStartX) {
		both = ((_fStartY < 0 && _fEndX > _fStartX) ||
		        (_fStartY > 0 && _fEndX < _fStartX));
	} else if (oldX == _fEndX) {
		both = ((-_fRadiusX < _fEndX && _fEndX < _fStartX && _fEndY > 0) ||
		        (_fStartX < _fEndX && _fEndX < _fRadiusX && _fEndY < 0));
	} else if (oldX < min_c(_fStartX, _fEndX) || oldX > max_c(_fStartX, _fEndX)) {
		both = true;
	} else {
		both = false;
		if ((_fStartY < 0 && _fEndX > _fStartX) || (_fStartY > 0 && _fEndX < _fStartX)) same_side = false;
	}

	x = _fCenterX + oldX;
	y = _fCenterY + (same_side ? lastY : -lastY);
	pixels = oldY - lastY;
	if (!same_side) pixels = -pixels;
	step = fStep;

	return true;
}


bool
BArcGenerator::Next(int32 &next, int32 &pixels, bool &both)
{
	if (fStep <= 0) return false;

	if (!fIsLoopX) {
		ETK_WARNING("[RENDER]: %s --- LoopY not supported yet!", __PRETTY_FUNCTION__);
		return false;
	}

	int32 oldX = _X, oldY = _Y, lastY = _Y;

	while (get_arc_12(_fRadius, _fStart, _fEnd, _X, _Y, fRadius2, fDeltaNext)) {
		if (oldX != _X) break;
		else lastY = _Y;
	}

	fStep--;

	bool same_side = true;

	if (oldX == _fRadiusX || oldX == -_fRadiusX) {
		both = !(oldY == lastY);
	} else if (oldX == _fStartX) {
		both = ((_fStartY < 0 && _fEndX > _fStartX) ||
		        (_fStartY > 0 && _fEndX < _fStartX));
	} else if (oldX == _fEndX) {
		both = ((-_fRadiusX < _fEndX && _fEndX < _fStartX && _fEndY > 0) ||
		        (_fStartX < _fEndX && _fEndX < _fRadiusX && _fEndY < 0));
	} else if (oldX < min_c(_fStartX, _fEndX) || oldX > max_c(_fStartX, _fEndX)) {
		both = true;
	} else {
		both = false;
		if ((_fStartY < 0 && _fEndX > _fStartX) || (_fStartY > 0 && _fEndX < _fStartX)) same_side = false;
	}

	next = _fCenterY + (same_side ? lastY : -lastY);
	pixels = oldY - lastY;
	if (!same_side) pixels = -pixels;

	return true;
}

