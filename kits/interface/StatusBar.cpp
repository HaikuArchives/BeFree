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
 * File: StatusBar.cpp
 *
 * --------------------------------------------------------------------------*/

#include <support/String.h>
#include <app/AppDefs.h>

#include "StatusBar.h"

BStatusBar::BStatusBar(BRect frame, const char *name, const char *label, const char *trailing_label)
		: BView(frame, name, B_FOLLOW_ALL, B_WILL_DRAW),
		fLabel(NULL), fTrailingLabel(NULL), fText(NULL), fTrailingText(NULL),
		fBarHeight(16), fMaxValue(100), fCurrentValue(0)
{
	if (label) fLabel = EStrdup(label);
	if (trailing_label) fTrailingLabel = EStrdup(trailing_label);
}


BStatusBar::~BStatusBar()
{
	if (fLabel) delete[] fLabel;
	if (fTrailingLabel) delete[] fTrailingLabel;
	if (fText) delete[] fText;
	if (fTrailingText) delete[] fTrailingText;
}


void
BStatusBar::SetBarHeight(float height)
{
	if (fBarHeight != height && height >= 0) fBarHeight = height;
}


void
BStatusBar::SetText(const char *str)
{
	if (fText) delete[] fText;
	fText = (str == NULL ? NULL : EStrdup(str));
}


void
BStatusBar::SetTrailingText(const char *str)
{
	if (fTrailingText) delete[] fTrailingText;
	fTrailingText = (str == NULL ? NULL : EStrdup(str));
}


void
BStatusBar::SetMaxValue(float max)
{
	if (fMaxValue != max && max >= 0) {
		fMaxValue = max;
		if (fCurrentValue > fMaxValue) fCurrentValue = fMaxValue;
	}
}


void
BStatusBar::Update(float delta, const char *text, const char *trailing_text)
{
	SetTo(max_c(0.f, min_c(fCurrentValue + delta, fMaxValue)), text, trailing_text);
}


void
BStatusBar::Reset(const char *label, const char *trailing_label)
{
	SetMaxValue(100);
	SetTo(0, "", "");

	if (fLabel) delete[] fLabel;
	fLabel = (label == NULL ? NULL : EStrdup(label));

	if (fTrailingLabel) delete[] fTrailingLabel;
	fTrailingLabel = (trailing_label == NULL ? NULL : EStrdup(trailing_label));
}


void
BStatusBar::SetTo(float value, const char *text, const char *trailing_text)
{
	if (value < 0 || value > fMaxValue) return;

	fCurrentValue = value;

	if (text) {
		if (fText) delete[] fText;
		fText = (strlen(text) > 0 ? EStrdup(text) : NULL);
	}

	if (trailing_text) {
		if (fTrailingText) delete[] fTrailingText;
		fTrailingText = (strlen(trailing_text) > 0 ? EStrdup(trailing_text) : NULL);
	}
}


float
BStatusBar::CurrentValue() const
{
	return fCurrentValue;
}


float
BStatusBar::MaxValue() const
{
	return fMaxValue;
}


float
BStatusBar::BarHeight() const
{
	return fBarHeight;
}


const char*
BStatusBar::Text() const
{
	return fText;
}


const char*
BStatusBar::TrailingText() const
{
	return fTrailingText;
}


const char*
BStatusBar::Label() const
{
	return fLabel;
}


const char*
BStatusBar::TrailingLabel() const
{
	return fTrailingLabel;
}


void
BStatusBar::MessageReceived(BMessage *msg)
{
	switch (msg->what) {
		case B_UPDATE_STATUS_BAR: {
			if (!IsEnabled()) break;

			float delta;
			const char *text = NULL, *trailing_text = NULL;
			if (msg->FindFloat("delta", &delta) == false) break;
			msg->FindString("text", &text);
			msg->FindString("trailing_text", &trailing_text);
			Update(delta, text, trailing_text);
			Invalidate();
		}
		break;

		case B_RESET_STATUS_BAR: {
			if (!IsEnabled()) break;

			const char *label = NULL, *trailing_label = NULL;
			msg->FindString("label", &label);
			msg->FindString("trailing_label", &trailing_label);
			Reset(label, trailing_label);
			Invalidate();
		}
		break;

		default:
			BView::MessageReceived(msg);
	}
}


void
BStatusBar::Draw(BRect updateRect)
{
	rgb_color shineColor = ui_color(B_SHINE_COLOR);
	rgb_color shadowColor = ui_color(B_SHADOW_COLOR);
	rgb_color barColor = ui_color(B_STATUSBAR_COLOR);

	BFont font;
	font_height fontHeight;

	GetFont(&font);
	font.GetHeight(&fontHeight);
	float sHeight = fontHeight.ascent + fontHeight.descent;

	if (!IsEnabled()) {
		shineColor.disable(ViewColor());
		shadowColor.disable(ViewColor());
		barColor.disable(ViewColor());
	}

	BPoint penLocation;

	MovePenTo(B_ORIGIN);
	MovePenBy(0, fontHeight.ascent + 1);

	if (fLabel != NULL) {
		if (!IsEnabled()) MovePenBy(1, 1);
		penLocation = PenLocation();

		SetHighColor(IsEnabled() ? ui_color(B_PANEL_TEXT_COLOR) : ui_color(B_SHINE_COLOR).disable(ViewColor()));
		SetLowColor(ViewColor());
		DrawString(fLabel);

		if (!IsEnabled()) {
			SetHighColor(ui_color(B_SHADOW_COLOR).disable(ViewColor()));
			DrawString(fLabel, penLocation - BPoint(1, 1));
		}

		MovePenBy(font.Spacing() * font.Size(), 0);
	}

	if (fText != NULL) {
		if (!IsEnabled()) MovePenBy(1, 1);
		penLocation = PenLocation();

		SetHighColor(IsEnabled() ? ui_color(B_PANEL_TEXT_COLOR) : ui_color(B_SHINE_COLOR).disable(ViewColor()));
		SetLowColor(ViewColor());
		DrawString(fText);

		if (!IsEnabled()) {
			SetHighColor(ui_color(B_SHADOW_COLOR).disable(ViewColor()));
			DrawString(fText, penLocation - BPoint(1, 1));
		}
	}

	MovePenTo(Frame().Width(), 0);
	MovePenBy(0, fontHeight.ascent + 1);

	if (fTrailingLabel != NULL) {
		MovePenBy(-(font.StringWidth(fTrailingLabel) + 1), 0);
		if (!IsEnabled()) MovePenBy(1, 1);
		penLocation = PenLocation();

		SetHighColor(IsEnabled() ? ui_color(B_PANEL_TEXT_COLOR) : ui_color(B_SHINE_COLOR).disable(ViewColor()));
		SetLowColor(ViewColor());
		DrawString(fTrailingLabel);

		if (!IsEnabled()) {
			SetHighColor(ui_color(B_SHADOW_COLOR).disable(ViewColor()));
			penLocation -= BPoint(1, 1);
			DrawString(fTrailingLabel, penLocation);
		}

		MovePenTo(penLocation + BPoint(-(font.Spacing() * font.Size()), 0));
	}

	if (fTrailingText != NULL) {
		MovePenBy(-(font.StringWidth(fTrailingText) + 1), 0);
		if (!IsEnabled()) MovePenBy(1, 1);
		penLocation = PenLocation();

		SetHighColor(IsEnabled() ? ui_color(B_PANEL_TEXT_COLOR) : ui_color(B_SHINE_COLOR).disable(ViewColor()));
		SetLowColor(ViewColor());
		DrawString(fTrailingText);

		if (!IsEnabled()) {
			SetHighColor(ui_color(B_SHADOW_COLOR).disable(ViewColor()));
			DrawString(fTrailingText, penLocation - BPoint(1, 1));
		}
	}

	BRect rect = Frame().OffsetToSelf(B_ORIGIN);
	if (fLabel != NULL || fTrailingLabel != NULL || fText != NULL || fTrailingText != NULL) rect.top += sHeight + 5;
	if (rect.IsValid() == false) return;

	rect.bottom = rect.top + fBarHeight;
	SetHighColor(shineColor.mix_copy(0, 0, 0, 5));
	FillRect(rect);
	SetHighColor(shineColor);
	StrokeRect(rect);
	SetHighColor(shadowColor);
	StrokeLine(rect.LeftBottom(), rect.RightBottom());
	StrokeLine(rect.RightTop());

	rect.InsetBy(1, 1);
	if (rect.IsValid() == false) return;

	BRect barRect = rect;
	SetHighColor(barColor);
	if (fCurrentValue < fMaxValue) barRect.right = barRect.left + barRect.Width() * fCurrentValue / fMaxValue;
	FillRect(barRect);
}


void
BStatusBar::GetPreferredSize(float *width, float *height)
{
	if (width == NULL && height == NULL) return;

	BFont font;
	GetFont(&font);
	font_height fontHeight;
	font.GetHeight(&fontHeight);
	float sHeight = fontHeight.ascent + fontHeight.descent;

	if (width) {
		*width = font.StringWidth(fLabel) + font.StringWidth(fTrailingLabel);
		*width += font.StringWidth(fText) + font.StringWidth(fTrailingText) + 20;
		if (*width < 50) *width = 50;
	}

	if (height) {
		*height = fBarHeight + 4;
		if (fLabel != NULL || fTrailingLabel != NULL || fText != NULL || fTrailingText != NULL) *height += sHeight + 5;
	}
}

