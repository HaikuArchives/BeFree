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
 * File: OS.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_OS_H__
#define __ETK_OS_H__

#include <support/SupportDefs.h>
#include <support/Errors.h>

typedef status_t				(*e_thread_func)(void*);

#define B_SYSTEM_TIMEBASE			0
#define B_REAL_TIME_TIMEBASE			1

#define B_OS_NAME_LENGTH			32
#define B_INFINITE_TIMEOUT			B_MAXINT64

#define B_READ_AREA				1
#define B_WRITE_AREA				2

#define B_LOW_PRIORITY				5
#define B_NORMAL_PRIORITY			10
#define B_DISPLAY_PRIORITY			15
#define	B_URGENT_DISPLAY_PRIORITY		20
#define	B_REAL_TIME_DISPLAY_PRIORITY		100
#define	B_URGENT_PRIORITY			110
#define B_REAL_TIME_PRIORITY			120

/* flags for semaphore control */
enum {
	B_CAN_INTERRUPT		= 1,	/* semaphore can be interrupted by a signal */
	B_DO_NOT_RESCHEDULE	= 2,	/* release() without rescheduling */
	B_TIMEOUT		= 8,	/* honor the (relative) timeout parameter */
	B_RELATIVE_TIMEOUT	= 8,
	B_ABSOLUTE_TIMEOUT	= 16	/* honor the (absolute) timeout parameter */
};

#ifndef __ETK_KERNEL_H__
#include <kernel/Kernel.h>
#endif /* __ETK_KERNEL_H__ */

/* time functions */
#define e_snooze(microseconds)			snooze(microseconds)
#define e_snooze_until(time, timebase)		snooze_until(time, timebase)
#define e_real_time_clock()			real_time_clock()
#define e_real_time_clock_usecs()		real_time_clock_usecs()
#define e_system_time()				system_time()

#endif /* __ETK_OS_H__ */

