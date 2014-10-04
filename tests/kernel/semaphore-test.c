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
 * File: semaphore-test.c
 *
 * --------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <kernel/OS.h>
#include <kernel/Kernel.h>
#include <kernel/Debug.h>

status_t test_func(void *sem)
{
	int n = 0, m = 0;
	status_t status;

	while (true) {
		ETK_OUTPUT("Acquiring with timeout 1000000 microseconds...\n");
		status = acquire_sem_etc(sem, B_INT64_CONSTANT(1), B_TIMEOUT, B_INT64_CONSTANT(1000000));
		if (status == B_TIMED_OUT) {
			ETK_OUTPUT("Acquire semaphore timed out, I'll retry.\n");
			n++;
			if (n > 10) {
				ETK_OUTPUT("Acquire semaphore timed out more than 10 times, I'll exit!\n");
				break;
			}
		} else if (status != B_OK) {
			ETK_OUTPUT("Acquire semaphore error, I'll exit!\n");
			return B_ERROR;
		} else {
			m++;
			ETK_OUTPUT("Acquired[%d], I'll snooze 1 second then unlock it and retry.\n", n);
			e_snooze(1000000);
			release_sem(sem);
			if (m > 10) {
				ETK_OUTPUT("Acquired semaphore more than 10 times, test finished.\n");
				break;
			}
		}
	}

	return B_OK;
}


int main(int argc, char **argv)
{
	void *sem, *thread;
	status_t status;

	if ((sem = create_sem(B_INT64_CONSTANT(1), "sem-test", ETK_AREA_ACCESS_OWNER)) == NULL) {
		ETK_OUTPUT("Creat semaphore failed!\n");
		exit(1);
	}

	acquire_sem(sem);

	if ((thread = create_thread(test_func, B_NORMAL_PRIORITY, sem, NULL)) != NULL) {
		resume_thread(thread);
	} else {
		ETK_OUTPUT("Create thread failed!\n");
		exit(1);
	}

	wait_for_thread(thread, &status);

	delete_thread(thread);
	delete_sem(sem);

	return 0;
}


