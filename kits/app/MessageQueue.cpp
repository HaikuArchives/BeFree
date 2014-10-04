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
 * File: MessageQueue.cpp
 *
 * --------------------------------------------------------------------------*/

#include <kernel/Kernel.h>
#include <support/Locker.h>

#include "MessageQueue.h"

extern BLocker* get_handler_operator_locker();

BMessageQueue::BMessageQueue()
		: fLocker(NULL)
{
	if ((fLocker = create_locker()) == NULL)
		ETK_ERROR("[APP]: %s --- Unable to create locker for looper.", __PRETTY_FUNCTION__);
}


BMessageQueue::~BMessageQueue()
{
	for (int32 i = 0; i < fMessagesList.CountItems(); i++) {
		BMessage *msg = (BMessage*)fMessagesList.ItemAt(i);
		if (msg) delete msg;
	}

	fMessagesList.MakeEmpty();

	if (fLocker) {
		close_locker(fLocker);
		delete_locker(fLocker);
	}
}


status_t
BMessageQueue::LockWithTimeout(bigtime_t timeout)
{
	status_t retVal = B_ERROR;

	BLocker *hLocker = get_handler_operator_locker();

	hLocker->Lock();
	void *locker = clone_locker(fLocker);
	int64 locksCount = hLocker->CountLocks();
	while (hLocker->CountLocks() >B_INT64_CONSTANT(0)) hLocker->Unlock();

	if (locker != NULL) {
		retVal = lock_locker_etc(fLocker, B_TIMEOUT, timeout);
		delete_locker(locker);
	}

	while (locksCount >B_INT64_CONSTANT(1)) {
		hLocker->Lock();
		locksCount--;
	}

	return retVal;
}


bool
BMessageQueue::Lock()
{
	return(LockWithTimeout(B_INFINITE_TIMEOUT) == B_OK);
}


void
BMessageQueue::Unlock()
{
	if (count_locker_locks(fLocker) >B_INT64_CONSTANT(0))
		unlock_locker(fLocker);
	else
		ETK_WARNING("[APP]: %s -- MessageQueue wasn't locked by current thread.", __PRETTY_FUNCTION__);
}


int32
BMessageQueue::CountMessages() const
{
	return fMessagesList.CountItems();
}


bool
BMessageQueue::IsEmpty() const
{
	return fMessagesList.IsEmpty();
}


bool
BMessageQueue::AddMessage(BMessage *an_event)
{
	if (!an_event) return false;

	if (fMessagesList.AddItem((void*)an_event)) return true;

	delete an_event;
	return false;
}


bool
BMessageQueue::RemoveMessage(BMessage *an_event)
{
	if (!an_event) return false;

	if (!fMessagesList.RemoveItem((void*)an_event)) return false;

	delete an_event;

	return true;
}


BMessage*
BMessageQueue::NextMessage()
{
	return((BMessage*)fMessagesList.RemoveItem(0));
}


BMessage*
BMessageQueue::FindMessage(int32 index) const
{
	return((BMessage*)fMessagesList.ItemAt(index));
}


BMessage*
BMessageQueue::FindMessage(uint32 what, int32 fromIndex) const
{
	for (int32 i = fromIndex; i < fMessagesList.CountItems(); i++) {
		BMessage *msg = (BMessage*)fMessagesList.ItemAt(i);

		if (msg)
			if (msg->what == what) return msg;
	}

	return NULL;
}


BMessage*
BMessageQueue::FindMessage(uint32 what, int32 fromIndex, int32 count) const
{
	for (int32 i = fromIndex, j = 0; i < fMessagesList.CountItems() && j < count; i++, j++) {
		BMessage *msg = (BMessage*)fMessagesList.ItemAt(i);

		if (msg)
			if (msg->what == what) return msg;
	}

	return NULL;
}


int32
BMessageQueue::IndexOfMessage(BMessage *an_event) const
{
	return fMessagesList.IndexOf((void*)an_event);
}

