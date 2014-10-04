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
 * File: etk-semaphore.cpp
 *
 * --------------------------------------------------------------------------*/

#include <stdlib.h>
#include <fcntl.h>

#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#if !defined(_POSIX_THREAD_PROCESS_SHARED) || _POSIX_THREAD_PROCESS_SHARED == -1
#undef ETK_PTHREAD_SHARED
#ifndef HAVE_SEM_TIMEDWAIT
#error "Posix thread DON'T support Process-shared synchronization, nor do support sem_timedwait!"
#endif
#else
#define ETK_PTHREAD_SHARED
#endif

#include <kernel/Kernel.h>
#include <support/String.h>

typedef struct posix_sem_info {
	posix_sem_info() {
		InitData();
	}

	void InitData() {
		bzero(name, B_OS_NAME_LENGTH + 1);
		latestHolderTeamId = B_INT64_CONSTANT(0);
		latestHolderThreadId = B_INT64_CONSTANT(0);
		count = B_INT64_CONSTANT(0);
#ifndef ETK_PTHREAD_SHARED
		minAcquiringCount = B_INT64_CONSTANT(0);
#endif
		acquiringCount = B_INT64_CONSTANT(0);
		closed = false;
		refCount = 0;
	}

	void SetLatestHolderTeamId(int64 id) {
		latestHolderTeamId = id;
	}

	void SetLatestHolderThreadId(int64 id) {
		latestHolderThreadId = id;
	}

	bool LatestHolderTeamIsCurrent(void) {
		return(latestHolderTeamId == get_current_team_id());
	}

	bool LatestHolderThreadIsCurrent(void) {
		return(latestHolderThreadId == get_current_thread_id());
	}

	char			name[B_OS_NAME_LENGTH + 1];
	int64			latestHolderTeamId;
	int64			latestHolderThreadId;
	int64			count;
#ifndef ETK_PTHREAD_SHARED
	int64			minAcquiringCount;
#endif
	int64			acquiringCount;
	bool			closed;

#ifdef ETK_PTHREAD_SHARED
	pthread_mutex_t		mutex;
	pthread_cond_t		cond;
#else
	sem_t			isem;
	sem_t			sem;
#endif
	uint32			refCount;
} posix_sem_info;


typedef struct posix_sem_t {
	posix_sem_t()
			: mapping(NULL), semInfo(NULL), iMutex(NULL), iCond(NULL),
#ifndef ETK_PTHREAD_SHARED
			remoteiSem(NULL), remoteSem(NULL),
#endif
			created(false), no_clone(false) {
	}

	~posix_sem_t() {
		if (created) {
			created = false;
			delete_sem((void*)this);
		}
	}

	// for IPC (name != NULL)
	void*			mapping;
	posix_sem_info*	semInfo;

	// for local
	pthread_mutex_t*	iMutex;
	pthread_cond_t*		iCond;

#ifndef ETK_PTHREAD_SHARED
	sem_t*			remoteiSem;
	sem_t*			remoteSem;
#endif

	bool			created;
	bool			no_clone;
} posix_sem_t;


// return value must be free by "free()"
static char* global_sem_ipc_name()
{
	const char *prefix, *slash;

	if ((prefix = getenv("POSIX_SEM_IPC_PREFIX")) == NULL) {
#ifdef POSIX_SEM_IPC_PREFIX
		prefix = POSIX_SEM_IPC_PREFIX;
#else
		prefix = "/";
#endif
	}

	slash = (prefix[strlen(prefix) - 1] == '/') ? "" : "/";

	return b_strdup_printf("%s%s%s", prefix, slash, "_global_");
}

#ifndef SEM_FAILED
#define SEM_FAILED	(-1)
#endif

class posix_sem_locker_t
{
	public:
		sem_t *fSem;
		pthread_mutex_t fLocker;

		posix_sem_locker_t()
				: fSem((sem_t*)SEM_FAILED) {
			pthread_mutex_init(&fLocker, NULL);
		}

		~posix_sem_locker_t() {
			pthread_mutex_destroy(&fLocker);

			if (fSem != (sem_t*)SEM_FAILED) {
				sem_close(fSem);
				// leave global semaphore, without sem_unlink
			}
		}

		void Init() {
			if (fSem != (sem_t*)SEM_FAILED) return;

			char *semName = global_sem_ipc_name();
			if (semName) {
				if ((fSem = (sem_t*)sem_open(semName, O_CREAT | O_EXCL, 0666, 1)) == (sem_t*)SEM_FAILED) {
//				ETK_DEBUG("[KERNEL]: Unable to create global semaphore, errno: %d", errno);
					fSem = (sem_t*)sem_open(semName, 0);
				}
				free(semName);
			}
			if (fSem == (sem_t*)SEM_FAILED)
				ETK_ERROR("[KERNEL]: Can't initialize global semaphore! errno: %d", errno);
		}

		void LockLocal() {
			pthread_mutex_lock(&fLocker);
		}

		void UnlockLocal() {
			pthread_mutex_unlock(&fLocker);
		}

		void LockIPC() {
			LockLocal();
			Init();
			UnlockLocal();
			sem_wait(fSem);
		}

		void UnlockIPC() {
			sem_post(fSem);
		}
};

static posix_sem_locker_t __semaphore_locker__;

#define _ETK_LOCK_IPC_SEMAPHORE_()		__semaphore_locker__.LockIPC()
#define _ETK_UNLOCK_IPC_SEMAPHORE_()		__semaphore_locker__.UnlockIPC()
#define _ETK_LOCK_LOCAL_SEMAPHORE_()		__semaphore_locker__.LockLocal()
#define _ETK_UNLOCK_LOCAL_SEMAPHORE_()		__semaphore_locker__.UnlockLocal()


static bool is_sem_for_IPC(const posix_sem_t *sem)
{
	if (!sem) return false;
	return(sem->mapping != NULL);
}


static void lock_sem_inter(posix_sem_t *sem)
{
#ifndef ETK_PTHREAD_SHARED
	if (is_sem_for_IPC(sem)) {
		sem_wait(sem->remoteiSem);
	} else {
#endif
		pthread_mutex_lock(sem->iMutex);
#ifndef ETK_PTHREAD_SHARED
	}
#endif
}


static void unlock_sem_inter(posix_sem_t *sem)
{
#ifndef ETK_PTHREAD_SHARED
	if (is_sem_for_IPC(sem)) {
		sem_post(sem->remoteiSem);
	} else {
#endif
		pthread_mutex_unlock(sem->iMutex);
#ifndef ETK_PTHREAD_SHARED
	}
#endif
}


static void* create_sem_for_IPC(int64 count, const char *name, area_access area_access)
{
	if (count <B_INT64_CONSTANT(0) || name == NULL || *name == 0 || strlen(name) >B_OS_NAME_LENGTH) return NULL;

	posix_sem_t *sem = new posix_sem_t();
	if (!sem) return NULL;

	_ETK_LOCK_IPC_SEMAPHORE_();

	if ((sem->mapping = create_area(name, (void**)&(sem->semInfo), sizeof(posix_sem_info),
	                                    B_READ_AREA |B_WRITE_AREA, ETK_AREA_SYSTEM_SEMAPHORE_DOMAIN, area_access)) == NULL ||
	        sem->semInfo == NULL) {
		ETK_DEBUG("[KERNEL]: %s --- Can't create sem : create area failed --- \"%s\"", __PRETTY_FUNCTION__, name);
		if (sem->mapping) delete_area(sem->mapping);
		_ETK_UNLOCK_IPC_SEMAPHORE_();
		delete sem;
		return NULL;
	}

	posix_sem_info *sem_info = sem->semInfo;
	sem_info->InitData();
	memcpy(sem_info->name, name, (size_t)strlen(name));

#ifndef ETK_PTHREAD_SHARED
	if (sem_init(&(sem_info->isem), 1, 1) != 0) {
		ETK_DEBUG("[KERNEL]: %s --- Can't create sem : sem_init failed --- \"%s\"", __PRETTY_FUNCTION__, name);
		delete_area(sem->mapping);
		_ETK_UNLOCK_IPC_SEMAPHORE_();
		delete sem;
		return NULL;
	}

	if (sem_init(&(sem_info->sem), 1, 0) != 0) {
		ETK_DEBUG("[KERNEL]: %s --- Can't create sem : sem_init failed --- \"%s\"", __PRETTY_FUNCTION__, name);
		sem_destroy(&(sem_info->isem));
		delete_area(sem->mapping);
		_ETK_UNLOCK_IPC_SEMAPHORE_();
		delete sem;
		return NULL;
	}

	sem->remoteiSem = &(sem_info->isem);
	sem->remoteSem = &(sem_info->sem);
#else // ETK_PTHREAD_SHARED
	uint32 successFlags = 0;

	pthread_mutexattr_t mattr;
	pthread_condattr_t cattr;

	if (pthread_mutexattr_init(&mattr) != 0) successFlags |= (1 << 1);
	else if (pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED) != 0) successFlags |= (1 << 2);
	if (pthread_condattr_init(&cattr) != 0) successFlags |= (1 << 3);
	else if (pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED) != 0) successFlags |= (1 << 4);

	if (successFlags != 0) {
		ETK_DEBUG("[KERNEL]: %s --- Can't create sem : init mutex/cond attr failed(%lu) --- \"%s\"", __PRETTY_FUNCTION__, successFlags, name);
		if (!(successFlags & (1 << 1))) pthread_mutexattr_destroy(&mattr);
		if (!(successFlags & (1 << 3))) pthread_condattr_destroy(&cattr);
		delete_area(sem->mapping);
		_ETK_UNLOCK_IPC_SEMAPHORE_();
		delete sem;
		return NULL;
	}

	if (pthread_mutex_init(&(sem_info->mutex), &mattr) != 0) successFlags |= (1 << 1);
	if (pthread_cond_init(&(sem_info->cond), &cattr) != 0) successFlags |= (1 << 2);

	pthread_mutexattr_destroy(&mattr);
	pthread_condattr_destroy(&cattr);

	if (successFlags != 0) {
		ETK_DEBUG("[KERNEL]: %s --- Can't create sem : create mutex/cond failed --- \"%s\"", __PRETTY_FUNCTION__, name);
		if (!(successFlags & (1 << 1))) pthread_mutex_destroy(&(sem_info->mutex));
		if (!(successFlags & (1 << 2))) pthread_cond_destroy(&(sem_info->cond));
		delete_area(sem->mapping);
		_ETK_UNLOCK_IPC_SEMAPHORE_();
		delete sem;
		return NULL;
	}

	sem->iMutex = &(sem_info->mutex);
	sem->iCond = &(sem_info->cond);
#endif // ETK_PTHREAD_SHARED

	sem->semInfo->count = count;
	sem->semInfo->refCount = 1;

	_ETK_UNLOCK_IPC_SEMAPHORE_();

	sem->created = true;

	return (void*)sem;
}


void* clone_sem(const char *name)
{
	if (name == NULL || *name == 0 || strlen(name) >B_OS_NAME_LENGTH) return NULL;

	posix_sem_t *sem = new posix_sem_t();
	if (!sem) return NULL;

	_ETK_LOCK_IPC_SEMAPHORE_();

	if ((sem->mapping = clone_area(name, (void**)&(sem->semInfo),
	                                   B_READ_AREA |B_WRITE_AREA, ETK_AREA_SYSTEM_SEMAPHORE_DOMAIN)) == NULL ||
	        sem->semInfo == NULL || sem->semInfo->refCount >= B_MAXUINT32) {
//		ETK_DEBUG("[KERNEL]: %s --- Can't clone semaphore : clone area failed --- \"%s\"", __PRETTY_FUNCTION__, name);
		if (sem->mapping) delete_area(sem->mapping);
		_ETK_UNLOCK_IPC_SEMAPHORE_();
		delete sem;
		return NULL;
	}

	posix_sem_info *sem_info = sem->semInfo;

#ifndef ETK_PTHREAD_SHARED
	sem->remoteiSem = &(sem_info->isem);
	sem->remoteSem = &(sem_info->sem);

	int sval;
	int err1 = (sem_getvalue(sem->remoteiSem, &sval) != 0 ? errno : 0);
	int err2 = (sem_getvalue(sem->remoteSem, &sval) != 0 ? errno : 0);
	if (err1 != 0 || err2 != 0) {
		ETK_WARNING("[KERNEL]: %s --- System seems not support process-shared semaphore, errno: %d - %d.",
		            __PRETTY_FUNCTION__, err1, err2);
		delete_area(sem->mapping);
		_ETK_UNLOCK_IPC_SEMAPHORE_();
		delete sem;
		return NULL;
	}
#else // ETK_PTHREAD_SHARED
	sem->iMutex = &(sem_info->mutex);
	sem->iCond = &(sem_info->cond);
#endif // ETK_PTHREAD_SHARED

	sem->semInfo->refCount += 1;

	_ETK_UNLOCK_IPC_SEMAPHORE_();

	sem->created = true;

	return (void*)sem;
}


void* clone_sem_by_source(void *data)
{
	posix_sem_t *sem = (posix_sem_t*)data;
	if (!sem || !sem->semInfo) return NULL;

	if (is_sem_for_IPC(sem)) return clone_sem(sem->semInfo->name);

	_ETK_LOCK_LOCAL_SEMAPHORE_();
	if (sem->no_clone || sem->semInfo->refCount >= B_MAXUINT32 || sem->semInfo->refCount == 0) {
		_ETK_UNLOCK_LOCAL_SEMAPHORE_();
		return NULL;
	}
	sem->semInfo->refCount += 1;
	_ETK_UNLOCK_LOCAL_SEMAPHORE_();

	return data;
}


static void* create_sem_for_local(int64 count)
{
	if (count <B_INT64_CONSTANT(0)) return NULL;

	posix_sem_t *sem = new posix_sem_t();
	if (!sem) return NULL;

	if ((sem->semInfo = new posix_sem_info()) == NULL ||
	        (sem->iMutex = new pthread_mutex_t) == NULL ||
	        (sem->iCond = new pthread_cond_t) == NULL) {
		if (sem->iMutex) delete sem->iMutex;
		if (sem->iCond) delete sem->iCond;
		if (sem->semInfo) delete sem->semInfo;
		delete sem;
		return NULL;
	}

	uint32 successFlags = 0;

	if (pthread_mutex_init(sem->iMutex, NULL) != 0) successFlags |= (1 << 1);
	if (pthread_cond_init(sem->iCond, NULL) != 0) successFlags |= (1 << 2);

	if (successFlags != 0) {
		if (!(successFlags & (1 << 1))) pthread_mutex_destroy(sem->iMutex);
		if (!(successFlags & (1 << 2))) pthread_cond_destroy(sem->iCond);
		delete sem->iMutex;
		delete sem->iCond;
		delete sem->semInfo;
		delete sem;
		return NULL;
	}

	sem->semInfo->count = count;
	sem->semInfo->refCount = 1;
	sem->created = true;

	return (void*)sem;
}


void* create_sem(int64 count, const char *name, area_access area_access)
{
	return((name == NULL || *name == 0) ?
	       create_sem_for_local(count) :
	       create_sem_for_IPC(count, name, area_access));
}


status_t get_sem_info(void *data, sem_info *info)
{
	posix_sem_t *sem = (posix_sem_t*)data;
	if (!sem || !info) return B_BAD_VALUE;

	bzero(info->name, B_OS_NAME_LENGTH + 1);

	lock_sem_inter(sem);

	if (is_sem_for_IPC(sem)) strcpy(info->name, sem->semInfo->name);
	info->latest_holder_team = sem->semInfo->latestHolderTeamId;
	info->latest_holder_thread = sem->semInfo->latestHolderThreadId;
	info->count = sem->semInfo->count - sem->semInfo->acquiringCount;
	info->closed = sem->semInfo->closed;

	unlock_sem_inter(sem);

	return B_OK;
}


status_t delete_sem(void *data)
{
	posix_sem_t *sem = (posix_sem_t*)data;
	if (!sem || !sem->semInfo) return B_BAD_VALUE;

	if (is_sem_for_IPC(sem)) _ETK_LOCK_IPC_SEMAPHORE_();
	else _ETK_LOCK_LOCAL_SEMAPHORE_();

	uint32 count = --(sem->semInfo->refCount);

	if (is_sem_for_IPC(sem)) _ETK_UNLOCK_IPC_SEMAPHORE_();
	else _ETK_UNLOCK_LOCAL_SEMAPHORE_();

	if (is_sem_for_IPC(sem)) {
		if (count == 0) {
#ifndef ETK_PTHREAD_SHARED
			sem_destroy(sem->remoteiSem);
			sem_destroy(sem->remoteSem);
#else
			pthread_mutex_destroy(sem->iMutex);
			pthread_cond_destroy(sem->iCond);
#endif
		}
		delete_area(sem->mapping);
	} else {
		if (count > 0) return B_OK;

		pthread_mutex_destroy(sem->iMutex);
		pthread_cond_destroy(sem->iCond);
		delete sem->iMutex;
		delete sem->iCond;
		delete sem->semInfo;
	}

	if (sem->created) {
		sem->created = false;
		delete sem;
	}

	return B_OK;
}


status_t delete_sem_etc(void *data, bool no_clone)
{
	posix_sem_t *sem = (posix_sem_t*)data;
	if (!sem || !sem->semInfo) return B_BAD_VALUE;

	if (is_sem_for_IPC(sem)) _ETK_LOCK_IPC_SEMAPHORE_();
	else _ETK_LOCK_LOCAL_SEMAPHORE_();

	if (!is_sem_for_IPC(sem) && no_clone) sem->no_clone = true;
	uint32 count = --(sem->semInfo->refCount);

	if (is_sem_for_IPC(sem)) _ETK_UNLOCK_IPC_SEMAPHORE_();
	else _ETK_UNLOCK_LOCAL_SEMAPHORE_();

	if (is_sem_for_IPC(sem)) {
		if (count == 0 && no_clone) {
#ifndef ETK_PTHREAD_SHARED
			sem_destroy(sem->remoteiSem);
			sem_destroy(sem->remoteSem);
#else
			pthread_mutex_destroy(sem->iMutex);
			pthread_cond_destroy(sem->iCond);
#endif
		}
		delete_area_etc(sem->mapping, no_clone);
	} else {
		if (count > 0) return B_OK;

		pthread_mutex_destroy(sem->iMutex);
		pthread_cond_destroy(sem->iCond);
		delete sem->iMutex;
		delete sem->iCond;
		delete sem->semInfo;
	}

	if (sem->created) {
		sem->created = false;
		delete sem;
	}

	return B_OK;
}


status_t close_sem(void *data)
{
	posix_sem_t *sem = (posix_sem_t*)data;
	if (!sem) return B_BAD_VALUE;

	lock_sem_inter(sem);
	if (sem->semInfo->closed) {
		unlock_sem_inter(sem);
		return B_ERROR;
	}
	sem->semInfo->closed = true;
#ifndef ETK_PTHREAD_SHARED
	if (is_sem_for_IPC(sem)) {
		if (sem->semInfo->acquiringCount >B_INT64_CONSTANT(0)) sem_post(sem->remoteSem);
	} else {
#endif
		pthread_cond_broadcast(sem->iCond);
#ifndef ETK_PTHREAD_SHARED
	}
#endif
	unlock_sem_inter(sem);

	return B_OK;
}


status_t acquire_sem_etc(void *data, int64 count, uint32 flags, bigtime_t microseconds_timeout)
{
	posix_sem_t *sem = (posix_sem_t*)data;
	if (!sem) return B_BAD_VALUE;

	if (microseconds_timeout <B_INT64_CONSTANT(0) || count <B_INT64_CONSTANT(1)) return B_BAD_VALUE;

	bigtime_t currentTime = real_time_clock_usecs();
	bool wait_forever = false;

	if (flags != B_ABSOLUTE_TIMEOUT) {
		if (microseconds_timeout == B_INFINITE_TIMEOUT || microseconds_timeout >B_MAXINT64 - currentTime)
			wait_forever = true;
		else
			microseconds_timeout += currentTime;
	}

	lock_sem_inter(sem);

	if (sem->semInfo->count - count >= B_INT64_CONSTANT(0)) {
		sem->semInfo->count -= count;
		sem->semInfo->SetLatestHolderTeamId(get_current_team_id());
		sem->semInfo->SetLatestHolderThreadId(get_current_thread_id());
		unlock_sem_inter(sem);
		return B_OK;
	} else if (sem->semInfo->closed) {
		unlock_sem_inter(sem);
		return B_ERROR;
	} else if (microseconds_timeout == currentTime && !wait_forever) {
		unlock_sem_inter(sem);
		return B_WOULD_BLOCK;
	}
	if (count >B_MAXINT64 - sem->semInfo->acquiringCount) {
		unlock_sem_inter(sem);
		return B_ERROR;
	}

	sem->semInfo->acquiringCount += count;
#ifndef ETK_PTHREAD_SHARED
	if (is_sem_for_IPC(sem)) {
		if (sem->semInfo->minAcquiringCount == B_INT64_CONSTANT(0) ||
		        sem->semInfo->minAcquiringCount > count) sem->semInfo->minAcquiringCount = count;
	}
#endif

	struct timespec ts;
	ts.tv_sec = (long)(microseconds_timeout /B_INT64_CONSTANT(1000000));
	ts.tv_nsec = (long)(microseconds_timeout %B_INT64_CONSTANT(1000000)) * 1000L;

	status_t retval = B_ERROR;

	while (true) {
#ifndef ETK_PTHREAD_SHARED
		if (is_sem_for_IPC(sem)) {
			sem_t *psem = sem->remoteSem;
			unlock_sem_inter(sem);
			int ret = (wait_forever ? sem_wait(psem) : sem_timedwait(psem, &ts));
			lock_sem_inter(sem);
			if (ret != 0) {
				if (errno == ETIMEDOUT && !wait_forever) retval = B_TIMED_OUT;
				break;
			}
		} else {
#endif
			int ret = (wait_forever ? pthread_cond_wait(sem->iCond, sem->iMutex) :
			           pthread_cond_timedwait(sem->iCond, sem->iMutex, &ts));

			if (ret != 0) {
				if (ret == ETIMEDOUT && !wait_forever)
					retval = B_TIMED_OUT;
				else
					lock_sem_inter(sem);
				break;
			}
#ifndef ETK_PTHREAD_SHARED
		}
#endif

		if (sem->semInfo->count - count >= B_INT64_CONSTANT(0)) {
			sem->semInfo->count -= count;
			sem->semInfo->SetLatestHolderTeamId(get_current_team_id());
			sem->semInfo->SetLatestHolderThreadId(get_current_thread_id());
			retval = B_OK;
			break;
		} else if (sem->semInfo->closed) {
			break;
		}

#ifndef ETK_PTHREAD_SHARED
		if (!is_sem_for_IPC(sem)) continue;

		if (sem->semInfo->minAcquiringCount > sem->semInfo->count) continue;
		if (sem->semInfo->minAcquiringCount == B_INT64_CONSTANT(0) ||
		        sem->semInfo->minAcquiringCount > count) sem->semInfo->minAcquiringCount = count;

		sem_post(sem->remoteSem);
#endif
	}

	sem->semInfo->acquiringCount -= count;
#ifndef ETK_PTHREAD_SHARED
	if (is_sem_for_IPC(sem)) {
		if (sem->semInfo->minAcquiringCount == count) sem->semInfo->minAcquiringCount = B_INT64_CONSTANT(0);
		sem_post(sem->remoteSem);
	}
#endif

	unlock_sem_inter(sem);

	return retval;
}


status_t acquire_sem(void *data)
{
	return acquire_sem_etc(data, B_INT64_CONSTANT(1), B_TIMEOUT, B_INFINITE_TIMEOUT);
}


status_t release_sem_etc(void *data, int64 count, uint32 flags)
{
	posix_sem_t *sem = (posix_sem_t*)data;
	if (!sem || count <B_INT64_CONSTANT(0)) return B_BAD_VALUE;

	lock_sem_inter(sem);

	status_t retval = B_ERROR;

	if (sem->semInfo->closed == false && (B_MAXINT64 - sem->semInfo->count >= count)) {
		sem->semInfo->count += count;

		if (flags != B_DO_NOT_RESCHEDULE) {
#ifndef ETK_PTHREAD_SHARED
			if (is_sem_for_IPC(sem)) {
				if (sem->semInfo->acquiringCount >B_INT64_CONSTANT(0)) sem_post(sem->remoteSem);
			} else {
#endif
				pthread_cond_broadcast(sem->iCond);
#ifndef ETK_PTHREAD_SHARED
			}
#endif
		}

		retval = B_OK;
	}

	unlock_sem_inter(sem);

	return retval;
}


status_t release_sem(void *data)
{
	return release_sem_etc(data, B_INT64_CONSTANT(1), 0);
}


status_t get_sem_count(void *data, int64 *count)
{
	posix_sem_t *sem = (posix_sem_t*)data;
	if (!sem || !count) return B_BAD_VALUE;

	lock_sem_inter(sem);
	*count = (sem->semInfo->acquiringCount <= B_INT64_CONSTANT(0) ?
	          sem->semInfo->count :B_INT64_CONSTANT(-1) * (sem->semInfo->acquiringCount));
	unlock_sem_inter(sem);

	return B_OK;
}

