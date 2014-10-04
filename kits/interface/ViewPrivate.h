/* --------------------------------------------------------------------------
 *
 * ETK++ --- The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2007, Anthony Lee, All Rights Reserved
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
 * File: ViewPrivate.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_VIEW_PRIVATE_H__
#define __ETK_VIEW_PRIVATE_H__

#include "layout/Layout.h"
#include "View.h"


class _LOCAL BViewLayout : public BLayoutItem
{
	public:
		BViewLayout(BView *view, BRect frame, uint32 resizingMode);
		virtual ~BViewLayout();

		void		SetEnabled(bool state);
		bool		IsEnabled() const;

		virtual void	GetPreferredSize(float *width, float *height);
		virtual void	ResizeToPreferred();
		virtual void	MoveTo(BPoint where);
		virtual void	ResizeTo(float width, float height);

		virtual void	Invalidate(BRect rect);
		virtual void	UpdateVisibleRegion();

		void		_GetVisibleRegion(BRegion **region);

	private:
		bool fEnabled;
};


inline
BViewLayout::BViewLayout(BView *view, BRect frame, uint32 resizingMode)
		: BLayoutItem(frame, resizingMode), fEnabled(true)
{
	SetPrivateData(view);
}


inline
BViewLayout::~BViewLayout()
{
}


inline void
BViewLayout::SetEnabled(bool state)
{
	fEnabled = state;
}


inline bool
BViewLayout::IsEnabled() const
{
	return fEnabled;
}


inline void
BViewLayout::GetPreferredSize(float *width, float *height)
{
	((BView*)PrivateData())->GetPreferredSize(width, height);
}


inline void
BViewLayout::ResizeToPreferred()
{
	((BView*)PrivateData())->ResizeToPreferred();
}


inline void
BViewLayout::MoveTo(BPoint where)
{
	BRect oldFrame = Frame();
	BLayoutItem::MoveTo(where);
	BRect newFrame = Frame();
	((BView*)PrivateData())->_FrameChanged(oldFrame, newFrame);
}


inline void
BViewLayout::ResizeTo(float width, float height)
{
	BRect oldFrame = Frame();
	BLayoutItem::ResizeTo(width, height);
	BRect newFrame = Frame();
	((BView*)PrivateData())->_FrameChanged(oldFrame, newFrame);
}


inline void
BViewLayout::Invalidate(BRect rect)
{
	if (((BView*)PrivateData())->Window() == NULL) return;
	((BView*)PrivateData())->ConvertToWindow(&rect);
	((BView*)PrivateData())->Window()->Invalidate(rect);
}


inline void
BViewLayout::UpdateVisibleRegion()
{
	BLayoutItem::UpdateVisibleRegion();
	((BView*)PrivateData())->_UpdateVisibleRegion();
}


inline void
BViewLayout::_GetVisibleRegion(BRegion **region)
{
	GetVisibleRegion(region);
}

#endif /* __ETK_VIEW_PRIVATE_H__ */

