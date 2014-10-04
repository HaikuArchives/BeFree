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
 * File: Box.cpp
 *
 * --------------------------------------------------------------------------*/

#include <support/ClassInfo.h>
#include <interface/StringView.h>
#include <add-ons/theme/ThemeEngine.h>

#include "Box.h"

BBox::BBox(BRect frame, const char *name, uint32 resizingMode, uint32 flags, border_style border)
		: BView(frame, name, resizingMode, flags), fLabelView(NULL), fBorder(B_NO_BORDER), fAlignment(B_ALIGN_LEFT)
{
	fBorder = border;
}


BBox::~BBox()
{
}


void
BBox::SetBorder(border_style border)
{
	if (fBorder != border) {
		fBorder = border;
		Invalidate();
	}
}


border_style
BBox::Border() const
{
	return fBorder;
}


void
BBox::SetLabelAlignment(e_alignment labelAlignment)
{
	if (fAlignment != labelAlignment) {
		fAlignment = labelAlignment;
		ReAdjustLabel();
	}
}


e_alignment
BBox::LabelAlignment() const
{
	return fAlignment;
}


void
BBox::SetLabel(const char *label)
{
	if (!(label == NULL || *label == 0)) {
		BStringView *strView = cast_as(fLabelView, BStringView);
		if (strView != NULL) {
			strView->SetText(label);
			strView->ResizeToPreferred();
			ReAdjustLabel();
			return;
		}

		if ((strView = new BStringView(BRect(0, 0, 1, 1), NULL, label, B_FOLLOW_NONE)) == NULL) return;
		strView->SetFont(be_bold_font);
		strView->ResizeToPreferred();
		if (SetLabel(strView) != B_OK) delete strView;
	} else if (fLabelView != NULL) {
		BView *view = fLabelView;
		fLabelView = NULL;

		view->RemoveSelf();
		delete view;
	}
}


status_t
BBox::SetLabel(BView *viewLabel)
{
	if (viewLabel != NULL) {
		if (viewLabel == this || viewLabel->Window() != NULL || viewLabel->Parent() != NULL) return B_ERROR;
		AddChild(viewLabel, ChildAt(0));
		if (viewLabel->Parent() != this) return B_ERROR;
		viewLabel->SetResizingMode(B_FOLLOW_NONE);
	} else if (fLabelView == NULL) {
		return B_OK;
	}

	if (fLabelView != NULL) {
		BView *view = fLabelView;
		fLabelView = NULL;

		view->RemoveSelf();
		delete view;
	}

	fLabelView = viewLabel;
	ReAdjustLabel();

	return B_OK;
}


const char*
BBox::Label() const
{
	if (fLabelView == NULL) return NULL;

	BStringView *strView = cast_as(fLabelView, BStringView);
	if (strView == NULL) return NULL;

	return strView->Text();
}


BView*
BBox::LabelView() const
{
	return fLabelView;
}


BRect
BBox::ContentBounds() const
{
	e_theme_engine *theme = get_current_theme_engine();

	float l = 0, t = 0, r = 0, b = 0;
	if (!(theme == NULL || theme->get_border_margins == NULL))
		theme->get_border_margins(theme, this, &l, &t, &r, &b, fBorder, PenSize());

	float labelHeight = ((fLabelView == NULL || fLabelView->Frame().Width() <= 0) ? 0.f : fLabelView->Frame().Height());

	BRect bounds = Frame().OffsetToSelf(B_ORIGIN);
	bounds.left += l;
	bounds.top += max_c(t, labelHeight);
	bounds.right -= r;
	bounds.bottom -= b;

	return bounds;
}


void
BBox::Draw(BRect updateRect)
{
	if (!IsVisible() || fBorder == B_NO_BORDER) return;

	e_theme_engine *theme = get_current_theme_engine();
	if (theme == NULL || theme->get_border_margins == NULL || theme->draw_border == NULL) return;

	float l = 0, t = 0, r = 0, b = 0;
	theme->get_border_margins(theme, this, &l, &t, &r, &b, fBorder, PenSize());

	BRect rect = Frame().OffsetToSelf(B_ORIGIN);
	if (!(fLabelView == NULL || fLabelView->Frame().Width() <= 0 || fLabelView->Frame().Height() < t))
		rect.top += (fLabelView->Frame().Height() - t) / 2.f;

	PushState();

	BRegion clipping(updateRect);
	if (!(fLabelView == NULL || fLabelView->Frame().IsValid() == false)) clipping.Exclude(fLabelView->Frame());
	ConstrainClippingRegion(&clipping);

	if (clipping.CountRects() > 0) theme->draw_border(theme, this, rect, fBorder, PenSize());

	PopState();
}


void
BBox::FrameResized(float new_width, float new_height)
{
	ReAdjustLabel();
}


void
BBox::ResizeToPreferred()
{
	if (fLabelView) fLabelView->ResizeToPreferred();
	BView::ResizeToPreferred();
}


void
BBox::GetPreferredSize(float *width, float *height)
{
	if (!width && !height) return;

	float w = 0, h = 0;
	if (fLabelView) fLabelView->GetPreferredSize(&w, &h);

	e_theme_engine *theme = get_current_theme_engine();

	float l = 0, t = 0, r = 0, b = 0;
	if (!(theme == NULL || theme->get_border_margins == NULL))
		theme->get_border_margins(theme, this, &l, &t, &r, &b, fBorder, PenSize());

	w += (l + r) + 2.f;
	if (h < t) h = t;
	h += b + 2.f;

	if (width) *width = w;
	if (height) *height = h;
}


void
BBox::ReAdjustLabel()
{
	if (fLabelView == NULL) return;

	switch (fAlignment) {
		case B_ALIGN_RIGHT:
			fLabelView->MoveTo(Frame().Width() - fLabelView->Frame().Width() - 5.f, 0);
			break;
		case B_ALIGN_CENTER:
			fLabelView->MoveTo((Frame().Width() - fLabelView->Frame().Width()) / 2.f, 0);
			break;
		default:
			fLabelView->MoveTo(5, 0);
	}
}


void
BBox::ChildRemoving(BView *child)
{
	if (fLabelView == child) fLabelView = NULL;
}

