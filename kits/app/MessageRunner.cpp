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
 * File: MessageRunner.cpp
 *
 * --------------------------------------------------------------------------*/

#include <kernel/Kernel.h>
#include <app/Application.h>
#include <support/Locker.h>
#include <support/Autolock.h>

#include "MessageRunner.h"

extern BLocker* get_handler_operator_locker();

BMessageRunner::BMessageRunner(const BMessenger &target, const BMessage *msg, bigtime_t interval, int32 count)
		: fToken(-1), fTarget(NULL), fReplyTo(NULL), fMessage(NULL), fPrevSendTime(B_INT64_CONSTANT(-1))
{
	if (!(msg == NULL || (fMessage = new BMessage(*msg)) != NULL)) return;
	if (target.IsValid()) {
		fTarget = new BMessenger(target);
		if (fTarget == NULL || fTarget->IsValid() == false) return;
	}
	fInterval = interval;
	fCount = count;

	BLocker *hLocker = get_handler_operator_locker();
	BAutolock <BLocker>autolock(hLocker);

	int32 token = BApplication::sRunnerList.IndexOf(NULL);
	if (token >= 0) {
		if (BApplication::sRunnerList.ReplaceItem(token, this) == false) return;
		fToken = token;
	} else {
		if (BApplication::sRunnerList.AddItem(this) == false) return;
		fToken = BApplication::sRunnerList.CountItems() - 1;
	}

	if (fCount != 0 && fInterval >B_INT64_CONSTANT(0) &&
	        !(fTarget == NULL || fTarget->IsValid() == false) && fMessage != NULL && app != NULL) {
		BApplication::sRunnerMinimumInterval = B_INT64_CONSTANT(-1);
		app->PostMessage(_EVENTS_PENDING_, app);
	}
}


BMessageRunner::BMessageRunner(const BMessenger &target, const BMessage *msg, bigtime_t interval, int32 count, const BMessenger &replyTo)
		: fToken(-1), fTarget(NULL), fReplyTo(NULL), fMessage(NULL), fPrevSendTime(B_INT64_CONSTANT(-1))
{
	if (!(msg == NULL || (fMessage = new BMessage(*msg)) != NULL)) return;
	if (target.IsValid()) {
		fTarget = new BMessenger(target);
		if (fTarget == NULL || fTarget->IsValid() == false) return;
	}
	if (replyTo.IsValid()) {
		fReplyTo = new BMessenger(replyTo);
		if (fReplyTo == NULL || fReplyTo->IsValid() == false) return;
	}
	fInterval = interval;
	fCount = count;

	BLocker *hLocker = get_handler_operator_locker();
	BAutolock <BLocker>autolock(hLocker);

	int32 token = BApplication::sRunnerList.IndexOf(NULL);
	if (token >= 0) {
		if (BApplication::sRunnerList.ReplaceItem(token, this) == false) return;
		fToken = token;
	} else {
		if (BApplication::sRunnerList.AddItem(this) == false) return;
		fToken = BApplication::sRunnerList.CountItems() - 1;
	}

	if (fCount != 0 && fInterval >B_INT64_CONSTANT(0) &&
	        !(fTarget == NULL || fTarget->IsValid() == false) && fMessage != NULL && app != NULL) {
		BApplication::sRunnerMinimumInterval = B_INT64_CONSTANT(-1);
		app->PostMessage(_EVENTS_PENDING_, app);
	}
}


BMessageRunner::~BMessageRunner()
{
	BLocker *hLocker = get_handler_operator_locker();
	BAutolock <BLocker>autolock(hLocker);

	if (fToken >= 0) {
		void *oldItem = NULL;
		bool removed = (fToken == BApplication::sRunnerList.CountItems() - 1 ?
		                (BApplication::sRunnerList.RemoveItem(fToken) == (void*)this) :
		                !(BApplication::sRunnerList.ReplaceItem(fToken, NULL, &oldItem) == false || oldItem != (void*)this));
		if (!removed)
			ETK_ERROR("[APP]: %s --- Unable to remove runner, it must something error.", __PRETTY_FUNCTION__);
	}
	if (fTarget) delete fTarget;
	if (fReplyTo) delete fReplyTo;
	if (fMessage) delete fMessage;
}


bool
BMessageRunner::IsValid() const
{
	return(fToken >= 0);
}


status_t
BMessageRunner::SetTarget(const BMessenger &target)
{
	if (fToken < 0) return B_ERROR;

	BLocker *hLocker = get_handler_operator_locker();
	BAutolock <BLocker>autolock(hLocker);

	if (target.IsValid()) {
		BMessenger *msgr = new BMessenger(target);
		if (msgr == NULL || msgr->IsValid() == false) {
			if (msgr) delete msgr;
			return B_ERROR;
		}
		if (fTarget) delete fTarget;
		fTarget = msgr;
	} else {
		if (fTarget) delete fTarget;
		fTarget = NULL;
	}

	fPrevSendTime = B_INT64_CONSTANT(-1);

	if (fCount != 0 && fInterval >B_INT64_CONSTANT(0) &&
	        !(fTarget == NULL || fTarget->IsValid() == false) && fMessage != NULL && app != NULL) {
		BApplication::sRunnerMinimumInterval = B_INT64_CONSTANT(-1);
		app->PostMessage(_EVENTS_PENDING_, app);
	}

	return B_OK;
}


status_t
BMessageRunner::SetReplyTo(const BMessenger &replyTo)
{
	if (fToken < 0) return B_ERROR;

	BLocker *hLocker = get_handler_operator_locker();
	BAutolock <BLocker>autolock(hLocker);

	if (replyTo.IsValid()) {
		BMessenger *msgr = new BMessenger(replyTo);
		if (msgr == NULL || msgr->IsValid() == false) {
			if (msgr) delete msgr;
			return B_ERROR;
		}
		if (fReplyTo) delete fReplyTo;
		fReplyTo = msgr;
	} else {
		if (fReplyTo) delete fReplyTo;
		fReplyTo = NULL;
	}

	fPrevSendTime = B_INT64_CONSTANT(-1);

	return B_OK;
}


status_t
BMessageRunner::SetMessage(const BMessage *msg)
{
	BMessage *aMsg = NULL;
	if (fToken < 0 || !(msg == NULL || (aMsg = new BMessage(*msg)) != NULL)) return B_ERROR;

	BLocker *hLocker = get_handler_operator_locker();
	BAutolock <BLocker>autolock(hLocker);

	if (fMessage) delete fMessage;
	fMessage = aMsg;

	fPrevSendTime = B_INT64_CONSTANT(-1);

	if (fCount != 0 && fInterval >B_INT64_CONSTANT(0) &&
	        !(fTarget == NULL || fTarget->IsValid() == false) && fMessage != NULL && app != NULL) {
		BApplication::sRunnerMinimumInterval = B_INT64_CONSTANT(-1);
		app->PostMessage(_EVENTS_PENDING_, app);
	}

	return B_OK;
}


status_t
BMessageRunner::SetInterval(bigtime_t interval)
{
	if (fToken < 0) return B_ERROR;

	BLocker *hLocker = get_handler_operator_locker();
	BAutolock <BLocker>autolock(hLocker);

	fInterval = interval;
	fPrevSendTime = B_INT64_CONSTANT(-1);

	if (fCount != 0 && fInterval >B_INT64_CONSTANT(0) &&
	        !(fTarget == NULL || fTarget->IsValid() == false) && fMessage != NULL && app != NULL) {
		BApplication::sRunnerMinimumInterval = B_INT64_CONSTANT(-1);
		app->PostMessage(_EVENTS_PENDING_, app);
	}

	return B_OK;
}


status_t
BMessageRunner::SetCount(int32 count)
{
	if (fToken < 0) return B_ERROR;

	BLocker *hLocker = get_handler_operator_locker();
	BAutolock <BLocker>autolock(hLocker);

	fCount = count;
	fPrevSendTime = B_INT64_CONSTANT(-1);

	if (fCount != 0 && fInterval >B_INT64_CONSTANT(0) &&
	        !(fTarget == NULL || fTarget->IsValid() == false) && fMessage != NULL && app != NULL) {
		BApplication::sRunnerMinimumInterval = B_INT64_CONSTANT(-1);
		app->PostMessage(_EVENTS_PENDING_, app);
	}

	return B_OK;
}


status_t
BMessageRunner::GetInfo(bigtime_t *interval, int32 *count) const
{
	if (fToken < 0 || (!interval && !count)) return B_ERROR;

	BLocker *hLocker = get_handler_operator_locker();
	BAutolock <BLocker>autolock(hLocker);

	if (interval) *interval = fInterval;
	if (count) *count = fCount;

	return B_OK;
}


status_t
BMessageRunner::GetInfo(BMessenger *target, BMessage *msg, bigtime_t *interval, int32 *count, BMessenger *replyTo) const
{
	if (fToken < 0 || (!target && !msg && interval && !count && !replyTo)) return B_ERROR;

	BLocker *hLocker = get_handler_operator_locker();
	BAutolock <BLocker>autolock(hLocker);

	if (target) *target = (fTarget ? *fTarget : BMessenger());
	if (replyTo) *replyTo = (fReplyTo ? *fReplyTo : BMessenger());
	if (msg) {
		msg->MakeEmpty();
		msg->what = 0;
		if (fMessage) *msg = *fMessage;
	}
	if (interval) *interval = fInterval;
	if (count) *count = fCount;

	return B_OK;
}

