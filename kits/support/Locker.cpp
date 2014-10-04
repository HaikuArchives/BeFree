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
 * File: Locker.cpp
 * Description: BLocker --- locker support nested-locking enough times
 *
 * --------------------------------------------------------------------------*/

#include <kernel/Kernel.h>

#include "Locker.h"


BLocker::BLocker()
{
	fLocker = create_locker();
}


BLocker::~BLocker()
{
	if (fLocker) delete_locker(fLocker);
}


bool
BLocker::Lock()
{
	return(lock_locker(fLocker) == B_OK);
}


void
BLocker::Unlock()
{
	if (fLocker == NULL) return;

	if (count_locker_locks(fLocker) <= B_INT64_CONSTANT(0)) {
		ETK_WARNING("[SUPPORT]: %s -- Locker didn't locked by current thread.", __PRETTY_FUNCTION__);
		return;
	}

	unlock_locker(fLocker);
}


status_t
BLocker::LockWithTimeout(bigtime_t microseconds)
{
	return lock_locker_etc(fLocker, B_TIMEOUT, microseconds);
}


int64
BLocker::CountLocks() const
{
	return count_locker_locks(fLocker);
}


bool
BLocker::IsLockedByCurrentThread() const
{
	return(count_locker_locks(fLocker) >B_INT64_CONSTANT(0));
}

