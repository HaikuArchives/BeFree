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
 * File: etk-timefuncs.cpp
 *
 * --------------------------------------------------------------------------*/

#include <sys/time.h>
#include <time.h>

#include <kernel/Kernel.h>
#include <support/SimpleLocker.h>

#define SECS_TO_US		B_INT64_CONSTANT(1000000)


// return the number of microseconds elapsed since 00:00 01 January 1970 UTC (Unix epoch)
bigtime_t real_time_clock_usecs(void)
{
	int64 current_time = B_INT64_CONSTANT(-1);
	struct timespec ts;

	if (clock_gettime(CLOCK_REALTIME, &ts) == 0)
		current_time = (int64)ts.tv_sec * SECS_TO_US + (int64)(ts.tv_nsec + 500) /B_INT64_CONSTANT(1000);
	return current_time;
}


// return the number of seconds elapsed since 00:00 01 January 1970 UTC (Unix epoch)
uint32 real_time_clock(void)
{
	uint32 current_time = 0;
	struct timespec ts;
	if (clock_gettime(CLOCK_REALTIME, &ts) == 0) current_time = (uint32)ts.tv_sec;
	return current_time;
}


static int64 unix_boot_time = B_INT64_CONSTANT(-1);
static BSimpleLocker unix_boot_time_locker(true);


bigtime_t system_boot_time(void)
{
	bigtime_t retValue = B_INT64_CONSTANT(-1);

	unix_boot_time_locker.Lock();

	if (unix_boot_time >= B_INT64_CONSTANT(0)) {
		retValue = unix_boot_time;
	} else {
		struct timespec ts;

		if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
			bigtime_t up_time = (int64)ts.tv_sec * SECS_TO_US + (int64)(ts.tv_nsec + 500) /B_INT64_CONSTANT(1000);
			retValue = unix_boot_time = real_time_clock_usecs() - up_time;
		}
	}

	unix_boot_time_locker.Unlock();

	return retValue;
}


bigtime_t system_time(void)
{
	// FIXME
	return(real_time_clock_usecs() - system_boot_time());
}

