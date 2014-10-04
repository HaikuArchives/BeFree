/* --------------------------------------------------------------------------
 *
 * ETK++ --- The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2007, Anthony Lee, All Rights Reserved
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
 * File: thread-suspend-test.cpp
 *
 * --------------------------------------------------------------------------*/

#include <Be.h>

#define MSG_SUSPEND_THREAD		'msus'

static void *thread = NULL;
static BButton *button = NULL;

static status_t thread_func(void *data)
{
	static int32 count = 0;

	while (true) {
		ETK_OUTPUT("Counter(%I32i)...\n", ++count);
		e_snooze(500000);

		if (count >= 20) {
			count = 0;
			ETK_OUTPUT("I will suspend myself...\n");

			button->Window()->Lock();
			button->SetLabel("Start counting");
			button->Window()->Unlock();

			if (suspend_thread(thread) != B_OK)
				ETK_ERROR("suspend_thread myself failed !!!");

			button->Window()->Lock();
			button->SetLabel("Pause counter");
			button->Window()->Unlock();
		}
	}
}


static e_filter_result msg_filter_hook(BMessage *message, BHandler **target, BMessageFilter *filter)
{
	switch (get_thread_run_state(thread)) {
		case ETK_THREAD_READY:
		case ETK_THREAD_SUSPENDED:
			if (resume_thread(thread) != B_OK)
				ETK_ERROR("resume_thread failed !!!");
			button->SetLabel("Pause counter");
			break;

		case ETK_THREAD_RUNNING:
			if (suspend_thread(thread) != B_OK)
				ETK_ERROR("suspend_thread failed !!!");
			button->SetLabel("Start counting");
			break;

		default:
			ETK_ERROR("something error !!!");
			break;
	}

	return B_SKIP_MESSAGE;
}


int main(int argc, char **argv)
{
	new BApplication("application/x-vnd.etkxx-suspend_thread_test-app");

	BWindow *win = new BWindow(BRect(100, 100, 400, 170),
	                           "Suspend thread test",
	                           B_TITLED_WINDOW, B_QUIT_ON_WINDOW_CLOSE);
	BButton *btn = new BButton(BRect(50, 20, 250, 50), NULL,
	                           "Start counting",
	                           new BMessage(MSG_SUSPEND_THREAD),
	                           B_FOLLOW_ALL);

	thread = create_thread(thread_func, B_NORMAL_PRIORITY, NULL, NULL);
	button = btn;

	win->Lock();
	win->AddChild(btn);
	win->Show();
	win->AddCommonFilter(new BMessageFilter(MSG_SUSPEND_THREAD, msg_filter_hook));
	win->Unlock();

	app->Run();

	delete app;

	return 0;
}

