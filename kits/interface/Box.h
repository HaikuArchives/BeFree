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
 * File: Box.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_BOX_H__
#define __ETK_BOX_H__

#include <interface/View.h>

#ifdef __cplusplus /* Just for C++ */

class BBox : public BView
{
	public:
		BBox(BRect frame,
		     const char *name = NULL,
		     uint32 resizingMode = B_FOLLOW_LEFT |B_FOLLOW_TOP,
		     uint32 flags = B_WILL_DRAW |B_FRAME_EVENTS |B_NAVIGABLE_JUMP,
		     border_style border = B_FANCY_BORDER);
		virtual ~BBox();

		virtual void	SetBorder(border_style border);
		border_style	Border() const;

		void		SetLabelAlignment(e_alignment labelAlignment);
		e_alignment	LabelAlignment() const;

		void		SetLabel(const char *label);
		status_t	SetLabel(BView *viewLabel);
		const char	*Label() const;
		BView		*LabelView() const;

		BRect		ContentBounds() const;

		virtual void	Draw(BRect updateRect);
		virtual void	FrameResized(float new_width, float new_height);

		virtual void	ResizeToPreferred();
		virtual void	GetPreferredSize(float *width, float *height);

	protected:
		virtual void	ChildRemoving(BView *child);

	private:
		BView *fLabelView;
		border_style fBorder;
		e_alignment fAlignment;

		void ReAdjustLabel();
};

#endif /* __cplusplus */

#endif /* __ETK_BOX_H__ */

