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
 * File: View.h
 * Description: BView --- drawing/layout/control within BWindow
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_VIEW_H__
#define __ETK_VIEW_H__

#include <support/List.h>
#include <interface/InterfaceDefs.h>
#include <interface/Region.h>
#include <interface/Polygon.h>
#include <interface/Font.h>
#include <app/Handler.h>

enum {
	B_ENTERED_VIEW = 0,
	B_INSIDB_VIEW,
	B_EXITED_VIEW,
	B_OUTSIDE_VIEW
};

enum {
	B_POINTER_EVENTS = 1,
	B_KEYBOARD_EVENTS = 1 << 1
};

enum {
	B_LOCK_WINDOW_FOCUS	= 1,
	B_SUSPEND_VIEW_FOCUS	= 1 << 1,
	B_NO_POINTER_HISTORY	= 1 << 2
};

enum {
	B_WILL_DRAW			= 1,
	B_PULSE_NEEDED			= 1 << 1,
	B_NAVIGABLE_JUMP		= 1 << 2,
	B_NAVIGABLE			= 1 << 3,
	B_FRAME_EVENTS			= 1 << 4,
	B_UPDATE_WITH_REGION		= 1 << 5,
	B_DRAW_ON_CHILDREN		= 1 << 6,
	E_INPUT_METHOD_AWARE		= 1 << 7
};

enum {
	B_FONT_FAMILY_AND_STYLE		= 1,
	B_FONT_SIZE			= 1 << 1,
	B_FONT_SHEAR			= 1 << 2,
	B_FONT_SPACING     		= 1 << 3,
	B_FONT_ALL			= 0xff
};

#ifdef __cplusplus /* Just for C++ */

class BWindow;
class BGraphicsContext;
class BScrollView;
class BBitmap;
class BCursor;
class BLayoutItem;


class BView : public BHandler
{
	public:
		BView(BRect frame,
		      const char *name,
		      uint32 resizingMode,
		      uint32 flags);
		virtual ~BView();

		virtual void	MessageReceived(BMessage *msg);

		void		AddChild(BView *child, BView *childNextSibling = NULL);
		bool		RemoveChild(BView *child);
		bool		RemoveSelf();
		int32		CountChildren() const;
		BView		*ChildAt(int32 index) const;

		BView		*NextSibling() const;
		BView		*PreviousSibling() const;
		bool		IsSibling(const BView *sibling) const;

		BWindow		*Window() const;
		BView		*Parent() const;
		BView		*Ancestor() const;
		BView		*FindView(const char *name) const;

		BRect		Bounds() const;
		BRect		Frame() const;
		BPoint		LeftTop() const;

		bool		IsVisible() const;
		BRect		VisibleBounds() const;
		BRect		VisibleFrame() const;
		BRegion		VisibleBoundsRegion() const;
		BRegion		VisibleFrameRegion() const;

		// Empty functions BEGIN --- just for derivative class
		virtual void	AttachedToWindow();
		virtual void	AllAttached();
		virtual void	DetachedFromWindow();
		virtual void	AllDetached();
		virtual void	Draw(BRect updateRect);
		virtual void	DrawAfterChildren(BRect updateRect);
		virtual void	MouseDown(BPoint where);
		virtual void	MouseUp(BPoint where);
		virtual void	MouseMoved(BPoint where, uint32 code, const BMessage *a_message);
		virtual void	WindowActivated(bool state);
		virtual void	KeyDown(const char *bytes, int32 numBytes);
		virtual void	KeyUp(const char *bytes, int32 numBytes);
		virtual void	Pulse();
		virtual void	FrameMoved(BPoint new_position);
		virtual void	FrameResized(float new_width, float new_height);
		// Empty functions END

		virtual void	Show();
		virtual void	Hide();
		bool		IsHidden() const;

		virtual void	SetEnabled(bool state);
		bool		IsEnabled() const;

		void		ConvertToScreen(BPoint* pt) const;
		BPoint		ConvertToScreen(BPoint pt) const;
		void		ConvertFromScreen(BPoint* pt) const;
		BPoint		ConvertFromScreen(BPoint pt) const;

		void		ConvertToScreen(BRect *r) const;
		BRect		ConvertToScreen(BRect r) const;
		void		ConvertFromScreen(BRect *r) const;
		BRect		ConvertFromScreen(BRect r) const;

		void		ConvertToScreen(BRegion *region) const;
		BRegion		ConvertToScreen(const BRegion &region) const;
		void		ConvertFromScreen(BRegion *region) const;
		BRegion		ConvertFromScreen(const BRegion &region) const;

		void		ConvertToParent(BPoint *pt) const;
		BPoint		ConvertToParent(BPoint pt) const;
		void		ConvertFromParent(BPoint *pt) const;
		BPoint		ConvertFromParent(BPoint pt) const;

		void		ConvertToParent(BRect *r) const;
		BRect		ConvertToParent(BRect r) const;
		void		ConvertFromParent(BRect *r) const;
		BRect		ConvertFromParent(BRect r) const;

		void		ConvertToParent(BRegion *region) const;
		BRegion		ConvertToParent(const BRegion &region) const;
		void		ConvertFromParent(BRegion *region) const;
		BRegion		ConvertFromParent(const BRegion &region) const;

		void		ConvertToWindow(BPoint *pt) const;
		BPoint		ConvertToWindow(BPoint pt) const;
		void		ConvertFromWindow(BPoint *pt) const;
		BPoint		ConvertFromWindow(BPoint pt) const;

		void		ConvertToWindow(BRect *r) const;
		BRect		ConvertToWindow(BRect r) const;
		void		ConvertFromWindow(BRect *r) const;
		BRect		ConvertFromWindow(BRect r) const;

		void		ConvertToWindow(BRegion *region) const;
		BRegion		ConvertToWindow(const BRegion &region) const;
		void		ConvertFromWindow(BRegion *region) const;
		BRegion		ConvertFromWindow(const BRegion &region) const;

		status_t	SetEventMask(uint32 mask, uint32 options = 0);
		uint32		EventMask() const;
		status_t	GetMouse(BPoint *location, int32 *buttons, bool checkMessageQueue = true);
		bool		QueryCurrentMouse(bool pushed, int32 buttons, bool btnsAlone = true, int32 *clicks = NULL) const;

		// Next KeyUp(B_KEYBOARD_EVENTS) or MouseUp(B_POINTER_EVENTS) will restore the previous general event_mask.
		// If the current message isn't B_KEY_DOWN(B_UNMAPPED_KEY_DOWN) or B_MOUSE_DOWN, B_ERROR is return.
		// That's means: you should use this funtion within "KeyDown" or "MouseDown" etc...
		// The argument "mask" should not be "0" or any union.
		// That's means: you should not pass "mask" with "B_KEYBOARD_EVENTS|B_POINTER_EVENTS".
		status_t	SetPrivateEventMask(uint32 mask, uint32 options = 0);

		virtual void	SetFlags(uint32 flags);
		uint32		Flags() const;
		virtual void	SetResizingMode(uint32 mode);
		uint32		ResizingMode() const;
		void		MoveBy(float dh, float dv);
		void		MoveTo(BPoint where);
		void		MoveTo(float x, float y);
		void		ResizeBy(float dh, float dv);
		void		ResizeTo(float width, float height);
		void		ScrollBy(float dh, float dv);
		void		ScrollTo(float x, float y);
		virtual void	ScrollTo(BPoint where);
		virtual void	MakeFocus(bool focusState = true);
		bool		IsFocus() const;

		virtual void	SetDrawingMode(e_drawing_mode mode);
		e_drawing_mode	DrawingMode() const;

		void		MovePenTo(BPoint pt);
		void		MovePenTo(float x, float y);
		void		MovePenBy(float dx, float dy);
		BPoint		PenLocation() const;

		virtual void	SetPenSize(float size);
		float		PenSize() const;

		virtual void	SetViewColor(rgb_color c);
		void		SetViewColor(uint8 r, uint8 g, uint8 b, uint8 a = 255);
		rgb_color	ViewColor() const;

		virtual void	SetHighColor(rgb_color c);
		void		SetHighColor(uint8 r, uint8 g, uint8 b, uint8 a = 255);
		rgb_color	HighColor() const;

		virtual void	SetLowColor(rgb_color c);
		void		SetLowColor(uint8 r, uint8 g, uint8 b, uint8 a = 255);
		rgb_color	LowColor() const;

		void		PushState();
		void		PopState();

		void		Invalidate(BRect invalRect, bool redraw = true);
		void		Invalidate(bool redraw = true);

		// Note: The "Fill*()" functions isn't affected by the "PenSize()", it won't draw out of the edge.
		void		SetSquarePointStyle(bool state);
		bool		IsSquarePointStyle() const;
		void		StrokePoint(BPoint pt, pattern p = B_SOLID_HIGH);
		void		StrokePoints(const BPoint *pts, int32 count, const uint8 *alpha = NULL, pattern p = B_SOLID_HIGH);

		void		StrokeLine(BPoint pt, pattern p = B_SOLID_HIGH);
		void		StrokeLine(BPoint pt0, BPoint pt1, pattern p = B_SOLID_HIGH);

		void		StrokePolygon(const BPolygon *aPolygon, bool closed = true, pattern p = B_SOLID_HIGH);
		void		StrokePolygon(const BPoint *ptArray, int32 numPts, bool closed = true, pattern p = B_SOLID_HIGH);
		void		FillPolygon(const BPolygon *aPolygon, pattern p = B_SOLID_HIGH);
		void		FillPolygon(const BPoint *ptArray, int32 numPts, pattern p = B_SOLID_HIGH);

		void		StrokeTriangle(BPoint pt1, BPoint pt2, BPoint pt3, pattern p = B_SOLID_HIGH);
		void		FillTriangle(BPoint pt1, BPoint pt2, BPoint pt3, pattern p = B_SOLID_HIGH);

		void		StrokeRect(BRect r, pattern p = B_SOLID_HIGH);
		void		FillRect(BRect r, pattern p = B_SOLID_HIGH);

		void		StrokeRects(const BRect *rects, int32 count, pattern p = B_SOLID_HIGH);
		void		FillRects(const BRect *rects, int32 count, pattern p = B_SOLID_HIGH);
		void		FillRegion(const BRegion *region, pattern p = B_SOLID_HIGH);

		void		StrokeRoundRect(BRect r, float xRadius, float yRadius, pattern p = B_SOLID_HIGH);
		void		FillRoundRect(BRect r, float xRadius, float yRadius, pattern p = B_SOLID_HIGH);

		void		StrokeArc(BPoint ctPt, float xRadius, float yRadius, float startAngle, float arcAngle, pattern p = B_SOLID_HIGH);
		void		StrokeArc(BRect r, float startAngle, float arcAngle, pattern p = B_SOLID_HIGH);
		void		FillArc(BPoint ctPt, float xRadius, float yRadius, float startAngle, float arcAngle, pattern p = B_SOLID_HIGH);
		void		FillArc(BRect r, float start_angle, float arc_angle, pattern p = B_SOLID_HIGH);

		void		StrokeEllipse(BPoint ctPt, float xRadius, float yRadius, pattern p = B_SOLID_HIGH);
		void		StrokeEllipse(BRect r, pattern p = B_SOLID_HIGH);
		void		FillEllipse(BPoint ctPt, float xRadius, float yRadius, pattern p = B_SOLID_HIGH);
		void		FillEllipse(BRect r, pattern p = B_SOLID_HIGH);

		void		DrawString(const char *aString, int32 length = -1, float tabWidth = 0);
		void		DrawString(const char *aString, BPoint location, int32 length = -1, float tabWidth = 0);
		void		DrawString(const char *aString, int32 length, BPoint location, float tabWidth = 0);

		virtual void	SetFont(const BFont *font, uint8 mask = B_FONT_ALL);
		void		SetFont(const e_font_desc *fontDesc, uint8 mask = B_FONT_ALL);
		void		GetFont(BFont *font) const;
		void		SetFontSize(float size);
		void		GetFontHeight(font_height *height) const;
		void		ForceFontAliasing(bool enable);

		virtual void	GetPreferredSize(float *width, float *height);
		virtual void	ResizeToPreferred();

		void		GetClippingRegion(BRegion *clipping) const;
		void		ConstrainClippingRegion(const BRegion *clipping);
		void		ConstrainClippingRegion(BRect clipping);

		bool		IsPrinting() const;
		float		UnitsPerPixel() const;

		void		DrawBitmap(const BBitmap *bitmap);
		void		DrawBitmap(const BBitmap *bitmap, BPoint where);
		void		DrawBitmap(const BBitmap *bitmap, BRect destRect);
		void		DrawBitmap(const BBitmap *bitmap, BRect srcRect, BRect destRect);
		void		CopyBits(BRect srcRect, BRect destRect);

		void		Flush() const;
		void		Sync() const;

	protected:
		// Empty functions BEGIN --- just for derivative class
		virtual void	ChildRemoving(BView *child);
		virtual void	TargetedByScrollView(BScrollView *scroll_view);
		// Empty functions END

	private:
		friend class BWindow;
		friend class BScrollBar;
		friend class BScrollView;
		friend class BGraphicsEngine;
		friend class BViewLayout;

		BGraphicsContext *fDC;
		BLayoutItem *fLayout;

		void *fStates;

		uint32 fViewFlags;
		rgb_color fViewColor;
		bool fForceFontAliasing;
		BRegion fClippingTemp;
		bool fMouseInside;

		BList fScrollBar;
		bigtime_t fScrollTimeStamp;

		bool fMouseGrabbed;
		bool fKeyboardGrabbed;
		bool fEventStored;
		uint32 fEventMaskStored;
		uint32 fEventOptionsStored;
		uint32 fEventMask;
		uint32 fEventOptions;

		void AttachToWindow();
		void DetachFromWindow();
		void DrawStringInDirectlyMode(const char *aString, BPoint location, int32 length);
		void DrawStringInPixmapMode(const char *aString, BPoint location, int32 length);

		status_t _SetEventMask(uint32 mask, uint32 options);
		void _Expose(BRegion region, bigtime_t when);

		void _UpdateVisibleRegion();
		void _FrameChanged(BRect oldFrame, BRect newFrame);

		void InitSelf(BRect, uint32, uint32);
};

#endif /* __cplusplus */

#endif /* __ETK_VIEW_H__ */

