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
 * File: ScrollBar.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_SCROLL_BAR_H__
#define __ETK_SCROLL_BAR_H__

#include <app/MessageRunner.h>
#include <interface/View.h>

#define B_V_SCROLL_BAR_WIDTH	e_ui_get_scrollbar_vertical_width()
#define B_H_SCROLL_BAR_HEIGHT	e_ui_get_scrollbar_horizontal_height()

#ifdef __cplusplus /* Just for C++ */

class BScrollBar : public BView
{
	public:
		BScrollBar(BRect frame, const char *name,
		           float value, float min, float max,
		           orientation direction);
		virtual ~BScrollBar();

		void		SetValue(float value);
		float		Value() const;
		void		SetProportion(float ratio);
		float		Proportion() const;

		// Empty functions BEGIN --- just for derivative class
		virtual void	ValueChanged(float value);
		// Empty functions END

		void		SetRange(float min, float max);
		void		GetRange(float *min, float *max) const;
		void		SetSteps(float smallStep, float largeStep);
		void		GetSteps(float *smallStep, float *largeStep) const;

		// SetTarget: If target isn't NULL, target->Ancestor() must equal to this->Ancestor().
		//            AKA. The function just be successful doing when it added to any parent.
		status_t	SetTarget(BView *target);
		BView		*Target() const;

		orientation	Orientation() const;

		virtual void	Draw(BRect updateRect);
		virtual void	MouseDown(BPoint where);
		virtual void	MouseUp(BPoint where);
		virtual void	MouseMoved(BPoint where, uint32 code, const BMessage *a_message);
		virtual void	DetachedFromWindow();

	private:
		friend class BWindow;
		friend class BView;

		orientation fOrientation;
		float fValue;
		float fRangeMin;
		float fRangeMax;
		float fStepSmall;
		float fStepLarge;
		BView *fTarget;

		bool fTracking;
		int8 fTrackingState;
		BPoint fMousePosition;
		BRegion fTrackingRegion;
		void doScroll(int8 state);
		void _SetValue(float value, bool response);

		BMessageRunner *fRunner;
};


#endif /* __cplusplus */

#endif /* __ETK_SCROLL_BAR_H__ */

