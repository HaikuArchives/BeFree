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
 * File: StringView.cpp
 *
 * --------------------------------------------------------------------------*/

#include "StringView.h"

#define ETK_STRING_VIEW_LINE_SPACING	0.25f


BStringView::BStringView(BRect frame, const char *name, const char *initial_text, uint32 resizeMode, uint32 flags)
		: BView(frame, name, resizeMode, flags), fTextArray(NULL), fAlignment(B_ALIGN_LEFT), fVerticalAlignment(B_ALIGN_TOP)
{
	if (initial_text) {
		fText = initial_text;
		if (fText.Length() > 0) fTextArray = fText.Split('\n');
	}

	SetHighColor(ui_color(B_PANEL_TEXT_COLOR));
	SetLowColor(ViewColor());
}


BStringView::~BStringView()
{
	if (fTextArray) delete fTextArray;
}


void
BStringView::SetText(const char *text)
{
	if (fText != text) {
		if (fTextArray) delete fTextArray;
		fTextArray = NULL;
		fText = text;
		if (fText.Length() > 0) fTextArray = fText.Split('\n');
		Invalidate();
	}
}


const char*
BStringView::Text() const
{
	return fText.String();
}


void
BStringView::SetAlignment(e_alignment alignment)
{
	if (fAlignment != alignment) {
		fAlignment = alignment;
		Invalidate();
	}
}


e_alignment
BStringView::Alignment() const
{
	return fAlignment;
}


void
BStringView::SetVerticalAlignment(e_vertical_alignment alignment)
{
	if (fVerticalAlignment != alignment) {
		fVerticalAlignment = alignment;
		Invalidate();
	}
}


e_vertical_alignment
BStringView::VerticalAlignment() const
{
	return fVerticalAlignment;
}


void
BStringView::Draw(BRect updateRect)
{
	if (Window() == NULL || !fTextArray || fTextArray->CountItems() <= 0) return;

	BRegion clipping;
	GetClippingRegion(&clipping);
	if (clipping.CountRects() > 0) clipping &= updateRect;
	else clipping = updateRect;
	if (clipping.CountRects() <= 0) return;

	rgb_color fgColor = HighColor();

	if (!IsEnabled()) {
		rgb_color color = ViewColor();
		color.alpha = 127;
		fgColor.mix(color);
	}

	BFont font;
	font_height fontHeight;
	GetFont(&font);
	font.GetHeight(&fontHeight);
	float sHeight = fontHeight.ascent + fontHeight.descent;

	float allHeight = (float)(fTextArray->CountItems() - 1) * (float)ceil((double)(sHeight * ETK_STRING_VIEW_LINE_SPACING)) +
	                  (float)(fTextArray->CountItems()) * sHeight;
	float lineHeight = sHeight + (float)ceil((double)(sHeight * ETK_STRING_VIEW_LINE_SPACING));

	BRect bounds = Frame().OffsetToSelf(B_ORIGIN);

	float yStart = 0;
	switch (fVerticalAlignment) {
		case B_ALIGN_BOTTOM:
			yStart = bounds.bottom - allHeight;
			break;

		case B_ALIGN_MIDDLE:
			yStart = bounds.Center().y - allHeight / 2.f;
			break;

		default:
			break;
	}


	PushState();
	ConstrainClippingRegion(&clipping);
	SetDrawingMode(B_OP_COPY);
	SetHighColor(fgColor);
	SetLowColor(ViewColor());
	for (int32 i = 0; i < fTextArray->CountItems(); i++) {
		const BString *str = fTextArray->ItemAt(i);
		float strWidth = 0;
		if (!(!str || str->Length() <= 0 || (strWidth = font.StringWidth(str->String())) <= 0)) {
			float xStart = 0;
			switch (fAlignment) {
				case B_ALIGN_RIGHT:
					xStart = bounds.right - strWidth;
					break;

				case B_ALIGN_CENTER:
					xStart = bounds.Center().x - strWidth / 2.f;
					break;

				default:
					break;
			}
			DrawString(str->String(), BPoint(xStart, yStart + fontHeight.ascent + 1));
		}
		yStart += lineHeight;
	}
	PopState();
}


void
BStringView::SetFont(const BFont *font, uint8 mask)
{
	BFont fontPrev;
	BFont fontCurr;
	GetFont(&fontPrev);
	BView::SetFont(font, mask);
	GetFont(&fontCurr);

	if (fontPrev != fontCurr) Invalidate();
}


void
BStringView::GetPreferredSize(float *width, float *height)
{
	if (!width && !height) return;

	BFont font;
	GetFont(&font);

	if (width) {
		*width = 0;
		if (fTextArray != NULL) for (int32 i = 0; i < fTextArray->CountItems(); i++) {
				const BString *str = fTextArray->ItemAt(i);
				if (str) *width = max_c(*width, (float)ceil((double)font.StringWidth(str->String())));
			}
	}

	if (height) {
		font_height fontHeight;
		font.GetHeight(&fontHeight);
		float sHeight = fontHeight.ascent + fontHeight.descent;

		if (fTextArray == NULL || fTextArray->CountItems() <= 0)
			*height = 0;
		else
			*height = (float)(fTextArray->CountItems() - 1) * (float)ceil((double)(sHeight * ETK_STRING_VIEW_LINE_SPACING)) +
			          (float)(fTextArray->CountItems()) * sHeight;
	}
}

