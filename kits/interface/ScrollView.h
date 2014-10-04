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
 * File: ScrollView.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_SCROLL_VIEW_H__
#define __ETK_SCROLL_VIEW_H__

#include <interface/ScrollBar.h>

#ifdef __cplusplus /* Just for C++ */

class BScrollView : public BView
{
	public:
		BScrollView(BRect frame, const char *name, BView *target,
		            uint32 resizingMode = B_FOLLOW_LEFT |B_FOLLOW_TOP,
		            uint32 flags = 0,
		            bool alwaysShowHorizontal = false,
		            bool alwaysShowVertical = false,
		            border_style border = B_FANCY_BORDER);
		virtual ~BScrollView();

		// SetTarget: When it return B_OK, the oldTarget was removed and destroy automatically.
		//            If you want to keep the oldTarget, try oldTarget->RemoveSelf() before.
		status_t	SetTarget(BView *newTarget);
		BView		*Target() const;
		BRect		TargetFrame() const;

		virtual void	SetBorder(border_style border);
		border_style	Border() const;

		void		SetScrollBarAutoState(bool alwaysShowHorizontal, bool alwaysShowVertical);
		void		GetScrollBarAutoState(bool *alwaysShowHorizontal, bool *alwaysShowVertical) const;

		BScrollBar	*ScrollBar(orientation direction) const;

		virtual void	SetFlags(uint32 flags); // auto-settingB_WILL_DRAW andB_FRAME_EVENTS
		virtual void	Draw(BRect updateRect);
		virtual void	FrameResized(float new_width, float new_height);

	protected:
		virtual void	ChildRemoving(BView *child);

	private:
		friend class BView;

		border_style fBorder;
		bool fAlwaysShowHorizontal;
		bool fAlwaysShowVertical;

		BScrollBar *fHSB;
		BScrollBar *fVSB;
		BView *fTarget;

		BRect TargetValidFrame(bool ignore_scrollbar = false) const;
};


#endif /* __cplusplus */

#endif /* __ETK_SCROLL_VIEW_H__ */

