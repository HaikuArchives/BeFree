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
 * File: Looper.h
 * Description: Looper for waiting/dispatching message
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_LOOPER_H__
#define __ETK_LOOPER_H__

#include <support/List.h>
#include <kernel/OS.h>
#include <app/AppDefs.h>
#include <app/Handler.h>
#include <app/MessageQueue.h>
#include <app/MessageFilter.h>

#ifdef __cplusplus /* Just for C++ */

class BApplication;
class BMessenger;

class BLooper : public BHandler
{
	public:
		BLooper(const char *name = NULL,
		        int32 priority = B_NORMAL_PRIORITY);
		virtual ~BLooper();

		// Archiving
		BLooper(const BMessage *from);
		virtual status_t Archive(BMessage *into, bool deep = true) const;
		static BArchivable *Instantiate(const BMessage *from);

		void		AddHandler(BHandler *handler);
		bool		RemoveHandler(BHandler *handler);
		int32		CountHandlers() const;
		BHandler	*HandlerAt(int32 index) const;
		int32		IndexOf(BHandler *handler) const;

		BHandler	*PreferredHandler() const;
		void		SetPreferredHandler(BHandler *handler);

		bool		IsRunning() const;
		virtual void	*Run();
		virtual void	Quit();
		virtual bool	QuitRequested();
		BLooper*	Proxy() const;
		bool		ProxyBy(BLooper *proxy);

		e_thread_id	Thread() const;

		bool		Lock();
		void		Unlock();
		status_t	LockWithTimeout(bigtime_t microseconds_timeout);

		int64		CountLocks() const;
		bool		IsLockedByCurrentThread() const;

		virtual void	DispatchMessage(BMessage *msg, BHandler *target);

		// Empty functions BEGIN --- just for derivative class
		virtual void	MessageReceived(BMessage *msg);
		// Empty functions END

		BMessage	*CurrentMessage() const;
		BMessage	*DetachCurrentMessage();
		BMessageQueue	*MessageQueue() const;

		status_t	PostMessage(uint32 command);
		status_t	PostMessage(const BMessage *message);
		status_t	PostMessage(uint32 command,
		                     BHandler *handler,
		                     BHandler *reply_to = NULL);
		status_t	PostMessage(const BMessage *message,
		                     BHandler *handler,
		                     BHandler *reply_to = NULL);

		virtual bool	AddCommonFilter(BMessageFilter *filter);
		virtual bool	RemoveCommonFilter(BMessageFilter *filter);
		virtual bool	SetCommonFilterList(const BList *filterList);
		const BList	*CommonFilterList() const;

		static BLooper	*LooperForThread(e_thread_id tid);

	protected:
		// NextLooperMessage & DispatchLooperMessage: called from task of looper, like below
		//	while(true)
		//	{
		//		...
		//		BMessage *aMsg = NextLooperMessage(B_INFINITE_TIMEOUT);
		//
		//		if(aMsg == NULL) /* after "this->QuitRequested()" return "true" or proxy deconstructing. */
		//		{
		//			...
		//			break;
		//		}
		//		else
		//		{
		//			/* leaked memory unless "DispatchLooperMessage" applied or "delete" instead */
		//			DispatchLooperMessage(aMsg);
		//		}
		//		...
		//	}
		BMessage	*NextLooperMessage(bigtime_t timeout = B_INFINITE_TIMEOUT);
		void		DispatchLooperMessage(BMessage *msg);

	private:
		friend class BHandler;
		friend class BApplication;
		friend class BMessenger;
		friend status_t lock_looper_of_handler(uint64 token, bigtime_t timeout);

		bool fDeconstructing;
		BLooper *fProxy;
		BList fClients;

		int32 fThreadPriority;

		BList fHandlers;
		BHandler *fPreferredHandler;

		void *fLocker;
		int64 fLocksCount;

		void *fThread;
		void *fSem;

		BMessageQueue *fMessageQueue;
		BMessage *fCurrentMessage;

		static status_t _task(void*);
		static status_t _taskLooper(BLooper*, void*);
		static void _taskError(void*);

		static BList sLooperList;

		BHandler *_MessageTarget(const BMessage *msg, bool *preferred);
		status_t _PostMessage(const BMessage *msg, uint64 handlerToken, uint64 replyToken, bigtime_t timeout);

		BLooper *_Proxy() const;
		bool _ProxyBy(BLooper *proxy);
		BLooper *_GetNextClient(BLooper *client) const;

		bool *fThreadExited;

		BList fCommonFilters;
		void _FilterAndDispatchMessage(BMessage *msg, BHandler *target);

		virtual bool	IsDependsOnOthersWhenQuitRequested() const;
};

#endif /* __cplusplus */

#endif /* __ETK_LOOPER_H__ */


