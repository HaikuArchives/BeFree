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
 * File: ColorControl.cpp
 * Description: BColorControl --- Displays a palette of selectable colors
 *
 * --------------------------------------------------------------------------*/

#include <support/String.h>
#include <render/Pixmap.h>

#include "Bitmap.h"
#include "ColorControl.h"


BColorControl::BColorControl(BPoint leftTop, const char *name, BMessage *message, bool bufferedDrawing)
		: BControl(BRect(leftTop, leftTop), name, NULL, message, B_FOLLOW_NONE, B_WILL_DRAW |B_UPDATE_WITH_REGION), fBitmap(NULL)
{
	SetValue(0);

	float width = 0, height = 0;
	BColorControl::GetPreferredSize(&width, &height);
	ResizeTo(width, height);

	if (bufferedDrawing) {
		BRect aRect;
#if defined(ETK_BIG_ENDIAN)
		BPixmap *pixmap = new BPixmap(256, 60, B_RGB24_BIG);
#else
		BPixmap *pixmap = new BPixmap(256, 60, B_RGB24);
#endif
		pixmap->SetDrawingMode(B_OP_COPY);

		for (uint8 i = 0; i < 4; i++) {
			uint8 j = 0;
			do {
				aRect.SetLeftTop(BPoint((float)j, 15.f * (float)i));
				aRect.SetRightBottom(aRect.LeftTop() + BPoint(0.f, 14.f));

				rgb_color color;
				color.set_to(i <= 1 ? j : 0, (i == 0 || i == 2) ? j : 0, (i == 0 || i == 3) ? j : 0);
				pixmap->SetHighColor(color);
				pixmap->FillRect(aRect);
			} while (j == 255 ? false : (j++, true));
		}

		fBitmap = new BBitmap(pixmap);
		delete pixmap;
	}
}


BColorControl::~BColorControl()
{
	if (fBitmap) delete fBitmap;
}


void
BColorControl::Draw(BRect updateRect)
{
	if (IsFocusChanging()) return;
	_DrawColors(updateRect);
	_DrawDescription(updateRect);
}


void
BColorControl::GetPreferredSize(float *width, float *height)
{
	if (width == NULL && height == NULL) return;

	BFont font;
	GetFont(&font);

	if (height) {
		font_height fontHeight;
		font.GetHeight(&fontHeight);

		*height = max_c(61.f, 3.f * (fontHeight.ascent + fontHeight.descent) + 17.f);
	}

	if (width) {
		*width = 280.f;
		*width += max_c(font.StringWidth("R:"), max_c(font.StringWidth("G:"), font.StringWidth("B:"))) + font.StringWidth("255");
	}
}


BRect
BColorControl::_MarkFrame(BRect colorsFrame, uint8 channel)
{
	BRect r;

	if (colorsFrame.IsValid()) {
		rgb_color fColor = ValueAsColor();

		if (channel == 0) { /* red */
			BPoint pt(colorsFrame.left + 1.f + (float)fColor.red, colorsFrame.top + 23.f);
			r.SetLeftTop(pt - BPoint(3.f, 3.f));
			r.SetRightBottom(pt + BPoint(3.f, 3.f));
		} else if (channel == 1) { /* green */
			BPoint pt(colorsFrame.left + 1.f + (float)fColor.green, colorsFrame.top + 38.f);
			r.SetLeftTop(pt - BPoint(3.f, 3.f));
			r.SetRightBottom(pt + BPoint(3.f, 3.f));
		} else if (channel == 2) { /* blue */
			BPoint pt(colorsFrame.left + 1.f + (float)fColor.blue, colorsFrame.top + 53.f);
			r.SetLeftTop(pt - BPoint(3.f, 3.f));
			r.SetRightBottom(pt + BPoint(3.f, 3.f));
		}
	}

	return r;
}


BRect
BColorControl::_ColorsFrame()
{
	BRect r = Frame().OffsetToSelf(B_ORIGIN);

	if (r.IsValid()) {
		font_height fontHeight;
		GetFontHeight(&fontHeight);

		r.bottom = r.top + max_c(61.f, 3.f * (fontHeight.ascent + fontHeight.descent) + 17.f);
		if (r.Height() > 61.f) {
			r.top += (r.Height() - 61.f) / 2.f;
			r.bottom = r.top + 61.f;
		}
		r.right = r.left + 257.f;
	}

	return r;
}


BRect
BColorControl::_DescriptionFrame()
{
	BRect r = Frame().OffsetToSelf(B_ORIGIN);

	if (r.IsValid()) {
		BFont font;
		font_height fontHeight;
		GetFont(&font);
		font.GetHeight(&fontHeight);

		r.right = (r.left += 267.f);
		r.right += max_c(font.StringWidth("R:"), max_c(font.StringWidth("G:"), font.StringWidth("B:"))) + 5.f;
		r.right += font.StringWidth("255") + 8.f;

		float h = 3.f * (fontHeight.ascent + fontHeight.descent) + 17.f;
		if (h < 61.f) r.top += (61.f - h) / 2.f;
		r.bottom = r.top + h;
	}

	return r;
}


void
BColorControl::_DrawColors(BRect r)
{
	BRect rect = _ColorsFrame();
	r &= rect;
	if (r.IsValid() == false) return;

	PushState();

	SetPenSize(0);
	SetDrawingMode(B_OP_COPY);
	ConstrainClippingRegion(r);

	if (fBitmap == NULL) {
		BRect aRect;
		for (uint8 i = 0; i < 4; i++) {
			uint8 j = 0;
			do {
				aRect.SetLeftTop(BPoint(1.f + (float)j, rect.top + 1.f + 15.f * (float)i));
				aRect.SetRightBottom(aRect.LeftTop() + BPoint(0.f, 14.f));
				if (aRect.Intersects(r) == false) continue;

				rgb_color color;
				color.set_to(i <= 1 ? j : 0, (i == 0 || i == 2) ? j : 0, (i == 0 || i == 3) ? j : 0);
				SetHighColor(color);
				StrokeLine(aRect.LeftTop(), aRect.RightBottom());
			} while (j == 255 ? false : (j++, true));
		}
	} else {
		BRect aRect = rect.InsetByCopy(1.f, 1.f);
		if (aRect.Intersects(r)) DrawBitmap(fBitmap, aRect.OffsetToCopy(B_ORIGIN), aRect);
	}

	for (uint8 i = 0; i < 3; i++) {
		BRect markFrame = _MarkFrame(rect, i);
		if (markFrame.Intersects(r) == false) continue;
		SetHighColor(255, 255, 255);
		StrokeEllipse(markFrame);
	}

	if (rect.InsetByCopy(1.f, 1.f).Contains(r) == false) {
		rgb_color shineColor = ui_color(B_SHINE_COLOR);
		rgb_color shadowColor = ui_color(B_SHADOW_COLOR);

		if (!IsEnabled()) {
			shineColor.disable(ViewColor());
			shadowColor.disable(ViewColor());
		}

		SetHighColor(shadowColor);
		StrokeRect(rect);
		SetHighColor(shineColor);
		StrokeLine(rect.LeftBottom(), rect.RightBottom());
		StrokeLine(rect.RightTop());
	}

	PopState();
}


void
BColorControl::_DrawDescription(BRect r)
{
	BRect rect = _DescriptionFrame();
	r &= rect;
	if (r.IsValid() == false) return;

	font_height fontHeight;
	GetFontHeight(&fontHeight);

	PushState();

	rgb_color shineColor = ui_color(B_SHINE_COLOR);
	rgb_color shadowColor = ui_color(B_SHADOW_COLOR);
	rgb_color textColor = ui_color(B_PANEL_TEXT_COLOR);

	if (!IsEnabled()) {
		shineColor.disable(ViewColor());
		shadowColor.disable(ViewColor());
		textColor.disable(ViewColor());
	}

	SetPenSize(0);
	SetDrawingMode(B_OP_COPY);
	ConstrainClippingRegion(r);
	SetHighColor(textColor);

	float x = rect.left;

	MovePenTo(rect.left, rect.top + fontHeight.ascent + 4.f);
	DrawString("R:");
	x = max_c(PenLocation().x, x);

	MovePenTo(rect.left, PenLocation().y + fontHeight.ascent + fontHeight.descent + 5.f);
	DrawString("G:");
	x = max_c(PenLocation().x, x);

	MovePenTo(rect.left, PenLocation().y + fontHeight.ascent + fontHeight.descent + 5.f);
	DrawString("B:");
	x = max_c(PenLocation().x, x);

	BString str;
	x += 9.f;

	rgb_color fColor = ValueAsColor();

	str << fColor.red;
	MovePenTo(x, rect.top + fontHeight.ascent + 4.f);
	DrawString(str.String());

	str.MakeEmpty();
	str << fColor.green;
	MovePenTo(x, PenLocation().y + fontHeight.ascent + fontHeight.descent + 5.f);
	DrawString(str.String());

	str.MakeEmpty();
	str << fColor.blue;
	MovePenTo(x, PenLocation().y + fontHeight.ascent + fontHeight.descent + 5.f);
	DrawString(str.String());

	BRect aRect;
	x -= 4.f;

	aRect.SetLeftTop(BPoint(x, rect.top));
	aRect.SetRightBottom(BPoint(rect.right, rect.top + fontHeight.ascent + fontHeight.descent + 4.f));
	for (int8 i = 0; i < 3; i++) {
		if (aRect.Intersects(r)) {
			SetHighColor(shadowColor);
			StrokeRect(aRect);
			SetHighColor(shineColor);
			StrokeLine(aRect.LeftBottom(), aRect.RightBottom());
			StrokeLine(aRect.RightTop());
		}

		aRect.OffsetBy(0, aRect.Height() + 2.f);
	}

	PopState();
}


void
BColorControl::SetValue(int32 color)
{
	if (color != Value()) {
		BControl::SetValueNoUpdate(color);
		Invalidate();
		if (Message() != NULL) Invoke();
	}
}


void BColorControl::SetValue(rgb_color color)
{
	int32 c = ((int32)color.red << 16) | ((int32)color.green << 8) | ((int32)color.blue);
	SetValue(c);
}


rgb_color BColorControl::ValueAsColor()
{
	int32 c = Value();
	return e_makrgb_color((c >> 16) & 0xff, (c >> 8) & 0xff, c & 0xff);
}


void
BColorControl::MouseDown(BPoint where)
{
	if (IsEnabled() == false) return;

	BRect rect = _ColorsFrame();
	rect.InsetBy(-5, -2);
	if (rect.Contains(where) == false) return;
	rect.InsetBy(6, 3);

	BPoint ptOffset = where - rect.LeftTop();
	if (ptOffset.x < 0) ptOffset.x = 0;
	if (ptOffset.x > 255) ptOffset.x = 255;
	if (ptOffset.y < 0) ptOffset.y = 0;
	if (ptOffset.y >= 60) ptOffset.y = 59;

	rgb_color color = ValueAsColor();

	if (ptOffset.y < 15.f) /* gray */
		color.red = color.green = color.blue = (uint8)ptOffset.x;
	else if (ptOffset.y < 30.f) /* red */
		color.red = (uint8)ptOffset.x;
	else if (ptOffset.y < 45.f) /* green */
		color.green = (uint8)ptOffset.x;
	else if (ptOffset.y < 60.f) /* blue */
		color.blue = (uint8)ptOffset.x;

	SetValue(color);
}

