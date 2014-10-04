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
 * File: ScrollBar.cpp
 *
 * --------------------------------------------------------------------------*/

#include <add-ons/theme/ThemeEngine.h>
#include <kernel/OS.h>
#include <interface/Window.h>

#include "ScrollBar.h"


BScrollBar::BScrollBar(BRect frame, const char *name, float value, float min, float max, orientation direction)
		: BView(frame, name, B_FOLLOW_NONE, B_WILL_DRAW),
		fStepSmall(1), fStepLarge(10), fTarget(NULL),
		fTracking(false), fTrackingState(0), fRunner(NULL)
{
	fRangeMin = min_c(min, max);
	fRangeMax = max_c(min, max);

	if (value < fRangeMin) value = fRangeMin;
	else if (value > fRangeMax) value = fRangeMax;
	fValue = value;

	fOrientation = direction;

	if (fOrientation == B_HORIZONTAL)
		SetResizingMode(B_FOLLOW_LEFT_RIGHT |B_FOLLOW_BOTTOM);
	else
		SetResizingMode(B_FOLLOW_TOP_BOTTOM |B_FOLLOW_RIGHT);
}


BScrollBar::~BScrollBar()
{
}


void
BScrollBar::_SetValue(float value, bool response)
{
	if (value < fRangeMin) value = fRangeMin;
	else if (value > fRangeMax) value = fRangeMax;

	if (fValue != value) {
		fValue = value;

		if (fTarget != NULL && response) {
			if (fOrientation == B_HORIZONTAL)
				fTarget->ScrollTo(fValue, fTarget->LeftTop().y);
			else
				fTarget->ScrollTo(fTarget->LeftTop().x, fValue);
		}

		Invalidate();

		ValueChanged(fValue);
	}
}


void
BScrollBar::SetValue(float value)
{
	_SetValue(value, true);
}


float
BScrollBar::Value() const
{
	return fValue;
}


void
BScrollBar::SetProportion(float ratio)
{
	if (ratio < 0 || ratio > 1) return;

	if (ratio == 0)
		SetValue(fRangeMin);
	else if (ratio == 1)
		SetValue(fRangeMax);
	else
		SetValue(fRangeMin + ratio * (fRangeMax - fRangeMin));
}


float
BScrollBar::Proportion() const
{
	float range = (fRangeMax - fRangeMin);
	if (range <= (UnitsPerPixel() * 0.1f)) return 0; // to avoid float divide error

	return((fValue - fRangeMin) / (fRangeMax - fRangeMin));
}


void
BScrollBar::ValueChanged(float value)
{
}


void
BScrollBar::SetRange(float min, float max)
{
	float rangeMin = min_c(min, max);
	float rangeMax = max_c(min, max);

	if (fRangeMin != rangeMin || fRangeMax != rangeMax) {
		fRangeMin = rangeMin;
		fRangeMax = rangeMax;

		if (fTarget != NULL && (fValue < fRangeMin || fValue > fRangeMax)) {
			fValue = (fValue < fRangeMin ? fRangeMin : fRangeMax);

			if (fOrientation == B_HORIZONTAL)
				fTarget->ScrollTo(fValue, fTarget->LeftTop().y);
			else
				fTarget->ScrollTo(fTarget->LeftTop().x, fValue);
		}

		Invalidate();
	}
}


void
BScrollBar::GetRange(float *min, float *max) const
{
	if (min != NULL) *min = fRangeMin;
	if (max != NULL) *max = fRangeMax;
}


void
BScrollBar::SetSteps(float smallStep, float largeStep)
{
	if (smallStep > 0)
		fStepSmall = smallStep;
	if (largeStep > 0)
		fStepLarge = largeStep;
}


void
BScrollBar::GetSteps(float *smallStep, float *largeStep) const
{
	if (smallStep != NULL) *smallStep = fStepSmall;
	if (largeStep != NULL) *largeStep = fStepLarge;
}


status_t
BScrollBar::SetTarget(BView *target)
{
	if (target == this) return B_ERROR;
	if (target == fTarget) return B_OK;

	if (target != NULL) {
		if (!(target->Ancestor() == Ancestor() || (target->Window() == Window() && Window() != NULL))) {
			ETK_WARNING("[INTERFACE]: %s --- target hasn't same ancestor as this.", __PRETTY_FUNCTION__);
			return B_BAD_VALUE;
		}
		if (target->fScrollBar.AddItem(this) == false) return B_ERROR;
	}

	if (fTarget != NULL) fTarget->fScrollBar.RemoveItem(this);
	fTarget = target;

	if (fTarget != NULL) {
		if (fOrientation == B_HORIZONTAL)
			fTarget->ScrollTo(fValue, fTarget->LeftTop().y);
		else
			fTarget->ScrollTo(fTarget->LeftTop().x, fValue);
	}

	return B_OK;
}


BView*
BScrollBar::Target() const
{
	return fTarget;
}


orientation
BScrollBar::Orientation() const
{
	return fOrientation;
}


void
BScrollBar::Draw(BRect updateRect)
{
	if (!IsVisible()) return;

	e_theme_engine *theme = get_current_theme_engine();
	if (theme == NULL || theme->draw_scrollbar == NULL) return;

	PushState();
	ConstrainClippingRegion(updateRect);
	theme->draw_scrollbar(theme, this, Frame().OffsetToSelf(B_ORIGIN),
	                      fOrientation, fRangeMin, fRangeMax, fValue,
	                      (fTracking && fTrackingState > 0) ? fTrackingRegion.Contains(fMousePosition) : false,
	                      fMousePosition);
	PopState();
}


void
BScrollBar::DetachedFromWindow()
{
	fTracking = false;
	fTrackingState = 0;
}


void
BScrollBar::MouseDown(BPoint where)
{
	if (fRangeMin == fRangeMax || IsEnabled() == false || !QueryCurrentMouse(true, B_PRIMARY_MOUSE_BUTTON)) return;

	BRect rect = VisibleBounds();
	if (!rect.Contains(where)) return;

	if (fTrackingState != 0) return;
	e_theme_engine *theme = get_current_theme_engine();
	if (theme == NULL || theme->get_scrollbar_respondent_region == NULL) return;
	BRegion dragTo, smallUp, smallDown, largeUp, largeDown;
	theme->get_scrollbar_respondent_region(theme, this, Frame().OffsetToSelf(B_ORIGIN),
	                                       fOrientation, fRangeMin, fRangeMax, fValue, NULL,
	                                       &dragTo, &smallUp, &smallDown, &largeUp, &largeDown);
	if (smallUp.Contains(where)) {
		fTrackingState = 1;
		fTrackingRegion = smallUp;
	} else if (smallDown.Contains(where)) {
		fTrackingState = 2;
		fTrackingRegion = smallDown;
	} else if (largeUp.Contains(where)) {
		fTrackingState = 3;
		fTrackingRegion = largeUp;
	} else if (largeDown.Contains(where)) {
		fTrackingState = 4;
		fTrackingRegion = largeDown;
	} else if (dragTo.Contains(where)) {
		fTrackingState = 5;
		fTrackingRegion = dragTo;
	} else fTrackingRegion.MakeEmpty();
	if (fTrackingState == 0) return;

	if (!fTracking) fTracking = true;
	fMousePosition = where;

	if (SetPrivateEventMask(B_POINTER_EVENTS, B_LOCK_WINDOW_FOCUS) != B_OK) {
		Invalidate();
		Window()->UpdateIfNeeded();
		e_snooze(50000);
		fTracking = false;
		int8 state = fTrackingState;
		fTrackingState = 0;
		Invalidate();
		doScroll(state);
	} else {
		Invalidate();
	}
}


void
BScrollBar::MouseUp(BPoint where)
{
	if (!fTracking) return;
	fTracking = false;

	if (fTrackingState != 0) {
		int8 state = fTrackingState;
		fTrackingState = 0;
		Invalidate();

		doScroll(state);
	}
}


void
BScrollBar::MouseMoved(BPoint where, uint32 code, const BMessage *a_message)
{
	if (fRangeMin == fRangeMax || IsEnabled() == false) return;

	bool update = false;

	if (fTrackingState == 5 || fTrackingState == -5) {
		while (fMousePosition != where) {
			e_theme_engine *theme = get_current_theme_engine();
			if (theme == NULL || theme->get_scrollbar_respondent_region == NULL) break;

			float ratio = 0;
			theme->get_scrollbar_respondent_region(theme, this, Frame().OffsetToSelf(B_ORIGIN),
			                                       fOrientation, fRangeMin, fRangeMax, fValue, &ratio,
			                                       NULL, NULL, NULL, NULL, NULL);
			if (ratio <= 0) break;

			if (fOrientation == B_HORIZONTAL)
				SetValue(fValue + (where.x - fMousePosition.x) / ratio);
			else
				SetValue(fValue + (where.y - fMousePosition.y) / ratio);

			theme->get_scrollbar_respondent_region(theme, this, Frame().OffsetToSelf(B_ORIGIN),
			                                       fOrientation, fRangeMin, fRangeMax, fValue, NULL,
			                                       &fTrackingRegion, NULL, NULL, NULL, NULL);

			update = true;
			break;
		}
	}

	fMousePosition = where;

	if (code == B_ENTERED_VIEW) {
		if (fTrackingState < 0 && fTracking) {
			fTrackingState = -fTrackingState;
			update = true;
		}
	} else if (code == B_EXITED_VIEW) {
		if (fTrackingState > 0 && fTracking) {
			fTrackingState = -fTrackingState;
			update = true;
		}
	}

	if (update) Invalidate();
}


void
BScrollBar::doScroll(int8 state)
{
	if (state <= 0 || state > 4) return;

	if (state == 1)
		SetValue(fValue - fStepSmall);
	else if (state == 2)
		SetValue(fValue + fStepSmall);
	else if (state == 3)
		SetValue(fValue - fStepLarge);
	else if (state == 4)
		SetValue(fValue + fStepLarge);
}

