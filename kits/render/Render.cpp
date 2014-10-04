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
 * File: Render.cpp
 *
 * --------------------------------------------------------------------------*/

#include <string.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
#define M_PI	3.14159265358979323846
#endif // M_PI

#include <support/List.h>

#include "LineGenerator.h"
#include "ArcGenerator.h"
#include "Render.h"

#ifdef _MSC_VER
#define isnan(a)	_isnan(a)
#endif


extern bool get_arc_12(BPoint &radius, BPoint &start, BPoint &end, int32 &x, int32 &y, BPoint &radius2, float &deltaNext);


#define SWAP(CLASS, a, b)	\
	do {			\
		CLASS tmp = a;	\
		a = b;		\
		b = tmp;	\
	} while(false)


class _LOCAL BRenderObject
{
	public:
		BRenderObject();
		virtual ~BRenderObject();

		virtual bool	IsValid() const = 0;
		virtual bool	Get(int32 *y, int32 *minX, int32 *maxX) const = 0;
		virtual bool	Next() = 0;

		static int	cmp(const void *objectA, const void *objectB);
};


BRenderObject::BRenderObject()
{
}


BRenderObject::~BRenderObject()
{
}


int
BRenderObject::cmp(const void *objectA, const void *objectB)
{
	const BRenderObject *A = *((const BRenderObject**)objectA);
	const BRenderObject *B = *((const BRenderObject**)objectB);

	if (A->IsValid() == B->IsValid()) {
		if (A->IsValid() == false) return 0;

		int32 yA, yB;
		A->Get(&yA, NULL, NULL);
		B->Get(&yB, NULL, NULL);

		if (yA == yB) return 0;
		return(yA < yB ? -1 : 1);
	}

	return(A->IsValid() ? -1 : 1);
}


class _LOCAL BRenderLine : public BRenderObject
{
	public:
		BRenderLine(BPoint start, BPoint end);

		virtual bool	IsValid() const;
		virtual bool	Get(int32 *y, int32 *minX, int32 *maxX) const;
		virtual bool	Next();

	private:
		BLineGenerator fLine;
		int32 fMinX;
		int32 fMaxX;
		int32 fX;
		int32 fY;
		bool fValid;
};


BRenderLine::BRenderLine(BPoint start, BPoint end)
		: BRenderObject(), fLine(start, end), fValid(false)
{
	if (start.y > end.y) return;

	int32 step, pixels;

	if (fLine.Start(fX, fY, step, pixels, false, 1)) {
		fValid = true;
		fMinX = min_c(fX, fX + pixels);
		fMaxX = max_c(fX, fX + pixels);
	}
}


bool
BRenderLine::IsValid() const
{
	return fValid;
}


bool
BRenderLine::Get(int32 *y, int32 *minX, int32 *maxX) const
{
	if (fValid == false) return false;

	if (y) *y = fY;
	if (minX) *minX = fMinX;
	if (maxX) *maxX = fMaxX;

	return true;
}


bool
BRenderLine::Next()
{
	if (fValid == false) return false;

	int32 pixels;

	fY++;
	if (fLine.Next(fX, pixels) == false) {
		fValid = false;
		return false;
	}

	fMinX = min_c(fX, fX + pixels);
	fMaxX = max_c(fX, fX + pixels);

	return true;
}


class _LOCAL BRenderLine2 : public BRenderObject
{
	public:
		BRenderLine2(BPoint pt0, BPoint pt1, BPoint pt2);

		virtual bool	IsValid() const;
		virtual bool	Get(int32 *y, int32 *minX, int32 *maxX) const;
		virtual bool	Next();

	private:
		BLineGenerator fLine1;
		BLineGenerator fLine2;

		int32 fMinX;
		int32 fMaxX;

		int32 fX1;
		int32 fX2;
		int32 fY1;
		int32 fY2;

		bool fValid;
};


BRenderLine2::BRenderLine2(BPoint pt0, BPoint pt1, BPoint pt2)
		: BRenderObject(), fLine1(pt0, pt1), fLine2(pt1, pt2), fValid(false)
{
	if (pt0.y > pt1.y || pt1.y > pt2.y) return;

	int32 tmp, pixels1, pixels2;

	if (fLine1.Start(fX1, fY1, tmp, pixels1, false, 1) && fLine2.Start(fX2, fY2, tmp, pixels2, false, 1)) {
		fValid = true;

		fMinX = min_c(fX1, fX1 + pixels1);
		fMaxX = max_c(fX1, fX1 + pixels1);

		if (fY2 == fY1) {
			fMinX = min_c(fMinX, min_c(fX2, fX2 + pixels2));
			fMaxX = max_c(fMaxX, max_c(fX2, fX2 + pixels2));
		}
	}
}


bool
BRenderLine2::IsValid() const
{
	return fValid;
}


bool
BRenderLine2::Get(int32 *y, int32 *minX, int32 *maxX) const
{
	if (fValid == false) return false;

	if (y) *y = fY1;
	if (minX) *minX = fMinX;
	if (maxX) *maxX = fMaxX;

	return true;
}


bool
BRenderLine2::Next()
{
	if (fValid == false) return false;

	int32 pixels1, pixels2;
	bool line1_has_next = false;

	fY1++;
	if (fY1 <= fY2) if ((line1_has_next = fLine1.Next(fX1, pixels1)) == false) fValid = (fY1 == fY2);
	if (fY1 > fY2) if (fLine2.Next(fX2, pixels2) == false) fValid = false;
	if (fValid == false) return false;

	if (line1_has_next) {
		fMinX = min_c(fX1, fX1 + pixels1);
		fMaxX = max_c(fX1, fX1 + pixels1);

		if (fY1 > fY2) {
			fMinX = min_c(fMinX, min_c(fX2, fX2 + pixels2));
			fMaxX = max_c(fMaxX, max_c(fX2, fX2 + pixels2));
		}
	} else {
		fMinX = min_c(fX2, fX2 + pixels2);
		fMaxX = max_c(fX2, fX2 + pixels2);
	}

	return true;
}


class _LOCAL BRenderTriangle : public BRenderLine2
{
	public:
		BRenderTriangle(BPoint pt0, BPoint pt1, BPoint pt2, bool stroke_edge);

		virtual bool	IsValid() const;
		virtual bool	Get(int32 *y, int32 *minX, int32 *maxX) const;
		virtual bool	Next();

	private:
		BLineGenerator fLine1;
		uint8 fFlags;

		bool fStrokeEdge;

		int32 fY;
		int32 fY0;
		int32 fY1;

		int32 fX1;
		int32 fPixels1;
};


BRenderTriangle::BRenderTriangle(BPoint pt0, BPoint pt1, BPoint pt2, bool stroke_edge)
		: BRenderLine2(pt0, pt1, pt2), fLine1(pt0, pt2), fFlags(0x00), fStrokeEdge(stroke_edge)
{
	int32 tmp;

	if (BRenderLine2::Get(&fY0, NULL, NULL) == false || fLine1.Start(fX1, fY1, tmp, fPixels1, false, 1) == false) return;
	fY = min_c(fY0, fY1);

	fFlags = 0x03;
}


bool
BRenderTriangle::IsValid() const
{
	return(fFlags != 0x00);
}


bool
BRenderTriangle::Get(int32 *y, int32 *_minX, int32 *_maxX) const
{
	if (fFlags == 0x00) return false;

	if (y) *y = fY;

	if (_minX || _maxX) {
		int32 minX = 0, maxX = -1;

		if (fFlags != 0x03) {
			if (fStrokeEdge) {
				if (fFlags == 0x01) {
					BRenderLine2::Get(NULL, &minX, &maxX);
				} else {
					minX = min_c(fX1, fX1 + fPixels1);
					maxX = max_c(fX1, fX1 + fPixels1);
				}
			}
		} else {
			int32 minX0, maxX0;
			BRenderLine2::Get(NULL, &minX0, &maxX0);

			if (fY < max_c(fY0, fY1)) {
				if (fStrokeEdge) {
					minX = fY0 < fY1 ? minX0 : min_c(fX1, fX1 + fPixels1);
					maxX = fY0 < fY1 ? maxX0 : max_c(fX1, fX1 + fPixels1);
				}
			} else {
				minX = min_c(minX0, min_c(fX1, fX1 + fPixels1));
				maxX = max_c(maxX0, max_c(fX1, fX1 + fPixels1));

				if (fStrokeEdge == false) {
#define SIMPLE_EXCLUDE(Start, End, f, t)		\
	if(Start <= End)			\
	{					\
		if(Start >= f) Start = t + 1;	\
		if(End <= t) End = f - 1;	\
	} (void)0
					SIMPLE_EXCLUDE(minX, maxX, minX0, maxX0);
					SIMPLE_EXCLUDE(minX, maxX, min_c(fX1, fX1 + fPixels1), max_c(fX1, fX1 + fPixels1));
#undef SIMPLE_EXCLUDE
				}
			}
		}

		if (_minX) *_minX = minX;
		if (_maxX) *_maxX = maxX;
	}

	return true;
}


bool
BRenderTriangle::Next()
{
	if (fFlags == 0x00) return false;

	if (fY >= fY0) {
		if (BRenderLine2::Next() == false) fFlags &= ~0x01;
	}

	if (fY >= fY1) {
		if (fLine1.Next(fX1, fPixels1) == false) fFlags &= ~0x02;
	}

	fY++;

	return(fFlags != 0x00);
}


BRender::BRender()
		: fDrawingMode(B_OP_COPY), fPenSize(0), fSquarePointStyle(false)
{
	fHighColor.set_to(0, 0, 0);
	fLowColor.set_to(255, 255, 255);
}


BRender::~BRender()
{
}


bool
BRender::IsValid() const
{
	return(InitCheck() == B_OK);
}


void
BRender::SetDrawingMode(e_drawing_mode drawing_mode)
{
	fDrawingMode = drawing_mode;
}


e_drawing_mode
BRender::DrawingMode() const
{
	return fDrawingMode;
}


void
BRender::SetHighColor(rgb_color highColor)
{
	fHighColor.set_to(highColor);
}


void
BRender::SetHighColor(uint8 r, uint8 g, uint8 b, uint8 a)
{
	rgb_color color;
	color.set_to(r, g, b, a);
	SetHighColor(color);
}


rgb_color
BRender::HighColor() const
{
	return fHighColor;
}


void
BRender::SetLowColor(rgb_color lowColor)
{
	fLowColor.set_to(lowColor);
}


void
BRender::SetLowColor(uint8 r, uint8 g, uint8 b, uint8 a)
{
	rgb_color color;
	color.set_to(r, g, b, a);
	SetLowColor(color);
}


rgb_color
BRender::LowColor() const
{
	return fLowColor;
}


void
BRender::SetPenSize(float pen_size)
{
	fPenSize = pen_size;
}


float
BRender::PenSize() const
{
	return fPenSize;
}


void
BRender::SetSquarePointStyle(bool state)
{
	fSquarePointStyle = state;
}


bool
BRender::IsSquarePointStyle() const
{
	return fSquarePointStyle;
}


void
BRender::PutRect(int32 x, int32 y, uint32 width, uint32 height, rgb_color color)
{
	if (width == 0 || height == 0) return;

	for (uint32 i = 0; i < height; i++, y++) {
		for (uint32 j = 0; j < width; j++) PutPixel(x + j, y, color);
	}
}


inline bool _is_pixel_high_color(const pattern &pattern, int32 x, int32 y)
{
	if (pattern == B_SOLID_HIGH) return true;
	else if (pattern == B_SOLID_LOW) return false;

	x %= 8;
	y %= 8;

	uint8 pat = pattern.data[y];
	if (pat & (1 << (7 - x))) return true;

	return false;
}


inline bool _is_line_mixed_color(const pattern &pattern, int32 y, bool &isHighColor)
{
	if (pattern == B_SOLID_HIGH) {
		isHighColor = true;
		return false;
	} else if (pattern == B_SOLID_LOW) {
		isHighColor = false;
		return false;
	}

	y %= 8;

	if (pattern.data[y] == 0x00 || pattern.data[y] == 0xff) {
		isHighColor = (pattern.data[y] == 0xff);
		return false;
	}

	return true;
}


void
BRender::drawPixel(int32 x, int32 y, pattern pattern)
{
	if (!IsValid()) return;

	int32 originX = 0, originY = 0;
	uint32 w = 0, h = 0;
	GetFrame(&originX, &originY, &w, &h);

	if (x < originX || y < originY || x > originX + (int32)w - 1 ||  y > originY + (int32)h - 1) return;

	rgb_color src = {0, 0, 0, 255};
	if (fDrawingMode != B_OP_COPY) GetPixel(x, y, src);
	uint32 srcAlpha = src.alpha;

	rgb_color color;
	color.set_to(_is_pixel_high_color(pattern, x, y) ? fHighColor : fLowColor);

	switch (fDrawingMode) {
		case B_OP_OVER:
		case B_OP_ERASE:
			if (color == B_TRANSPARENT_COLOR || color == fLowColor) return;
			src.set_to((fDrawingMode == B_OP_ERASE) ? fLowColor : color);
			src.alpha = srcAlpha;
			break;

		case B_OP_XOR:
			src.set_to(color.red ^ src.red, color.green ^ src.green, color.blue ^ src.blue, srcAlpha);
			break;

		case B_OP_INVERT:
			if (color == B_TRANSPARENT_COLOR || color == fLowColor) return;
			src.set_to(255 - src.red, 255 - src.green, 255 - src.blue, srcAlpha);
			break;

		case B_OP_SELECT: {
			if (color == B_TRANSPARENT_COLOR || color == fLowColor) return;
			if (src.blue == fHighColor.blue && src.green == fHighColor.green && src.red == fHighColor.red) {
				src.set_to(fLowColor);
				src.alpha = srcAlpha;
			} else if (src.blue == fLowColor.blue && src.green == fLowColor.green && src.red == fLowColor.red) {
				src.set_to(fHighColor);
				src.alpha = srcAlpha;
			}
		}
		break;

		case B_OP_ADD:
			if (color == B_TRANSPARENT_COLOR || color == fLowColor) return;
			src.red += color.red;
			src.green += color.green;
			src.blue += color.blue;
			break;

		case B_OP_SUBTRACT:
			if (color == B_TRANSPARENT_COLOR || color == fLowColor) return;
			src.red -= color.red;
			src.green -= color.green;
			src.blue -= color.blue;
			break;

		case B_OP_BLEND:
			if (color == B_TRANSPARENT_COLOR || color == fLowColor) return;
			src.red = (uint8)(((uint16)color.red + (uint16)src.red) / 2U);
			src.green = (uint8)(((uint16)color.green + (uint16)src.green) / 2U);
			src.blue = (uint8)(((uint16)color.blue + (uint16)src.blue) / 2U);
			break;

		case B_OP_MIN:
			if (color == B_TRANSPARENT_COLOR || color == fLowColor) return;
			if (color.red < src.red) src.red = color.red;
			if (color.green < src.green) src.green = color.green;
			if (color.blue < src.blue) src.blue = color.blue;
			break;

		case B_OP_MAX:
			if (color == B_TRANSPARENT_COLOR || color == fLowColor) return;
			if (color.red > src.red) src.red = color.red;
			if (color.green > src.green) src.green = color.green;
			if (color.blue > src.blue) src.blue = color.blue;
			break;

		case B_OP_ALPHA: {
			if (color.alpha == 0) return;
			src.mix(color);
		}
		break;

		default:
			src.set_to(color);
			src.alpha = srcAlpha;
			break;
	}

	PutPixel(x, y, src);
}


void
BRender::FillRect(int32 x, int32 y, uint32 width, uint32 height, pattern pattern)
{
	if (width == height && width == 1) {
		drawPixel(x, y, pattern);
		return;
	}
	if (!IsValid() || width == 0 || height == 0) return;

	int32 originX = 0, originY = 0;
	uint32 w = 0, h = 0;
	GetFrame(&originX, &originY, &w, &h);

	BRect r((float)x, (float)y, (float)x + (float)width - 1, (float)y + (float)height - 1);
	r &= BRect((float)originX, (float)originY, (float)(originX + (int32)w - 1), (float)(originY + (int32)h - 1));
	if (r.IsValid() == false) return;

	x = (int32)r.left;
	y = (int32)r.top;
	width = (uint32)r.Width() + 1;
	height = (uint32)r.Height() + 1;

	if ((pattern == B_SOLID_HIGH || pattern == B_SOLID_LOW) && fDrawingMode == B_OP_COPY) {
		rgb_color color;
		color.set_to(pattern == B_SOLID_HIGH ? fHighColor : fLowColor);
		PutRect(x, y, width, height, color);
	} else for (uint32 i = 0; i < height; i++, y++) {
			if (fDrawingMode == B_OP_COPY) {
				bool isHighColor;
				if (!_is_line_mixed_color(pattern, y, isHighColor)) {
					rgb_color color;
					color.set_to(isHighColor ? fHighColor : fLowColor);

					PutRect(x, y, width, 1, color);
					continue;
				}
			}

			for (uint32 j = 0; j < width; j++) drawPixel(x + j, y, pattern);
		}
}


void
BRender::FillRect(BRect rect, pattern pattern)
{
	if (!IsValid()) return;

	rect.Floor();
	if (!rect.IsValid()) return;

	FillRect((int32)rect.left, (int32)rect.top, (uint32)rect.Width() + 1, (uint32)rect.Height() + 1, pattern);
}


void
BRender::StrokePoint(int32 x, int32 y, pattern pattern)
{
	if (!IsValid()) return;

	if (fPenSize <= 1) {
		int32 originX = 0, originY = 0;
		uint32 w = 0, h = 0;
		GetFrame(&originX, &originY, &w, &h);

		if (x < originX || y < originY || x > originX + (int32)w - 1 ||  y > originY + (int32)h - 1) return;

		drawPixel(x, y, pattern);
	} else {
		BRect rect;
		rect.left = (float)x + 0.5f - fPenSize / 2.f;
		rect.top = (float)y + 0.5f - fPenSize / 2.f;
		rect.right = rect.left + fPenSize;
		rect.bottom = rect.top + fPenSize;

		if (fSquarePointStyle) FillRect(rect, pattern);
		else FillEllipse(rect, true, pattern);
	}
}


void
BRender::StrokePoint(BPoint pt, pattern pattern)
{
	if (!IsValid()) return;

	if (fPenSize <= 1) {
		int32 x, y, originX = 0, originY = 0;
		uint32 w = 0, h = 0;

		GetFrame(&originX, &originY, &w, &h);

		pt.Floor();
		x = (int32)pt.x;
		y = (int32)pt.y;

		if (x < originX || y < originY || x > originX + (int32)w - 1 ||  y > originY + (int32)h - 1) return;

		drawPixel(x, y, pattern);
	} else {
		BRect rect;
		rect.left = pt.x - fPenSize / 2.f;
		rect.top = pt.y - fPenSize / 2.f;
		rect.right = rect.left + fPenSize;
		rect.bottom = rect.top + fPenSize;

		if (fSquarePointStyle) FillRect(rect, pattern);
		else FillEllipse(rect, true, pattern);
	}
}


void
BRender::StrokeLine(int32 x0, int32 y0, int32 x1, int32 y1, pattern pattern)
{
	StrokeLine(BPoint((float)x0 + 0.5f, (float)y0 + 0.5f), BPoint((float)x1 + 0.5f, (float)y1 + 0.5f), pattern);
}


void
BRender::StrokeLine(BPoint pt0, BPoint pt1, pattern pattern)
{
	if (!IsValid()) return;

	// TODO: clipping

	if (fPenSize > 1) {
		// TODO
		ETK_WARNING("[RENDER]: %s --- not support large pen yet.", __PRETTY_FUNCTION__);
		return;
	} else {
		if (pt0.y > pt1.y) SWAP(BPoint, pt0, pt1); // for compacting with FillTriangle()

		BLineGenerator line(pt0, pt1);
		int32 x, y, step, pixels;

		if (line.Start(x, y, step, pixels, false, 1) == false) return;
		do {
			if (pixels >= 0) FillRect(x, y, 1 + pixels, 1, pattern);
			else FillRect(x + pixels, y, 1 - pixels, 1, pattern);
		} while (y++, line.Next(x, pixels));
	}
}


void
BRender::StrokeTriangle(int32 x0, int32 y0, int32 x1, int32 y1, int32 x2, int32 y2, pattern pattern)
{
	BPoint pts[3];
	pts[0].Set((float)x0 + 0.5f, (float)y0 + 0.5f);
	pts[1].Set((float)x1 + 0.5f, (float)y1 + 0.5f);
	pts[2].Set((float)x2 + 0.5f, (float)y2 + 0.5f);
	StrokePolygon(pts, 3, true, pattern);
}


void
BRender::StrokeTriangle(BPoint pt1, BPoint pt2, BPoint pt3, pattern pattern)
{
	BPoint pts[3];
	pts[0] = pt1;
	pts[1] = pt2;
	pts[2] = pt3;
	StrokePolygon(pts, 3, true, pattern);
}


void
BRender::FillTriangle(int32 x0, int32 y0, int32 x1, int32 y1, int32 x2, int32 y2, bool stroke_edge, pattern pattern)
{
	FillTriangle(BPoint((float)x0 + 0.5f, (float)y0 + 0.5f),
	             BPoint((float)x1 + 0.5f, (float)y1 + 0.5f),
	             BPoint((float)x2 + 0.5f, (float)y2 + 0.5f),
	             stroke_edge, pattern);
}


void
BRender::FillTriangle(BPoint pt0, BPoint pt1, BPoint pt2, bool stroke_edge, pattern pattern)
{
	if (!IsValid()) return;

	// TODO: clipping

	if (pt0.y > pt1.y) SWAP(BPoint, pt0, pt1);
	if (pt0.y > pt2.y) SWAP(BPoint, pt0, pt2);
	if (pt1.y > pt2.y) SWAP(BPoint, pt1, pt2);

	BRenderTriangle triangle(pt0, pt1, pt2, stroke_edge);

	do {
		int32 y, minX, maxX;
		if (triangle.Get(&y, &minX, &maxX) == false) break;
		if (minX <= maxX) FillRect(minX, y, maxX - minX + 1, 1, pattern);
	} while (triangle.Next());
}


void
BRender::StrokeEllipse(int32 x, int32 y, uint32 width, uint32 height, pattern pattern)
{
	if (!IsValid() || width == 0 || height == 0) return;

	BRect rect;
	rect.SetLeftTop((float)x + 0.5f, (float)y + 0.5f);
	rect.SetRightBottom((float)x + (float)width - 0.5f, (float)y + (float)height - 0.5f);

	StrokeEllipse(rect, pattern);
}


void
BRender::FillEllipse(int32 x, int32 y, uint32 width, uint32 height, bool stroke_edge, pattern pattern)
{
	if (!IsValid() || width == 0 || height == 0) return;

	BRect rect;
	rect.SetLeftTop((float)x + 0.5f, (float)y + 0.5f);
	rect.SetRightBottom((float)x + (float)width - 0.5f, (float)y + (float)height - 0.5f);

	FillEllipse(rect, stroke_edge, pattern);
}


void
BRender::StrokeEllipse(int32 xCenter, int32 yCenter, int32 xRadius, int32 yRadius, pattern pattern)
{
	if (!IsValid()) return;

	if (xRadius < 0) xRadius = -xRadius;
	if (yRadius < 0) yRadius = -yRadius;

	BRect rect;
	rect.left = (float)(xCenter - xRadius);
	rect.right = (float)(xCenter + xRadius);
	rect.top = (float)(yCenter - yRadius);
	rect.bottom = (float)(yCenter + yRadius);

	rect.OffsetBy(0.5, 0.5);

	StrokeEllipse(rect, pattern);
}


void
BRender::FillEllipse(int32 xCenter, int32 yCenter, int32 xRadius, int32 yRadius, bool stroke_edge, pattern pattern)
{
	if (!IsValid()) return;

	if (xRadius < 0) xRadius = -xRadius;
	if (yRadius < 0) yRadius = -yRadius;

	BRect rect;
	rect.left = (float)(xCenter - xRadius);
	rect.right = (float)(xCenter + xRadius);
	rect.top = (float)(yCenter - yRadius);
	rect.bottom = (float)(yCenter + yRadius);

	rect.OffsetBy(0.5, 0.5);

	FillEllipse(rect, stroke_edge, pattern);
}


void
BRender::StrokeEllipse(BRect rect, pattern pattern)
{
	if (!IsValid() || !rect.IsValid()) return;

	// TODO: clipping

	if (fPenSize > 1) {
		// TODO
		ETK_WARNING("[RENDER]: %s --- not support large pen yet.", __PRETTY_FUNCTION__);
		return;
	} else {
		BPoint radius;
		radius.x = rect.Width() / 2.f;
		radius.y = rect.Height() / 2.f;

		BPoint start, end, radius2;
		start.Set(-radius.x, 0);
		end.Set(radius.x, 0);
		radius2.Set(-1, -1);

		int32 x, y;
		int32 xCenter, yCenter;
		float deltaNext;

		BPoint center = rect.Center().FloorSelf();
		xCenter = (int32)center.x;
		yCenter = (int32)center.y;

		while (get_arc_12(radius, start, end, x, y, radius2, deltaNext)) {
			if (x > 0) break;

			drawPixel(xCenter + x, yCenter + y, pattern);
			if (x != 0) drawPixel(xCenter - x, yCenter + y, pattern);

			if (y == 0) continue;
			drawPixel(xCenter + x, yCenter - y, pattern);
			if (x != 0) drawPixel(xCenter - x, yCenter - y, pattern);
		}
	}
}


void
BRender::FillEllipse(BRect rect, bool stroke_edge, pattern pattern)
{
	if (!IsValid() || !rect.IsValid()) return;

	// TODO: clipping

	BPoint radius;
	radius.x = rect.Width() / 2.f;
	radius.y = rect.Height() / 2.f;

	BPoint start, end, radius2;
	start.Set(-radius.x, 0);
	end.Set(radius.x, 0);
	radius2.Set(-1, -1);

	int32 x, y, old_x = 0, old_y = 0, last_y = 0;
	int32 xCenter, yCenter;
	float deltaNext;
	bool first = true;

	BPoint center = rect.Center().FloorSelf();
	xCenter = (int32)center.x;
	yCenter = (int32)center.y;

	while (true) {
		bool status = get_arc_12(radius, start, end, x, y, radius2, deltaNext);

		if (first) {
			if (!status) break;
			old_x = x;
			old_y = last_y = y;
			first = false;
		}

		if (status && old_x == x) {
			last_y = y;
			continue;
		}

		if (stroke_edge)
			last_y = min_c(old_y, last_y);
		else
			last_y = max_c(old_y, last_y) + 1;

		if (last_y == 0) {
			drawPixel(xCenter + old_x, yCenter, pattern);
			if (old_x != 0) drawPixel(xCenter - old_x, yCenter, pattern);
		} else if (last_y < 0) {
			FillRect(xCenter + old_x, yCenter + last_y, 1, -(last_y * 2 - 1), pattern);
			if (old_x != 0) FillRect(xCenter - old_x, yCenter + last_y, 1, -(last_y * 2 - 1), pattern);
		}

		if (!status || x > 0) break;
		old_x = x;
		old_y = last_y = y;
	}
}


void
BRender::StrokeArc(int32 x, int32 y, uint32 width, uint32 height,
                   int32 startAngle, int32 endAngle, pattern pattern)
{
	if (!IsValid() || width == 0 || height == 0) return;

	BRect rect;
	rect.SetLeftTop((float)x + 0.5f, (float)y + 0.5f);
	rect.SetRightBottom((float)x + (float)width - 0.5f, (float)y + (float)height - 0.5f);

	StrokeArc(rect, (float)startAngle / 64.f, (float)(endAngle - startAngle) / 64.f, pattern);
}


void
BRender::StrokeArc(int32 xCenter, int32 yCenter, int32 xRadius, int32 yRadius,
                   int32 startAngle, int32 endAngle, pattern pattern)
{
	if (!IsValid()) return;

	BPoint ctPt((float)xCenter + 0.5f, (float)yCenter + 0.5f);

	StrokeArc(ctPt, (float)xRadius, (float)yRadius, (float)startAngle / 64.f, (float)(endAngle - startAngle) / 64.f, pattern);
}


void
BRender::StrokeArc(BPoint ctPt, float xRadius, float yRadius, float startAngle, float arcAngle, pattern pattern)
{
	if (!IsValid()) return;

	if (xRadius < 0) xRadius = -xRadius;
	if (yRadius < 0) yRadius = -yRadius;

	BRect rect;
	rect.SetLeftTop(ctPt - BPoint(xRadius, yRadius));
	rect.SetRightBottom(ctPt + BPoint(xRadius, yRadius));

	StrokeArc(rect, startAngle, arcAngle, pattern);
}


void
BRender::StrokeArc(BRect rect, float startAngle, float arcAngle, pattern pattern)
{
	if (!IsValid() || !rect.IsValid()) return;

	startAngle = (float)fmod((double)startAngle, (double)360);
	if (arcAngle >= 360.f || arcAngle <= -360.f) arcAngle = 360;
	if (startAngle < 0) startAngle += 360.f;

	BPoint radius;
	radius.x = rect.Width() / 2.f;
	radius.y = rect.Height() / 2.f;

	BPoint start, end;
	start.x = rect.Center().x + radius.x * (float)cos(M_PI * (double)startAngle / 180.f);
	start.y = rect.Center().y - radius.y * (float)sin(M_PI * (double)startAngle / 180.f);
	end.x = rect.Center().x + radius.x * (float)cos(M_PI * (double)(startAngle + arcAngle) / 180.f);
	end.y = rect.Center().y - radius.y * (float)sin(M_PI * (double)(startAngle + arcAngle) / 180.f);

	StrokeArc(rect, start, end, pattern);
}


void
BRender::StrokeArc(BRect rect, BPoint start, BPoint end, pattern pattern)
{
	if (!IsValid() || !rect.IsValid()) return;

	// TODO: clipping

	if (fPenSize > 1) {
		// TODO
		ETK_WARNING("[RENDER]: %s --- not support large pen yet.", __PRETTY_FUNCTION__);
		return;
	} else {
		BArcGenerator arc(rect.Center(), rect.Width() / 2.f, rect.Height() / 2.f, start, end);

		int32 x, y, step, pixels, centerY = (int32)floor((double)rect.Center().y);
		bool firstTime = true;
		bool both = false;

		while (firstTime ? arc.Start(x, y, step, pixels, both, true, 1) : arc.Next(y, pixels, both)) {
			if (!firstTime)
				x += (step > 0 ? 1 : -1);
			else
				firstTime = false;

			if (pixels == 0) {
				drawPixel(x, y, pattern);
				if (both && y != centerY - (y - centerY)) drawPixel(x, centerY - (y - centerY), pattern);
			} else if (pixels > 0) {
				FillRect(x, y, 1, 1 + pixels, pattern);
				if (both) {
					int32 y1 = centerY - (y - centerY) - pixels;
					if (y1 == y + pixels) {
						y1--;
						pixels--;
					}
					if (pixels > 0) FillRect(x, y1, 1, 1 + pixels, pattern);
				}
			} else { /* pixels < 0 */
				FillRect(x, y + pixels, 1, 1 - pixels, pattern);
				if (both) {
					int32 y1 = centerY - (y - centerY);
					if (y1 == y) {
						y1++;
						pixels++;
					}
					if (pixels < 0) FillRect(x, y1, 1, 1 - pixels, pattern);
				}
			}
		}
	}
}


static bool get_line_intersection(BPoint line0_start, BPoint line0_end, BPoint line1_start, BPoint line1_end,
                                      BPoint *pt, bool line0_extend = false, bool line1_extend = false)
{
	if (pt == NULL) return false;

	if (line0_start == line1_start || line0_start == line1_end) {
		*pt = line0_start;
		return true;
	}
	if (line0_end == line1_end) {
		*pt = line0_end;
		return true;
	}

	BRect rect0, rect1;
	rect0.SetLeftTop(min_c(line0_start.x, line0_end.x), min_c(line0_start.y, line0_end.y));
	rect0.SetRightBottom(max_c(line0_start.x, line0_end.x), max_c(line0_start.y, line0_end.y));
	rect1.SetLeftTop(min_c(line1_start.x, line1_end.x), min_c(line1_start.y, line1_end.y));
	rect1.SetRightBottom(max_c(line1_start.x, line1_end.x), max_c(line1_start.y, line1_end.y));

	BRect r = (rect0 & rect1);
	if (r.IsValid() == false && line0_extend == false && line1_extend == false) return false;

	if (r.LeftTop() == r.RightBottom()) {
		if (r.LeftTop() == line0_start || r.LeftTop() == line0_end || r.LeftTop() == line1_start || r.LeftTop() == line1_end) {
			*pt = r.LeftTop();
			return true;
		}
		if (line0_extend == false && line1_extend == false) return false;
	}

	BPoint delta0 = line0_end - line0_start;
	BPoint delta1 = line1_end - line1_start;

	float a = delta0.y * delta1.x - delta1.y * delta0.x;
	if (a == 0.f || line0_end == line1_start) {
		if (a != 0.f) {
			*pt = line0_end;
			return true;
		}
		if (line0_end != line1_start) return false; // parallel or same

		// same line, here it return the point for polygon-drawing
		*pt = (delta0.x * delta0.x + delta0.y * delta0.y > delta1.x * delta1.x + delta1.y * delta1.y ? line1_end : line0_start);
		return true;
	}

	float b = delta0.x * delta1.x * (line1_start.y - line0_start.y) +
	          (line0_start.x * delta0.y * delta1.x - line1_start.x * delta1.y * delta0.x);
	pt->x = b / a;
	if (isnan(pt->x)) return false;

	if (delta0.x != 0.f) pt->y = line0_start.y + (pt->x - line0_start.x) * delta0.y / delta0.x;
	if (delta0.x == 0.f || isnan(pt->y)) pt->y = line1_start.y + (pt->x - line1_start.x) * delta1.y / delta1.x;
	if (isnan(pt->y)) return false;

	return((line0_extend || rect0.Contains(*pt)) && (line1_extend || rect1.Contains(*pt)));
}


void
BRender::StrokePolygon(const BPolygon *aPolygon, bool closed, pattern pattern)
{
	if (!IsValid() || aPolygon == NULL) return;
	StrokePolygon(aPolygon->Points(), aPolygon->CountPoints(), closed, pattern);
}


#define INTERSECTS(v, s1, e1, s2, e2)		\
	(max_c(s1, s2) <= min_c(e1, e2) ? ((v = (((int64)min_c(s1, s2)) << 32) | (int64)max_c(e1, e2)), true) : false)
static void include_region(int64 *region, int32 *count, int32 minX, int32 maxX)
{
	if (minX > maxX) return;

	int32 i = 0;
	while (i < *count) {
		int64 v0 = *(region + i);

		if (INTERSECTS(v0, (int32)(v0 >> 32), (int32)(v0 & 0xffffffff), minX, maxX) == false) {
			i++;
			continue;
		}

		minX = (int32)(v0 >> 32);
		maxX = (int32)(v0 & 0xffffffff);

		if (i < *count - 1) memmove(region + i, region + i + 1, (size_t)(*count - i - 1) * sizeof(int64));
		(*count) -= 1;
	}

	*(region + ((*count)++)) = (((int64)minX) << 32) | (int64)maxX;
}
#undef INTERSECTS


static void stroke_objects(BRender *render, BList *objects, pattern pattern)
{
	if (objects->CountItems() <= 0) return;

	BRenderObject *aObject;

	objects->SortItems(BRenderObject::cmp);

	int64 *region = (int64*)malloc(sizeof(int64) * objects->CountItems());
	if (region == NULL) {
		while ((aObject = (BRenderObject*)objects->RemoveItem((int32)0)) != NULL) delete aObject;
		return;
	}

	while (objects->CountItems() > 0) {
		int32 curY, tmpY, minX, maxX;
		int32 count = 0;

		aObject = (BRenderObject*)objects->FirstItem();
		if (aObject->Get(&curY, &minX, &maxX) == false) {
			objects->RemoveItem((int32)0);
			delete aObject;
			continue;
		}

		include_region(region, &count, minX, maxX);

		for (int32 i = 1; i < objects->CountItems(); i++) {
			aObject = (BRenderObject*)objects->ItemAt(i);

			if (aObject->Get(&tmpY, &minX, &maxX) == false) {
				objects->RemoveItem(i);
				delete aObject;
				i--;
				continue;
			}

			if (tmpY != curY) break;
			aObject->Next();

			include_region(region, &count, minX, maxX);
		}

		((BRenderObject*)objects->FirstItem())->Next();

		for (int32 i = 0; i < count; i++) {
			int64 v = *(region + i);
			minX = (int32)(v >> 32);
			maxX = (int32)(v & 0xffffffff);

			render->FillRect(minX, curY, maxX - minX + 1, 1, pattern);
		}
	}

	free(region);
}


void
BRender::StrokePolygon(const BPoint *ptArray, int32 numPts, bool closed, pattern pattern)
{
	if (!IsValid() || ptArray == NULL || numPts <= 0) return;

	// TODO: clipping

	if (numPts < 3) {
		StrokeLine(ptArray[0], ptArray[numPts == 1 ? 0 : 1], pattern);
		return;
	}

	if (fPenSize > 1) {
		// TODO
		ETK_WARNING("[RENDER]: %s --- not support large pen yet.", __PRETTY_FUNCTION__);
		return;
	} else {
		BList lines;
		BPoint pt0, pt1;

		for (int32 k = 0; k < numPts; k++) {
			if (k == numPts - 1 && (closed == false || *(ptArray + k) == *ptArray)) break;

			pt0 = *(ptArray + k);
			pt1 = *(ptArray + (k < numPts - 1 ? k + 1 : 0));

			if (pt0.y > pt1.y) SWAP(BPoint, pt0, pt1);
			BRenderLine *aLine = new BRenderLine(pt0, pt1);

			if (lines.AddItem(aLine) == false) {
				delete aLine;
				break;
			}
		}

		stroke_objects(this, &lines, pattern);
	}
}


void
BRender::FillPolygon(const BPolygon *aPolygon, bool stroke_edge, pattern pattern)
{
	if (!IsValid() || aPolygon == NULL) return;
	FillPolygon(aPolygon->Points(), aPolygon->CountPoints(), stroke_edge, pattern);
}


#if 0
static bool triangle_contains(BPoint pt0, BPoint pt1, BPoint pt2, BPoint aPt, bool ignore_edge)
{
	if (aPt == pt0 || aPt == pt1 || aPt == pt2) return(ignore_edge ? false : true);

	BRect r;
	r.SetLeftTop(min_c(min_c(pt0.x, pt1.x), pt2.x), min_c(min_c(pt0.y, pt1.y), pt2.y));
	r.SetRightBottom(max_c(max_c(pt0.x, pt1.x), pt2.x), max_c(max_c(pt0.y, pt1.y), pt2.y));
	if (r.Contains(aPt) == false) return false;

	BPoint tmp, center;
	center.x = pt0.x / 3.f + pt1.x / 3.f + pt2.x / 3.f;
	center.y = pt0.y / 3.f + pt1.y / 3.f + pt2.y / 3.f;

	if (get_line_intersection(aPt, center, pt0, pt1, &tmp)) return(ignore_edge ? false : tmp == aPt);
	if (get_line_intersection(aPt, center, pt0, pt2, &tmp)) return(ignore_edge ? false : tmp == aPt);
	if (get_line_intersection(aPt, center, pt1, pt2, &tmp)) return(ignore_edge ? false : tmp == aPt);

	return true;
}
#endif


void
BRender::FillPolygon(const BPoint *ptArray, int32 numPts, bool stroke_edge, pattern pattern)
{
	BPoint *aPt = NULL;
	BPolygon *aPolygon = NULL;

	if (!IsValid() || ptArray == NULL || numPts <= 0) return;

	while (numPts > 3) {
		if (ptArray[numPts - 1] == ptArray[0]) {
			numPts--;
			continue;
		}
		break;
	}

	if (numPts <= 3) {
		FillTriangle(ptArray[0],
		             ptArray[max_c(0, min_c(numPts - 2, 1))],
		             ptArray[max_c(0, min_c(numPts - 1, 2))],
		             stroke_edge, pattern);
		return;
	}

	BList pts(numPts);
	bool readyForDraw = true;

	for (int32 i = 0; i < numPts; i++) {
		if (!(i == 0 || ptArray[i] != ptArray[i - 1])) continue;

		aPt = new BPoint(ptArray[i]);
		if (pts.AddItem(aPt)) continue;

		delete aPt;
		readyForDraw = false;
		break;
	}

	BPoint psPt, pePt, sPt, ePt, iPt;
	BList polygons;
	BPolygon tmp;

	for (int32 i = 2; readyForDraw && i <= pts.CountItems(); i++) { // split to polygons
		if (i < 2) continue;

		sPt = *((BPoint*)pts.ItemAt(i - 1));
		ePt = (i < pts.CountItems() ? *((BPoint*)pts.ItemAt(i)) : *((BPoint*)pts.FirstItem()));

		for (int32 k = i; readyForDraw && k >= 2; k--) {
			psPt = *((BPoint*)pts.ItemAt(k - 2));
			pePt = *((BPoint*)pts.ItemAt(k - 1));

			if (get_line_intersection(psPt, pePt, sPt, ePt, &iPt) == false ||
			        iPt == sPt || (iPt == ePt && i == pts.CountItems())) continue;

			aPolygon = new BPolygon(&iPt, 1);
			for (int32 m = k - 1; m < i; m++) {
				aPt = (BPoint*)pts.ItemAt(k - 1);
				if (aPt == NULL || aPolygon->AddPoints(aPt, 1, false) == false) break;

				if (m == i - 1) *aPt = iPt;
				else {
					pts.RemoveItem(k - 1);
					delete aPt;
				}
			}

			if (aPolygon->CountPoints() != (i - k + 2) || polygons.AddItem(aPolygon) == false) {
				delete aPolygon;
				readyForDraw = false;
				break;
			}

			i = k - 1;

			break;
		}
	}

	while ((aPt = (BPoint*)pts.RemoveItem((int32)0)) != NULL) {
		if (readyForDraw) readyForDraw = tmp.AddPoints(aPt, 1, false);
		delete aPt;
	}

	aPolygon = &tmp;
	BList objects;

	do {
		if (stroke_edge) for (int32 i = 0; readyForDraw && i < aPolygon->CountPoints(); i++) {
				sPt = (*aPolygon)[i];
				ePt = (*aPolygon)[(i < aPolygon->CountPoints() - 1) ? i + 1 : 0];

				if (sPt.y > ePt.y) SWAP(BPoint, sPt, ePt);
				BRenderLine *aLine = new BRenderLine(sPt, ePt);
				if (objects.AddItem(aLine) == false) {
					delete aLine;
					readyForDraw = false;
					break;
				}
			}

		while (readyForDraw && aPolygon->CountPoints() > 0) {
			BPolygon drawingPolygon;

			int32 flags[2] = {0, 0};
			for (int32 i = 0; i < aPolygon->CountPoints(); i++) {
				sPt = (*aPolygon)[i == 0 ? aPolygon->CountPoints() - 1: i - 1];
				iPt = (*aPolygon)[i];
				ePt = (*aPolygon)[(i < aPolygon->CountPoints() - 1) ? i + 1 : 0];

				psPt = sPt - iPt;
				pePt = ePt - iPt;

				float zValue = psPt.x * pePt.y - psPt.y * pePt.x;
				if (zValue == 0.f) continue;
				if (zValue > 0.f) flags[0] += 1;
				else flags[1] += 1;
			}

			if (flags[0] == 0 || flags[1] == 0) {
				drawingPolygon = *aPolygon;
				if (drawingPolygon.CountPoints() != aPolygon->CountPoints()) {
					readyForDraw = false;
					break;
				}
				aPolygon->RemovePoints(0, aPolygon->CountPoints() - 1, false);
			} else {
				// TODO: split polygon to polygons without nooks
				ETK_WARNING("[RENDER]: %s --- TODO", __PRETTY_FUNCTION__);
				break;
			}

			while (readyForDraw) {
				BPoint pt0 = drawingPolygon[0];
				BPoint pt1 = drawingPolygon[max_c(0, drawingPolygon.CountPoints() - 2)];
				BPoint pt2 = drawingPolygon[drawingPolygon.CountPoints() - 1];

				if (pt0.y > pt1.y) SWAP(BPoint, pt0, pt1);
				if (drawingPolygon.CountPoints() > 3) {
					BRenderLine *aLine = new BRenderLine(pt0, pt1);
					if (objects.AddItem(aLine) == false) {
						delete aLine;
						readyForDraw = false;
						break;
					}
				}

				if (pt0.y > pt2.y) SWAP(BPoint, pt0, pt2);
				if (pt1.y > pt2.y) SWAP(BPoint, pt1, pt2);
				BRenderTriangle *aTriangle = new BRenderTriangle(pt0, pt1, pt2, false);
				if (objects.AddItem(aTriangle) == false) {
					delete aTriangle;
					readyForDraw = false;
					break;
				}

				if (drawingPolygon.CountPoints() <= 3) break;
				drawingPolygon.RemovePoint(drawingPolygon.CountPoints() - 1, false);
			}
		}

		if (aPolygon != (&tmp)) delete aPolygon;
	} while ((aPolygon = (BPolygon*)polygons.RemoveItem((int32)0)) != NULL);

	if (readyForDraw) {
		stroke_objects(this, &objects, pattern);
	} else {
		BRenderObject *aObject;
		while ((aObject = (BRenderObject*)objects.RemoveItem((int32)0)) != NULL) delete aObject;
	}
}

