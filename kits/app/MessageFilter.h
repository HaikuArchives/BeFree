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
 * File: MessageFilter.h
 * Description: Filter message before BLooper::DispatchMessage
 * Warning: ignoreB_QUIT_REQUESTED & _QUIT_
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_MESSAGE_FILTER_H__
#define __ETK_MESSAGE_FILTER_H__

#include <app/Handler.h>

typedef enum e_filter_result {
	B_SKIP_MESSAGE,
	B_DISPATCH_MESSAGE
} e_filter_result;

typedef enum e_message_delivery {
	B_ANY_DELIVERY,
	E_DROPPED_DELIVERY,
	E_PROGRAMMED_DELIVERY
} e_message_delivery;

typedef enum e_message_source {
	B_ANY_SOURCE,
	E_REMOTE_SOURCE,
	E_LOCAL_SOURCE
} e_message_source;

#ifdef __cplusplus /* Just for C++ */

class BMessage;
class BMessageFilter;

typedef e_filter_result (*e_filter_hook)(BMessage *message, BHandler **target, BMessageFilter *filter);


class BMessageFilter
{
	public:
		BMessageFilter(e_message_delivery delivery, e_message_source source,
		               uint32 command, e_filter_hook filter = NULL);
		BMessageFilter(e_message_delivery delivery, e_message_source source,
		               e_filter_hook filter = NULL);
		BMessageFilter(uint32 command, e_filter_hook filter = NULL);
		BMessageFilter(const BMessageFilter &filter);
		BMessageFilter(const BMessageFilter *filter);
		virtual ~BMessageFilter();

		BMessageFilter &operator=(const BMessageFilter &from);

		virtual e_filter_result		Filter(BMessage *message, BHandler **target);

		e_message_delivery		MessageDelivery() const;
		e_message_source		MessageSource() const;
		uint32				Command() const;
		bool				FiltersAnyCommand() const;
		BLooper				*Looper() const;

	private:
		friend class BLooper;
		friend class BHandler;

		uint32 fCommand;
		bool fFiltersAny;
		e_message_delivery fDelivery;
		e_message_source fSource;
		e_filter_hook fFilterHook;

		BLooper *fLooper;
		BHandler *fHandler;

		e_filter_result doFilter(BMessage *message, BHandler **target);
};

#endif /* __cplusplus */

#endif /* __ETK_MESSAGE_FILTER_H__ */

