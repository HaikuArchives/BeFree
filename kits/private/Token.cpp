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
 * File: Token.cpp
 *
 * --------------------------------------------------------------------------*/

#include <kernel/Kernel.h>
#include <support/Locker.h>
#include <support/List.h>

#include "Token.h"


struct _LOCAL _token_t {
	uint64 count;
	bigtime_t time_stamp;
	void *data;
};


class _LOCAL BTokensDepotPrivateData : public BList
{
	public:
		BTokensDepotPrivateData();
		virtual ~BTokensDepotPrivateData();

		uint64		AddToken(void *data);
		void		RemoveToken(uint64 token);
		_token_t	*TokenAt(uint64 token) const;

		bool		PushToken(uint64 token);
		void		PopToken(uint64 token);
};


BTokensDepotPrivateData::BTokensDepotPrivateData()
		: BList()
{
}


BTokensDepotPrivateData::~BTokensDepotPrivateData()
{
	BList *list;

	while ((list = (BList*)RemoveItem(0)) != NULL) {
		for (int32 i = 0; i < list->CountItems(); i++) {
			_token_t *aToken = (_token_t*)list->ItemAt(i);
			free(aToken);
		}
		delete list;
	}
}


uint64
BTokensDepotPrivateData::AddToken(void *data)
{
	_token_t *aToken = (_token_t*)malloc(sizeof(_token_t));

	if (aToken == NULL) return B_MAXUINT64;

	uint64 token = B_MAXUINT64;

	BList *list = NULL;
	for (int32 i = 0; i < CountItems(); i++) {
		list = (BList*)ItemAt(i);
		if (list->CountItems() >= B_MAXINT32 - 1 || list->AddItem(aToken) == false) {
			list = NULL;
			continue;
		}
		token = ((uint64)i << 32) | (uint64)(list->CountItems() - 1);
	}

	if (list == NULL && CountItems() <B_MAXINT32 - 1) {
		if (AddItem(list = new BList()) == false || list->AddItem(aToken) == false) {
			delete list;
		} else {
			token = ((uint64)(CountItems() - 1) << 32) | (uint64)(list->CountItems() - 1);
		}
	} else for (int32 i = 0; i < CountItems(); i++) {
			list = (BList*)ItemAt(i);
			int32 index = list->IndexOf(NULL);
			if (index < 0) {
				list = NULL;
				continue;
			}
			list->ReplaceItem(index, aToken);
			token = ((uint64)i << 32) | (uint64)index;
		}

	if (token != B_MAXUINT64) {
		aToken->count = 1;
		aToken->time_stamp = e_system_time();
		while (aToken->time_stamp == e_system_time()) {
			// do nothing, waiting till "e_system_time()" changed.
		}
		aToken->data = data;
	} else {
		free(aToken);
	}

	return token;
}


void
BTokensDepotPrivateData::RemoveToken(uint64 token)
{
	uint64 index = token >> 32;
	if (index > (uint64)B_MAXINT32 - 1) return;

	BList *list = (BList*)ItemAt((int32)index);
	if (list == NULL) return;

	index = token & 0xffffffff;
	_token_t *aToken = (_token_t*)(list->ItemAt((int32)index));
	if (aToken == NULL) return;

	if (aToken->count > 1) {
		aToken->count -= 1;
		aToken->data = NULL;
	} else {
		if (index < (uint64)list->CountItems() - 1) {
			list->ReplaceItem((int32)index, NULL);
		} else {
			list->RemoveItem((int32)index);
			while (list->LastItem() == NULL && list->IsEmpty() == false) list->RemoveItem(list->CountItems() - 1);
			if (list->IsEmpty() && LastItem() == (void*)list) delete	(BList*)RemoveItem(CountItems() - 1);
		}
		free(aToken);
	}
}


_token_t*
BTokensDepotPrivateData::TokenAt(uint64 token) const
{
	uint64 index = token >> 32;
	if (index > (uint64)B_MAXINT32 - 1) return NULL;

	BList *list = (BList*)ItemAt((int32)index);
	index = token & 0xffffffff;
	return((_token_t*)(list->ItemAt((int32)index)));
}


bool
BTokensDepotPrivateData::PushToken(uint64 token)
{
	_token_t *aToken = TokenAt(token);
	if (aToken == NULL || aToken->count == B_MAXUINT64) return false;
	aToken->count += 1;
	return true;
}


void
BTokensDepotPrivateData::PopToken(uint64 token)
{
	_token_t *aToken = TokenAt(token);
	if (!(aToken == NULL || aToken->count == 0)) {
		aToken->count -= 1;
		if (aToken->count == 0) RemoveToken(token);
	}
}


BTokensDepot::BTokensDepot(BLocker *locker, bool deconstruct_locker)
		: fLocker(locker), fDeconstructLocker(deconstruct_locker)
{
	fData = reinterpret_cast<void*>(new BTokensDepotPrivateData());
}


BTokensDepot::~BTokensDepot()
{
	delete reinterpret_cast<BTokensDepotPrivateData*>(fData);
	if (fDeconstructLocker && fLocker != NULL) delete fLocker;
}


BToken*
BTokensDepot::CreateToken(void *data)
{
	BToken *aToken = NULL;

	if (Lock()) {
		uint64 token = (reinterpret_cast<BTokensDepotPrivateData*>(fData))->AddToken(data);
		if (token != B_MAXUINT64) {
			aToken = new BToken();
			aToken->fOriginal = true;
			aToken->fToken = token;
			aToken->fDepot = this;
		}
		Unlock();
	}

	return aToken;
}


BToken*
BTokensDepot::OpenToken(uint64 token, BToken *fetch_token)
{
	BToken *aToken = NULL;

	if (fetch_token == NULL || fetch_token->fDepot == NULL) {
		if (Lock()) {
			if ((reinterpret_cast<BTokensDepotPrivateData*>(fData))->PushToken(token)) {
				aToken = (fetch_token != NULL ? fetch_token : new BToken());
				aToken->fToken = token;
				aToken->fDepot = this;
			}
			Unlock();
		}
	} else {
		ETK_WARNING("[PRIVATE]: %s --- fetch_token->fDepot != NULL\n", __PRETTY_FUNCTION__);
	}

	return aToken;
}


bool
BTokensDepot::PushToken(uint64 token)
{
	bool retVal = false;

	if (Lock()) {
		retVal = (reinterpret_cast<BTokensDepotPrivateData*>(fData))->PushToken(token);
		Unlock();
	}

	return retVal;
}


void
BTokensDepot::PopToken(uint64 token)
{
	if (Lock()) {
		(reinterpret_cast<BTokensDepotPrivateData*>(fData))->PopToken(token);
		Unlock();
	}
}


bool
BTokensDepot::Lock()
{
	if (fLocker == NULL) return true;
	return fLocker->Lock();
}


void
BTokensDepot::Unlock()
{
	if (fLocker != NULL) fLocker->Unlock();
}


BToken::BToken()
		: fOriginal(false), fToken(B_MAXUINT64), fDepot(NULL)
{
}


BToken::~BToken()
{
	MakeEmpty();
}


uint64
BToken::Token() const
{
	return fToken;
}


bigtime_t
BToken::TimeStamp() const
{
	bigtime_t retVal = B_MAXINT64;

	if (fToken != B_MAXUINT64 && fDepot != NULL) {
		if (fDepot->Lock()) {
			BTokensDepotPrivateData *depot_private = reinterpret_cast<BTokensDepotPrivateData*>(fDepot->fData);
			_token_t *aToken = depot_private->TokenAt(fToken);
			if (aToken != NULL) retVal = aToken->time_stamp;
			fDepot->Unlock();
		}
	}

	return retVal;
}


void*
BToken::Data() const
{
	void *retVal = NULL;

	if (fToken != B_MAXUINT64 && fDepot != NULL) {
		if (fDepot->Lock()) {
			BTokensDepotPrivateData *depot_private = reinterpret_cast<BTokensDepotPrivateData*>(fDepot->fData);
			_token_t *aToken = depot_private->TokenAt(fToken);
			if (aToken != NULL) retVal = aToken->data;
			fDepot->Unlock();
		}
	}

	return retVal;
}


void
BToken::SetData(void *data)
{
	if (fOriginal == false || fToken == B_MAXUINT64 || fDepot == NULL) return;

	if (fDepot->Lock()) {
		BTokensDepotPrivateData *depot_private = reinterpret_cast<BTokensDepotPrivateData*>(fDepot->fData);
		_token_t *aToken = depot_private->TokenAt(fToken);
		if (aToken != NULL) aToken->data = data;
		fDepot->Unlock();
	}
}


BTokensDepot*
BToken::Depot() const
{
	return fDepot;
}


void
BToken::MakeEmpty()
{
	if (fToken != B_MAXUINT64 && fDepot != NULL) {
		if (fDepot->Lock()) {
			BTokensDepotPrivateData *depot_private = reinterpret_cast<BTokensDepotPrivateData*>(fDepot->fData);
			if (fOriginal)
				depot_private->RemoveToken(fToken);
			else
				depot_private->PopToken(fToken);
			fDepot->Unlock();
		}
	}

	fToken = B_MAXUINT64;
	fDepot = NULL;
}

