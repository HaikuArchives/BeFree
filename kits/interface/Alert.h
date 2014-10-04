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
 * File: Alert.h
 * Description: BAlert --- Display a modal window that notifies something
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_ALERT_H__
#define __ETK_ALERT_H__

#include <interface/InterfaceDefs.h>
#include <interface/Window.h>

enum e_alert_type {
	B_EMPTY_ALERT = 0,
	B_INFO_ALERT,
	B_IDEA_ALERT,
	B_WARNING_ALERT,
	B_STOP_ALERT
};

#ifdef __cplusplus /* Just for C++ */

class BInvoker;
class BButton;
class BTextView;

class BAlert : public BWindow
{
	public:
		BAlert(const char *title,
		       const char *text,
		       const char *button1_label,
		       const char *button2_label = NULL,
		       const char *button3_label = NULL,
		       e_button_width width = B_WIDTH_AS_USUAL,
		       e_alert_type type = B_INFO_ALERT);
		virtual ~BAlert();

		// run synchronously then auto-destruct when it return.
		// "could_proxy" must be "true" when it called from looper of BApplication!
		int32		Go(bool could_proxy = true);

		// run asynchronously and auto-destruct after message send
		status_t	Go(BInvoker *invoker);

		BButton		*ButtonAt(int32 index) const;
		BTextView	*TextView() const;

		virtual bool	QuitRequested();

	private:
		BButton *fButtons[3];
		BTextView *fTextView;
};

#endif /* __cplusplus */

#endif /* __ETK_ALERT_H__ */

