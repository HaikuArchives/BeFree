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
 * File: RadioButton.cpp
 * Description: BRadioButton --- Radio buttons within their parent only one be on
 *
 * --------------------------------------------------------------------------*/

#include <support/ClassInfo.h>
#include <interface/Window.h>

#include "RadioButton.h"


BRadioButton::BRadioButton(BRect frame, const char *name, const char *label,
                           BMessage *message, uint32 resizeMode, uint32 flags)
		: BControl(frame, name, label, message, resizeMode, flags)
{
}


BRadioButton::~BRadioButton()
{
}


void
BRadioButton::SetLabel(const char *label)
{
	BControl::SetLabel(label);
	Invalidate();
}


void
BRadioButton::WindowActivated(bool state)
{
	Invalidate();
}


void
BRadioButton::Draw(BRect updateRect)
{
	if (Window() == NULL) return;

	BFont font;
	GetFont(&font);
	font_height fontHeight;
	font.GetHeight(&fontHeight);
	float sHeight = fontHeight.ascent + fontHeight.descent;

	BRect rect = Frame().OffsetToSelf(B_ORIGIN);
	rect.InsetBy(5, (rect.Height() - sHeight) / 2);
	if (rect.IsValid() == false) return;

	if ((IsFocus() || IsFocusChanging()) && IsEnabled() && Label() != NULL) {
		BPoint penLocation;
		penLocation.Set(rect.left + rect.Height() + 5, rect.Center().y + sHeight / 2 + 1);

		SetHighColor((IsFocus() && Window()->IsActivate()) ? ui_color(B_NAVIGATION_BASE_COLOR) : ViewColor());
		StrokeLine(penLocation, penLocation + BPoint(font.StringWidth(Label()), 0));
	}

	if (IsFocusChanging()) return;

	rgb_color shineColor = ui_color(B_SHINE_COLOR);
	rgb_color shadowColor = ui_color(B_SHADOW_COLOR);

	if (!IsEnabled()) {
		shineColor.disable(ViewColor());
		shadowColor.disable(ViewColor());
	}

	rect.right = rect.left + rect.Height();
	SetHighColor(shineColor.mix_copy(0, 0, 0, 5));
	FillEllipse(rect);
	SetHighColor(shineColor);
	StrokeArc(rect, 225, 180);
	SetHighColor(shadowColor);
	StrokeArc(rect, 45, 180);

	if (Value() == B_CONTROL_ON) {
		SetHighColor(shadowColor.mix_copy(255, 255, 255, 50));
		BRect r = rect.InsetByCopy(3, 3);
		FillEllipse(r);
	}

	if (Label() != NULL) {
		BPoint penLocation;
		penLocation.x = rect.right + 5;
		penLocation.y = rect.Center().y - sHeight / 2.f;
		penLocation.y += fontHeight.ascent + 1;

		SetHighColor(IsEnabled() ? ui_color(B_PANEL_TEXT_COLOR) : ui_color(B_SHINE_COLOR).disable(ViewColor()));
		SetLowColor(ViewColor());

		DrawString(Label(), penLocation);
		if (!IsEnabled()) {
			SetHighColor(ui_color(B_SHADOW_COLOR).disable(ViewColor()));
			DrawString(Label(), penLocation - BPoint(1, 1));
		}
	}
}

void
BRadioButton::MouseDown(BPoint where)
{
	if (!IsEnabled() || !QueryCurrentMouse(true, B_PRIMARY_MOUSE_BUTTON)) return;

#if 0
	font_height fontHeight;
	GetFontHeight(&fontHeight);
	float sHeight = fontHeight.ascent + fontHeight.descent;

	BRect rect = Frame().OffsetToSelf(B_ORIGIN);
	rect.InsetBy(5, (rect.Height() - sHeight) / 2);
	if (rect.IsValid() == false) return;
	rect.right = rect.left + rect.Height();
	if (rect.Contains(where) == false) return;
#endif

	if ((Flags() &B_NAVIGABLE) && !IsFocus()) MakeFocus();

//	SetValue((Value() == B_CONTROL_ON) ?B_CONTROL_OFF :B_CONTROL_ON);

	if (Value() == B_CONTROL_ON) return;
	SetValue(B_CONTROL_ON);

	Invoke();
}


void
BRadioButton::KeyDown(const char *bytes, int32 numBytes)
{
	if (!IsEnabled() || !IsFocus() || numBytes != 1) return;
	if (!(bytes[0] == B_ENTER || bytes[0] == B_SPACE)) return;

//	SetValue((Value() == B_CONTROL_ON) ?B_CONTROL_OFF :B_CONTROL_ON);

	if (Value() == B_CONTROL_ON) return;
	SetValue(B_CONTROL_ON);

	Invoke();
}


void
BRadioButton::SetFont(const BFont *font, uint8 mask)
{
	BControl::SetFont(font, mask);
	Invalidate();
}


void
BRadioButton::GetPreferredSize(float *width, float *height)
{
	if (width == NULL && height == NULL) return;

	BFont font;
	GetFont(&font);
	font_height fontHeight;
	font.GetHeight(&fontHeight);
	float sHeight = fontHeight.ascent + fontHeight.descent;

	if (width) *width = font.StringWidth(Label()) + sHeight + 20;
	if (height) *height = sHeight + 4;
}


void
BRadioButton::SetValue(int32 value)
{
	if (value != Value()) {
		BControl::SetValueNoUpdate(value);
		Invalidate();

		if (value == B_CONTROL_ON) {
			for (BView *sibling = NextSibling(); sibling != NULL; sibling = sibling->NextSibling()) {
				BRadioButton *rbtn = cast_as(sibling, BRadioButton);
				if (rbtn != NULL) rbtn->SetValue(B_CONTROL_OFF);
			}
			for (BView *sibling = PreviousSibling(); sibling != NULL; sibling = sibling->PreviousSibling()) {
				BRadioButton *rbtn = cast_as(sibling, BRadioButton);
				if (rbtn != NULL) rbtn->SetValue(B_CONTROL_OFF);
			}
		}
	}
}

