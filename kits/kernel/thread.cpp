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
 * File: etk-thread.cpp
 *
 * --------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

#include <kernel/Kernel.h>
#include <support/List.h>
#include <support/String.h>

typedef struct _threadCallback_ {
	e_thread_func	func;
	void		*user_data;
} _threadCallback_;


typedef struct posix_thread_t {
	int32			priority;
	int32			running;
	bool			exited;
	status_t		status;
	int64			ID;
	_threadCallback_	callback;
	BList			exit_callbacks;

	pthread_mutex_t		locker;
	pthread_cond_t		cond;

	bool			existent;

	BList			private_threads;
} posix_thread_t;


typedef struct posix_thread_private_t {
	posix_thread_t *thread;
	bool copy;
} posix_thread_private_t;


static posix_thread_t* __create_thread__()
{
	posix_thread_t *thread = new posix_thread_t;
	if (thread == NULL) return NULL;

	thread->priority = -1;
	thread->running = 0;
	thread->exited = false;
	thread->status = B_OK;
	thread->ID = B_INT64_CONSTANT(0);
	thread->callback.func = NULL;
	thread->callback.user_data = NULL;

	pthread_mutex_init(&(thread->locker), NULL);
	pthread_cond_init(&(thread->cond), NULL);

	thread->existent = false;

	return thread;
}


static void __delete_thread__(posix_thread_t *thread)
{
	if (thread == NULL) return;

	posix_thread_private_t *priThread;
	while ((priThread = (posix_thread_private_t*)thread->private_threads.RemoveItem(0)) != NULL) delete priThread;

	_threadCallback_ *exitCallback;
	while ((exitCallback = (_threadCallback_*)thread->exit_callbacks.RemoveItem(0)) != NULL) delete exitCallback;

	pthread_mutex_destroy(&(thread->locker));
	pthread_cond_destroy(&(thread->cond));

	delete thread;
}


static pthread_mutex_t __thread_locker__ = PTHREAD_MUTEX_INITIALIZER;
#define _ETK_LOCK_THREAD_()	pthread_mutex_lock(&__thread_locker__)
#define _ETK_UNLOCK_THREAD_()	pthread_mutex_unlock(&__thread_locker__)


static int64 convert_pthread_id_to_etk(pthread_t tid)
{
	if (sizeof(int64) < sizeof(pthread_t)) {
		// not support
		return B_INT64_CONSTANT(0);
	}

	int64 cid = B_INT64_CONSTANT(0);

	if (memcpy(&cid, &tid, sizeof(pthread_t)) == NULL) return B_INT64_CONSTANT(0);

	return cid;
}


static pthread_t convert_thread_id_to_pthread(int64 cid)
{
	pthread_t tid;
	bzero(&tid, sizeof(pthread_t));

	if (sizeof(int64) >= sizeof(pthread_t) && cid != B_INT64_CONSTANT(0)) memcpy(&tid, &cid, sizeof(pthread_t));

	return tid;
}


class BThreadsList
{
	public:
		BList fList;

		BThreadsList() {
		}

		~BThreadsList() {
			posix_thread_t *td;
			while ((td = (posix_thread_t*)fList.RemoveItem(0)) != NULL) {
				ETK_WARNING("[KERNEL]: Thread %I64i leaked.", td->ID);
				__delete_thread__(td);
			}
		}

		posix_thread_private_t* AddThread(posix_thread_t *td) {
			if (td == NULL || td->private_threads.CountItems() != 0 || fList.AddItem((void*)td, 0) == false) return NULL;
			posix_thread_private_t *priThread = new posix_thread_private_t;
			if (priThread == NULL || td->private_threads.AddItem((void*)priThread, 0) == false) {
				fList.RemoveItem((void*)td);
				if (priThread != NULL) delete priThread;
				return NULL;
			}
			priThread->thread = td;
			priThread->copy = false;
			return priThread;
		}

		posix_thread_private_t* RefThread(posix_thread_t *td) {
			if (td == NULL || td->private_threads.CountItems() == 0 || fList.IndexOf((void*)td) < 0) return NULL;
			posix_thread_private_t *priThread = new posix_thread_private_t;
			if (priThread == NULL || td->private_threads.AddItem((void*)priThread, 0) == false) {
				if (priThread != NULL) delete priThread;
				return NULL;
			}
			priThread->thread = td;
			priThread->copy = true;
			return priThread;
		}

		int32 UnrefThread(posix_thread_private_t *priThread) {
			posix_thread_t *td = (priThread == NULL ? NULL : priThread->thread);
			if (td == NULL || td->private_threads.CountItems() == 0 || fList.IndexOf((void*)td) < 0) return -1;
			if (td->private_threads.RemoveItem((void*)priThread) == false) return -1;
			delete priThread;
			int32 count = td->private_threads.CountItems();
			if (count == 0) fList.RemoveItem((void*)td);
			return count;
		}

		posix_thread_private_t* OpenThread(int64 tid) {
			if (tid == B_INT64_CONSTANT(0)) return NULL;
			for (int32 i = 0; i < fList.CountItems(); i++) {
				posix_thread_t* td = (posix_thread_t*)fList.ItemAt(i);
				if (td->ID == tid) return RefThread(td);
			}
			return NULL;
		}
};


static BThreadsList __thread_lists__;
#define _ETK_ADD_THREAD_(td)	__thread_lists__.AddThread(td)
#define _ETK_REF_THREAD_(td)	__thread_lists__.RefThread(td)
#define _ETK_UNREF_THREAD_(td)	__thread_lists__.UnrefThread(td)
#define _ETK_OPEN_THREAD_(tid)	__thread_lists__.OpenThread(tid)


int64 get_current_thread_id(void)
{
	return(convert_pthread_id_to_etk(pthread_self()));
}


static void lock_thread_inter(posix_thread_t *thread)
{
	pthread_mutex_lock(&(thread->locker));
}


static void unlock_thread_inter(posix_thread_t *thread)
{
	pthread_mutex_unlock(&(thread->locker));
}


static void* spawn_thread_func(void *data)
{
	posix_thread_t *thread = (posix_thread_t*)data;
	posix_thread_private_t *priThread = NULL;

	lock_thread_inter(thread);
	pthread_cond_wait(&(thread->cond), &(thread->locker));
	if (thread->callback.func == NULL) {
		thread->exited = true;
		pthread_cond_broadcast(&(thread->cond));
		unlock_thread_inter(thread);
		return NULL;
	}
	e_thread_func threadFunc = thread->callback.func;
	void *userData = thread->callback.user_data;
	thread->callback.func = NULL;
	thread->running = 1;
	unlock_thread_inter(thread);

	_ETK_LOCK_THREAD_();
	if ((priThread = _ETK_REF_THREAD_(thread)) == NULL) {
		_ETK_UNLOCK_THREAD_();

		lock_thread_inter(thread);
		thread->exited = true;
		pthread_cond_broadcast(&(thread->cond));
		unlock_thread_inter(thread);

		return NULL;
	}
	_ETK_UNLOCK_THREAD_();

	if (on_exit_thread((void (*)(void *))delete_thread, priThread) != B_OK) {
		ETK_WARNING("[KERNEL]: %s --- Unexpected error! Thread WON'T RUN!", __PRETTY_FUNCTION__);

		lock_thread_inter(thread);

		thread->running = 0;
		thread->exited = true;

		pthread_cond_broadcast(&(thread->cond));

		BList exitCallbackList(thread->exit_callbacks);
		thread->exit_callbacks.MakeEmpty();

		unlock_thread_inter(thread);

		_threadCallback_ *exitCallback;
		while ((exitCallback = (_threadCallback_*)exitCallbackList.RemoveItem(0)) != NULL) delete exitCallback;

		delete_thread(priThread);

		return NULL;
	}

	status_t status = (threadFunc == NULL ?B_ERROR : (*threadFunc)(userData));

	lock_thread_inter(thread);

	thread->running = 0;
	thread->exited = true;
	thread->status = status;
	pthread_cond_broadcast(&(thread->cond));

	BList exitCallbackList(thread->exit_callbacks);
	thread->exit_callbacks.MakeEmpty();

	unlock_thread_inter(thread);

	_threadCallback_ *exitCallback;
	while ((exitCallback = (_threadCallback_*)exitCallbackList.RemoveItem(0)) != NULL) {
		if (exitCallback->func) (*(exitCallback->func))(exitCallback->user_data);
		delete exitCallback;
	}

	return NULL;
}


void* create_thread_by_current_thread(void)
{
	posix_thread_private_t *priThread = NULL;

	_ETK_LOCK_THREAD_();
	if ((priThread = _ETK_OPEN_THREAD_(get_current_thread_id())) != NULL) {
		_ETK_UNREF_THREAD_(priThread);
		_ETK_UNLOCK_THREAD_();
		return NULL;
	}

	posix_thread_t *thread = __create_thread__();
	if (thread == NULL) return NULL;

	if ((priThread = _ETK_ADD_THREAD_(thread)) == NULL) {
		_ETK_UNLOCK_THREAD_();
		__delete_thread__(thread);
		return NULL;
	}

	thread->priority = 0;
	thread->running = 1;
	thread->exited = false;
	thread->ID = get_current_thread_id();
	thread->existent = true;

	_ETK_UNLOCK_THREAD_();

	return (void*)priThread;
}


void* create_thread(e_thread_func threadFunction,
                        int32 priority,
                        void *arg,
                        int64 *threadId)
{
	if (!threadFunction) return NULL;

	posix_thread_t *thread = __create_thread__();
	if (thread == NULL) return NULL;

	thread->callback.func = threadFunction;
	thread->callback.user_data = arg;

	pthread_t posixThreadId;

	pthread_attr_t posixThreadAttr;
	pthread_attr_init(&posixThreadAttr);
	pthread_attr_setdetachstate(&posixThreadAttr, PTHREAD_CREATE_JOINABLE);
#ifdef _POSIX_THREAD_ATTR_STACKSIZE
	pthread_attr_setstacksize(&posixThreadAttr, 0x40000);
#endif // _POSIX_THREAD_ATTR_STACKSIZE

	if (pthread_create(&posixThreadId, &posixThreadAttr, spawn_thread_func, (void*)thread) != 0) {
		pthread_attr_destroy(&posixThreadAttr);
		ETK_WARNING("[KERNEL]: %s --- Not enough system resources to create a new thread.", __PRETTY_FUNCTION__);

		__delete_thread__(thread);
		return NULL;
	}
	pthread_attr_destroy(&posixThreadAttr);

	posix_thread_private_t *priThread = NULL;

	_ETK_LOCK_THREAD_();
	if ((priThread = _ETK_ADD_THREAD_(thread)) == NULL) {
		_ETK_UNLOCK_THREAD_();

		ETK_WARNING("[KERNEL]: %s --- Unexpected error! Thread WON'T RUN!", __PRETTY_FUNCTION__);

		lock_thread_inter(thread);
		thread->callback.func = NULL;
		while (thread->exited == false && thread->running != 1) {
			pthread_cond_broadcast(&(thread->cond));
			unlock_thread_inter(thread);
			e_snooze(500);
			lock_thread_inter(thread);
		}
		unlock_thread_inter(thread);

		pthread_join(posixThreadId, NULL);

		__delete_thread__(thread);
		return NULL;
	}

	thread->priority = -1;
	thread->running = 0;
	thread->exited = false;
	thread->ID = convert_pthread_id_to_etk(posixThreadId);
	thread->existent = false;

	_ETK_UNLOCK_THREAD_();

	set_thread_priority(priThread, priority);

	if (threadId) *threadId = thread->ID;
	return (void*)priThread;
}


void* open_thread(int64 threadId)
{
	_ETK_LOCK_THREAD_();
	posix_thread_private_t *priThread = _ETK_OPEN_THREAD_(threadId);
	_ETK_UNLOCK_THREAD_();

	return (void*)priThread;
}


status_t delete_thread(void *data)
{
	posix_thread_private_t *priThread = (posix_thread_private_t*)data;
	posix_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if (priThread == NULL || thread == NULL) return B_BAD_VALUE;

	bool threadIsCopy = priThread->copy;

	_ETK_LOCK_THREAD_();
	int32 count = _ETK_UNREF_THREAD_(priThread);
	_ETK_UNLOCK_THREAD_();

	if (count < 0) return B_ERROR;

	if (thread->existent && !threadIsCopy) {
		BList exitCallbackList;

		lock_thread_inter(thread);
		if (thread->ID == get_current_thread_id()) {
			thread->running = 0;
			thread->exited = true;
			thread->status = B_OK;

			pthread_cond_broadcast(&(thread->cond));

			exitCallbackList = thread->exit_callbacks;
			thread->exit_callbacks.MakeEmpty();
		}
		unlock_thread_inter(thread);

		_threadCallback_ *exitCallback;
		while ((exitCallback = (_threadCallback_*)exitCallbackList.RemoveItem(0)) != NULL) {
			if (exitCallback->func) (*(exitCallback->func))(exitCallback->user_data);
			delete exitCallback;
		}
	}

	if (count > 0) return B_OK;

	if (thread->existent == false) {
		pthread_t posixThreadId = convert_thread_id_to_pthread(thread->ID);
		if (pthread_equal(posixThreadId, pthread_self()) == 0) {
			lock_thread_inter(thread);
			thread->callback.func = NULL;
			while (thread->exited == false && thread->running != 1) {
				pthread_cond_broadcast(&(thread->cond));
				unlock_thread_inter(thread);
				e_snooze(500);
				lock_thread_inter(thread);
			}
			unlock_thread_inter(thread);

			pthread_join(posixThreadId, NULL);
		} else {
			pthread_detach(posixThreadId);
		}
	}

	BList exitCallbackList(thread->exit_callbacks);
	thread->exit_callbacks.MakeEmpty();

	_threadCallback_ *exitCallback;
	while ((exitCallback = (_threadCallback_*)exitCallbackList.RemoveItem(0)) != NULL) {
		if (exitCallback->func) (*(exitCallback->func))(exitCallback->user_data);
		delete exitCallback;
	}

	__delete_thread__(thread);

	return B_OK;
}


status_t resume_thread(void *data)
{
	posix_thread_private_t *priThread = (posix_thread_private_t*)data;
	posix_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if (thread == NULL) return B_BAD_VALUE;

	status_t retVal = B_ERROR;

	lock_thread_inter(thread);
	if (((thread->callback.func != NULL && thread->running == 0) || thread->running == 2) &&
	        thread->exited == false) {
		retVal = B_OK;

		while (thread->exited == false && thread->running != 1) {
			pthread_cond_broadcast(&(thread->cond));
			unlock_thread_inter(thread);
			e_snooze(500);
			lock_thread_inter(thread);
		}
	}
	unlock_thread_inter(thread);

	return retVal;
}


status_t suspend_thread(void *data)
{
	posix_thread_private_t *priThread = (posix_thread_private_t*)data;
	posix_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if (thread == NULL) return B_BAD_VALUE;

	status_t retVal = B_ERROR;

	lock_thread_inter(thread);
	bool suspend_cur_thread = (thread->ID == get_current_thread_id());
	if (thread->running == 1 && thread->exited == false) {
		if (suspend_cur_thread) {
			thread->running = 2;
			retVal = pthread_cond_wait(&(thread->cond), &(thread->locker)) != 0 ?B_ERROR :B_OK;
			if (retVal != B_OK) lock_thread_inter(thread);
			thread->running = 1;
		} else {
			// TODO
			ETK_WARNING("[KERNEL]: %s --- Only supported to suspend the current thread !!!", __PRETTY_FUNCTION__);
		}
	}
	unlock_thread_inter(thread);

	return retVal;
}


int64 get_thread_id(void *data)
{
	posix_thread_private_t *priThread = (posix_thread_private_t*)data;
	posix_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if (thread == NULL) return B_INT64_CONSTANT(0);

	lock_thread_inter(thread);
	int64 thread_id = thread->ID;
	unlock_thread_inter(thread);

	return thread_id;
}


uint32 get_thread_run_state(void *data)
{
	posix_thread_private_t *priThread = (posix_thread_private_t*)data;
	posix_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if (thread == NULL) return ETK_THREAD_INVALID;

	uint32 retVal = ETK_THREAD_INVALID;

	lock_thread_inter(thread);

	if (thread->exited) {
		if (thread->running == 0)
			retVal = ETK_THREAD_EXITED;
	} else switch (thread->running) {
			case 0:
				retVal = ETK_THREAD_READY;
				break;

			case 1:
				retVal = ETK_THREAD_RUNNING;
				break;

			case 2:
				retVal = ETK_THREAD_SUSPENDED;
				break;

			default:
				break;
		}

	unlock_thread_inter(thread);

	return retVal;
}


status_t set_thread_priority(void *data, int32 new_priority)
{
	posix_thread_private_t *priThread = (posix_thread_private_t*)data;
	posix_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if (thread == NULL) return -1;

	if (new_priority < 0) new_priority = 15;
	else if (new_priority > 120) new_priority = 120;

	int policy = (new_priority >= 100 ? SCHED_RR : SCHED_OTHER);
	sched_param param;
	int priority_max, priority_min;

	lock_thread_inter(thread);

	if (thread->exited || (priority_max = sched_get_priority_max(policy)) < 0 || (priority_min = sched_get_priority_min(policy)) < 0) {
		ETK_WARNING("[KERNEL]: %s --- %s", __PRETTY_FUNCTION__,
		            thread->exited ? "Thread exited." : "Sched get priority region failed.");
		unlock_thread_inter(thread);
		return B_ERROR;
	}

	if (new_priority < 100)
		param.sched_priority = priority_min + (int)(((float)new_priority / 100.f) * (float)(priority_max - priority_min));
	else
		param.sched_priority = priority_min + (int)(((float)(new_priority - 100) / 20.f) * (float)(priority_max - priority_min));

//	ETK_DEBUG("[KERNEL]: POLICY: %d, PRIORITY_MAX: %d, PRIORITY_MIN: %d, Current Priority: %d",
//		  policy, priority_max, priority_min, param.sched_priority);

	if (pthread_setschedparam(convert_thread_id_to_pthread(thread->ID), policy, &param) != 0) {
		ETK_WARNING("[KERNEL]: %s --- Set thread priority failed.", __PRETTY_FUNCTION__);
		unlock_thread_inter(thread);
		return B_ERROR;
	}

	int32 old_priority = thread->priority;
	thread->priority = new_priority;

	unlock_thread_inter(thread);

	return old_priority;
}


int32 get_thread_priority(void *data)
{
	posix_thread_private_t *priThread = (posix_thread_private_t*)data;
	posix_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if (thread == NULL) return -1;

	lock_thread_inter(thread);
	int32 priority = thread->priority;
	unlock_thread_inter(thread);

	return priority;
}


status_t on_exit_thread(void (*callback)(void *), void *user_data)
{
	if (!callback) return B_BAD_VALUE;

	posix_thread_private_t *priThread = (posix_thread_private_t*)open_thread(get_current_thread_id());
	if (priThread == NULL) {
		ETK_WARNING("[KERNEL]: %s --- Thread wasn't created by this toolkit!", __PRETTY_FUNCTION__);
		return B_ERROR;
	}

	posix_thread_t *thread = priThread->thread;

	_threadCallback_ *exitCallback = new _threadCallback_;
	if (exitCallback == NULL) {
		delete_thread(priThread);
		return B_NO_MEMORY;
	}

	exitCallback->func = (e_thread_func)callback;
	exitCallback->user_data = user_data;

	status_t retVal = B_OK;

	lock_thread_inter(thread);
	if (thread->exited || thread->exit_callbacks.AddItem((void*)exitCallback, 0) == false) {
		delete exitCallback;
		retVal = B_ERROR;
	}
	unlock_thread_inter(thread);

	delete_thread(priThread);

	return retVal;
}


void exit_thread(status_t status)
{
	posix_thread_private_t *priThread = (posix_thread_private_t*)open_thread(get_current_thread_id());
	if (priThread == NULL) {
		ETK_WARNING("[KERNEL]: %s --- thread wasn't created by this toolkit!", __PRETTY_FUNCTION__);
		return;
	}

	posix_thread_t *thread = priThread->thread;

	lock_thread_inter(thread);

	thread->running = 0;
	thread->exited = true;
	thread->status = status;

	pthread_cond_broadcast(&(thread->cond));

	BList exitCallbackList(thread->exit_callbacks);
	thread->exit_callbacks.MakeEmpty();

	unlock_thread_inter(thread);

	_threadCallback_ *exitCallback;
	while ((exitCallback = (_threadCallback_*)exitCallbackList.RemoveItem(0)) != NULL) {
		if (exitCallback->func) (*(exitCallback->func))(exitCallback->user_data);
		delete exitCallback;
	}

	delete_thread(priThread);

	pthread_exit(NULL);
}


status_t wait_for_thread_etc(void *data, status_t *thread_return_value, uint32 flags, bigtime_t microseconds_timeout)
{
	posix_thread_private_t *priThread = (posix_thread_private_t*)data;
	posix_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if (thread == NULL || microseconds_timeout <B_INT64_CONSTANT(0) || thread_return_value == NULL) return B_BAD_VALUE;

	bigtime_t currentTime = real_time_clock_usecs();
	bool wait_forever = false;

	if (flags != B_ABSOLUTE_TIMEOUT) {
		if (microseconds_timeout == B_INFINITE_TIMEOUT || microseconds_timeout >B_MAXINT64 - currentTime)
			wait_forever = true;
		else
			microseconds_timeout += currentTime;
	}

	lock_thread_inter(thread);

	pthread_t posixThreadId = convert_thread_id_to_pthread(thread->ID);

	if (pthread_equal(posixThreadId, pthread_self()) != 0) {
		ETK_WARNING("[KERNEL]: %s --- Can't wait self.", __PRETTY_FUNCTION__);
		unlock_thread_inter(thread);
		return B_ERROR;
	} else if (thread->exited) {
		unlock_thread_inter(thread);
		pthread_join(posixThreadId, NULL);
		return B_OK;
	} else if (microseconds_timeout == currentTime && !wait_forever) {
		unlock_thread_inter(thread);
		return B_WOULD_BLOCK;
	}

	status_t retVal = B_ERROR;

	if (((thread->callback.func != NULL && thread->running == 0) || thread->running == 2) &&
	        thread->exited == false) {
		while (thread->exited == false && thread->running != 1) {
			pthread_cond_broadcast(&(thread->cond));
			unlock_thread_inter(thread);
			e_snooze(500);
			lock_thread_inter(thread);
		}
	}

	struct timespec ts;
	ts.tv_sec = (long)(microseconds_timeout /B_INT64_CONSTANT(1000000));
	ts.tv_nsec = (long)(microseconds_timeout %B_INT64_CONSTANT(1000000)) * 1000L;

	while (true) {
		if (thread->exited) {
			*thread_return_value = thread->status;

			retVal = B_OK;
			break;
		}

		int ret = (wait_forever ? pthread_cond_wait(&(thread->cond), &(thread->locker)) :
		           pthread_cond_timedwait(&(thread->cond), &(thread->locker), &ts));

		if (ret != 0) {
			if (ret == ETIMEDOUT && !wait_forever) {
				unlock_thread_inter(thread);
				return B_TIMED_OUT;
			} else return B_ERROR;
		}
	}

	unlock_thread_inter(thread);

	if (retVal == B_OK) pthread_join(posixThreadId, NULL);

	return retVal;
}


status_t wait_for_thread(void *data, status_t *thread_return_value)
{
	return wait_for_thread_etc(data, thread_return_value, B_TIMEOUT, B_INFINITE_TIMEOUT);
}


status_t snooze(bigtime_t microseconds)
{
	if (microseconds <= 0) return B_ERROR;

	microseconds += real_time_clock_usecs();

	struct timespec ts;
	ts.tv_sec = (long)(microseconds /B_INT64_CONSTANT(1000000));
	ts.tv_nsec = (long)(microseconds %B_INT64_CONSTANT(1000000)) * 1000L;

	pthread_mutex_t mptr;
	pthread_cond_t cptr;

	pthread_mutex_init(&mptr, NULL);
	pthread_cond_init(&cptr, NULL);

	pthread_mutex_lock(&mptr);
	int ret = pthread_cond_timedwait(&cptr, &mptr, &ts);
	pthread_mutex_unlock(&mptr);

	pthread_mutex_destroy(&mptr);
	pthread_cond_destroy(&cptr);

	if (ret == 0 || ret == ETIMEDOUT)
		return B_OK;
	else
		return B_ERROR;
}


status_t snooze_until(bigtime_t time, int timebase)
{
	if (time <B_INT64_CONSTANT(0)) return B_ERROR;

	switch (timebase) {
		case B_SYSTEM_TIMEBASE:
			time += system_boot_time();
			break;

		case B_REAL_TIME_TIMEBASE:
			break;

		default:
			return B_ERROR;
	}

	struct timespec ts;
	ts.tv_sec = (long)(time /B_INT64_CONSTANT(1000000));
	ts.tv_nsec = (long)(time %B_INT64_CONSTANT(1000000)) * 1000L;

	pthread_mutex_t mptr;
	pthread_cond_t cptr;

	pthread_mutex_init(&mptr, NULL);
	pthread_cond_init(&cptr, NULL);

	pthread_mutex_lock(&mptr);
	int ret = pthread_cond_timedwait(&cptr, &mptr, &ts);
	pthread_mutex_unlock(&mptr);

	pthread_mutex_destroy(&mptr);
	pthread_cond_destroy(&cptr);

	if (ret == 0 || ret == ETIMEDOUT)
		return B_OK;
	else
		return B_ERROR;
}


static pthread_mutex_t __team_id_locker__ = PTHREAD_MUTEX_INITIALIZER;
class __pid_impl__
{
	public:
		bool warning;
		int64 fTeam;

		__pid_impl__()
				: warning(true) {
			fTeam = (int64)getpid();
		}

		~__pid_impl__() {
		}

		int64 Team() {
			pid_t id = getpid();

			if ((pid_t)fTeam != id) {
				pthread_mutex_lock(&__team_id_locker__);
				if (warning) {
					fprintf(stdout, "\x1b[31m[KERNEL]: You need GNU C Library that support for NPTL.\x1b[0m\n");
					warning = false;
				}
				pthread_mutex_unlock(&__team_id_locker__);
			}

			return fTeam;
		}
};
static __pid_impl__ __team_id__;


int64 get_current_team_id(void)
{
	return __team_id__.Team();
}

