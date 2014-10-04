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
 * File: Kernel.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_KERNEL_H__
#define __ETK_KERNEL_H__

#include <kernel/OS.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

	/* time functions */
	uint32	real_time_clock(void);
	bigtime_t	real_time_clock_usecs(void);
	bigtime_t	system_boot_time(void); /* system boot time in microseconds */
	bigtime_t system_time(void); /* time since booting in microseconds */

	/* area functions */
	typedef struct area_info {
		char		name[B_OS_NAME_LENGTH + 1];
		size_t		size;
		uint32		protection;
		void		*address;
		char		domain[5];
	} area_info;

#define ETK_AREA_SYSTEM_SEMAPHORE_DOMAIN	"ssem"
#define ETK_AREA_SYSTEM_PORT_DOMAIN		"spot"
#define ETK_AREA_USER_DOMAIN			"user"

	typedef enum area_access {
		ETK_AREA_ACCESS_OWNER = 0,
		ETK_AREA_ACCESS_GROUP_READ = 1,
		ETK_AREA_ACCESS_GROUP_WRITE = 1 << 1,
		ETK_AREA_ACCESS_OTHERS_READ = 1 << 2,
		ETK_AREA_ACCESS_OTHERS_WRITE = 1 << 3,
		ETK_AREA_ACCESS_ALL = 0xFF
	} area_access;

#ifdef __cplusplus
	void*	create_area(const char *name, void **start_addr, size_t size, uint32 protection,
	                      const char *domain, area_access area_access = ETK_AREA_ACCESS_OWNER);
#else
	void*	create_area(const char *name, void **start_addr, size_t size, uint32 protection,
	                      const char *domain, area_access area_access);
#endif
	void*	clone_area(const char *name, void **dest_addr, uint32 protection, const char *domain);
	void*	clone_area_by_source(void *source_area, void **dest_addr, uint32 protection);
	status_t	get_area_info(void *area, area_info *info);
	status_t	delete_area(void *area);
	status_t	delete_area_etc(void *area, bool no_clone);

	/* resize_area:
	 * 	Only the original area that created by "create_area" is allowed resizing.
	 * 	When it was resized, the clone-area must reclone to get the valid address.
	 * */
	status_t	resize_area(void *area, void **start_addr, size_t new_size);
	status_t	set_area_protection(void *area, uint32 new_protection);

	/* locker functions */
	void*	create_locker(void);
	void*	clone_locker(void* locker);
	status_t	delete_locker(void* locker);

	/* after you calling "close_locker":
	 * 	1. the next "lock_locker..." function call will be failed
	 * */
	status_t	close_locker(void* locker);

	status_t	lock_locker(void *locker);
	status_t	lock_locker_etc(void *locker, uint32 flags, bigtime_t timeout);
	status_t	unlock_locker(void *locker);

	/* count_locker_locks:
	 * 	return count of locks when locked by current thread,
	 * 	return less than 0 when locked by other thread or invalid,
	 * 	return 0 when it isn't locked or valid.
	 * */
	int64	count_locker_locks(void *locker);

	/* *_simple_locker:
	 *	The "simple_locker" DO NOT support nested-locking
	 * */
	void*	create_simple_locker(void);
	status_t	delete_simple_locker(void* slocker);
	bool	lock_simple_locker(void *slocker);
	void	unlock_simple_locker(void *slocker);

#ifdef ETK_BUILD_WITH_MEMORY_TRACING
	/* memory_tracing_*:
	 *	The ETK++ use this to handle synchronization problem.
	 * */
	bool	memory_tracing_lock(void);
	void	memory_tracing_unlock(void);
#endif

	/* semaphore functions */
	typedef struct sem_info {
		char		name[B_OS_NAME_LENGTH + 1];
		int64		latest_holder_team;
		int64		latest_holder_thread;
		int64		count;
		bool		closed;
	} sem_info;

#ifdef __cplusplus
	void*	create_sem(int64 count, const char *name, area_access area_access = ETK_AREA_ACCESS_OWNER);
#else
	void*	create_sem(int64 count, const char *name, area_access area_access);
#endif
	void*	clone_sem(const char *name);
	void*	clone_sem_by_source(void *sem);
	status_t	get_sem_info(void *sem, sem_info *info);
	status_t	delete_sem(void *sem);
	status_t	delete_sem_etc(void *sem, bool no_clone);

	/* after you calling "close_sem()":
	 * 	1. the next "release_sem..." function call will be failed
	 * 	2. the next "acquire_sem..." function call will be failed when the sem's count <= 0
	 * */
	status_t	close_sem(void* sem);

	status_t	acquire_sem(void *sem);
	status_t	release_sem(void *sem);
	status_t	acquire_sem_etc(void *sem, int64 count, uint32 flags, bigtime_t timeout);
	status_t	release_sem_etc(void *sem, int64 count, uint32 flags);
	status_t	get_sem_count(void *sem, int64 *count);


	/* thread functions */
	/* Default stack size of thread: 256KB */
	status_t	snooze(bigtime_t microseconds);
	status_t	snooze_until(bigtime_t time, int timebase);

	int64	get_current_team_id(void);
	int64	get_current_thread_id(void);

	void*	create_thread_by_current_thread(void);
	void*	create_thread(e_thread_func threadFunction,
	                        int32 priority,
	                        void *arg,
	                        int64 *threadId);
	void*	open_thread(int64 threadId);
	status_t	delete_thread(void *thread);

	/* suspend_thread():
	 * 	Be careful please !!!
	 * 	In POSIX-Thread implementation only supported to suspend the current thread.
	 * 	It return B_OK if successed. */
	status_t	suspend_thread(void *thread);
	status_t	resume_thread(void *thread);

	status_t	on_exit_thread(void (*callback)(void *), void *user_data);

	int64	get_thread_id(void *thread);

	enum {
		ETK_THREAD_INVALID = 0,
		ETK_THREAD_READY,
		ETK_THREAD_RUNNING,
		ETK_THREAD_EXITED,
		ETK_THREAD_SUSPENDED,
	};
	uint32	get_thread_run_state(void *thread);

	status_t	set_thread_priority(void *thread, int32 new_priority);
	int32	get_thread_priority(void *thread);
	void	exit_thread(status_t status);
	status_t	wait_for_thread(void *thread, status_t *thread_return_value);
	status_t	wait_for_thread_etc(void *thread, status_t *thread_return_value, uint32 flags, bigtime_t timeout);


#define ETK_MAX_PORT_BUFFER_SIZE		((size_t)4096)
#define ETK_VALID_MAX_PORT_QUEUE_LENGTH		((int32)300)


	/* port functions */
#ifdef __cplusplus
	void*	create_port(int32 queue_length, const char *name, area_access area_access = ETK_AREA_ACCESS_OWNER);
#else
	void*	create_port(int32 queue_length, const char *name, area_access area_access);
#endif
	void*	open_port(const char *name);
	void*	open_port_by_source(void *port);
	status_t	delete_port(void *port);

	/* after you calling "close_port":
	 * 	1. the next "write_port..." function call will be failed
	 * 	2. the next "read_port..." function call will be failed when queue is empty
	 * */
	status_t	close_port(void *port);

	status_t	write_port(void *port, int32 code, const void *buf, size_t buf_size);
	ssize_t	port_buffer_size(void *port);
	status_t	read_port(void *port, int32 *code, void *buf, size_t buf_size);

	status_t	write_port_etc(void *port, int32 code, const void *buf, size_t buf_size, uint32 flags, bigtime_t timeout);
	ssize_t	port_buffer_size_etc(void *port, uint32 flags, bigtime_t timeout);
	status_t	read_port_etc(void *port, int32 *code, void *buf, size_t buf_size, uint32 flags, bigtime_t timeout);

	int32	port_count(void *port);


	/* image functions */

	void*	load_addon(const char* path);
	status_t	unload_addon(void *image);
	status_t	get_image_symbol(void *image, const char *name, void **ptr);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __ETK_KERNEL_H__ */

