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
 * File: Render.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_RENDER_H__
#define __ETK_RENDER_H__

#include <interface/GraphicsDefs.h>
#include <interface/Rect.h>
#include <interface/Polygon.h>

#ifdef __cplusplus /* Just for C++ */

class BRender
{
	public:
		BRender();
		virtual ~BRender();

		bool		IsValid() const;

		void		SetDrawingMode(e_drawing_mode drawing_mode);
		e_drawing_mode	DrawingMode() const;

		void		SetHighColor(rgb_color highColor);
		void		SetHighColor(uint8 r, uint8 g, uint8 b, uint8 a = 255);
		rgb_color	HighColor() const;

		void		SetLowColor(rgb_color lowColor);
		void		SetLowColor(uint8 r, uint8 g, uint8 b, uint8 a = 255);
		rgb_color	LowColor() const;

		void		SetPenSize(float pen_size);
		float		PenSize() const;

		void		SetSquarePointStyle(bool state);
		bool		IsSquarePointStyle() const;

		void		FillRect(int32 x, int32 y, uint32 width, uint32 height,
		               pattern pattern = B_SOLID_HIGH);
		void		FillRect(BRect rect, pattern pattern = B_SOLID_HIGH);

		void		StrokePoint(int32 x, int32 y, pattern pattern = B_SOLID_HIGH);
		void		StrokePoint(BPoint pt, pattern pattern = B_SOLID_HIGH);

		void		StrokeLine(int32 x0, int32 y0, int32 x1, int32 y1,
		                 pattern pattern = B_SOLID_HIGH);
		void		StrokeLine(BPoint pt0, BPoint pt1, pattern pattern = B_SOLID_HIGH);

		void		StrokeTriangle(int32 x0, int32 y0, int32 x1, int32 y1, int32 x2, int32 y2, pattern pattern = B_SOLID_HIGH);
		void		StrokeTriangle(BPoint pt1, BPoint pt2, BPoint pt3, pattern pattern = B_SOLID_HIGH);
		void		FillTriangle(int32 x0, int32 y0, int32 x1, int32 y1, int32 x2, int32 y2,
		                   bool stroke_edge = true, pattern pattern = B_SOLID_HIGH);
		void		FillTriangle(BPoint pt0, BPoint pt1, BPoint pt2, bool stroke_edge = true, pattern pattern = B_SOLID_HIGH);

		void		StrokePolygon(const BPolygon *aPolygon, bool closed = true, pattern pattern = B_SOLID_HIGH);
		void		StrokePolygon(const BPoint *ptArray, int32 numPts, bool closed = true, pattern pattern = B_SOLID_HIGH);
		void		FillPolygon(const BPolygon *aPolygon, bool stroke_edge = true, pattern pattern = B_SOLID_HIGH);
		void		FillPolygon(const BPoint *ptArray, int32 numPts, bool stroke_edge = true, pattern pattern = B_SOLID_HIGH);

		void		StrokeEllipse(int32 x, int32 y, uint32 width, uint32 height,
		                    pattern pattern = B_SOLID_HIGH);
		void		StrokeEllipse(int32 xCenter, int32 yCenter, int32 xRadius, int32 yRadius,
		                    pattern pattern = B_SOLID_HIGH);
		void		StrokeEllipse(BRect rect, pattern pattern = B_SOLID_HIGH);
		void		FillEllipse(int32 x, int32 y, uint32 width, uint32 height,
		                  bool stroke_edge = true, pattern pattern = B_SOLID_HIGH);
		void		FillEllipse(int32 xCenter, int32 yCenter, int32 xRadius, int32 yRadius,
		                  bool stroke_edge = true, pattern pattern = B_SOLID_HIGH);
		void		FillEllipse(BRect rect, bool stroke_edge = true, pattern pattern = B_SOLID_HIGH);

		void		StrokeArc(int32 x, int32 y, uint32 width, uint32 height,
		                int32 startAngle, int32 endAngle,
		                pattern pattern = B_SOLID_HIGH);
		void		StrokeArc(int32 xCenter, int32 yCenter, int32 xRadius, int32 yRadius,
		                int32 startAngle, int32 endAngle,
		                pattern pattern = B_SOLID_HIGH);
		void		StrokeArc(BPoint ctPt, float xRadius, float yRadius,
		                float startAngle, float arcAngle,
		                pattern pattern = B_SOLID_HIGH);
		void		StrokeArc(BRect rect, float startAngle, float arcAngle, pattern pattern = B_SOLID_HIGH);
		void		StrokeArc(BRect rect, BPoint start, BPoint end, pattern pattern = B_SOLID_HIGH);

	private:
		e_drawing_mode fDrawingMode;
		rgb_color fHighColor;
		rgb_color fLowColor;
		float fPenSize;
		bool fSquarePointStyle;

		virtual status_t InitCheck() const = 0;
		virtual void GetFrame(int32 *originX, int32 *originY, uint32 *width, uint32 *height) const = 0;
		virtual void GetPixel(int32 x, int32 y, rgb_color &color) const = 0;
		virtual void PutPixel(int32 x, int32 y, rgb_color color) = 0;
		virtual void PutRect(int32 x, int32 y, uint32 width, uint32 height, rgb_color color);

		void drawPixel(int32 x, int32 y, pattern pattern);
};

#endif /* __cplusplus */

#endif /* __ETK_RENDER_H__ */

