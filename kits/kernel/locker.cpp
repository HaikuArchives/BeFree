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
 * File: etk-locker.cpp
 *
 * --------------------------------------------------------------------------*/

#include <pthread.h>
#include <errno.h>

#include <kernel/Kernel.h>
#include <support/String.h>

typedef struct posix_locker_t {
	posix_locker_t()
			: holderThreadId(B_INT64_CONSTANT(0)), lockCount(B_INT64_CONSTANT(0)), closed(false), created(false), refCount(0) {
	}

	~posix_locker_t() {
		if (created) {
			created = false;
			delete_locker((void*)this);
		}
	}

	void SetHolderThreadId(int64 id) {
		holderThreadId = id;
	}

	bool HolderThreadIsCurrent(void) {
		return(holderThreadId == get_current_thread_id());
	}

	int64			holderThreadId;
	int64			lockCount;
	bool			closed;
	pthread_mutex_t		iLocker;
	pthread_mutex_t		Locker;
	pthread_cond_t		Cond;

	bool			created;

	uint32			refCount;
} posix_locker_t;


static void lock_locker_inter(posix_locker_t *locker)
{
	pthread_mutex_lock(&(locker->iLocker));
}


static void unlock_locker_inter(posix_locker_t *locker)
{
	pthread_mutex_unlock(&(locker->iLocker));
}


void* create_locker(void)
{
	posix_locker_t *locker = new posix_locker_t();
	if (!locker) return NULL;

	uint32 successFlags = 0;

	if (pthread_mutex_init(&(locker->iLocker), NULL) != 0) successFlags |= (1 << 1);
	if (pthread_mutex_init(&(locker->Locker), NULL) != 0) successFlags |= (1 << 2);
	if (pthread_cond_init(&(locker->Cond), NULL) != 0) successFlags |= (1 << 3);

	if (successFlags != 0) {
		if (!(successFlags & (1 << 1))) pthread_mutex_destroy(&(locker->iLocker));
		if (!(successFlags & (1 << 2))) pthread_mutex_destroy(&(locker->Locker));
		if (!(successFlags & (1 << 3))) pthread_cond_destroy(&(locker->Cond));
		delete locker;
		return NULL;
	}

	locker->refCount = 1;
	locker->created = true;

	return (void*)locker;
}


void* clone_locker(void *data)
{
	posix_locker_t *locker = (posix_locker_t*)data;
	if (!locker) return NULL;

	lock_locker_inter(locker);

	if (locker->closed || locker->refCount >= B_MAXUINT32) {
		unlock_locker_inter(locker);
		return NULL;
	}

	locker->refCount += 1;

	unlock_locker_inter(locker);

	return data;
}


status_t delete_locker(void *data)
{
	posix_locker_t *locker = (posix_locker_t*)data;
	if (!locker) return B_BAD_VALUE;

	lock_locker_inter(locker);
	uint32 count = --(locker->refCount);
#if 0
	bool showWarning = (locker->HolderThreadIsCurrent() && locker->closed == false && count > 0);
#endif
	unlock_locker_inter(locker);

#if 0
	if (showWarning)
		ETK_OUTPUT("\n\
		           **************************************************************************\n\
		           *                      [KERNEL]: delete_locker                       *\n\
		           *                                                                        *\n\
		           *  Locker still locked by current thread, and some clone-copies existed  *\n\
		           *  It's recommended that unlock or close the locker before delete it.    *\n\
		           *  Otherwise, the waitting thread will block!                            *\n\
		           **************************************************************************\n");
#endif

	if (count > 0) return B_OK;

	pthread_mutex_destroy(&(locker->iLocker));
	pthread_mutex_destroy(&(locker->Locker));
	pthread_cond_destroy(&(locker->Cond));

	if (locker->created) {
		locker->created = false;
		delete locker;
	}

	return B_OK;
}


/* after you call "close_locker":
 * 	1. the next "lock_locker..." function call will be failed
 * */
status_t close_locker(void *data)
{
	posix_locker_t *locker = (posix_locker_t*)data;
	if (!locker) return B_BAD_VALUE;

	lock_locker_inter(locker);
	if (locker->closed) {
		unlock_locker_inter(locker);
		return B_ERROR;
	}
	locker->closed = true;
	pthread_cond_broadcast(&(locker->Cond));
	unlock_locker_inter(locker);

	return B_OK;
}


status_t lock_locker(void *data)
{
	return lock_locker_etc(data, B_TIMEOUT, B_INFINITE_TIMEOUT);
}


status_t lock_locker_etc(void *data, uint32 flags, bigtime_t microseconds_timeout)
{
	posix_locker_t *locker = (posix_locker_t*)data;
	if (!locker) return B_BAD_VALUE;

	if (microseconds_timeout <B_INT64_CONSTANT(0)) return B_BAD_VALUE;

	bool wait_forever = false;
	bigtime_t currentTime = real_time_clock_usecs();
	if (flags != B_ABSOLUTE_TIMEOUT) {
		if (microseconds_timeout == B_INFINITE_TIMEOUT || microseconds_timeout >B_MAXINT64 - currentTime)
			wait_forever = true;
		else
			microseconds_timeout += currentTime;
	}

	lock_locker_inter(locker);

	if (locker->closed) {
		unlock_locker_inter(locker);
		return B_ERROR;
	}

	if (locker->HolderThreadIsCurrent() == false) {
		pthread_mutex_t *iLocker = &(locker->iLocker);
		pthread_mutex_t *Locker = &(locker->Locker);
		pthread_cond_t *Cond = &(locker->Cond);

		if (!wait_forever && microseconds_timeout == currentTime) {
			if (pthread_mutex_trylock(Locker) != 0) {
				unlock_locker_inter(locker);
				return B_WOULD_BLOCK;
			}
		} else {
			struct timespec ts;

			ts.tv_sec = (long)(microseconds_timeout /B_INT64_CONSTANT(1000000));
			ts.tv_nsec = (long)(microseconds_timeout %B_INT64_CONSTANT(1000000)) * 1000L;

			int ret;
			while ((ret = pthread_mutex_trylock(Locker)) != 0) {
				if (ret != EBUSY || locker->closed) {
					unlock_locker_inter(locker);
					return B_ERROR;
				}

				ret = (wait_forever ? pthread_cond_wait(Cond, iLocker) : pthread_cond_timedwait(Cond, iLocker, &ts));

				if (ret != 0) {
					if (ret == ETIMEDOUT && !wait_forever) {
						unlock_locker_inter(locker);
						return B_TIMED_OUT;
					} else return B_ERROR;
				}
			}
		}

		if (locker->closed) {
			pthread_mutex_unlock(Locker);
			unlock_locker_inter(locker);
			return B_ERROR;
		}

		locker->SetHolderThreadId(get_current_thread_id());
		locker->lockCount = B_INT64_CONSTANT(1);

		unlock_locker_inter(locker);
	} else {
		if (B_MAXINT64 - locker->lockCount <B_INT64_CONSTANT(1)) {
			unlock_locker_inter(locker);
			return B_ERROR;
		}

		locker->lockCount++;
		unlock_locker_inter(locker);
	}

	return B_OK;
}


status_t unlock_locker(void *data)
{
	posix_locker_t *locker = (posix_locker_t*)data;
	if (!locker) return B_BAD_VALUE;

	lock_locker_inter(locker);

	if (locker->HolderThreadIsCurrent() == false) {
		unlock_locker_inter(locker);
		ETK_WARNING("[KERNEL]: %s -- Can't unlock when didn't hold it in current thread!", __PRETTY_FUNCTION__);
		return B_ERROR;
	} else {
		if (locker->lockCount <= B_INT64_CONSTANT(1)) {
			if (pthread_mutex_unlock(&(locker->Locker)) != 0) {
				unlock_locker_inter(locker);
				return B_ERROR;
			}
		}

		locker->lockCount--;

		if (locker->lockCount <= B_INT64_CONSTANT(0)) {
			locker->SetHolderThreadId(B_INT64_CONSTANT(0));
			locker->lockCount = B_INT64_CONSTANT(0);
			pthread_cond_broadcast(&(locker->Cond));
		}

		unlock_locker_inter(locker);
	}

	return B_OK;
}


int64 count_locker_locks(void *data)
{
	int64 retVal = B_INT64_CONSTANT(0);

	posix_locker_t *locker = (posix_locker_t*)data;

	if (locker) {
		lock_locker_inter(locker);
		if (locker->HolderThreadIsCurrent()) retVal = locker->lockCount;
		else if (locker->lockCount >B_INT64_CONSTANT(0)) retVal = -(locker->lockCount);
		unlock_locker_inter(locker);
	}

	return retVal;
}


void* create_simple_locker(void)
{
	pthread_mutex_t *locker = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	if (!locker) return NULL;

	if (pthread_mutex_init(locker, NULL) != 0) {
		free(locker);
		return NULL;
	}

	return (void*)locker;
}


status_t delete_simple_locker(void* data)
{
	pthread_mutex_t *locker = (pthread_mutex_t*)data;
	if (!locker) return B_ERROR;
	pthread_mutex_destroy(locker);
	free(locker);
	return B_OK;
}


bool lock_simple_locker(void *data)
{
	pthread_mutex_t *locker = (pthread_mutex_t*)data;
	if (!locker) return false;

	if (pthread_mutex_lock(locker) != 0) return false;

	return true;
}


void unlock_simple_locker(void *data)
{
	pthread_mutex_t *locker = (pthread_mutex_t*)data;
	if (!locker) return;

	pthread_mutex_unlock(locker);
}


#ifdef ETK_BUILD_WITH_MEMORY_TRACING
static pthread_mutex_t __posix_memory_tracing_locker = PTHREAD_MUTEX_INITIALIZER;


bool memory_tracing_lock(void)
{
	if (pthread_mutex_lock(&__posix_memory_tracing_locker) != 0) return false;
	return true;
}


void memory_tracing_unlock(void)
{
	pthread_mutex_unlock(&__posix_memory_tracing_locker);
}
#endif // ETK_BUILD_WITH_MEMORY_TRACING

