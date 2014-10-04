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
 * File: Control.cpp
 *
 * --------------------------------------------------------------------------*/

#include <support/String.h>
#include <kernel/OS.h>
#include <app/Looper.h>

#include "Window.h"
#include "Control.h"


BControl::BControl(BRect frame, const char *name, const char *label,
                   BMessage *message, uint32 resizeMode, uint32 flags)
		: BView(frame, name, resizeMode, flags), BInvoker(message, NULL, NULL),
		fLabel(NULL), fValue(B_CONTROL_OFF), fFocusChanging(false)
{
	if (label) fLabel = EStrdup(label);
}


BControl::~BControl()
{
	if (fLabel) delete[] fLabel;
}


void
BControl::SetLabel(const char *label)
{
	if (fLabel) delete[] fLabel;
	if (label)
		fLabel = EStrdup(label);
	else
		fLabel = NULL;
}


const char*
BControl::Label() const
{
	return fLabel;
}


void
BControl::SetValue(int32 value)
{
	if (fValue != value) {
		fValue = value;
		if ((Flags() &B_WILL_DRAW) && Window() != NULL) Draw(Bounds());
	}
}


int32
BControl::Value() const
{
	return fValue;
}


status_t
BControl::Invoke(const BMessage *aMsg)
{
	bool IsNotify = false;
	uint32 kind = InvokeKind(&IsNotify);

	BMessage msg(kind);
	status_t status = B_BAD_VALUE;

	if (!aMsg && !IsNotify) aMsg = Message();

	if (!aMsg) {
		if (!IsNotify || !IsWatched()) return status;
	} else {
		msg = *aMsg;
	}

	msg.AddInt64("when", e_real_time_clock_usecs());
	msg.AddPointer("source", this);
	if (aMsg) status = BInvoker::Invoke(&msg);

	if (IsNotify) SendNotices(kind, &msg);

	return status;
}


void
BControl::AttachedToWindow()
{
	if (Target() == NULL) SetTarget(Window());
}


void
BControl::DetachedFromWindow()
{
	if (Target() == Window()) SetTarget(NULL);
}


void
BControl::MakeFocus(bool focusState)
{
	if (IsFocus() != focusState) {
		BView::MakeFocus(focusState);

		if (IsVisible() && (Flags() &B_WILL_DRAW)) {
			BRegion aRegion = VisibleBoundsRegion();
			if (aRegion.CountRects() <= 0) return;

			fFocusChanging = true;
			if (Flags() &B_UPDATE_WITH_REGION)
				for (int32 i = 0; i < aRegion.CountRects(); i++) Draw(aRegion.RectAt(i));
			else
				Draw(aRegion.Frame());
			fFocusChanging = false;
		}
	}
}


bool
BControl::IsFocusChanging() const
{
	return fFocusChanging;
}


void
BControl::SetValueNoUpdate(int32 value)
{
	fValue = value;
}


