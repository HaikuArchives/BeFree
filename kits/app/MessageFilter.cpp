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
 * File: MessageFilter.cpp
 * Description: Filter message before BLooper::DispatchMessage
 *
 * --------------------------------------------------------------------------*/

#include "Looper.h"
#include "MessageFilter.h"


BMessageFilter::BMessageFilter(e_message_delivery delivery, e_message_source source, uint32 command, e_filter_hook filter)
		: fFiltersAny(false), fLooper(NULL), fHandler(NULL)
{
	fDelivery = delivery;
	fSource = source;
	fCommand = command;
	fFilterHook = filter;
}


BMessageFilter::BMessageFilter(e_message_delivery delivery, e_message_source source, e_filter_hook filter)
		: fCommand(0), fFiltersAny(true), fLooper(NULL), fHandler(NULL)
{
	fDelivery = delivery;
	fSource = source;
	fFilterHook = filter;
}


BMessageFilter::BMessageFilter(uint32 command, e_filter_hook filter)
		: fFiltersAny(false), fDelivery(B_ANY_DELIVERY), fSource(B_ANY_SOURCE), fLooper(NULL), fHandler(NULL)
{
	fCommand = command;
	fFilterHook = filter;
}


BMessageFilter::BMessageFilter(const BMessageFilter &filter)
		: fLooper(NULL), fHandler(NULL)
{
	fCommand = filter.fCommand;
	fFiltersAny = filter.fFiltersAny;
	fDelivery = filter.fDelivery;
	fSource = filter.fSource;
	fFilterHook = filter.fFilterHook;
}


BMessageFilter::BMessageFilter(const BMessageFilter *filter)
		: fLooper(NULL), fHandler(NULL)
{
	fCommand = (filter ? 0 : filter->fCommand);
	fFiltersAny = (filter ? true : filter->fFiltersAny);
	fDelivery = (filter ?B_ANY_DELIVERY : filter->fDelivery);
	fSource = (filter ?B_ANY_SOURCE : filter->fSource);
	fFilterHook = (filter ? NULL : filter->fFilterHook);
}


BMessageFilter::~BMessageFilter()
{
	// TODO

	if (fHandler != NULL) {
		if (fHandler->BHandler::RemoveFilter(this) == false && fLooper != NULL) fLooper->BLooper::RemoveCommonFilter(this);
	}
}


BMessageFilter&
BMessageFilter::operator=(const BMessageFilter &filter)
{
	// TODO

	fCommand = filter.fCommand;
	fFiltersAny = filter.fFiltersAny;
	fDelivery = filter.fDelivery;
	fSource = filter.fSource;
	fFilterHook = filter.fFilterHook;

	return *this;
}


e_filter_result
BMessageFilter::Filter(BMessage *message, BHandler **target)
{
	return B_DISPATCH_MESSAGE;
}


e_filter_result
BMessageFilter::doFilter(BMessage *message, BHandler **target)
{
	if (message == NULL || target == NULL || fHandler == NULL) return B_SKIP_MESSAGE;

	e_filter_result retVal = B_DISPATCH_MESSAGE;

	// TODO: delivery & source
	if (fFiltersAny || message->what == fCommand) {
		if (fFilterHook != NULL) retVal = (*fFilterHook)(message, target, this);
		if (retVal == B_DISPATCH_MESSAGE) retVal = Filter(message, target);
	}

	return retVal;
}


e_message_delivery
BMessageFilter::MessageDelivery() const
{
	return fDelivery;
}


e_message_source
BMessageFilter::MessageSource() const
{
	return fSource;
}


uint32
BMessageFilter::Command() const
{
	return fCommand;
}


bool
BMessageFilter::FiltersAnyCommand() const
{
	return fFiltersAny;
}


BLooper*
BMessageFilter::Looper() const
{
	return fLooper;
}

