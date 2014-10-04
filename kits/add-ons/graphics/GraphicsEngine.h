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
 * File: GraphicsEngine.h
 * Description: Graphics Engine Addon
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_GRAPHICS_ENGINB_H__
#define __ETK_GRAPHICS_ENGINB_H__

#include <interface/Window.h>
#include <interface/View.h>

#ifdef __cplusplus /* Just for C++ */


class BCursor;
class BPixmap;


class BGraphicsContext {
public:
	BGraphicsContext();
	virtual ~BGraphicsContext();

	virtual status_t		SetDrawingMode(e_drawing_mode mode);
	virtual status_t		SetClipping(const BRegion &clipping);
	virtual status_t		SetHighColor(rgb_color highColor);
	virtual status_t		SetLowColor(rgb_color lowColor);
	virtual status_t		SetPattern(pattern pattern);
	virtual status_t		SetPenSize(uint32 penSize);
	virtual status_t		SetSquarePointStyle(bool state);

	e_drawing_mode			DrawingMode() const;
	const BRegion			*Clipping() const;
	pattern			Pattern() const;
	uint32				PenSize() const;
	bool				IsSquarePointStyle() const;

	status_t			SetHighColor(uint8 r, uint8 g, uint8 b, uint8 a);
	status_t			SetLowColor(uint8 r, uint8 g, uint8 b, uint8 a);
	rgb_color			HighColor() const;
	rgb_color			LowColor() const;

private:
	e_drawing_mode fDrawingMode;
	BRegion fClipping;
	rgb_color fHighColor;
	rgb_color fLowColor;
	pattern fPattern;
	uint32 fPenSize;
	bool fSquarePoint;
};


class BGraphicsDrawable {
public:
	BGraphicsDrawable();
	virtual ~BGraphicsDrawable();

	virtual status_t		SetBackgroundColor(rgb_color bkColor);

	virtual status_t		ResizeTo(uint32 w, uint32 h) = 0;
	virtual status_t		CopyTo(BGraphicsContext *dc,
	                         BGraphicsDrawable *dstDrawable,
	                         int32 x, int32 y, uint32 w, uint32 h,
	                         int32 dstX, int32 dstY, uint32 dstW, uint32 dstH) = 0;
	virtual status_t		DrawPixmap(BGraphicsContext *dc, const BPixmap *pix,
	                             int32 x, int32 y, uint32 w, uint32 h,
	                             int32 dstX, int32 dstY, uint32 dstW, uint32 dstH) = 0;

	virtual status_t		StrokePoint(BGraphicsContext *dc,
	                              int32 x, int32 y) = 0;
	virtual status_t		StrokePoints(BGraphicsContext *dc,
	                               const int32 *pts, int32 count) = 0;
	virtual status_t		StrokePoints_Colors(BGraphicsContext *dc,
	                                      const BList *ptsArrayLists, int32 arrayCount,
	                                      const rgb_color *highColors) = 0;
	virtual status_t		StrokePoints_Alphas(BGraphicsContext *dc,
	                                      const int32 *pts, const uint8 *alpha, int32 count) = 0;
	virtual status_t		StrokeLine(BGraphicsContext *dc,
	                             int32 x0, int32 y0, int32 x1, int32 y1) = 0;
	virtual status_t		StrokePolygon(BGraphicsContext *dc,
	                                const int32 *pts, int32 count, bool closed) = 0;
	virtual status_t		FillPolygon(BGraphicsContext *dc,
	                              const int32 *pts, int32 count) = 0;
	virtual status_t		StrokeRect(BGraphicsContext *dc,
	                             int32 x, int32 y, uint32 w, uint32 h) = 0;
	virtual status_t		FillRect(BGraphicsContext *dc,
	                           int32 x, int32 y, uint32 w, uint32 h) = 0;
	virtual status_t		StrokeRects(BGraphicsContext *dc,
	                              const int32 *rects, int32 count) = 0;
	virtual status_t		FillRects(BGraphicsContext *dc,
	                            const int32 *rects, int32 count) = 0;
	virtual status_t		FillRegion(BGraphicsContext *dc,
	                             const BRegion &region) = 0;
	virtual status_t		StrokeRoundRect(BGraphicsContext *dc,
	                                  int32 x, int32 y, uint32 w, uint32 h, uint32 xRadius, uint32 yRadius) = 0;
	virtual status_t		FillRoundRect(BGraphicsContext *dc,
	                                int32 x, int32 y, uint32 w, uint32 h, uint32 xRadius, uint32 yRadius) = 0;
	virtual status_t		StrokeArc(BGraphicsContext *dc,
	                            int32 x, int32 y, uint32 w, uint32 h, float startAngle, float endAngle) = 0;
	virtual status_t		FillArc(BGraphicsContext *dc,
	                          int32 x, int32 y, uint32 w, uint32 h, float startAngle, float endAngle) = 0;

	status_t			SetBackgroundColor(uint8 r, uint8 g, uint8 b, uint8 a);
	rgb_color			BackgroundColor() const;

private:
	rgb_color fBkColor;
};


class BGraphicsWindow :
public BGraphicsDrawable {
public:
	BGraphicsWindow();
	virtual ~BGraphicsWindow();

	virtual status_t		ContactTo(const BMessenger *msgr) = 0;
	virtual status_t		SetFlags(uint32 flags) = 0;
	virtual status_t		SetLook(window_look look) = 0;
	virtual status_t		SetFeel(window_feel feel) = 0;
	virtual status_t		SetTitle(const char *title) = 0;
	virtual status_t		SetWorkspaces(uint32 workspaces) = 0;
	virtual status_t		GetWorkspaces(uint32 *workspaces) = 0;
	virtual status_t		Iconify() = 0;
	virtual status_t		Show() = 0;
	virtual status_t		Hide() = 0;
	virtual status_t		Raise() = 0;
	virtual status_t		Lower(BGraphicsWindow* frontWin) = 0;
	virtual status_t		Activate(bool state) = 0;
	virtual status_t		GetActivatedState(bool *state) const = 0;
	virtual status_t		MoveTo(int32 x, int32 y) = 0;
	virtual status_t		MoveAndResizeTo(int32 x, int32 y, uint32 w, uint32 h) = 0;
	virtual status_t		SetSizeLimits(uint32 min_w, uint32 max_w, uint32 min_h, uint32 max_h) = 0;
	virtual status_t		GetSizeLimits(uint32 *min_w, uint32 *max_w, uint32 *min_h, uint32 *max_h) = 0;
	virtual status_t		GrabMouse() = 0;
	virtual status_t		UngrabMouse() = 0;
	virtual status_t		GrabKeyboard() = 0;
	virtual status_t		UngrabKeyboard() = 0;
	virtual status_t		QueryMouse(int32 *x, int32 *y, int32 *buttons) = 0;
};


// NOTE:
// 1. Addon must have C function like below and all the null virtual functions of class must be implemented.
// 		extern "C" _EXPORT BGraphicsEngine* instantiate_graphics_engine();
// 2. Usually, addons were put into the directory located at "$B_ADDONS_DIRECTORY/etkxx/graphics" or
//    "$B_USER_ADDONS_DIRECTORY/etkxx/graphics", such as "/usr/lib/add-ons/etkxx/graphics".
// 3. When ETK++ find no graphics-engine addons, it try built-in graphics-engine when possible.
class BGraphicsEngine {
public:
	BGraphicsEngine();
	virtual ~BGraphicsEngine();

	virtual status_t		Initalize() = 0;
	virtual void			Cancel() = 0;

	virtual BGraphicsContext*	CreateContext() = 0;
	virtual BGraphicsDrawable*	CreatePixmap(uint32 w, uint32 h) = 0;
	virtual BGraphicsWindow*	CreateWindow(int32 x, int32 y, uint32 w, uint32 h) = 0;

	virtual status_t		InitalizeFonts() = 0;
	virtual void			DestroyFonts() = 0;
	virtual status_t		UpdateFonts(bool check_only) = 0;

	virtual status_t		GetDesktopBounds(uint32 *w, uint32 *h) = 0;
	virtual status_t		GetCurrentWorkspace(uint32 *workspace) = 0;
	virtual status_t		SetCursor(const void *cursor_data) = 0;
	virtual status_t		GetDefaultCursor(BCursor *cursor) = 0;

	static BGraphicsWindow		*GetWindow(BWindow *win);
	static BGraphicsDrawable	*GetPixmap(BWindow *win);
	static BGraphicsContext		*GetContext(BView *view);
};


#endif /* __cplusplus */

#endif /* __ETK_GRAPHICS_ENGINB_H__ */

