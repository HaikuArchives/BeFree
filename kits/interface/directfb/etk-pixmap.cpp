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
 * File: etk-pixmap.cpp
 *
 * --------------------------------------------------------------------------*/

#include <support/Autolock.h>
#include <support/ClassInfo.h>

#include "etk-dfb.h"


BDFBGraphicsDrawable::BDFBGraphicsDrawable(BDFBGraphicsEngine *dfbEngine, uint32 w, uint32 h)
		: BGraphicsDrawable(), fEngine(NULL)
{
	if (w >= B_MAXINT32 || h >= B_MAXINT32) {
		ETK_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return;
	}

	fEngine = dfbEngine;
	if (fEngine == NULL) return;

	rgb_color whiteColor = e_makrgb_color(255, 255, 255, 255);
	BGraphicsDrawable::SetBackgroundColor(whiteColor);

	BAutolock <BDFBGraphicsEngine> autolock(fEngine);
	if (autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) {
		fEngine = NULL;
		return;
	}

	DFBSurfaceDescription desc;
	desc.flags = (DFBSurfaceDescriptionFlags)(DSDESC_CAPS | DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT);
	desc.caps = DSCAPS_SYSTEMONLY;
	desc.pixelformat = DSPF_ARGB;
	desc.width = w + 1;
	desc.height = h + 1;

	if (fEngine->dfbDisplay->CreateSurface(fEngine->dfbDisplay, &desc, &dfbSurface) != DFB_OK) {
		fEngine = NULL;
		return;
	}

#if 0
	DFBSurfacePixelFormat pixel_format;
	dfbSurface->GetPixelFormat(dfbSurface, &pixel_format);
	ETK_DEBUG("[GRAPHICS]: DFBSurface created (PixelFormat: 0x%x).", pixel_format);
#endif

	fWidth = w;
	fHeight = h;

	dfbSurface->Clear(dfbSurface, 255, 255, 255, 255);
}


BDFBGraphicsDrawable::~BDFBGraphicsDrawable()
{
	if (fEngine != NULL) {
		BAutolock <BDFBGraphicsEngine> autolock(fEngine);
		if (autolock.IsLocked() == false || fEngine->InitCheck() != B_OK)
			ETK_ERROR("[GRAPHICS]: %s --- Invalid graphics engine.", __PRETTY_FUNCTION__);

		dfbSurface->Release(dfbSurface);
	}
}


status_t
BDFBGraphicsDrawable::SetBackgroundColor(rgb_color bkColor)
{
	if (fEngine == NULL) return B_ERROR;

	BAutolock <BDFBGraphicsEngine> autolock(fEngine);
	if (autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	rgb_color c = BackgroundColor();
	if (c != bkColor) {
		BGraphicsDrawable::SetBackgroundColor(c);
		dfbSurface->Clear(dfbSurface, c.red, c.green, c.blue, 255);
	}

	return B_OK;
}


status_t
BDFBGraphicsDrawable::ResizeTo(uint32 w, uint32 h)
{
	if (w >= B_MAXINT32 || h >= B_MAXINT32) {
		ETK_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return B_ERROR;
	}

	if (fEngine == NULL) return B_ERROR;

	BAutolock <BDFBGraphicsEngine> autolock(fEngine);
	if (autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	DFBSurfaceDescription desc;
	desc.flags = (DFBSurfaceDescriptionFlags)(DSDESC_CAPS | DSDESC_WIDTH | DSDESC_HEIGHT);
	desc.caps = DSCAPS_SYSTEMONLY;
	desc.width = w + 1;
	desc.height = h + 1;

	IDirectFBSurface *newSurface;
	if (fEngine->dfbDisplay->CreateSurface(fEngine->dfbDisplay, &desc, &newSurface) != DFB_OK) return B_ERROR;

	dfbSurface->Release(dfbSurface);
	dfbSurface = newSurface;

	rgb_color c = BackgroundColor();
	dfbSurface->Clear(dfbSurface, c.red, c.green, c.blue, 255);

	return B_OK;
}


status_t
BDFBGraphicsDrawable::CopyTo(BGraphicsContext *dc,
                             BGraphicsDrawable *dstDrawable,
                             int32 x, int32 y, uint32 w, uint32 h,
                             int32 dstX, int32 dstY, uint32 dstW, uint32 dstH)
{
	if (w >= B_MAXINT32 || h >= B_MAXINT32 || dstW >= B_MAXINT32 || dstH >= B_MAXINT32) {
		ETK_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return B_ERROR;
	}

	if (fEngine == NULL || dc == NULL || dstDrawable == NULL) return B_ERROR;

	if (dc->DrawingMode() != B_OP_COPY) {
		ETK_DEBUG("[GRAPHICS]: %s --- FIXME: unsupported drawing mode.", __PRETTY_FUNCTION__);
		return B_ERROR;
	}

	BAutolock <BDFBGraphicsEngine> autolock(fEngine);
	if (autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	BDFBGraphicsWindow *win = NULL;
	BDFBGraphicsDrawable *pix = NULL;
	IDirectFBSurface *destSurface = NULL;
	BRect margins(0, 0, 0, 0);

	if ((win = cast_as(dstDrawable, BDFBGraphicsWindow)) != NULL) {
		destSurface = win->dfbSurface;
		margins = win->fMargins;
	} else if ((pix = cast_as(dstDrawable, BDFBGraphicsDrawable)) != NULL) destSurface = pix->dfbSurface;

	if (destSurface == NULL) return B_ERROR;

	DFBRegion *dfbRegions = NULL;
	int nRegions = 0;

	if (fEngine->ConvertRegion(dc->Clipping(), &dfbRegions, &nRegions) == false) return B_ERROR;

	destSurface->SetBlittingFlags(destSurface, DSBLIT_NOFX);

	for (int i = 0; i < nRegions; i++) {
		destSurface->SetClip(destSurface, dfbRegions + i);

		DFBRectangle srcRect, destRect;
		srcRect.x = (int)x;
		srcRect.y = (int)y;
		srcRect.w = (int)w + 1;
		srcRect.h = (int)h + 1;
		destRect.x = (int)dstX + (int)margins.left;
		destRect.y = (int)dstY + (int)margins.top;
		destRect.w = (int)dstW + 1;
		destRect.h = (int)dstH + 1;

		if (dstW == w && dstH == h)
			destSurface->Blit(destSurface, dfbSurface, &srcRect, destRect.x, destRect.y);
		else
			destSurface->StretchBlit(destSurface, dfbSurface, &srcRect, &destRect);
	}

	if (win != NULL) destSurface->Flip(destSurface, NULL, DSFLIP_WAITFORSYNC);

	free(dfbRegions);

	return B_OK;
}


status_t
BDFBGraphicsDrawable::DrawPixmap(BGraphicsContext *dc, const BPixmap *pix,
                                 int32 x, int32 y, uint32 w, uint32 h,
                                 int32 dstX, int32 dstY, uint32 dstW, uint32 dstH)
{
	if (fEngine == NULL) return B_ERROR;

	BAutolock <BDFBGraphicsEngine> autolock(fEngine);
	if (autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return dfb_draw_epixmap(dfbSurface, dc, pix, x, y, w, h, dstX, dstY, dstW, dstH);
}


status_t
BDFBGraphicsDrawable::StrokePoint(BGraphicsContext *dc,
                                  int32 x, int32 y)
{
	if (fEngine == NULL) return B_ERROR;

	BAutolock <BDFBGraphicsEngine> autolock(fEngine);
	if (autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return dfb_stroke_point(dfbSurface, dc, x, y);
}


status_t
BDFBGraphicsDrawable::StrokePoints(BGraphicsContext *dc,
                                   const int32 *pts, int32 count)
{
	if (fEngine == NULL) return B_ERROR;

	BAutolock <BDFBGraphicsEngine> autolock(fEngine);
	if (autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return dfb_stroke_points(dfbSurface, dc, pts, count);
}


status_t
BDFBGraphicsDrawable::StrokePoints_Colors(BGraphicsContext *dc,
        const BList *ptsArrayLists, int32 arrayCount,
        const rgb_color *highColors)
{
	if (fEngine == NULL) return B_ERROR;

	BAutolock <BDFBGraphicsEngine> autolock(fEngine);
	if (autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return dfb_stroke_points_color(dfbSurface, dc, ptsArrayLists, arrayCount, highColors);
}


status_t
BDFBGraphicsDrawable::StrokePoints_Alphas(BGraphicsContext *dc,
        const int32 *pts, const uint8 *alpha, int32 count)
{
	if (fEngine == NULL) return B_ERROR;

	BAutolock <BDFBGraphicsEngine> autolock(fEngine);
	if (autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return dfb_stroke_points_alphas(dfbSurface, dc, pts, alpha, count);
}


status_t
BDFBGraphicsDrawable::StrokeLine(BGraphicsContext *dc,
                                 int32 x0, int32 y0, int32 x1, int32 y1)
{
	if (fEngine == NULL) return B_ERROR;

	BAutolock <BDFBGraphicsEngine> autolock(fEngine);
	if (autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return dfb_stroke_line(dfbSurface, dc, x0, y0, x1, y1);
}


status_t
BDFBGraphicsDrawable::StrokePolygon(BGraphicsContext *dc,
                                    const int32 *pts, int32 count, bool closed)
{
	if (fEngine == NULL) return B_ERROR;

	BAutolock <BDFBGraphicsEngine> autolock(fEngine);
	if (autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return dfb_stroke_polygon(dfbSurface, dc, pts, count, closed);
}


status_t
BDFBGraphicsDrawable::FillPolygon(BGraphicsContext *dc,
                                  const int32 *pts, int32 count)
{
	if (fEngine == NULL) return B_ERROR;

	BAutolock <BDFBGraphicsEngine> autolock(fEngine);
	if (autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return dfb_fill_polygon(dfbSurface, dc, pts, count);
}


status_t
BDFBGraphicsDrawable::StrokeRect(BGraphicsContext *dc,
                                 int32 x, int32 y, uint32 w, uint32 h)
{
	if (fEngine == NULL) return B_ERROR;

	BAutolock <BDFBGraphicsEngine> autolock(fEngine);
	if (autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return dfb_stroke_rect(dfbSurface, dc, x, y, w, h);
}


status_t
BDFBGraphicsDrawable::FillRect(BGraphicsContext *dc,
                               int32 x, int32 y, uint32 w, uint32 h)
{
	if (fEngine == NULL) return B_ERROR;

	BAutolock <BDFBGraphicsEngine> autolock(fEngine);
	if (autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return dfb_fill_rect(dfbSurface, dc, x, y, w, h);
}


status_t
BDFBGraphicsDrawable::StrokeRects(BGraphicsContext *dc,
                                  const int32 *rects, int32 count)
{
	if (fEngine == NULL) return B_ERROR;

	BAutolock <BDFBGraphicsEngine> autolock(fEngine);
	if (autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return dfb_stroke_rects(dfbSurface, dc, rects, count);
}


status_t
BDFBGraphicsDrawable::FillRects(BGraphicsContext *dc,
                                const int32 *rects, int32 count)
{
	if (fEngine == NULL) return B_ERROR;

	BAutolock <BDFBGraphicsEngine> autolock(fEngine);
	if (autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return dfb_fill_rects(dfbSurface, dc, rects, count);
}


status_t
BDFBGraphicsDrawable::FillRegion(BGraphicsContext *dc,
                                 const BRegion &region)
{
	if (fEngine == NULL) return B_ERROR;

	BAutolock <BDFBGraphicsEngine> autolock(fEngine);
	if (autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return dfb_fill_region(dfbSurface, dc, region);
}


status_t
BDFBGraphicsDrawable::StrokeRoundRect(BGraphicsContext *dc,
                                      int32 x, int32 y, uint32 w, uint32 h, uint32 xRadius, uint32 yRadius)
{
	if (fEngine == NULL) return B_ERROR;

	BAutolock <BDFBGraphicsEngine> autolock(fEngine);
	if (autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return dfb_stroke_round_rect(dfbSurface, dc, x, y, w, h, xRadius, yRadius);
}


status_t
BDFBGraphicsDrawable::FillRoundRect(BGraphicsContext *dc,
                                    int32 x, int32 y, uint32 w, uint32 h, uint32 xRadius, uint32 yRadius)
{
	if (fEngine == NULL) return B_ERROR;

	BAutolock <BDFBGraphicsEngine> autolock(fEngine);
	if (autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return dfb_fill_round_rect(dfbSurface, dc, x, y, w, h, xRadius, yRadius);
}


status_t
BDFBGraphicsDrawable::StrokeArc(BGraphicsContext *dc,
                                int32 x, int32 y, uint32 w, uint32 h, float startAngle, float endAngle)
{
	if (fEngine == NULL) return B_ERROR;

	BAutolock <BDFBGraphicsEngine> autolock(fEngine);
	if (autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return dfb_stroke_arc(dfbSurface, dc, x, y, w, h, startAngle, endAngle);
}


status_t
BDFBGraphicsDrawable::FillArc(BGraphicsContext *dc,
                              int32 x, int32 y, uint32 w, uint32 h, float startAngle, float endAngle)
{
	if (fEngine == NULL) return B_ERROR;

	BAutolock <BDFBGraphicsEngine> autolock(fEngine);
	if (autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return dfb_fill_arc(dfbSurface, dc, x, y, w, h, startAngle, endAngle);
}

