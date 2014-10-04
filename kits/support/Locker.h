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
 * File: Locker.h
 * Description: BLocker --- locker support nested-locking enough times
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_LOCKER_H__
#define __ETK_LOCKER_H__

#include <support/SupportDefs.h>

#ifdef __cplusplus /* Just for C++ */

class BLocker
{
	public:
		BLocker();
		virtual ~BLocker();

		bool		Lock();
		void		Unlock();
		status_t	LockWithTimeout(bigtime_t microseconds);

		// CountLocks():
		// 	return count of locks when locked by current thread,
		// 	return less than 0 when locked by other thread or invalid,
		// 	return 0 when it isn't locked or valid.
		int64		CountLocks() const;
		bool		IsLockedByCurrentThread() const;

	private:
		void		*fLocker;
};

#endif /* __cplusplus */

#endif /* __ETK_LOCKER_H__ */


