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
 * File: etk-port.cpp
 *
 * --------------------------------------------------------------------------*/

#include <stdlib.h>

#include <kernel/Kernel.h>
#include <support/String.h>
#include <support/SimpleLocker.h>

typedef struct port_info {
	port_info() {
		InitData();
	}

	void InitData() {
		bzero(name, B_OS_NAME_LENGTH + 1);
		queue_length = 0;
		queue_count = 0;
		readerWaitCount = B_INT64_CONSTANT(0);
		writerWaitCount = B_INT64_CONSTANT(0);
		closed = false;
	}

	char			name[B_OS_NAME_LENGTH + 1];
	int32			queue_length;
	int32			queue_count;
	int64			readerWaitCount;
	int64			writerWaitCount;
	bool			closed;
} port_info;

typedef struct port_t {
	port_t()
			: iLocker(NULL), readerSem(NULL), writerSem(NULL), mapping(NULL), queueBuffer(NULL),
			openedIPC(false), portInfo(NULL), created(false), refCount(0) {
	}

	~port_t() {
		if (created) {
			created = false;
			delete_port((void*)this);
		}
	}

	void*			iLocker;
	void*			readerSem;
	void*			writerSem;

	// for IPC (name != NULL)
	void*			mapping;

	void*			queueBuffer;

	bool			openedIPC;

	port_info*		portInfo;

	bool			created;
	uint32			refCount;
} port_t;

class port_locker_t
{
	public:
		void *fSem;
		BSimpleLocker fLocker;

		port_locker_t()
				: fSem(NULL) {
		}

		~port_locker_t() {
			if (fSem != NULL) {
				// leave global semaphore, without "delete_sem(fSem)"
				delete_sem_etc(fSem, false);
			}
		}

		void Init() {
			if (fSem != NULL) return;

			if ((fSem = clone_sem("_port_global_")) == NULL)
				fSem = create_sem(1, "_port_global_", ETK_AREA_ACCESS_ALL);
			if (fSem == NULL) ETK_ERROR("[KERNEL]: Can't initialize global port!");
		}

		void LockLocal() {
			fLocker.Lock();
		}

		void UnlockLocal() {
			fLocker.Unlock();
		}

		void LockIPC() {
			LockLocal();
			Init();
			UnlockLocal();
			acquire_sem(fSem);
		}

		void UnlockIPC() {
			release_sem(fSem);
		}
};

static port_locker_t __port_locker__;
#define _ETK_LOCK_IPC_PORT_()		__port_locker__.LockIPC()
#define _ETK_UNLOCK_IPC_PORT_()		__port_locker__.UnlockIPC()
#define _ETK_LOCK_LOCAL_PORT_()		__port_locker__.LockLocal()
#define _ETK_UNLOCK_LOCAL_PORT_()	__port_locker__.UnlockLocal()


static bool is_port_for_IPC(const port_t *port)
{
	if (!port) return false;
	return(port->mapping != NULL);
}


static void lock_port_inter(port_t *port)
{
	if (is_port_for_IPC(port))
		acquire_sem(port->iLocker);
	else
		lock_locker(port->iLocker);
}


static void unlock_port_inter(port_t *port)
{
	if (is_port_for_IPC(port))
		release_sem(port->iLocker);
	else
		unlock_locker(port->iLocker);
}

#define ETK_PORT_PER_MESSAGE_LENGTH	(sizeof(int32) + sizeof(size_t) + ETK_MAX_PORT_BUFFER_SIZE)

static void* create_port_for_IPC(int32 queue_length, const char *name, area_access area_access)
{
	if (queue_length <= 0 || queue_length > ETK_VALID_MAX_PORT_QUEUE_LENGTH ||
	        name == NULL || *name == 0 || strlen(name) >B_OS_NAME_LENGTH - 1) return NULL;

	char *tmpSemName = b_strdup_printf("%s ", name);
	if (!tmpSemName) return NULL;

	port_t *port = new port_t();
	if (!port) {
		free(tmpSemName);
		return NULL;
	}

	_ETK_LOCK_IPC_PORT_();
	if ((port->mapping = create_area(name, (void**)&(port->portInfo),
	                                     sizeof(port_info) + (size_t)queue_length * ETK_PORT_PER_MESSAGE_LENGTH,
	                                     B_READ_AREA |B_WRITE_AREA, ETK_AREA_SYSTEM_PORT_DOMAIN, area_access)) == NULL ||
	        port->portInfo == NULL) {
		if (port->mapping) delete_area(port->mapping);
		_ETK_UNLOCK_IPC_PORT_();
		delete port;
		free(tmpSemName);
		return NULL;
	}

	port_info *port_info = port->portInfo;
	port_info->InitData();
	memcpy(port_info->name, name, (size_t)strlen(name));
	port_info->queue_length = queue_length;

	if ((port->iLocker = create_sem(1, name, area_access)) == NULL) {
		delete_area(port->mapping);
		_ETK_UNLOCK_IPC_PORT_();
		delete port;
		free(tmpSemName);
		return NULL;
	}
	tmpSemName[strlen(tmpSemName) - 1] = 'r';
	if ((port->readerSem = create_sem(0, tmpSemName, area_access)) == NULL) {
		delete_sem(port->iLocker);
		delete_area(port->mapping);
		_ETK_UNLOCK_IPC_PORT_();
		delete port;
		free(tmpSemName);
		return NULL;
	}
	tmpSemName[strlen(tmpSemName) - 1] = 'w';
	if ((port->writerSem = create_sem(0, tmpSemName, area_access)) == NULL) {
		delete_sem(port->readerSem);
		delete_sem(port->iLocker);
		delete_area(port->mapping);
		_ETK_UNLOCK_IPC_PORT_();
		delete port;
		free(tmpSemName);
		return NULL;
	}
	_ETK_UNLOCK_IPC_PORT_();

	free(tmpSemName);

	char *buffer = (char*)(port->portInfo);
	buffer += sizeof(port_info);
	port->queueBuffer = (void*)buffer;

	port->openedIPC = false;
	port->created = true;

	return (void*)port;
}


void* open_port(const char *name)
{
	if (name == NULL || *name == 0 || strlen(name) >B_OS_NAME_LENGTH - 1) return NULL;

	char *tmpSemName = b_strdup_printf("%s ", name);
	if (!tmpSemName) return NULL;

	port_t *port = new port_t();
	if (!port) {
		free(tmpSemName);
		return NULL;
	}

	_ETK_LOCK_IPC_PORT_();
	if ((port->mapping = clone_area(name, (void**)&(port->portInfo),
	                                    B_READ_AREA |B_WRITE_AREA, ETK_AREA_SYSTEM_PORT_DOMAIN)) == NULL ||
	        port->portInfo == NULL) {
		if (port->mapping) delete_area(port->mapping);
		_ETK_UNLOCK_IPC_PORT_();
		delete port;
		free(tmpSemName);
		return NULL;
	}

	if ((port->iLocker = clone_sem(name)) == NULL) {
		delete_area(port->mapping);
		_ETK_UNLOCK_IPC_PORT_();
		delete port;
		free(tmpSemName);
		return NULL;
	}
	tmpSemName[strlen(tmpSemName) - 1] = 'r';
	if ((port->readerSem = clone_sem(tmpSemName)) == NULL) {
		delete_sem(port->iLocker);
		delete_area(port->mapping);
		_ETK_UNLOCK_IPC_PORT_();
		delete port;
		free(tmpSemName);
		return NULL;
	}
	tmpSemName[strlen(tmpSemName) - 1] = 'w';
	if ((port->writerSem = clone_sem(tmpSemName)) == NULL) {
		delete_sem(port->readerSem);
		delete_sem(port->iLocker);
		delete_area(port->mapping);
		_ETK_UNLOCK_IPC_PORT_();
		delete port;
		free(tmpSemName);
		return NULL;
	}
	_ETK_UNLOCK_IPC_PORT_();

	free(tmpSemName);

	char *buffer = (char*)(port->portInfo);
	buffer += sizeof(port_info);
	port->queueBuffer = (void*)buffer;

	port->openedIPC = true;
	port->created = true;

	return (void*)port;
}


void* open_port_by_source(void *data)
{
	port_t *port = (port_t*)data;
	if (!port || !port->portInfo) return NULL;

	if (is_port_for_IPC(port)) return open_port(port->portInfo->name);

	_ETK_LOCK_LOCAL_PORT_();
	if (port->refCount == B_MAXUINT32 || port->refCount == 0 || port->portInfo->closed) {
		_ETK_UNLOCK_LOCAL_PORT_();
		return NULL;
	}
	port->refCount += 1;
	_ETK_UNLOCK_LOCAL_PORT_();

	return data;
}


static void* create_port_for_local(int32 queue_length)
{
	if (queue_length <= 0 || queue_length > ETK_VALID_MAX_PORT_QUEUE_LENGTH) return NULL;

	port_t *port = new port_t();
	if (!port) return NULL;

	if ((port->iLocker = create_locker()) == NULL) {
		delete port;
		return NULL;
	}
	if ((port->readerSem = create_sem(0, NULL)) == NULL) {
		delete_locker(port->iLocker);
		delete port;
		return NULL;
	}
	if ((port->writerSem = create_sem(0, NULL)) == NULL) {
		delete_sem(port->readerSem);
		delete_locker(port->iLocker);
		delete port;
		return NULL;
	}

	if ((port->portInfo = new port_info()) == NULL) {
		delete_sem(port->writerSem);
		delete_sem(port->readerSem);
		delete_locker(port->iLocker);
		delete port;
		return NULL;
	}

	if ((port->queueBuffer = malloc((size_t)queue_length * ETK_PORT_PER_MESSAGE_LENGTH)) == NULL) {
		delete port->portInfo;
		delete_sem(port->writerSem);
		delete_sem(port->readerSem);
		delete_locker(port->iLocker);
		delete port;
		return NULL;
	}

	port->portInfo->queue_length = queue_length;

	port->refCount = 1;
	port->created = true;

	return (void*)port;
}


void* create_port(int32 queue_length, const char *name, area_access area_access)
{
	return((name == NULL || *name == 0) ?
	       create_port_for_local(queue_length) :
	       create_port_for_IPC(queue_length, name, area_access));
}


status_t delete_port(void *data)
{
	port_t *port = (port_t*)data;
	if (!port) return B_BAD_VALUE;

	if (is_port_for_IPC(port)) {
		delete_area(port->mapping);
		delete_sem(port->iLocker);
	} else {
		_ETK_LOCK_LOCAL_PORT_();
		if (port->refCount == 0) {
			_ETK_UNLOCK_LOCAL_PORT_();
			return B_ERROR;
		}
		uint32 count = --(port->refCount);
		_ETK_UNLOCK_LOCAL_PORT_();

		if (count > 0) return B_OK;

		free(port->queueBuffer);
		delete port->portInfo;
		delete_locker(port->iLocker);
	}

	delete_sem(port->writerSem);
	delete_sem(port->readerSem);

	if (port->created) {
		port->created = false;
		delete port;
	}

	return B_OK;
}


status_t close_port(void *data)
{
	port_t *port = (port_t*)data;
	if (!port) return B_BAD_VALUE;

	lock_port_inter(port);
	if (port->portInfo->closed) {
		unlock_port_inter(port);
		return B_ERROR;
	}
	port->portInfo->closed = true;
	release_sem_etc(port->readerSem, port->portInfo->writerWaitCount, 0);
	release_sem_etc(port->writerSem, port->portInfo->readerWaitCount, 0);
	unlock_port_inter(port);

	return B_OK;
}


status_t write_port_etc(void *data, int32 code, const void *buf, size_t buf_size, uint32 flags, bigtime_t microseconds_timeout)
{
	port_t *port = (port_t*)data;
	if (!port) return B_BAD_VALUE;

	if ((!buf && buf_size > 0) || buf_size > ETK_MAX_PORT_BUFFER_SIZE || microseconds_timeout <B_INT64_CONSTANT(0)) return B_BAD_VALUE;

	bigtime_t currentTime = real_time_clock_usecs();
	bool wait_forever = false;

	if (flags != B_ABSOLUTE_TIMEOUT) {
		if (microseconds_timeout == B_INFINITE_TIMEOUT || microseconds_timeout >B_MAXINT64 - currentTime)
			wait_forever = true;
		else
			microseconds_timeout += currentTime;
	}

	lock_port_inter(port);

	if (port->portInfo->closed) {
		unlock_port_inter(port);
		return B_ERROR;
	} else if (port->portInfo->queue_count < port->portInfo->queue_length) {
		size_t offset = (size_t)port->portInfo->queue_count * ETK_PORT_PER_MESSAGE_LENGTH;
		char* buffer = (char*)(port->queueBuffer);
		buffer += offset;
		memcpy(buffer, &code, sizeof(int32));
		buffer += sizeof(int32);
		memcpy(buffer, &buf_size, sizeof(size_t));
		buffer += sizeof(size_t);
		if (buf_size > 0) memcpy(buffer, buf, buf_size);

		port->portInfo->queue_count++;

		release_sem_etc(port->writerSem, port->portInfo->readerWaitCount, 0);

		unlock_port_inter(port);
		return B_OK;
	} else if (microseconds_timeout == currentTime && !wait_forever) {
		unlock_port_inter(port);
		return B_WOULD_BLOCK;
	}

	port->portInfo->writerWaitCount += B_INT64_CONSTANT(1);

	status_t retval = B_ERROR;

	while (true) {
		unlock_port_inter(port);
		status_t status = (wait_forever ?
		                   acquire_sem(port->readerSem) :
		                   acquire_sem_etc(port->readerSem, 1, B_ABSOLUTE_TIMEOUT, microseconds_timeout));
		lock_port_inter(port);

		if (status != B_OK) {
			retval = status;
			break;
		}

		if (port->portInfo->closed) {
			retval = B_ERROR;
			break;
		} else if (port->portInfo->queue_count < port->portInfo->queue_length) {
			size_t offset = (size_t)port->portInfo->queue_count * ETK_PORT_PER_MESSAGE_LENGTH;
			char* buffer = (char*)(port->queueBuffer);
			buffer += offset;
			memcpy(buffer, &code, sizeof(int32));
			buffer += sizeof(int32);
			memcpy(buffer, &buf_size, sizeof(size_t));
			buffer += sizeof(size_t);
			if (buf_size > 0) memcpy(buffer, buf, buf_size);

			port->portInfo->queue_count++;
			release_sem_etc(port->writerSem, port->portInfo->readerWaitCount, 0);

			retval = B_OK;
			break;
		}
	}

	port->portInfo->writerWaitCount -= B_INT64_CONSTANT(1);

	unlock_port_inter(port);

	return retval;
}


ssize_t port_buffer_size_etc(void *data, uint32 flags, bigtime_t microseconds_timeout)
{
	port_t *port = (port_t*)data;
	if (!port) return B_BAD_VALUE;

	if (microseconds_timeout <B_INT64_CONSTANT(0)) return (ssize_t)B_BAD_VALUE;

	bigtime_t currentTime = real_time_clock_usecs();
	bool wait_forever = false;

	if (flags != B_ABSOLUTE_TIMEOUT) {
		if (microseconds_timeout == B_INFINITE_TIMEOUT || microseconds_timeout >B_MAXINT64 - currentTime)
			wait_forever = true;
		else
			microseconds_timeout += currentTime;
	}

	lock_port_inter(port);

	if (port->portInfo->queue_count > 0) {
		const char* buffer = (const char*)(port->queueBuffer);
		size_t msgLen = 0;

		buffer += sizeof(int32);
		memcpy(&msgLen, buffer, sizeof(size_t));

		unlock_port_inter(port);
		return (ssize_t)msgLen;
	} else if (port->portInfo->closed) {
		unlock_port_inter(port);
		return B_ERROR;
	} else if (microseconds_timeout == currentTime && !wait_forever) {
		unlock_port_inter(port);
		return B_WOULD_BLOCK;
	}

	port->portInfo->readerWaitCount += B_INT64_CONSTANT(1);

	status_t retval = B_ERROR;

	while (true) {
		unlock_port_inter(port);
		status_t status = (wait_forever ?
		                   acquire_sem(port->writerSem) :
		                   acquire_sem_etc(port->writerSem, 1, B_ABSOLUTE_TIMEOUT, microseconds_timeout));
		lock_port_inter(port);

		if (status != B_OK) {
			retval = status;
			break;
		}

		if (port->portInfo->queue_count > 0) {
			const char* buffer = (const char*)(port->queueBuffer);
			size_t msgLen = 0;

			buffer += sizeof(int32);
			memcpy(&msgLen, buffer, sizeof(size_t));

			retval = (status_t)msgLen;
			break;
		} else if (port->portInfo->closed) {
			retval = B_ERROR;
			break;
		}
	}

	port->portInfo->readerWaitCount -= B_INT64_CONSTANT(1);

	unlock_port_inter(port);

	return (ssize_t)retval;
}


status_t read_port_etc(void *data, int32 *code, void *buf, size_t buf_size, uint32 flags, bigtime_t microseconds_timeout)
{
	port_t *port = (port_t*)data;
	if (!port) return B_BAD_VALUE;

	if (!code || (!buf && buf_size > 0) || microseconds_timeout <B_INT64_CONSTANT(0)) return B_BAD_VALUE;

	bigtime_t currentTime = real_time_clock_usecs();
	bool wait_forever = false;

	if (flags != B_ABSOLUTE_TIMEOUT) {
		if (microseconds_timeout == B_INFINITE_TIMEOUT || microseconds_timeout >B_MAXINT64 - currentTime)
			wait_forever = true;
		else
			microseconds_timeout += currentTime;
	}

	lock_port_inter(port);

	if (port->portInfo->queue_count > 0) {
		char* buffer = (char*)(port->queueBuffer);
		size_t msgLen = 0;
		memcpy(code, buffer, sizeof(int32));
		buffer += sizeof(int32);
		memcpy(&msgLen, buffer, sizeof(size_t));
		buffer += sizeof(size_t);
		if (msgLen > 0 && buf_size > 0) memcpy(buf, buffer, min_c(msgLen, buf_size));
		if (port->portInfo->queue_count > 1) {
			buffer = (char*)(port->queueBuffer);
			memmove(buffer, buffer + ETK_PORT_PER_MESSAGE_LENGTH, (size_t)(port->portInfo->queue_count - 1) * ETK_PORT_PER_MESSAGE_LENGTH);
		}

		port->portInfo->queue_count--;

		release_sem_etc(port->readerSem, port->portInfo->writerWaitCount, 0);

		unlock_port_inter(port);
		return B_OK;
	} else if (port->portInfo->closed) {
		unlock_port_inter(port);
		return B_ERROR;
	} else if (microseconds_timeout == currentTime && !wait_forever) {
		unlock_port_inter(port);
		return B_WOULD_BLOCK;
	}

	port->portInfo->readerWaitCount += B_INT64_CONSTANT(1);

	status_t retval = B_ERROR;

	while (true) {
		unlock_port_inter(port);
		status_t status = (wait_forever ?
		                   acquire_sem(port->writerSem) :
		                   acquire_sem_etc(port->writerSem, 1, B_ABSOLUTE_TIMEOUT, microseconds_timeout));
		lock_port_inter(port);

		if (status != B_OK) {
			retval = status;
			break;
		}

		if (port->portInfo->queue_count > 0) {
			char* buffer = (char*)(port->queueBuffer);
			size_t msgLen = 0;
			memcpy(code, buffer, sizeof(int32));
			buffer += sizeof(int32);
			memcpy(&msgLen, buffer, sizeof(size_t));
			buffer += sizeof(size_t);
			if (msgLen > 0 && buf_size > 0) memcpy(buf, buffer, min_c(msgLen, buf_size));
			if (port->portInfo->queue_count > 1) {
				buffer = (char*)(port->queueBuffer);
				memmove(buffer, buffer + ETK_PORT_PER_MESSAGE_LENGTH, (size_t)(port->portInfo->queue_count - 1) * ETK_PORT_PER_MESSAGE_LENGTH);
			}

			port->portInfo->queue_count--;

			release_sem_etc(port->readerSem, port->portInfo->writerWaitCount, 0);

			retval = B_OK;
			break;
		} else if (port->portInfo->closed) {
			retval = B_ERROR;
			break;
		}
	}

	port->portInfo->readerWaitCount -= B_INT64_CONSTANT(1);

	unlock_port_inter(port);

	return retval;
}


status_t write_port(void *data, int32 code, const void *buf, size_t buf_size)
{
	return write_port_etc(data, code, buf, buf_size, B_TIMEOUT, B_INFINITE_TIMEOUT);
}


ssize_t port_buffer_size(void *data)
{
	return port_buffer_size_etc(data, B_TIMEOUT, B_INFINITE_TIMEOUT);
}


status_t read_port(void *data, int32 *code, void *buf, size_t buf_size)
{
	return read_port_etc(data, code, buf, buf_size, B_TIMEOUT, B_INFINITE_TIMEOUT);
}


int32 port_count(void *data)
{
	port_t *port = (port_t*)data;
	if (!port) return B_BAD_VALUE;

	lock_port_inter(port);
	int32 retval = port->portInfo->queue_count;
	unlock_port_inter(port);

	return retval;
}

