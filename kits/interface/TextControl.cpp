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
 * File: TextControl.cpp
 * Description: BTextControl --- display a labeled field could deliver a message as modifying
 *
 * --------------------------------------------------------------------------*/

#include <app/Application.h>
#include <app/Clipboard.h>

#include "Window.h"
#include "TextControl.h"


BTextControl::BTextControl(BRect frame, const char *name,
                           const char *label, const char *text, BMessage *message,
                           uint32 resizeMode, uint32 flags)
		: BTextEditable(frame, name, text, message, resizeMode, flags),
		fLabelAlignment(B_ALIGN_LEFT), fDivider(-1), fModificationMessage(NULL)
{
	SetTextAlignment(B_ALIGN_LEFT);
	SetLabel(label);
}


BTextControl::~BTextControl()
{
	if (fModificationMessage) delete fModificationMessage;
}


void
BTextControl::SetDivider(float divider)
{
	if (fDivider < 0 && divider < 0) return;

	if (fDivider != divider) {
		fDivider = divider;

		BFont font;
		GetFont(&font);
		font_height fontHeight;
		font.GetHeight(&fontHeight);
		float sHeight = fontHeight.ascent + fontHeight.descent;

		BRect margins(1, 1, 1, 1);
		margins.left += (fDivider >= 0 ? fDivider : max_c(font.StringWidth(Label()), 0)) + UnitsPerPixel();
		if (sHeight + 2.f * UnitsPerPixel() > Frame().Height())
			margins.top = margins.bottom = (sHeight - Frame().Height()) / 2.f + UnitsPerPixel();
		SetMargins(margins.left, margins.top, margins.right, margins.bottom);
	}
}


float
BTextControl::Divider() const
{
	return fDivider;
}


void
BTextControl::SetModificationMessage(BMessage *msg)
{
	if (msg == fModificationMessage) return;
	if (fModificationMessage) delete fModificationMessage;
	fModificationMessage = msg;
}


BMessage*
BTextControl::ModificationMessage() const
{
	return fModificationMessage;
}


void
BTextControl::SetText(const char *text)
{
	BTextEditable::SetText(text);
	if (fModificationMessage != NULL) Invoke(fModificationMessage);
}


void
BTextControl::SetLabel(const char *label)
{
	BRect margins(1, 1, 1, 1);

	BControl::SetLabel(label);

	BFont font;
	GetFont(&font);
	font_height fontHeight;
	font.GetHeight(&fontHeight);
	float sHeight = fontHeight.ascent + fontHeight.descent;
	margins.left += (fDivider >= 0 ? fDivider : max_c(font.StringWidth(Label()), 0)) + UnitsPerPixel();
	if (sHeight + 2.f * UnitsPerPixel() > Frame().Height())
		margins.top = margins.bottom = (sHeight - Frame().Height()) / 2.f + UnitsPerPixel();

	SetMargins(margins.left, margins.top, margins.right, margins.bottom);
}


void
BTextControl::Draw(BRect updateRect)
{
	if (Label() != NULL && fDivider != 0 && !IsFocusChanging()) {
		BFont font;
		GetFont(&font);
		font_height fontHeight;
		font.GetHeight(&fontHeight);
		float sHeight = fontHeight.ascent + fontHeight.descent;

		BRect rect = Frame().OffsetToSelf(B_ORIGIN);
		rect.right = (fDivider >= 0 ? fDivider : max_c(font.StringWidth(Label()), 0));

		BPoint penLocation;

		if (fLabelAlignment == B_ALIGN_LEFT || fDivider < 0)
			penLocation.x = 0;
		else if (fLabelAlignment == B_ALIGN_RIGHT)
			penLocation.x = rect.right - max_c(font.StringWidth(Label()), 0);
		else
			penLocation.x = (rect.right - max_c(font.StringWidth(Label()), 0)) / 2;

		penLocation.y = rect.Center().y - sHeight / 2.f;
		penLocation.y += fontHeight.ascent + 1;

		PushState();
		ConstrainClippingRegion(rect);
		SetHighColor(IsEnabled() ? ui_color(B_PANEL_TEXT_COLOR) : ui_color(B_SHINE_COLOR).disable(ViewColor()));
		SetLowColor(ViewColor());
		DrawString(Label(), penLocation);
		if (!IsEnabled()) {
			SetHighColor(ui_color(B_SHADOW_COLOR).disable(ViewColor()));
			DrawString(Label(), penLocation - BPoint(1, 1));
		}
		PopState();
	}

	BTextEditable::Draw(updateRect);

	if (IsFocusChanging()) return;

	rgb_color shineColor = ui_color(B_SHINE_COLOR);
	rgb_color shadowColor = ui_color(B_SHADOW_COLOR);

	if (!IsEnabled()) {
		shineColor.disable(ViewColor());
		shadowColor.disable(ViewColor());
	}

	float l, t, r, b;
	GetMargins(&l, &t, &r, &b);
	BRect rect = Frame().OffsetToSelf(B_ORIGIN);
	rect.left += l;
	rect.top += t;
	rect.right -= r;
	rect.bottom -= b;
	rect.InsetBy(-1, -1);

	SetHighColor(shineColor);
	StrokeRect(rect);
	SetHighColor(shadowColor);
	StrokeLine(rect.LeftBottom(), rect.LeftTop());
	StrokeLine(rect.RightTop());
}


void
BTextControl::GetPreferredSize(float *width, float *height)
{
	if (width) *width = 0;
	if (height) *height = 0;

	BTextEditable::GetPreferredSize(width, height);

	if (width != NULL) {
		BFont font;
		GetFont(&font);
		*width += (fDivider >= 0 ? fDivider : max_c(font.StringWidth(Label()), 0)) + 20;
	}

	if (height != NULL) {
		font_height fontHeight;
		GetFontHeight(&fontHeight);
		float sHeight = fontHeight.ascent + fontHeight.descent;
		if (sHeight + 10 > *height) *height = sHeight + 10;
	}
}


void
BTextControl::SetAlignment(e_alignment forLabel, e_alignment forText)
{
	if (forLabel == fLabelAlignment && TextAlignment() == forText) return;

	fLabelAlignment = forLabel;
	SetTextAlignment(forText);

	Invalidate();
}


void
BTextControl::GetAlignment(e_alignment *forLabel, e_alignment *forText) const
{
	if (forLabel) *forLabel = fLabelAlignment;
	if (forText) *forText = TextAlignment();
}


void
BTextControl::KeyDown(const char *bytes, int32 numBytes)
{
	BMessage *msg = (Window() == NULL ? NULL : Window()->CurrentMessage());

	int32 modifiers = 0;
	if (msg != NULL) msg->FindInt32("modifiers", &modifiers);

	if (!(numBytes != 1 || msg == NULL || msg->what != B_KEY_DOWN || !IsEnabled() || !IsEditable() || !(modifiers &B_CONTROL_KEY))) {
		if (*bytes == 'c' || *bytes == 'C' || *bytes == 'x' || *bytes == 'X') {
			int32 startPos, endPos;
			char *selText = NULL;
			BMessage *clipMsg = NULL;

			if (GetSelection(&startPos, &endPos) == false || clipboard.Lock() == false) return;
			if ((selText = DuplicateText(startPos, endPos)) != NULL && (clipMsg = clipboard.Data()) != NULL) {
				const char *text = NULL;
				ssize_t textLen = 0;
				if (clipMsg->FindData("text/plain", B_MIME_TYPE, (const void**)&text, &textLen) == false ||
				        text == NULL || textLen != (ssize_t)strlen(selText) || strncmp(text, selText, (size_t)textLen) != 0) {
					clipboard.Clear();
					clipMsg->AddData("text/plain", B_MIME_TYPE, selText, strlen(selText));
					clipboard.Commit();
				}
			}
			if (selText) free(selText);
			clipboard.Unlock();

			if (*bytes == 'x' || *bytes == 'X') {
				RemoveText(startPos, endPos);
				SetPosition(startPos);
			}

			return;
		} else if (*bytes == 'v' || *bytes == 'V') {
			BString str;
			BMessage *clipMsg = NULL;

			if (clipboard.Lock() == false) return;
			if ((clipMsg = clipboard.Data()) != NULL) {
				const char *text = NULL;
				ssize_t len = 0;
				if (clipMsg->FindData("text/plain", B_MIME_TYPE, (const void**)&text, &len)) str.SetTo(text, (int32)len);
			}
			clipboard.Unlock();

			if (str.Length() <= 0) return;

			int32 curPos = Position();
			int32 startPos, endPos;
			if (GetSelection(&startPos, &endPos)) {
				RemoveText(startPos, endPos);
				curPos = startPos;
			}
			InsertText(str.String(), -1, curPos);
			SetPosition(curPos + str.CountChars());

			return;
		}
	}

	BTextEditable::KeyDown(bytes, numBytes);
}

