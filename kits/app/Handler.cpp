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
 * File: Handler.cpp
 *
 * --------------------------------------------------------------------------*/

#include <kernel/Kernel.h>
#include <support/String.h>
#include <support/List.h>
#include <support/Locker.h>
#include <support/Autolock.h>
#include <support/ClassInfo.h>

#include <private/Token.h>

#include "Handler.h"
#include "Looper.h"
#include "Messenger.h"
#include "MessageFilter.h"


class _EObserverList
{
	public:
		_EObserverList();
		~_EObserverList();

		status_t StartWatching(BMessenger msgr, uint32 what);
		status_t StopWatching(BMessenger msgr, uint32 what);
		BList* GetObservers(uint32 what);
		bool IsWatched(uint32 what) const;

	private:
		BList fListWatching;
		BList fListWatchingAll;
};


static BLocker handler_operator_locker;
static BTokensDepot handlers_depot(&handler_operator_locker, false);

_LOCAL BLocker* get_handler_operator_locker()
{
	return &handler_operator_locker;
}


_LOCAL uint64 get_handler_token(const BHandler *handler)
{
	return((handler == NULL || handler->fToken == NULL) ?B_MAXUINT64 : handler->fToken->Token());
}


_LOCAL uint64 get_ref_handler_token(const BHandler *handler)
{
	BAutolock <BLocker>autolock(handler_operator_locker);

	uint64 token = get_handler_token(handler);
	return(handlers_depot.PushToken(token) ? token :B_MAXUINT64);
}


_LOCAL BHandler* get_handler(uint64 token)
{
	BHandler *retVal = NULL;

	BToken *aToken = handlers_depot.OpenToken(token);
	if (aToken != NULL) {
		retVal = reinterpret_cast<BHandler*>(aToken->Data());
		delete aToken;
	}

	return retVal;
}


_LOCAL bigtime_t get_handler_create_time_stamp(uint64 token)
{
	bigtime_t retVal = B_MAXINT64;

	BToken *aToken = handlers_depot.OpenToken(token);
	if (aToken != NULL) {
		retVal = aToken->TimeStamp();
		delete aToken;
	}

	return retVal;
}


_LOCAL BLooper* get_handler_looper(uint64 token)
{
	BAutolock <BLocker>autolock(handler_operator_locker);

	BHandler *handler = get_handler(token);
	return(handler == NULL ? NULL : handler->fLooper);
}


_LOCAL uint64 get_ref_looper_token(uint64 token)
{
	BAutolock <BLocker>autolock(handler_operator_locker);

	BHandler *handler = get_handler(token);
	return(handler == NULL ?B_MAXUINT64 : get_ref_handler_token(handler->fLooper));
}


_LOCAL status_t lock_looper_of_handler(uint64 token, bigtime_t timeout)
{
	status_t retVal = B_ERROR;

	handler_operator_locker.Lock();
	BLooper *looper = get_handler_looper(token);
	BLooper *looper_proxy = (looper != NULL ? looper->_Proxy() : NULL);
	void *locker = ((looper == NULL || looper->fLocker == NULL) ? NULL : clone_locker(looper->fLocker));
	int64 locksCount = handler_operator_locker.CountLocks();
	while (handler_operator_locker.CountLocks() >B_INT64_CONSTANT(0)) handler_operator_locker.Unlock();

	if (locker) {
		if ((retVal = lock_locker_etc(locker, B_TIMEOUT, timeout)) == B_OK) {
			handler_operator_locker.Lock();

			if (looper != get_handler_looper(token) || looper_proxy != looper->_Proxy()) retVal = B_ERROR;

			if (locksCount >B_INT64_CONSTANT(1))
				locksCount--;
			else
				handler_operator_locker.Unlock();

			if (retVal != B_OK) unlock_locker(locker);
		}

		delete_locker(locker);
	}

	while (locksCount >B_INT64_CONSTANT(1)) {
		handler_operator_locker.Lock();
		locksCount--;
	}

	return retVal;
}


_LOCAL bool is_current_at_looper_thread(uint64 token)
{
	BAutolock <BLocker>autolock(handler_operator_locker);

	BLooper *looper = cast_as(get_handler(token), BLooper);
	if (looper == NULL) return false;

	bool retVal = (looper->Thread() == get_current_thread_id() ? true : false);

	return retVal;
}


_LOCAL bool ref_handler(uint64 token)
{
	return handlers_depot.PushToken(token);
}


_LOCAL void unref_handler(uint64 token)
{
	handlers_depot.PopToken(token);
}


BHandler::BHandler(const char *name)
		: BArchivable(),
		fName(NULL),
		fLooper(NULL),
		forceSetNextHandler(false), fNextHandler(NULL)
{
	fName = EStrdup(name);
	fToken = handlers_depot.CreateToken(reinterpret_cast<void*>(this));
	fObserverList = new _EObserverList();
}


BHandler::~BHandler()
{
	while (fFilters.CountItems() > 0) {
		BMessageFilter *filter = (BMessageFilter*)fFilters.ItemAt(0);
		BHandler::RemoveFilter(filter);
		delete filter;
	}

	if (fName != NULL) delete[] fName;
	if (fToken != NULL) delete fToken;
	delete fObserverList;
}


BHandler::BHandler(const BMessage *from)
		: BArchivable(from),
		fName(NULL),
		fLooper(NULL),
		forceSetNextHandler(false), fNextHandler(NULL)
{
	ETK_ERROR("[APP]: %s --- unimplemented yet.", __PRETTY_FUNCTION__);
}


status_t
BHandler::Archive(BMessage *into, bool deep) const
{
	if (!into) return B_ERROR;

	BArchivable::Archive(into, deep);
	into->AddString("class", "BHandler");

	// TODO

	return B_OK;
}


BArchivable*
BHandler::Instantiate(const BMessage *from)
{
	return(e_validate_instantiation(from, "BHandler") == false ? NULL : new BHandler(from));
}


void
BHandler::SetName(const char *name)
{
	if (fName != NULL) delete[] fName;
	fName = EStrdup(name);
}


const char*
BHandler::Name() const
{
	return fName;
}


void
BHandler::MessageReceived(BMessage *message)
{
	if (fNextHandler != NULL) fNextHandler->MessageReceived(message);
}


void
BHandler::SetNextHandler(BHandler *handler)
{
	if (handler == this || fLooper == NULL || fNextHandler == handler) {
		if (handler == this) ETK_WARNING("[APP]: %s --- next-handler is this-handler.", __PRETTY_FUNCTION__);
		else if (fLooper == NULL) ETK_WARNING("[APP]: %s --- this-handler didn't belong to looper.", __PRETTY_FUNCTION__);
		else ETK_WARNING("[APP]: %s --- next-handler already be the next handler of this-handler.", __PRETTY_FUNCTION__);
		return;
	}

	if (forceSetNextHandler) {
		fNextHandler = handler;
		forceSetNextHandler = false;
	} else {
		if (handler == NULL || handler == fLooper) {
			ETK_WARNING("[APP]: %s --- next-handler can't be NULL or looper it belong to.", __PRETTY_FUNCTION__);
		} else if (handler->fLooper != fLooper) {
			ETK_WARNING("[APP]: %s --- this-handler and next-handler didn't belong to the same looper.", __PRETTY_FUNCTION__);
		} else {
			// reorder
			int32 handlerIndex = fLooper->fHandlers.IndexOf(handler);
			int32 selfIndex = fLooper->fHandlers.IndexOf(this);

			if (handlerIndex < 0 || selfIndex < 0 || handlerIndex == selfIndex) return;

			if (handlerIndex - 1 != selfIndex) {
				BHandler *handlerPrevHandler = (BHandler*)(fLooper->fHandlers.ItemAt(handlerIndex - 1));
				BHandler *handlerNextHandler = (BHandler*)(fLooper->fHandlers.ItemAt(handlerIndex + 1));
				int32 toIndex = handlerIndex < selfIndex ? selfIndex : selfIndex + 1;
				if (!(fLooper->fHandlers.MoveItem(handlerIndex, toIndex))) return;

				if (handlerPrevHandler) {
					handlerPrevHandler->forceSetNextHandler = true;
					handlerPrevHandler->SetNextHandler(handlerNextHandler);
					if (handlerPrevHandler->forceSetNextHandler) {
						handlerPrevHandler->fNextHandler = handlerNextHandler;
						handlerPrevHandler->forceSetNextHandler = false;
					}
				}
			}

			fNextHandler = handler;
		}
	}
}


BHandler*
BHandler::NextHandler() const
{
	return fNextHandler;
}


BLooper*
BHandler::Looper() const
{
	return fLooper;
}


void
BHandler::SetLooper(BLooper *looper)
{
	BAutolock <BLocker>autolock(handler_operator_locker);
	fLooper = looper;
}


bool
BHandler::LockLooper()
{
	return(LockLooperWithTimeout(B_INFINITE_TIMEOUT) == B_OK);
}


status_t
BHandler::LockLooperWithTimeout(bigtime_t microseconds_timeout)
{
	BAutolock <BLocker>autolock(handler_operator_locker);
	return(fLooper ? fLooper->LockWithTimeout(microseconds_timeout) :B_ERROR);
}


void
BHandler::UnlockLooper()
{
	BAutolock <BLocker>autolock(handler_operator_locker);
	if (fLooper) fLooper->Unlock();
}


typedef struct _watching_info_ {
	BMessenger msgr;
	BList whatsList;

	_watching_info_(BMessenger _msgr_) {
		msgr = _msgr_;
	}

	~_watching_info_() {
		if (!whatsList.IsEmpty()) {
			for (int32 i = 0; i < whatsList.CountItems(); i++) {
				uint32 *what = (uint32*)whatsList.ItemAt(i);
				if (what) delete what;
			}
			whatsList.MakeEmpty();
		}
	}

	bool IsValid() const {
		return msgr.IsValid();
	}

	bool IsSameMessenger(BMessenger _msgr_) const {
		return(msgr == _msgr_);
	}

	bool AddWhat(uint32 _what_) {
		if (_what_ == B_OBSERVER_OBSERVE_ALL || !msgr.IsValid()) return false;
		uint32 *what = new uint32;
		if (!what) return false;
		*what = _what_;
		if (!whatsList.AddItem((void*)what)) {
			delete what;
			return false;
		}
		return true;
	}

	bool RemoveWhat(uint32 _what_) {
		if (_what_ == B_OBSERVER_OBSERVE_ALL || !msgr.IsValid()) return false;
		for (int32 i = 0; i < whatsList.CountItems(); i++) {
			uint32 *what = (uint32*)whatsList.ItemAt(i);
			if (!what) continue;
			if (*what == _what_) {
				if ((what = (uint32*)whatsList.RemoveItem(i)) != NULL) {
					delete what;
					return true;
				}

				break;
			}
		}
		return false;
	}

	bool HasWhat(uint32 _what_) {
		if (_what_ == B_OBSERVER_OBSERVE_ALL || !msgr.IsValid()) return false;
		for (int32 i = 0; i < whatsList.CountItems(); i++) {
			uint32 *what = (uint32*)whatsList.ItemAt(i);
			if (!what) continue;
			if (*what == _what_) return true;
		}
		return false;
	}

	int32 CountWhats() const {
		return whatsList.CountItems();
	}

	void MakeEmpty() {
		if (!whatsList.IsEmpty()) {
			for (int32 i = 0; i < whatsList.CountItems(); i++) {
				uint32 *what = (uint32*)whatsList.ItemAt(i);
				if (what) delete what;
			}
			whatsList.MakeEmpty();
		}
	}
} _watching_info_;


_EObserverList::_EObserverList()
{
}


_EObserverList::~_EObserverList()
{
	if (!fListWatching.IsEmpty()) {
		for (int32 i = 0; i < fListWatching.CountItems(); i++) {
			_watching_info_ *aInfo = (_watching_info_*)fListWatching.ItemAt(i);
			if (aInfo) delete aInfo;
		}
		fListWatching.MakeEmpty();
	}

	if (!fListWatchingAll.IsEmpty()) {
		for (int32 i = 0; i < fListWatchingAll.CountItems(); i++) {
			_watching_info_ *aInfo = (_watching_info_*)fListWatchingAll.ItemAt(i);
			if (aInfo) delete aInfo;
		}
		fListWatchingAll.MakeEmpty();
	}
}


status_t
_EObserverList::StartWatching(BMessenger _msgr_, uint32 what)
{
	if (!_msgr_.IsValid()) return B_BAD_HANDLER;

	_watching_info_ *info = NULL;

	int32 msgrIndex = -1;
	for (int32 i = 0; i < fListWatching.CountItems(); i++) {
		_watching_info_ *aInfo = (_watching_info_*)fListWatching.ItemAt(i);
		if (!aInfo) continue;
		if (aInfo->IsSameMessenger(_msgr_)) {
			msgrIndex = i;
			break;
		}
	}

	int32 msgrAllIndex = -1;
	for (int32 i = 0; i < fListWatchingAll.CountItems(); i++) {
		_watching_info_ *aInfo = (_watching_info_*)fListWatchingAll.ItemAt(i);
		if (!aInfo) continue;
		if (aInfo->IsSameMessenger(_msgr_)) {
			msgrAllIndex = i;
			break;
		}
	}

	if (what == B_OBSERVER_OBSERVE_ALL) {
		if (msgrIndex >= 0) {
			if ((info = (_watching_info_*)fListWatching.RemoveItem(msgrIndex)) == NULL) return B_ERROR;
			delete info;
		}

		info = (_watching_info_*)fListWatchingAll.ItemAt(msgrAllIndex);

		if (!info) {
			info = new _watching_info_(_msgr_);
			if (!info || !info->IsValid() || !fListWatchingAll.AddItem((void*)info)) {
				if (info) delete info;
				return B_ERROR;
			}
		} else {
			info->MakeEmpty();
		}

		return B_OK;
	} else if (msgrAllIndex >= 0) {
		info = (_watching_info_*)fListWatchingAll.ItemAt(msgrAllIndex);
		info->RemoveWhat(what);
		return B_OK;
	}

	info = (_watching_info_*)fListWatching.ItemAt(msgrIndex);

	if (!info) {
		info = new _watching_info_(_msgr_);
		if (!info || !info->IsValid() || !info->AddWhat(what) || !fListWatching.AddItem((void*)info)) {
			if (info) delete info;
			return B_ERROR;
		}
		return B_OK;
	}

	if (info->HasWhat(what)) return B_OK;
	return(info->AddWhat(what) ?B_OK :B_ERROR);
}


status_t
_EObserverList::StopWatching(BMessenger _msgr_, uint32 what)
{
	if (!_msgr_.IsValid()) return B_BAD_HANDLER;

	_watching_info_ *info = NULL;

	int32 msgrIndex = -1;
	for (int32 i = 0; i < fListWatching.CountItems(); i++) {
		_watching_info_ *aInfo = (_watching_info_*)fListWatching.ItemAt(i);
		if (!aInfo) continue;
		if (aInfo->IsSameMessenger(_msgr_)) {
			msgrIndex = i;
			break;
		}
	}

	int32 msgrAllIndex = -1;
	if (msgrIndex >= 0) for (int32 i = 0; i < fListWatchingAll.CountItems(); i++) {
			_watching_info_ *aInfo = (_watching_info_*)fListWatchingAll.ItemAt(i);
			if (!aInfo) continue;
			if (aInfo->IsSameMessenger(_msgr_)) {
				msgrAllIndex = i;
				break;
			}
		}

	if (what == B_OBSERVER_OBSERVE_ALL) {
		if (msgrIndex >= 0) {
			if ((info = (_watching_info_*)fListWatching.RemoveItem(msgrIndex)) == NULL) return B_ERROR;
			delete info;
		} else if (msgrAllIndex >= 0) {
			if ((info = (_watching_info_*)fListWatchingAll.RemoveItem(msgrAllIndex)) == NULL) return B_ERROR;
			delete info;
		}

		return B_OK;
	}

	if (msgrAllIndex >= 0) {
		info = (_watching_info_*)fListWatchingAll.ItemAt(msgrAllIndex);
		if (!info->HasWhat(what)) return(info->AddWhat(what) ?B_OK :B_ERROR);

		return B_OK;
	}

	info = (_watching_info_*)fListWatching.ItemAt(msgrIndex);

	if (!info || !info->HasWhat(what)) return B_OK;

	if (!info->RemoveWhat(what)) return B_ERROR;

	if (info->CountWhats() <= 0) {
		if (!fListWatching.RemoveItem(info)) return B_ERROR;
		delete info;
	}

	return B_OK;
}


BList*
_EObserverList::GetObservers(uint32 what)
{
	BList *retList = new BList();
	if (!retList) return NULL;

	if (what == B_OBSERVER_OBSERVE_ALL) {
		for (int32 i = 0; i < fListWatchingAll.CountItems(); i++) {
			_watching_info_ *aInfo = (_watching_info_*)fListWatchingAll.ItemAt(i);
			if (!aInfo) continue;
			retList->AddItem((void*)(&(aInfo->msgr)));
		}

		for (int32 i = 0; i < fListWatching.CountItems(); i++) {
			_watching_info_ *aInfo = (_watching_info_*)fListWatching.ItemAt(i);
			if (!aInfo) continue;
			retList->AddItem((void*)(&(aInfo->msgr)));
		}
	} else {
		for (int32 i = 0; i < fListWatching.CountItems(); i++) {
			_watching_info_ *aInfo = (_watching_info_*)fListWatching.ItemAt(i);
			if (!aInfo) continue;
			if (aInfo->HasWhat(what)) retList->AddItem((void*)(&(aInfo->msgr)));
		}

		for (int32 i = 0; i < fListWatchingAll.CountItems(); i++) {
			_watching_info_ *aInfo = (_watching_info_*)fListWatchingAll.ItemAt(i);
			if (!aInfo) continue;
			if (!aInfo->HasWhat(what)) retList->AddItem((void*)(&(aInfo->msgr)));
		}
	}

	if (retList->IsEmpty()) {
		delete retList;
		retList = NULL;
	}

	return retList;
}


bool
_EObserverList::IsWatched(uint32 what) const
{
	if (what == B_OBSERVER_OBSERVE_ALL) {
		return(!fListWatching.IsEmpty() || !fListWatchingAll.IsEmpty());
	} else {
		for (int32 i = 0; i < fListWatching.CountItems(); i++) {
			_watching_info_ *aInfo = (_watching_info_*)fListWatching.ItemAt(i);
			if (!aInfo) continue;
			if (aInfo->HasWhat(what)) return true;
		}

		for (int32 i = 0; i < fListWatchingAll.CountItems(); i++) {
			_watching_info_ *aInfo = (_watching_info_*)fListWatchingAll.ItemAt(i);
			if (!aInfo) continue;
			if (!aInfo->HasWhat(what)) return true;
		}
	}

	return false;
}


status_t
BHandler::StartWatching(BMessenger msgr, uint32 what)
{
	if (!fObserverList) return B_ERROR;
	return fObserverList->StartWatching(msgr, what);
}


status_t
BHandler::StartWatchingAll(BMessenger msgr)
{
	return StartWatching(msgr, B_OBSERVER_OBSERVE_ALL);
}


status_t
BHandler::StopWatching(BMessenger msgr, uint32 what)
{
	if (!fObserverList) return B_ERROR;
	return fObserverList->StopWatching(msgr, what);
}


status_t
BHandler::StopWatchingAll(BMessenger msgr)
{
	return StopWatching(msgr, B_OBSERVER_OBSERVE_ALL);
}


status_t
BHandler::StartWatching(BHandler *handler, uint32 what)
{
	status_t status;
	BMessenger msgr(handler, NULL, &status);
	if (status != B_OK) return status;

	return StartWatching(msgr, what);
}


status_t
BHandler::StartWatchingAll(BHandler *handler)
{
	status_t status;
	BMessenger msgr(handler, NULL, &status);
	if (status != B_OK) return status;

	return StartWatchingAll(msgr);
}


status_t
BHandler::StopWatching(BHandler *handler, uint32 what)
{
	status_t status;
	BMessenger msgr(handler, NULL, &status);
	if (status != B_OK) return status;

	return StopWatching(msgr, what);
}


status_t
BHandler::StopWatchingAll(BHandler *handler)
{
	status_t status;
	BMessenger msgr(handler, NULL, &status);
	if (status != B_OK) return status;

	return StopWatchingAll(msgr);
}


void
BHandler::SendNotices(uint32 what, const BMessage *_msg_)
{
	if (!fObserverList) return;
	BList *msgrsList = fObserverList->GetObservers(what);
	if (!msgrsList) return;

	BMessage msg(B_OBSERVER_NOTICE_CHANGE);
	if (_msg_) {
		msg = *_msg_;
		msg.what = B_OBSERVER_NOTICE_CHANGE;
		msg.AddInt32(B_OBSERVE_ORIGINAL_WHAT, _msg_->what);
	}
	msg.AddInt32(B_OBSERVE_WHAT_CHANGE, what);

	for (int32 i = 0; i < msgrsList->CountItems(); i++) {
		BMessenger *aMsgr = (BMessenger*)msgrsList->ItemAt(i);
		if (!aMsgr) continue;

		if (aMsgr->SendMessage(&msg, (BHandler*)NULL, 20000) != B_OK) {
			if (aMsgr->IsTargetLocal()) {
				BLooper *looper = NULL;
				aMsgr->Target(&looper);
				if (looper == NULL) StopWatchingAll(*aMsgr);
			} else {
				// TODO
			}
		}
	}

	delete msgrsList;
}


bool
BHandler::IsWatched(uint32 what) const
{
	if (!fObserverList) return false;
	return fObserverList->IsWatched(what);
}


bool
BHandler::AddFilter(BMessageFilter *filter)
{
	if (filter == NULL || filter->fHandler != NULL || fFilters.AddItem(filter) == false) return false;
	filter->fHandler = this;
	filter->fLooper = Looper();
	return true;
}


bool
BHandler::RemoveFilter(BMessageFilter *filter)
{
	if (filter == NULL || filter->fHandler != this || fFilters.RemoveItem(filter) == false) return false;
	filter->fHandler = NULL;
	filter->fLooper = NULL;
	return true;
}


const BList*
BHandler::FilterList() const
{
	return(&fFilters);
}


bool
BHandler::SetFilterList(const BList *filterList)
{
	while (fFilters.CountItems() > 0) {
		BMessageFilter *filter = (BMessageFilter*)fFilters.ItemAt(0);
		BHandler::RemoveFilter(filter);
		delete filter;
	}

	if (filterList == NULL) return true;
	for (int32 i = 0; i < filterList->CountItems(); i++) BHandler::AddFilter((BMessageFilter*)filterList->ItemAt(i));
	return true;
}

