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
 * File: etk-os.cpp
 *
 * --------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <signal.h>

#include <kernel/Kernel.h>
#include <support/StringArray.h>
#include <support/Locker.h>
#include <app/Application.h>


#if 0
extern BLocker* get_handler_operator_locker();

class posix_sig_action
{
	public:
		posix_sig_action();
		void (*old_intr)(int);
		void (*old_abrt)(int);
		void (*old_term)(int);
		void (*old_quit)(int);
};
static posix_sig_action _posix_sig_action_;

static void posix_signal(int signumber)
{
	BLocker *hLocker = get_handler_operator_locker();

	hLocker->Lock();
	if (app != NULL) app->PostMessage(signumber == SIGINT ?B_QUIT_REQUESTED : _QUIT_);
	hLocker->Unlock();

	void (*old_func)(int) = NULL;
	switch (signumber) {
		case SIGINT:
			old_func = _posix_sig_action_.old_intr;
			break;
		case SIGABRT:
			old_func = _posix_sig_action_.old_abrt;
			break;
		case SIGTERM:
			old_func = _posix_sig_action_.old_term;
			break;
		case SIGQUIT:
			old_func = _posix_sig_action_.old_quit;
			break;
		default:
			break;
	}

//	ETK_WARNING("[KERNEL]: Signal(%s) done.", (signumber == SIGINT ? "SIGINT" : (
//						   signumber == SIGABRT ? "SIGABRT" : (
//						   signumber == SIGTERM ? "SIGTERM" : "SIGQUIT"))));

	while (signumber != SIGINT && app != NULL) e_snooze(1000);

	if (old_func != NULL) {
//		ETK_WARNING("[KERNEL]: Calling old signal functions...");
		(*old_func)(signumber);
	}

	return;
}


posix_sig_action::posix_sig_action()
		: old_intr(NULL), old_abrt(NULL), old_term(NULL), old_quit(NULL)
{
#ifdef HAVE_SIGACTION
	struct sigaction act, oact;
	act.sa_handler = posix_signal;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
#ifdef SA_RESTART
	act.sa_flags |= SA_RESTART;
#endif // SA_RESTART
	sigaction(SIGINT, &act, &oact);
	if (!(oact.sa_handler == SIG_IGN || oact.sa_handler == SIG_DFL || oact.sa_handler == SIG_ERR)) old_intr = oact.sa_handler;
//	sigaction(SIGABRT, &act, &oact);
//	if(!(oact.sa_handler == SIG_IGN || oact.sa_handler == SIG_DFL || oact.sa_handler == SIG_ERR)) old_abrt = oact.sa_handler;
	sigaction(SIGTERM, &act, &oact);
	if (!(oact.sa_handler == SIG_IGN || oact.sa_handler == SIG_DFL || oact.sa_handler == SIG_ERR)) old_term = oact.sa_handler;
	sigaction(SIGQUIT, &act, &oact);
	if (!(oact.sa_handler == SIG_IGN || oact.sa_handler == SIG_DFL || oact.sa_handler == SIG_ERR)) old_quit = oact.sa_handler;
#else // !HAVE_SIGACTION
#warning "FIXME: signal"
#endif // HAVE_SIGACTION
}
#endif


bool get_prog_argc_argv_linux(BString &progName, BStringArray &progArgv)
{
	bool retVal = false;
	long maxPath = pathconf("/", _PC_PATH_MAX);

	if (maxPath > 0) {
		BString procFileName("/proc/");
		procFileName << (int)getpid() << "/exe";
		char *procFileNameBuffer = (char*)malloc((size_t)(maxPath + 1));
		if (procFileNameBuffer) {
			bzero(procFileNameBuffer, (size_t)(maxPath + 1));
			int length = readlink(procFileName.String(), procFileNameBuffer, (size_t)(maxPath + 1));
			BString str;
			int32 strFound;
			str.Append(procFileNameBuffer, length > 0 ? (int32)length : 0);
			if ((strFound = str.FindLast('/')) >= 0) {
				str.Remove(strFound + 1, -1);
				long maxFilename = pathconf(str.String(), _PC_NAME_MAX);
				if ((long)strFound + 1 + maxFilename > maxPath) {
					maxPath = (long)strFound + 1 + maxFilename;
					char *newBuffer = (char*)realloc(procFileNameBuffer, (size_t)(maxPath + 1));
					if (newBuffer) {
						procFileNameBuffer = newBuffer;
						length = readlink(procFileName.String(), procFileNameBuffer, (size_t)(maxPath + 1));
					}
				}

				if (length > 0) {
					progName.SetTo(procFileNameBuffer, (int32)length);
					progArgv.MakeEmpty();

					procFileName.RemoveLast("/exe");
					procFileName << "/cmdline";
					FILE *fp = fopen(procFileName.String(), "r");
					if (fp) {
						char cmdline = '\0';
						BString sArgv;

						while (true) {
							if (fread(&cmdline, 1, 1, fp) > 0) {
								if (cmdline == '\0') {
									progArgv.AddItem(sArgv);
									sArgv.Remove(0, -1);
								} else {
									sArgv.Append(cmdline, 1);
								}
							} else {
								if (sArgv.Length() > 0) progArgv.AddItem(sArgv);
								break;
							}
						}

						fclose(fp);

						retVal = true;
					}
				}
			}

			free(procFileNameBuffer);
		}
	}

	return retVal;
}

