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
 * File: thread-exit-test.c
 *
 * --------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>

#include <kernel/Kernel.h>
#include <kernel/Debug.h>


static void first(void *data)
{
	ETK_OUTPUT("first on_exit_thread()\n");
}

static void first_on_exit(int val, void *arg)
{
	ETK_OUTPUT("first on_exit(): delete thread that create before\n");
	delete_thread(arg);
}

static void second(void *data)
{
	ETK_OUTPUT("second on_exit_thread()\n");
}

static void second_on_exit(int val, void *arg)
{
	ETK_OUTPUT("second on_exit()\n");
}

static void third(void *data)
{
	ETK_OUTPUT("third on_exit_thread()\n");
}

static int32 thread(void *data)
{
	ETK_OUTPUT("Thread going...\n");
#ifdef HAVE_ON_EXIT
	on_exit(second_on_exit, NULL);
#endif
	on_exit_thread(second, NULL);
	on_exit_thread(third, NULL);

	ETK_OUTPUT("Thread ready to exit...\n");

	return 0;
}

int main(int argc, char **argv)
{
	status_t status;
	void* thr;
	void* curThr;

	curThr = create_thread_by_current_thread();
#ifdef HAVE_ON_EXIT
	on_exit(first_on_exit, curThr);
#endif

	if (on_exit_thread(first, NULL) != B_OK)
		ETK_OUTPUT("%s(Line: %d):on_exit_thread failed.\n", __FILE__, __LINE__);

	thr = create_thread(thread, B_NORMAL_PRIORITY, NULL, NULL);
	resume_thread(thr);

	wait_for_thread(thr, &status);
	delete_thread(thr);

#ifndef HAVE_ON_EXIT
	ETK_OUTPUT("Deleting curThr...\n");
	delete_thread(curThr);
#endif

	snooze(100000);

	ETK_OUTPUT("Going to exit...\n");

	return 0;
}

