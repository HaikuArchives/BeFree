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
 * File: semaphore-locking-test.c
 *
 * --------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <kernel/Kernel.h>
#include <support/String.h>

static int32 test_count = 324;
static void* sem_locker = NULL;

int32 test_subtract(void *arg)
{
	char *name = (char*)arg;
	int32 i;

	for (i = 0; i < 5; i++) {
		int32 origin_count;
		status_t status = acquire_sem(sem_locker);
		if (status != B_OK) {
			ETK_OUTPUT("%s: lock error, error_code = %ld\n", name, status);
			break;
		}
		origin_count = test_count;
		test_count -= i;
		ETK_OUTPUT("%s: [Origin:%ld] - [%ld] = [Now: %ld]\n",
		           name, origin_count, i, test_count);
		release_sem(sem_locker);
		e_snooze(1000);
	}

	if (name) free(name);

	return 0;
}


int32 test_plus(void *arg)
{
	char *name = (char*)arg;
	int32 i;

	for (i = 0; i < 5; i++) {
		int32 origin_count;
		status_t status = acquire_sem(sem_locker);

		if (status != B_OK) {
			ETK_OUTPUT("%s: lock error, error_code = %ld\n", name, status);
			break;
		}
		origin_count = test_count;
		test_count += i;
		ETK_OUTPUT("%s: [Origin:%ld] + [%ld] = [Now: %ld]\n",
		           name, origin_count, i, test_count);
		release_sem(sem_locker);
		e_snooze(1000);
	}

	if (name) free(name);

	return 0;
}

#define TEST_THREAD_NUM 50

int main(int argc, char **argv)
{
	void *thread_plus[TEST_THREAD_NUM];
	void *thread_subtract[TEST_THREAD_NUM];
	int32 i;

	if ((sem_locker = create_sem(1, "locking-test", ETK_AREA_ACCESS_OWNER)) == NULL) {
		ETK_OUTPUT("Creat semaphore failed!\n");
		exit(1);
	}

	for (i = 0; i < TEST_THREAD_NUM; i++) {
		char name[512];
		sprintf(name, "[PLUS][Thread %d]", i);

		if ((thread_plus[i] = create_thread(test_plus, B_NORMAL_PRIORITY, (void*)b_strdup(name), NULL)) == NULL) {
			close_sem(sem_locker);
			ETK_ERROR("[MAIN THREAD]: Unable to create %s\n", name);
			continue;
		}

		resume_thread(thread_plus[i]);
	}

	for (i = 0; i < TEST_THREAD_NUM; i++) {
		char name[512];
		sprintf(name, "[SUBTRACT][Thread %d]", i);

		if ((thread_subtract[i] = create_thread(test_subtract, B_NORMAL_PRIORITY, (void*)b_strdup(name), NULL)) == NULL) {
			close_sem(sem_locker);
			ETK_ERROR("[MAIN THREAD]: Unable to create %s\n", name);
			continue;
		}
		resume_thread(thread_subtract[i]);
	}

	ETK_OUTPUT("[MAIN THREAD][Wait 10 seconds...]\n");
	e_snooze(10000000); // wait 10 seconds
	ETK_OUTPUT("[MAIN THREAD][I'll close the semaphore...]\n");

	close_sem(sem_locker);

	for (i = 0; i < TEST_THREAD_NUM; i++) {
		status_t status;
		wait_for_thread(thread_plus[i], &status);
		delete_thread(thread_plus[i]);
	}

	for (i = 0; i < TEST_THREAD_NUM; i++) {
		status_t status;
		wait_for_thread(thread_subtract[i], &status);
		delete_thread(thread_subtract[i]);
	}

	delete_sem(sem_locker);

	return 0;
}


