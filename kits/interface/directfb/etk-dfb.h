/* --------------------------------------------------------------------------
 *
 * DirectFB Graphics Add-on for ETK++
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
 * File: etk-dfb.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_DIRECTFB_H__
#define __ETK_DIRECTFB_H__

#include <directfb.h>
#include <directfb_version.h>

#if DIRECTFB_MAJOR_VERSION == 0 && DIRECTFB_MINOR_VERSION == 9 && DIRECTFB_MICRO_VERSION < 21
#define DSFLIP_NONE		(DFBSurfaceFlipFlags)0
#define DWCAPS_NODECORATION	0
#endif

#if DIRECTFB_MAJOR_VERSION == 0 && DIRECTFB_MINOR_VERSION == 9 && DIRECTFB_MICRO_VERSION >= 22
#define DFB_HAVE_FILLRECTANGLES
#endif

#include <add-ons/graphics/GraphicsEngine.h>
#include <support/Locker.h>
#include <support/List.h>
#include <interface/Window.h>
#include <app/MessageFilter.h>

#ifdef __cplusplus /* just for C++ */


class BDFBGraphicsEngine : public BGraphicsEngine
{
	public:
		BDFBGraphicsEngine();
		virtual ~BDFBGraphicsEngine();

		status_t			InitCheck();

		bool				Lock();
		void				Unlock();

		bool				SetDFBWindowData(IDirectFBWindow *dfbWin, void *data, void **old_data = NULL);
		void				*GetDFBWindowData(IDirectFBWindow *dfbWin);
		void				*GetDFBWindowData(DFBWindowID dfbWinID);

		bool				ConvertRegion(const BRegion *region, DFBRegion **dfbRegions, int *nRegions);

		virtual status_t		Initalize();
		virtual void			Cancel();

		virtual BGraphicsContext*	CreateContext();
		virtual BGraphicsDrawable*	CreatePixmap(uint32 w, uint32 h);
		virtual BGraphicsWindow*	CreateWindow(int32 x, int32 y, uint32 w, uint32 h);

		virtual status_t		InitalizeFonts();
		virtual void			DestroyFonts();
		virtual status_t		UpdateFonts(bool check_only);

		virtual status_t		GetDesktopBounds(uint32 *w, uint32 *h);
		virtual status_t		GetCurrentWorkspace(uint32 *workspace);
		virtual status_t		SetCursor(const void *cursor_data);
		virtual status_t		GetDefaultCursor(BCursor *cursor);

		IDirectFB *dfbDisplay;
		IDirectFBDisplayLayer *dfbDisplayLayer;
		IDirectFBEventBuffer *dfbEventBuffer;

		int dfbDisplayWidth;
		int dfbDisplayHeight;

		DFBWindowID dfbCurFocusWin;
		DFBWindowID dfbCurPointerGrabbed;
		struct timeval dfbClipboardTimeStamp;
		IDirectFBSurface *dfbCursor;

		bool dfbDoQuit;

	private:
		BLocker fLocker;
		void *fDFBThread;
		BMessageFilter *fClipboardFilter;

		struct dfb_data {
			IDirectFBWindow *win;
			void *data;
		};
		BList fDFBDataList;
};


class BDFBGraphicsDrawable : public BGraphicsDrawable
{
	public:
		BDFBGraphicsDrawable(BDFBGraphicsEngine *dfbEngine, uint32 w, uint32 h);
		virtual ~BDFBGraphicsDrawable();

		virtual status_t		SetBackgroundColor(rgb_color bkColor);

		virtual status_t		ResizeTo(uint32 w, uint32 h);
		virtual status_t		CopyTo(BGraphicsContext *dc,
		                         BGraphicsDrawable *dstDrawable,
		                         int32 x, int32 y, uint32 w, uint32 h,
		                         int32 dstX, int32 dstY, uint32 dstW, uint32 dstH);
		virtual status_t		DrawPixmap(BGraphicsContext *dc, const BPixmap *pix,
		                             int32 x, int32 y, uint32 w, uint32 h,
		                             int32 dstX, int32 dstY, uint32 dstW, uint32 dstH);

		virtual status_t		StrokePoint(BGraphicsContext *dc,
		                              int32 x, int32 y);
		virtual status_t		StrokePoints(BGraphicsContext *dc,
		                               const int32 *pts, int32 count);
		virtual status_t		StrokePoints_Colors(BGraphicsContext *dc,
		                                      const BList *ptsArrayLists, int32 arrayCount,
		                                      const rgb_color *highColors);
		virtual status_t		StrokePoints_Alphas(BGraphicsContext *dc,
		                                      const int32 *pts, const uint8 *alpha, int32 count);
		virtual status_t		StrokeLine(BGraphicsContext *dc,
		                             int32 x0, int32 y0, int32 x1, int32 y1);
		virtual status_t		StrokePolygon(BGraphicsContext *dc,
		                                const int32 *pts, int32 count, bool closed);
		virtual status_t		FillPolygon(BGraphicsContext *dc,
		                              const int32 *pts, int32 count);
		virtual status_t		StrokeRect(BGraphicsContext *dc,
		                             int32 x, int32 y, uint32 w, uint32 h);
		virtual status_t		FillRect(BGraphicsContext *dc,
		                           int32 x, int32 y, uint32 w, uint32 h);
		virtual status_t		StrokeRects(BGraphicsContext *dc,
		                              const int32 *rects, int32 count);
		virtual status_t		FillRects(BGraphicsContext *dc,
		                            const int32 *rects, int32 count);
		virtual status_t		FillRegion(BGraphicsContext *dc,
		                             const BRegion &region);
		virtual status_t		StrokeRoundRect(BGraphicsContext *dc,
		                                  int32 x, int32 y, uint32 w, uint32 h, uint32 xRadius, uint32 yRadius);
		virtual status_t		FillRoundRect(BGraphicsContext *dc,
		                                int32 x, int32 y, uint32 w, uint32 h, uint32 xRadius, uint32 yRadius);
		virtual status_t		StrokeArc(BGraphicsContext *dc,
		                            int32 x, int32 y, uint32 w, uint32 h, float startAngle, float endAngle);
		virtual status_t		FillArc(BGraphicsContext *dc,
		                          int32 x, int32 y, uint32 w, uint32 h, float startAngle, float endAngle);

		IDirectFBSurface *dfbSurface;
		uint32 fWidth;
		uint32 fHeight;

	private:
		BDFBGraphicsEngine *fEngine;
};


class BDFBGraphicsWindow : public BGraphicsWindow
{
	public:
		BDFBGraphicsWindow(BDFBGraphicsEngine *dfbEngine, int32 x, int32 y, uint32 w, uint32 h);
		virtual ~BDFBGraphicsWindow();

		status_t			GetContactor(BMessenger *msgr);

		virtual status_t		ContactTo(const BMessenger *msgr);
		virtual status_t		SetBackgroundColor(rgb_color bkColor);
		virtual status_t		SetFlags(uint32 flags);
		virtual status_t		SetLook(window_look look);
		virtual status_t		SetFeel(window_feel feel);
		virtual status_t		SetTitle(const char *title);
		virtual status_t		SetWorkspaces(uint32 workspaces);
		virtual status_t		GetWorkspaces(uint32 *workspaces);
		virtual status_t		Iconify();
		virtual status_t		Show();
		virtual status_t		Hide();
		virtual status_t		Raise();
		virtual status_t		Lower(BGraphicsWindow *frontWin);
		virtual status_t		Activate(bool state);
		virtual status_t		GetActivatedState(bool *state) const;
		virtual status_t		MoveTo(int32 x, int32 y);
		virtual status_t		ResizeTo(uint32 w, uint32 h);
		virtual status_t		MoveAndResizeTo(int32 x, int32 y, uint32 w, uint32 h);
		virtual status_t		SetSizeLimits(uint32 min_w, uint32 max_w, uint32 min_h, uint32 max_h);
		virtual status_t		GetSizeLimits(uint32 *min_w, uint32 *max_w, uint32 *min_h, uint32 *max_h);
		virtual status_t		GrabMouse();
		virtual status_t		UngrabMouse();
		virtual status_t		GrabKeyboard();
		virtual status_t		UngrabKeyboard();
		virtual status_t		QueryMouse(int32 *x, int32 *y, int32 *buttons);

		virtual status_t		CopyTo(BGraphicsContext *dc,
		                         BGraphicsDrawable *dstDrawable,
		                         int32 x, int32 y, uint32 w, uint32 h,
		                         int32 dstX, int32 dstY, uint32 dstW, uint32 dstH);
		virtual status_t		DrawPixmap(BGraphicsContext *dc, const BPixmap *pix,
		                             int32 x, int32 y, uint32 w, uint32 h,
		                             int32 dstX, int32 dstY, uint32 dstW, uint32 dstH);

		virtual status_t		StrokePoint(BGraphicsContext *dc,
		                              int32 x, int32 y);
		virtual status_t		StrokePoints(BGraphicsContext *dc,
		                               const int32 *pts, int32 count);
		virtual status_t		StrokePoints_Colors(BGraphicsContext *dc,
		                                      const BList *ptsArrayLists, int32 arrayCount,
		                                      const rgb_color *highColors);
		virtual status_t		StrokePoints_Alphas(BGraphicsContext *dc,
		                                      const int32 *pts, const uint8 *alpha, int32 count);
		virtual status_t		StrokeLine(BGraphicsContext *dc,
		                             int32 x0, int32 y0, int32 x1, int32 y1);
		virtual status_t		StrokePolygon(BGraphicsContext *dc,
		                                const int32 *pts, int32 count, bool closed);
		virtual status_t		FillPolygon(BGraphicsContext *dc,
		                              const int32 *pts, int32 count);
		virtual status_t		StrokeRect(BGraphicsContext *dc,
		                             int32 x, int32 y, uint32 w, uint32 h);
		virtual status_t		FillRect(BGraphicsContext *dc,
		                           int32 x, int32 y, uint32 w, uint32 h);
		virtual status_t		StrokeRects(BGraphicsContext *dc,
		                              const int32 *rects, int32 count);
		virtual status_t		FillRects(BGraphicsContext *dc,
		                            const int32 *rects, int32 count);
		virtual status_t		FillRegion(BGraphicsContext *dc,
		                             const BRegion &region);
		virtual status_t		StrokeRoundRect(BGraphicsContext *dc,
		                                  int32 x, int32 y, uint32 w, uint32 h, uint32 xRadius, uint32 yRadius);
		virtual status_t		FillRoundRect(BGraphicsContext *dc,
		                                int32 x, int32 y, uint32 w, uint32 h, uint32 xRadius, uint32 yRadius);

		virtual status_t		StrokeArc(BGraphicsContext *dc,
		                            int32 x, int32 y, uint32 w, uint32 h, float startAngle, float endAngle);
		virtual status_t		FillArc(BGraphicsContext *dc,
		                          int32 x, int32 y, uint32 w, uint32 h, float startAngle, float endAngle);

		IDirectFBWindow *dfbWindow;
		IDirectFBSurface *dfbSurface;
		BRect fMargins;
		DFBWindowID dfbWindowID;
		uint32 fFlags;
		int32 fOriginX;
		int32 fOriginY;
		uint32 fWidth;
		uint32 fHeight;
		bool fHidden;

	private:
		friend class BDFBGraphicsEngine;

		BDFBGraphicsEngine *fEngine;

		BMessenger fMsgr;

		window_look fLook;
		window_feel fFeel;

		char *fTitle;

		bool fHandlingMove;
		bool fHandlingResize;

		int wmPointerOffsetX;
		int wmPointerOffsetY;

#if 0
		int minW;
		int minH;
		int maxW;
		int maxH;
#endif
};


#define DUET_EVENTPENDING	0
#define DUET_WINDOWREDRAWALL	B_MAXUINT


extern status_t dfb_stroke_point(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
	                                     int32 x, int32 y, BRect *margins = NULL);
extern status_t dfb_stroke_points(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
	                                      const int32 *pts, int32 count, BRect *margins = NULL);
extern status_t dfb_stroke_points_color(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
	        const BList *ptsArrayLists, int32 arrayCount, const rgb_color *highColors,
	        BRect *margins = NULL);
extern status_t dfb_stroke_line(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
	                                    int32 x0, int32 y0, int32 x1, int32 y1, BRect *margins = NULL);
extern status_t dfb_stroke_rect(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
	                                    int32 x, int32 y, uint32 w, uint32 h, BRect *margins = NULL);
extern status_t dfb_fill_rect(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
	                                  int32 x, int32 y, uint32 w, uint32 h, BRect *margins = NULL);
extern status_t dfb_stroke_rects(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
	                                     const int32 *rects, int32 count, BRect *margins = NULL);
extern status_t dfb_fill_rects(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
	                                   const int32 *rects, int32 count, BRect *margins = NULL);
extern status_t dfb_fill_region(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
	                                    const BRegion &region, BRect *margins = NULL);
extern status_t dfb_stroke_arc(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
	                                   int32 x, int32 y, uint32 w, uint32 h, float startAngle, float endAngle, BRect *margins = NULL);
extern status_t dfb_fill_arc(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
	                                 int32 x, int32 y, uint32 w, uint32 h, float startAngle, float endAngle, BRect *margins = NULL);
extern status_t dfb_draw_epixmap(IDirectFBSurface *dfbSurface, BGraphicsContext *dc, const BPixmap *pix,
	                                     int32 x, int32 y, uint32 w, uint32 h,
	                                     int32 dstX, int32 dstY, uint32 dstW, uint32 dstH, BRect *margins = NULL);

extern status_t dfb_stroke_points_alphas(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
	        const int32 *pts, const uint8 *alpha, int32 count, BRect *margins = NULL);
extern status_t dfb_stroke_polygon(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
	                                       const int32 *pts, int32 count, bool closed, BRect *margins = NULL);
extern status_t dfb_fill_polygon(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
	                                     const int32 *pts, int32 count, BRect *margins = NULL);
extern status_t dfb_stroke_round_rect(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
	        int32 x, int32 y, uint32 w, uint32 h, uint32 xRadius, uint32 yRadius,
	        BRect *margins = NULL);
extern status_t dfb_fill_round_rect(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
	                                        int32 x, int32 y, uint32 w, uint32 h, uint32 xRadius, uint32 yRadius,
	                                        BRect *margins = NULL);


#endif /* __cplusplus */

#endif /* __ETK_DIRECTFB_H__ */

