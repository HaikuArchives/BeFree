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
 * File: Handler.h
 * Description: Basic object model for Application Kit
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_HANDLER_H__
#define __ETK_HANDLER_H__

#include <support/Archivable.h>
#include <support/List.h>

#ifdef __cplusplus /* Just for C++ */

class BLooper;
class BMessage;
class BMessageFilter;
class BMessenger;
class BToken;
class _EObserverList;

#define B_OBSERVE_WHAT_CHANGE		"etk:observe_change_what"
#define B_OBSERVE_ORIGINAL_WHAT		"etk:observe_orig_what"
#define B_OBSERVER_OBSERVE_ALL		B_MAXUINT32

class BHandler : public BArchivable
{
	public:
		BHandler(const char *name = NULL);
		virtual ~BHandler();

		// Archiving
		BHandler(const BMessage *from);
		virtual status_t Archive(BMessage *into, bool deep = true) const;
		static BArchivable *Instantiate(const BMessage *from);

		void		SetName(const char *name);
		const char	*Name() const;

		virtual void	MessageReceived(BMessage *message);

		BLooper		*Looper() const;

		virtual void	SetNextHandler(BHandler *handler);
		BHandler	*NextHandler() const;

		bool		LockLooper();
		status_t	LockLooperWithTimeout(bigtime_t microseconds_timeout);
		void		UnlockLooper();

		status_t	StartWatching(BMessenger msgr, uint32 what);
		status_t	StartWatchingAll(BMessenger msgr);
		status_t	StopWatching(BMessenger msgr, uint32 what);
		status_t	StopWatchingAll(BMessenger msgr);

		status_t	StartWatching(BHandler *handler, uint32 what);
		status_t	StartWatchingAll(BHandler *handler);
		status_t	StopWatching(BHandler *handler, uint32 what);
		status_t	StopWatchingAll(BHandler *handler);

		virtual void	SendNotices(uint32 what, const BMessage *msg = NULL);
		bool		IsWatched(uint32 what = B_OBSERVER_OBSERVE_ALL) const;

		virtual bool	AddFilter(BMessageFilter *filter);
		virtual bool	RemoveFilter(BMessageFilter *filter);
		virtual bool	SetFilterList(const BList *filterList);
		const BList	*FilterList() const;

	private:
		friend class BLooper;
		friend class BMessage;

		friend uint64 get_handler_token(const BHandler *handler);
		friend void set_handler_token(BHandler *handler, uint64 token);
		friend BLooper* get_handler_looper(uint64 token);
		friend uint64 get_ref_looper_token(uint64 token);

		BToken *fToken;
		char *fName;
		BLooper *fLooper;
		bool forceSetNextHandler;
		BHandler *fNextHandler;
		_EObserverList *fObserverList;
		BList fFilters;

		void SetLooper(BLooper *looper);
};

#endif /* __cplusplus */

#endif /* __ETK_HANDLER_H__ */


