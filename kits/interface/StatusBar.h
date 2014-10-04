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
 * File: StatusBar.h
 * Description: BStatusBar --- A view that graphically indicates the progress
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_STATUS_BAR_H__
#define __ETK_STATUS_BAR_H__

#include <interface/View.h>

#ifdef __cplusplus /* Just for C++ */

class BStatusBar : public BView
{
	public:
		BStatusBar(BRect frame,
		           const char *name,
		           const char *label = NULL,
		           const char *trailing_label = NULL);
		virtual ~BStatusBar();

		virtual void	SetBarHeight(float height);
		virtual void	SetText(const char *str);
		virtual void	SetTrailingText(const char *str);
		virtual void	SetMaxValue(float max);

		void		Update(float delta, const char *text = NULL, const char *trailing_text = NULL);
		void		Reset(const char *label = NULL, const char *trailing_label = NULL);
		virtual void	SetTo(float value, const char *text = NULL, const char *trailing_text = NULL);

		float		CurrentValue() const;
		float		MaxValue() const;
		float		BarHeight() const;
		const char	*Text() const;
		const char	*TrailingText() const;
		const char	*Label() const;
		const char	*TrailingLabel() const;

		virtual void	MessageReceived(BMessage *msg);
		virtual void	Draw(BRect updateRect);
		virtual void	GetPreferredSize(float *width, float *height);

	private:
		char *fLabel;
		char *fTrailingLabel;
		char *fText;
		char *fTrailingText;
		float fBarHeight;
		float fMaxValue;
		float fCurrentValue;
};

#endif /* __cplusplus */

#endif /* __ETK_STATUS_BAR_H__ */

