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
 * File: Messenger.cpp
 *
 * --------------------------------------------------------------------------*/

#include <stdlib.h>

#include <kernel/Kernel.h>
#include <support/ClassInfo.h>
#include <support/Locker.h>
#include <support/Autolock.h>

#include "Messenger.h"

extern BHandler* get_handler(uint64 token);
extern BLooper* get_handler_looper(uint64 token);
extern bool ref_handler(uint64 token);
extern void unref_handler(uint64 token);
extern bool is_current_at_looper_thread(uint64 token);
extern uint64 get_ref_handler_token(const BHandler *handler);
extern bigtime_t get_handler_create_time_stamp(uint64 token);
extern BLocker* get_handler_operator_locker();
extern uint64 get_ref_looper_token(uint64 token);


BMessenger::BMessenger()
		: fHandlerToken(B_MAXUINT64), fLooperToken(B_MAXUINT64),
		fPort(NULL), fSem(NULL), fTargetTeam(B_INT64_CONSTANT(0))
{
}


BMessenger::BMessenger(const char *signature, int64 team, status_t *perr)
		: fHandlerToken(B_MAXUINT64), fLooperToken(B_MAXUINT64),
		fPort(NULL), fSem(NULL), fTargetTeam(B_INT64_CONSTANT(0))
{
	// TODO
	ETK_WARNING("[APP]: %s --- Remote target unsupported yet.", __PRETTY_FUNCTION__);
	if (perr) *perr = B_ERROR;
}


BMessenger::BMessenger(const BHandler *handler,
                       const BLooper *looper,
                       status_t *perr)
		: fHandlerToken(B_MAXUINT64), fLooperToken(B_MAXUINT64),
		fPort(NULL), fSem(NULL), fTargetTeam(B_INT64_CONSTANT(0))
{
	InitData(handler, looper, perr);
}


BMessenger::BMessenger(int64 targetTeam, uint64 targetToken, bigtime_t timestamp, status_t *perr)
		: fHandlerToken(B_MAXUINT64), fLooperToken(B_MAXUINT64),
		fPort(NULL), fSem(NULL), fTargetTeam(B_INT64_CONSTANT(0))
{
	if (targetTeam != get_current_team_id()) {
		ETK_WARNING("[APP]: %s --- Remote target unsupported yet.", __PRETTY_FUNCTION__);
		if (perr) *perr = B_ERROR;
		return;
	}

	if (ref_handler(targetToken) == false) {
		if (perr) *perr = B_ERROR;
		return;
	}

	uint64 looperToken = B_MAXUINT64;

	if (get_handler_create_time_stamp(targetToken) != timestamp ||
	        (looperToken = get_ref_looper_token(targetToken)) == B_MAXUINT64) {
		unref_handler(targetToken);
		if (perr) *perr = B_ERROR;
		return;
	}

	fTargetTeam = targetTeam;
	fHandlerToken = targetToken;
	fLooperToken = looperToken;

	if (perr) *perr = B_OK;
}


void
BMessenger::InitData(const BHandler *handler, const BLooper *looper, status_t *perr)
{
	if (fHandlerToken != B_MAXUINT64) unref_handler(fHandlerToken);
	if (fLooperToken != B_MAXUINT64) unref_handler(fLooperToken);

	fHandlerToken = B_MAXUINT64;
	fLooperToken = B_MAXUINT64;

	if (fSem) delete_sem(fSem);
	if (fPort) delete_port(fPort);

	fSem = NULL;
	fPort = NULL;
	fTargetTeam = B_INT64_CONSTANT(0);

	if (perr) *perr = B_BAD_HANDLER;

	if (handler) {
		uint64 handlerToken = get_ref_handler_token(handler);
		if (handlerToken == B_MAXUINT64) return;

		uint64 looperToken = get_ref_looper_token(handlerToken);
		if (looperToken == B_MAXUINT64) {
			unref_handler(handlerToken);
			return;
		}

		fTargetTeam = get_current_team_id();
		fHandlerToken = handlerToken;
		fLooperToken = looperToken;

		if (perr) *perr = B_OK;
	} else if (looper) {
		uint64 looperToken = get_ref_handler_token(looper);
		if (looperToken == B_MAXUINT64) return;

		fTargetTeam = get_current_team_id();
		fHandlerToken = B_MAXUINT64;
		fLooperToken = looperToken;

		if (perr) *perr = B_OK;
	}
}


BMessenger::BMessenger(const BMessenger &from)
		: fHandlerToken(B_MAXUINT64), fLooperToken(B_MAXUINT64),
		fPort(NULL), fSem(NULL), fTargetTeam(B_INT64_CONSTANT(0))
{
	*this = from;
}


BMessenger&
BMessenger::operator=(const BMessenger &from)
{
	if (fHandlerToken != B_MAXUINT64) unref_handler(fHandlerToken);
	if (fLooperToken != B_MAXUINT64) unref_handler(fLooperToken);

	fHandlerToken = B_MAXUINT64;
	fLooperToken = B_MAXUINT64;

	if (fSem) delete_sem(fSem);
	if (fPort) delete_port(fPort);

	fSem = NULL;
	fPort = NULL;
	fTargetTeam = B_INT64_CONSTANT(0);

	if (!from.IsValid()) return *this;

	if (!from.IsTargetLocal()) {
		// TODO
		return *this;
	}

	if (from.fHandlerToken != B_MAXUINT64) if (ref_handler(from.fHandlerToken)) fHandlerToken = from.fHandlerToken;
	if (from.fLooperToken != B_MAXUINT64) if (ref_handler(from.fLooperToken)) fLooperToken = from.fLooperToken;

	if (fLooperToken == B_MAXUINT64 || fHandlerToken != from.fHandlerToken) {
		if (fHandlerToken != B_MAXUINT64) unref_handler(fHandlerToken);
		if (fLooperToken != B_MAXUINT64) unref_handler(fLooperToken);

		fHandlerToken = B_MAXUINT64;
		fLooperToken = B_MAXUINT64;

		return *this;
	}

	fTargetTeam = from.fTargetTeam;

	return *this;
}


bool
BMessenger::operator==(const BMessenger &other) const
{
	if (fTargetTeam != other.fTargetTeam) return false;
	return(fHandlerToken == other.fHandlerToken && fLooperToken == other.fLooperToken);
}


bool
BMessenger::operator!=(const BMessenger &other) const
{
	if (fTargetTeam != other.fTargetTeam) return true;
	return((fHandlerToken == other.fHandlerToken && fLooperToken == other.fLooperToken) ? false : true);
}


BMessenger::~BMessenger()
{
	InitData(NULL, NULL, NULL);
}


bool
BMessenger::IsTargetLocal() const
{
	return(fTargetTeam == get_current_team_id());
}


bool
BMessenger::IsAtTargetLooperThread() const
{
	if (fTargetTeam != get_current_team_id()) return false;
	return is_current_at_looper_thread(fLooperToken);
}


BHandler*
BMessenger::Target(BLooper **looper) const
{
	if (looper) *looper = NULL;
	if (!IsTargetLocal()) return NULL;

	BHandler *handler = get_handler(fHandlerToken);
	if (looper) *looper = cast_as(get_handler(fLooperToken), BLooper);
	return handler;
}


bool
BMessenger::LockTarget() const
{
	return(LockTargetWithTimeout(B_INFINITE_TIMEOUT) == B_OK);
}


status_t
BMessenger::LockTargetWithTimeout(bigtime_t timeout) const
{
	if (!IsTargetLocal()) return B_ERROR;

	BLocker *hLocker = get_handler_operator_locker();
	BAutolock <BLocker>autolock(hLocker);
	BLooper *looper = get_handler_looper(fLooperToken);

	return(looper ? looper->LockWithTimeout(timeout) :B_ERROR);
}


bool
BMessenger::IsValid() const
{
	if (IsTargetLocal()) return(fLooperToken != B_MAXUINT64);
	else return(fPort != NULL && fSem != NULL);
}


status_t
BMessenger::SendMessage(uint32 command, BHandler *reply_to) const
{
	BMessage msg(command);
	return SendMessage(&msg, reply_to, B_INFINITE_TIMEOUT);
}


status_t
BMessenger::SendMessage(const BMessage *a_message,
                        BHandler *reply_to,
                        bigtime_t timeout) const
{
	if (a_message == NULL) {
		ETK_WARNING("[APP]: %s --- Can't post empty message.", __PRETTY_FUNCTION__);
		return B_BAD_VALUE;
	}

	uint64 replyToken = get_handler_token(reply_to);

	BMessage aMsg(*a_message);
	aMsg.fIsReply = false;
	if (aMsg.fSource != NULL) {
		delete_port(aMsg.fSource);
		aMsg.fSource = NULL;
	}

	return _SendMessage(&aMsg, replyToken, timeout);
}


status_t
BMessenger::SendMessage(const BMessage *a_message, BMessage *reply_message, bigtime_t sendTimeout, bigtime_t replyTimeout) const
{
	if (a_message == NULL || reply_message == NULL) {
		ETK_WARNING("[APP]: %s --- Can't post empty message or \"reply_message\" assigned to be \"NULL\".", __PRETTY_FUNCTION__);
		return B_BAD_VALUE;
	}

	void *port = create_port(1, NULL, ETK_AREA_ACCESS_OWNER);
	if (port == NULL) return B_NO_MORE_PORTS;

	BMessage *aMsg = new BMessage(*a_message);
	if (aMsg->fSource != NULL) delete_port(aMsg->fSource);
	aMsg->fTeam = get_current_team_id();
	aMsg->fIsReply = false;
	aMsg->fReplyToken = B_MAXUINT64;
	aMsg->fReplyTokenTimestamp = B_MAXINT64;
	aMsg->fNoticeSource = true; // auto close the port when deleted
	aMsg->fSource = port; // auto delete the port when deleted

	status_t status = _SendMessage(aMsg, B_MAXUINT64, sendTimeout);
	if (status == B_OK) {
		BMessage *reply = _GetMessageFromPort(port, B_TIMEOUT, replyTimeout, &status);
		if (reply != NULL) {
			*reply_message = *reply;
			delete reply;
		} else {
			reply_message->what = B_NO_REPLY;
		}
	}

	delete aMsg;
	return status;
}


status_t
BMessenger::_SendMessage(const BMessage *a_message,
                         uint64 replyToken,
                         bigtime_t timeout) const
{
	if (a_message == NULL) return B_BAD_VALUE;
	if (!IsValid()) return B_ERROR;

	if (!IsTargetLocal()) {
		// TODO
		return B_ERROR;
	}

	status_t retVal = B_ERROR;

	BLocker *hLocker = get_handler_operator_locker();
	BAutolock <BLocker>autolock(hLocker);

	BLooper *looper = cast_as(get_handler(fLooperToken), BLooper);
	if (looper) retVal = looper->_PostMessage(a_message, fHandlerToken, replyToken, timeout);

	return retVal;
}


status_t
BMessenger::_SendMessageToPort(void *port, const BMessage *msg, uint32 flags, bigtime_t timeout)
{
	if (!port || !msg) return B_ERROR;

	size_t flattenedSize = msg->FlattenedSize();
	if (flattenedSize <= 0) {
		ETK_WARNING("[APP]: Faltten size little than 1. (%s:%d)", __FILE__, __LINE__);
		return B_ERROR;
	}

	char *buffer = (char*)malloc(flattenedSize);
	if (!buffer) {
		ETK_WARNING("[APP]: Buffer malloc failed. (%s:%d)", __FILE__, __LINE__);
		return B_NO_MEMORY;
	}

	if (msg->Flatten(buffer, flattenedSize) == false) {
		free(buffer);
		ETK_WARNING("[APP]: Flatten message failed. (%s:%d)", __FILE__, __LINE__);
		return B_ERROR;
	}

	status_t status;
	if ((status = write_port_etc(port, _EVENTS_PENDING_, buffer, flattenedSize, flags, timeout)) != B_OK) {
		free(buffer);
		ETK_WARNING("[APP]: write port %s. (%s:%d)", status == B_TIMEOUT ? "time out" : "failed", __FILE__, __LINE__);
		return status;
	}

	free(buffer);

	return B_OK;
}


BMessage*
BMessenger::_GetMessageFromPort(void *port, uint32 flags, bigtime_t timeout, status_t *err)
{
	status_t retErr = B_OK;
	BMessage* retMsg = NULL;

	do {
		ssize_t bufferSize = port_buffer_size_etc(port, flags, timeout);
		if (bufferSize == 0) {
			int32 code;
			retErr = read_port_etc(port, &code, NULL, 0, B_TIMEOUT, B_INT64_CONSTANT(0));
//			if(retErr != B_OK) ETK_DEBUG("[APP]: Port read failed(0x%x). (%s:%d)", retErr, __FILE__, __LINE__);
			break;
		} else if (bufferSize < 0) {
			retErr = bufferSize;
//			if(!(retErr == B_WOULD_BLOCK || retErr == B_TIMED_OUT))
//				ETK_DEBUG("[APP]: Port read failed(0x%x). (%s:%d)", retErr, __FILE__, __LINE__);
			break;
		}

		char *buffer = (char*)malloc((size_t)bufferSize);
		if (!buffer) {
			retErr = B_NO_MEMORY;
			ETK_WARNING("[APP]: Memory alloc failed. (%s:%d)", __FILE__, __LINE__);
			break;
		}
		bzero(buffer, (size_t)bufferSize);

		int32 code;
		if ((retErr = read_port_etc(port, &code, buffer, bufferSize, B_TIMEOUT, B_INT64_CONSTANT(0))) != B_OK) {
//			ETK_DEBUG("[APP]: Port read failed(0x%x). (%s:%d)", retErr, __FILE__, __LINE__);
			free(buffer);
			break;
		}
		if (code != _EVENTS_PENDING_ || (size_t)bufferSize < sizeof(size_t)) {
			ETK_WARNING("[APP]: Message is invalid. (%s:%d)", __FILE__, __LINE__);
			free(buffer);
			retErr = B_ERROR;
			break;
		}

		size_t msgBufferSize = 0;
		memcpy(&msgBufferSize, buffer, sizeof(size_t));
		if (bufferSize != (ssize_t)msgBufferSize) { /* the first "size_t" == FlattenedSize() */
			ETK_WARNING("[APP]: Message length is invalid. (%s:%d)", __FILE__, __LINE__);
			free(buffer);
			retErr = B_ERROR;
			break;
		}

		if ((retMsg = new BMessage()) == NULL) {
			ETK_WARNING("[APP]: Memory alloc failed. (%s:%d)", __FILE__, __LINE__);
			free(buffer);
			retErr = B_NO_MEMORY;
			break;
		}

		if (retMsg->Unflatten(buffer, msgBufferSize) == false) {
			ETK_WARNING("[APP]: Message unflatten failed. (%s:%d)", __FILE__, __LINE__);
			delete retMsg;
			retMsg = NULL;
			retErr = B_ERROR;
		}

		free(buffer);
	} while (false);

	if (err) *err = retErr;
	return retMsg;
}


size_t
BMessenger::FlattenedSize() const
{
	return(2 * sizeof(uint64) + 2 * sizeof(bigtime_t) + sizeof(int64));
}


bool
BMessenger::Flatten(char *buffer, size_t bufferSize) const
{
	if (buffer == NULL || bufferSize < FlattenedSize()) return false;

	bigtime_t handler_stamp = get_handler_create_time_stamp(fHandlerToken);
	bigtime_t looper_stamp = get_handler_create_time_stamp(fLooperToken);

	memcpy(buffer, &fTargetTeam, sizeof(int64));
	buffer += sizeof(int64);
	memcpy(buffer, &fHandlerToken, sizeof(uint64));
	buffer += sizeof(uint64);
	memcpy(buffer, &handler_stamp, sizeof(bigtime_t));
	buffer += sizeof(bigtime_t);
	memcpy(buffer, &fLooperToken, sizeof(uint64));
	buffer += sizeof(uint64);
	memcpy(buffer, &looper_stamp, sizeof(bigtime_t));

	return true;
}


bool
BMessenger::Unflatten(const char *buffer, size_t bufferSize)
{
	if (buffer == NULL || bufferSize < FlattenedSize()) return false;

	bigtime_t handler_stamp = B_MAXINT64;
	bigtime_t looper_stamp = B_MAXINT64;

	int64 target_team = B_INT64_CONSTANT(0);
	uint64 handler_token = B_MAXUINT64;
	uint64 looper_token = B_MAXUINT64;

	memcpy(&target_team, buffer, sizeof(int64));
	buffer += sizeof(int64);
	memcpy(&handler_token, buffer, sizeof(uint64));
	buffer += sizeof(uint64);
	memcpy(&handler_stamp, buffer, sizeof(bigtime_t));
	buffer += sizeof(bigtime_t);
	memcpy(&looper_token, buffer, sizeof(uint64));
	buffer += sizeof(uint64);
	memcpy(&looper_stamp, buffer, sizeof(bigtime_t));

	if (fHandlerToken != B_MAXUINT64) unref_handler(fHandlerToken);
	if (fLooperToken != B_MAXUINT64) unref_handler(fLooperToken);

	fHandlerToken = B_MAXUINT64;
	fLooperToken = B_MAXUINT64;

	if (fSem) delete_sem(fSem);
	if (fPort) delete_port(fPort);

	fSem = NULL;
	fPort = NULL;
	fTargetTeam = B_INT64_CONSTANT(0);

	do {
		if (target_team != get_current_team_id()) {
			// TODO
			ETK_DEBUG("[APP]: %s --- Remote target unsupported.", __PRETTY_FUNCTION__);
			break;
		}

		if (handler_token == B_MAXUINT64 && looper_token == B_MAXUINT64) break;
		if (handler_stamp == B_MAXINT64 && looper_stamp == B_MAXINT64) break;

		if (handler_token != B_MAXUINT64) if (ref_handler(handler_token)) fHandlerToken = handler_token;
		if (looper_token != B_MAXUINT64) if (ref_handler(looper_token)) fLooperToken = looper_token;

		if (fLooperToken == B_MAXUINT64 || fHandlerToken != handler_token ||
		        (fHandlerToken == B_MAXUINT64 ? false : get_handler_create_time_stamp(fHandlerToken) != handler_stamp) ||
		        (fLooperToken == B_MAXUINT64 ? false : get_handler_create_time_stamp(fLooperToken) != looper_stamp)) {
			if (fHandlerToken != B_MAXUINT64) unref_handler(fHandlerToken);
			if (fLooperToken != B_MAXUINT64) unref_handler(fLooperToken);
			fHandlerToken = B_MAXUINT64;
			fLooperToken = B_MAXUINT64;
			ETK_DEBUG("[APP]: %s --- Invalid target.", __PRETTY_FUNCTION__);
			break;
		}

		fTargetTeam = target_team;
	} while (false);

	return true;
}


void
BMessenger::PrintToStream() const
{
	ETK_OUTPUT("******** BMessenger Debug Output **********\n");

	ETK_OUTPUT("\tTarget team: %I64i\n", fTargetTeam);
	ETK_OUTPUT("\tToken of target handler: ");

	if (fHandlerToken == B_MAXUINT64) ETK_OUTPUT("B_MAXUINT64\n");
	else ETK_OUTPUT("%I64u\n", fHandlerToken);

	ETK_OUTPUT("\tToken of target looper: ");
	if (fLooperToken == B_MAXUINT64) ETK_OUTPUT("B_MAXUINT64\n");
	else ETK_OUTPUT("%I64u - %p\n", fLooperToken, get_handler(fLooperToken));

	if (fPort == NULL) ETK_OUTPUT("\tPort invalid.\n");
	if (fSem == NULL) ETK_OUTPUT("\tSemaphore invalid.\n");

	ETK_OUTPUT("*******************************************\n");
}

