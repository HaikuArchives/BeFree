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
 * File: ScrollView.cpp
 *
 * --------------------------------------------------------------------------*/

#include "ScrollView.h"

BRect
BScrollView::TargetValidFrame(bool ignore_scrollbar) const
{
	// TODO: affect by border style

	if (fTarget == NULL) return BRect();

	BRect r = Frame().OffsetToSelf(B_ORIGIN);

	if (!ignore_scrollbar) {
		if (!(fVSB == NULL || fVSB->IsHidden())) r.right -= B_V_SCROLL_BAR_WIDTH + UnitsPerPixel();
		if (!(fHSB == NULL || fHSB->IsHidden())) r.bottom -= B_H_SCROLL_BAR_HEIGHT + UnitsPerPixel();
	}

	return r;
}


BScrollView::BScrollView(BRect frame, const char *name, BView *target, uint32 resizingMode, uint32 flags,
                         bool alwaysShowHorizontal, bool alwaysShowVertical, border_style border)
		: BView(frame, name, resizingMode, 0), fTarget(NULL)
{
	fBorder = border;
	fAlwaysShowHorizontal = alwaysShowHorizontal;
	fAlwaysShowVertical = alwaysShowVertical;

	BRect hR = Bounds();
	BRect vR = Bounds();
	hR.top = hR.bottom -B_H_SCROLL_BAR_HEIGHT;
	hR.right -= B_V_SCROLL_BAR_WIDTH;
	vR.left = vR.right -B_V_SCROLL_BAR_WIDTH;
	vR.bottom -= B_H_SCROLL_BAR_HEIGHT;

	fHSB = new BScrollBar(hR, NULL, 0, 0, 0, B_HORIZONTAL);
	fVSB = new BScrollBar(vR, NULL, 0, 0, 0, B_VERTICAL);
	fHSB->Hide();
	fVSB->Hide();
	AddChild(fHSB);
	AddChild(fVSB);

	if (fHSB->Parent() != this) {
		delete fHSB;
		fHSB = NULL;
	}
	if (fVSB->Parent() != this) {
		delete fVSB;
		fVSB = NULL;
	}

	flags |= B_FRAME_EVENTS;

	if (fBorder != B_NO_BORDER)
		BView::SetFlags(flags |B_WILL_DRAW);
	else
		BView::SetFlags(flags & ~B_WILL_DRAW);

	if (target != NULL) SetTarget(target);
}


BScrollView::~BScrollView()
{
	SetTarget(NULL);
}


status_t
BScrollView::SetTarget(BView *newTarget)
{
	if (newTarget == fTarget || newTarget == this) return B_ERROR;
	if (newTarget != NULL)
		if (newTarget->Window() != NULL || newTarget->Parent() != NULL) return B_ERROR;

	if (fTarget != NULL) {
		BView *target = fTarget;
		fTarget = NULL;

		target->RemoveSelf();
		delete target;
	}

	if (newTarget == NULL) {
		if (fHSB != NULL) {
			if (!fAlwaysShowHorizontal) fHSB->Hide();
			fHSB->SetRange(0, 0);
			fHSB->SetEnabled(false);
		}

		if (fVSB != NULL) {
			if (!fAlwaysShowVertical) fVSB->Hide();
			fVSB->SetRange(0, 0);
			fVSB->SetEnabled(false);
		}

		return B_OK;
	}

	fTarget = newTarget;
	AddChild(newTarget);
	if (newTarget->Parent() != this) {
		ETK_WARNING("[INTERFACE]: %s --- Unable to add target.", __PRETTY_FUNCTION__);
		fTarget = NULL;
		return B_ERROR;
	}

	if (fHSB != NULL) fHSB->SetTarget(fTarget);
	if (fVSB != NULL) fVSB->SetTarget(fTarget);
	BScrollView::FrameResized(Frame().Width(), Frame().Height());
	fTarget->TargetedByScrollView(this);

	return B_OK;
}


BView*
BScrollView::Target() const
{
	return fTarget;
}


void
BScrollView::SetBorder(border_style border)
{
	if (fBorder != border) {
		fBorder = border;

		if (fBorder != B_NO_BORDER)
			BView::SetFlags(Flags() |B_WILL_DRAW);
		else
			BView::SetFlags(Flags() & ~B_WILL_DRAW);

		Invalidate();
	}
}


border_style
BScrollView::Border() const
{
	return fBorder;
}


BScrollBar*
BScrollView::ScrollBar(orientation direction) const
{
	return(direction == B_HORIZONTAL ? fHSB : fVSB);
}


void
BScrollView::Draw(BRect updateRect)
{
	// TODO
}


void
BScrollView::FrameResized(float new_width, float new_height)
{
	if (fTarget == NULL) return;

	BRect targetFrame = fTarget->Frame();

	if (fHSB != NULL) {
		if (!fAlwaysShowHorizontal && TargetValidFrame(true).Width() >= targetFrame.Width()) {
			fHSB->SetValue(0);
			fHSB->Hide();
		} else {
			fHSB->Show();
			fHSB->SetEnabled(TargetValidFrame(true).Width() >= targetFrame.Width() ? false : true);
			fHSB->SetRange(0, max_c(targetFrame.Width() - TargetValidFrame(false).Width(), 0.f));
		}
	}

	if (fVSB != NULL) {
		if (!fAlwaysShowVertical && TargetValidFrame(true).Height() >= targetFrame.Height()) {
			fVSB->SetValue(0);
			fVSB->Hide();
		} else {
			fVSB->Show();
			fVSB->SetEnabled(TargetValidFrame(true).Height() >= targetFrame.Height() ? false : true);
			fVSB->SetRange(0, max_c(targetFrame.Height() - TargetValidFrame(false).Height(), 0.f));
		}
	}

	bool hsbHidden = (fHSB == NULL ? true : fHSB->IsHidden());
	bool vsbHidden = (fVSB == NULL ? true : fVSB->IsHidden());

	if ((hsbHidden != vsbHidden) && (hsbHidden || vsbHidden)) {
		if (vsbHidden && fHSB != NULL) {
			BRect hR = Frame().OffsetToSelf(B_ORIGIN);
			hR.top = hR.bottom -B_H_SCROLL_BAR_HEIGHT;
			fHSB->ResizeTo(hR.Width(), hR.Height());
			fHSB->MoveTo(hR.LeftTop());
		} else if (hsbHidden && fVSB != NULL) {
			BRect vR = Frame().OffsetToSelf(B_ORIGIN);
			vR.left = vR.right -B_V_SCROLL_BAR_WIDTH;
			fVSB->ResizeTo(vR.Width(), vR.Height());
			fVSB->MoveTo(vR.LeftTop());
		}
	} else {
		BRect hR = Frame().OffsetToSelf(B_ORIGIN);
		BRect vR = Frame().OffsetToSelf(B_ORIGIN);
		hR.top = hR.bottom -B_H_SCROLL_BAR_HEIGHT;
		hR.right -= B_V_SCROLL_BAR_WIDTH;
		vR.left = vR.right -B_V_SCROLL_BAR_WIDTH;
		vR.bottom -= B_H_SCROLL_BAR_HEIGHT;

		fHSB->ResizeTo(hR.Width(), hR.Height());
		fHSB->MoveTo(hR.LeftTop());
		fVSB->ResizeTo(vR.Width(), vR.Height());
		fVSB->MoveTo(vR.LeftTop());
	}
}


void
BScrollView::SetScrollBarAutoState(bool alwaysShowHorizontal, bool alwaysShowVertical)
{
	fAlwaysShowHorizontal = alwaysShowHorizontal;
	fAlwaysShowVertical = alwaysShowVertical;

	BScrollView::FrameResized(Frame().Width(), Frame().Height());
}


void
BScrollView::GetScrollBarAutoState(bool *alwaysShowHorizontal, bool *alwaysShowVertical) const
{
	if (alwaysShowHorizontal) *alwaysShowHorizontal = fAlwaysShowHorizontal;
	if (alwaysShowVertical) *alwaysShowVertical = fAlwaysShowVertical;
}


void
BScrollView::SetFlags(uint32 flags)
{
	flags |= B_FRAME_EVENTS;
	if (fBorder != B_NO_BORDER)
		flags |= B_WILL_DRAW;
	else
		flags &= ~B_WILL_DRAW;
	BView::SetFlags(flags);
}


void
BScrollView::ChildRemoving(BView *child)
{
	if (fHSB == child) {
		fHSB = NULL;
	} else if (fVSB == child) {
		fVSB = NULL;
	} else if (fTarget == child) {
		fTarget->TargetedByScrollView(NULL);
		fTarget = NULL;
	}
}


BRect
BScrollView::TargetFrame() const
{
	BRect r = TargetValidFrame(false);
	if (fTarget) r &= fTarget->ConvertToParent(fTarget->Frame().OffsetToSelf(B_ORIGIN));
	return r;
}

