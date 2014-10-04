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
 * File: Application.cpp
 *
 * --------------------------------------------------------------------------*/

#define __ETK_APPLICATION_COMPILING__

#include <support/Locker.h>
#include <support/Autolock.h>
#include <kernel/Kernel.h>
#include <interface/Font.h>
#include <add-ons/graphics/GraphicsEngine.h>
#include <storage/FindDirectory.h>
#include <storage/Directory.h>

#include "Application.h"
#include "Clipboard.h"

_LOCAL BCursor _B_CURSOR_SYSTEM_DEFAULT(NULL);

BApplication *app = NULL;
BMessenger app_messenger;
BClipboard clipboard("system");
const BCursor *B_CURSOR_SYSTEM_DEFAULT = &_B_CURSOR_SYSTEM_DEFAULT;

BList BApplication::sRunnerList;
bigtime_t BApplication::sRunnerMinimumInterval = B_INT64_CONSTANT(0);

extern BLocker* get_handler_operator_locker();
extern bool font_init(void);
extern void font_cancel(void);
extern bool font_lock(void);
extern void font_unlock(void);

void
BApplication::Init(const char *signature, bool tryInterface)
{
	BLocker *hLocker = get_handler_operator_locker();

	hLocker->Lock();

	if (app != NULL)
		ETK_ERROR("[APP]: %s --- Another application running!", __PRETTY_FUNCTION__);

	if (signature) fSignature = EStrdup(signature);

	BMessenger msgr(this);
	BMessage pulseMsg(B_PULSE);
	fPulseRunner = new BMessageRunner(msgr, &pulseMsg, fPulseRate, 0);
	if (!(fPulseRunner == NULL || fPulseRunner->IsValid())) {
		delete fPulseRunner;
		fPulseRunner = NULL;
	}

	app = this;
	app_messenger = BMessenger(this);

	hLocker->Unlock();

	clipboard.StartWatching(app_messenger);

	if (tryInterface) InitGraphicsEngine();
}


BApplication::BApplication(const char *signature, bool tryInterface)
		: BLooper(signature), fQuit(false), fSignature(NULL),
		fPulseRate(B_INT64_CONSTANT(500000)), fPulseRunner(NULL),
		fGraphicsEngine(NULL), fGraphicsEngineAddon(NULL),
		fCursor(NULL), fCursorHidden(false), fCursorObscure(false)
{
	Init(signature, tryInterface);
}


BApplication::~BApplication()
{
	BLocker *hLocker = get_handler_operator_locker();

	hLocker->Lock();
	if (!(fThread == NULL || (get_current_thread_id() == get_thread_id(fThread) && fQuit == true)))
		ETK_ERROR("[APP]: Task must call \"PostMessage(B_QUIT_REQUESTED)\" instead \"delete\" to quit the application!!!");
	hLocker->Unlock();

	quit_all_loopers(true);

	if (fGraphicsEngine != NULL) {
		font_lock();
		fGraphicsEngine->DestroyFonts();
		font_unlock();
	}

	font_cancel();

	if (fGraphicsEngine != NULL) {
		fGraphicsEngine->Cancel();
		delete fGraphicsEngine;
	}

	if (fGraphicsEngineAddon != NULL) unload_addon(fGraphicsEngineAddon);

	if (fPulseRunner) delete fPulseRunner;
	if (fSignature) delete[] fSignature;

	hLocker->Lock();
	for (int32 i = 0; i < fModalWindows.CountItems(); i++) {
		BMessenger *tMsgr = (BMessenger*)fModalWindows.ItemAt(i);
		delete tMsgr;
	}
	fModalWindows.MakeEmpty();
	hLocker->Unlock();
}


BApplication::BApplication(const BMessage *from)
		: BLooper(NULL, B_DISPLAY_PRIORITY), fQuit(false), fSignature(NULL),
		fPulseRate(B_INT64_CONSTANT(500000)), fPulseRunner(NULL),
		fGraphicsEngine(NULL), fGraphicsEngineAddon(NULL),
		fCursor(NULL), fCursorHidden(false), fCursorObscure(false)
{
	// TODO
	Init(NULL, !(from == NULL || from->HasBool("etk:has_gui") == false));
}


status_t
BApplication::Archive(BMessage *into, bool deep) const
{
	if (!into) return B_ERROR;

	BLooper::Archive(into, deep);
	into->AddString("class", "BApplication");

	// TODO

	return B_OK;
}


BArchivable*
BApplication::Instantiate(const BMessage *from)
{
	if (e_validate_instantiation(from, "BApplication"))
		return new BApplication(from);
	return NULL;
}


void*
BApplication::Run()
{
	BLocker *hLocker = get_handler_operator_locker();

	hLocker->Lock();

	if (fThread)
		ETK_ERROR("[APP]: %s --- Thread must run only one time!", __PRETTY_FUNCTION__);

	if ((fThread = open_thread(get_current_thread_id())) == NULL)
		fThread = create_thread_by_current_thread();

	if (fThread == NULL)
		ETK_ERROR("[APP]: %s --- Unable to create thread!", __PRETTY_FUNCTION__);

	hLocker->Unlock();

	Lock();

	ReadyToRun();

	BMessage *aMsg = NULL;
	while (!fQuit) {
		if ((aMsg = NextLooperMessage(B_INFINITE_TIMEOUT)) != NULL) DispatchLooperMessage(aMsg);
		if (!fQuit) {
			MessageQueue()->Lock();
			aMsg = MessageQueue()->FindMessage((int32)0);
			if (!(aMsg == NULL || aMsg->what != _QUIT_)) fQuit = true;
			MessageQueue()->Unlock();
		}
	}

	fQuit = true;

	Unlock();

	return NULL;
}


void
BApplication::dispatch_message_runners()
{
	BLocker *hLocker = get_handler_operator_locker();

	hLocker->Lock();

	if (sRunnerMinimumInterval == B_INT64_CONSTANT(0)) {
		hLocker->Unlock();
		return;
	}

	sRunnerMinimumInterval = B_INT64_CONSTANT(0);
	bigtime_t curTime = real_time_clock_usecs();
	for (int32 i = 0; i < sRunnerList.CountItems(); i++) {
		BMessageRunner *runner = (BMessageRunner*)sRunnerList.ItemAt(i);
		if (runner == NULL || runner->IsValid() == false || runner->fCount == 0 || runner->fInterval <= B_INT64_CONSTANT(0) ||
		        runner->fTarget == NULL || runner->fTarget->IsValid() == false || runner->fMessage == NULL) continue;

		if (runner->fPrevSendTime <B_INT64_CONSTANT(0) || curTime - runner->fPrevSendTime >= runner->fInterval) {
			// TODO: replyTo
			runner->fPrevSendTime = curTime;
			bool send = (runner->fTarget->SendMessage(runner->fMessage, (BHandler*)NULL, B_INT64_CONSTANT(50000)) == B_OK);

			if (sRunnerList.ItemAt(i) != (void*)runner || sRunnerMinimumInterval <B_INT64_CONSTANT(0)) {
				i = 0;
				continue;
			}
			if (send && runner->fCount > 0) runner->fCount -= 1;
			if (runner->IsValid() == false || runner->fCount == 0 || runner->fInterval <= B_INT64_CONSTANT(0) ||
			        runner->fTarget == NULL || runner->fTarget->IsValid() == false || runner->fMessage == NULL) continue;
			if (sRunnerMinimumInterval == B_INT64_CONSTANT(0) ||
			        runner->fInterval < sRunnerMinimumInterval) sRunnerMinimumInterval = runner->fInterval;
		} else if (sRunnerMinimumInterval == B_INT64_CONSTANT(0) ||
		           runner->fInterval - (curTime - runner->fPrevSendTime) <  sRunnerMinimumInterval) {
			sRunnerMinimumInterval = runner->fInterval - (curTime - runner->fPrevSendTime);
		}
	}

	hLocker->Unlock();
}


bool
BApplication::QuitRequested()
{
	return(quit_all_loopers(false));
}


void
BApplication::DispatchMessage(BMessage *msg, BHandler *target)
{
	if (fQuit) return;

	if (target == NULL) target = fPreferredHandler;
	if (!target || target->Looper() != this) return;

	if (msg->what == B_QUIT_REQUESTED && target == this) {
		if (QuitRequested()) Quit();
	} else if (msg->what == B_APP_CURSOR_REQUESTED && target == this) {
		const void *cursor_data = NULL;
		ssize_t len;
		bool show_cursor;

		if (msg->FindData("etk:cursor_data", B_ANY_TYPE, &cursor_data, &len)) {
			if (len > 0) {
				BCursor newCursor(cursor_data);
				SetCursor(&newCursor);
			}
		} else if (msg->FindBool("etk:show_cursor", &show_cursor)) {
			if (show_cursor) ShowCursor();
			else HideCursor();
		} else if (msg->HasBool("etk:obscure_cursor")) {
			ObscureCursor();
		}
	} else {
		BLooper::DispatchMessage(msg, target);
	}
}


void
BApplication::Quit()
{
	if (!IsLockedByCurrentThread())
		ETK_ERROR("[APP]: %s --- Application must LOCKED before this call!", __PRETTY_FUNCTION__);
	else if (fThread == NULL)
		ETK_ERROR("[APP]: %s --- Application isn't running!", __PRETTY_FUNCTION__);
	else if (get_thread_id(fThread) != get_current_thread_id())
		ETK_ERROR("\n\
		          **************************************************************************\n\
		          *                           [APP]: BApplication                          *\n\
		          *                                                                        *\n\
		          *      Task must call \"PostMessage(B_QUIT_REQUESTED)\" instead of         *\n\
		          *      \"Quit()\" outside the looper!!!                                    *\n\
		          **************************************************************************\n\n");

	close_sem(fSem);

	fQuit = true;

	ETK_DEBUG("[APP]: Application Quit.");
}


void
BApplication::ReadyToRun()
{
}


void
BApplication::Pulse()
{
}


void
BApplication::SetPulseRate(bigtime_t rate)
{
	if (fPulseRunner == NULL) {
		ETK_DEBUG("[APP]: %s --- No message runner.", __PRETTY_FUNCTION__);
		return;
	}

	if (fPulseRunner->SetInterval(rate) == B_OK) {
		fPulseRate = rate;
		fPulseRunner->SetCount(rate >B_INT64_CONSTANT(0) ? -1 : 0);
	} else {
		ETK_DEBUG("[APP]: %s --- Unable to set pulse rate.", __PRETTY_FUNCTION__);
	}
}


bigtime_t
BApplication::PulseRate() const
{
	if (fPulseRunner == NULL) return B_INT64_CONSTANT(-1);
	return fPulseRate;
}


void
BApplication::MessageReceived(BMessage *msg)
{
	switch (msg->what) {
		case B_PULSE:
			Pulse();
			break;

		case B_MOUSE_DOWN:
		case B_MOUSE_UP:
		case B_MOUSE_MOVED:
		case B_MOUSE_WHEEL_CHANGED:
		case B_KEY_DOWN:
		case B_KEY_UP:
		case B_MODIFIERS_CHANGED: {
			if (msg->what == B_MOUSE_MOVED && !fCursorHidden && fCursorObscure) ShowCursor();

			BMessenger msgr;
			if (msg->FindMessenger("etk:msg_for_target", &msgr)) {
				BLocker *hLocker = get_handler_operator_locker();
				hLocker->Lock();
				BMessenger *tMsgr = (BMessenger*)fModalWindows.ItemAt(0);
				if (tMsgr != NULL) msgr = *tMsgr;
				hLocker->Unlock();

				if (msgr.IsValid() == false) {
					ETK_DEBUG("[APP]: %s --- Invalid messenger.", __PRETTY_FUNCTION__);
					break;
				}

				msg->RemoveMessenger("etk:msg_for_target");
				if (tMsgr != NULL) msg->RemovePoint("where");
				msgr.SendMessage(msg);
			} else {
				BLooper::MessageReceived(msg);
			}
		}
		break;

		default:
			BLooper::MessageReceived(msg);
	}
}


#if 0
int32
BApplication::CountLoopers()
{
	BLocker *hLocker = get_handler_operator_locker();
	BAutolock <BLocker>autolock(hLocker);

	return BLooper::sLooperList.CountItems();
}


BLooper*
BApplication::LooperAt(int32 index)
{
	BLocker *hLocker = get_handler_operator_locker();
	BAutolock <BLocker>autolock(hLocker);

	return (BLooper*)(BLooper::sLooperList.ItemAt(index));
}
#endif


bool
BApplication::quit_all_loopers(bool force)
{
	int32 index;
	BLooper *looper = NULL;
	BLocker *hLocker = get_handler_operator_locker();

	while (true) {
		hLocker->Lock();

		if (app != this) {
			hLocker->Unlock();
			return false;
		}

		looper = NULL;
		for (index = 0; index < BLooper::sLooperList.CountItems(); index++) {
			looper = (BLooper*)(BLooper::sLooperList.ItemAt(index));
			if (looper == app) looper = NULL; // ignore app
			if (looper != NULL) break;
		}

		if (looper == NULL) {
			hLocker->Unlock();
			break;
		}

		if (looper->IsDependsOnOthersWhenQuitRequested()) {
			BLooper::sLooperList.MoveItem(index, BLooper::sLooperList.CountItems() - 1);
			hLocker->Unlock();
			ETK_DEBUG("[APP]: %s --- Looper depends on others, retry again...", __PRETTY_FUNCTION__);
			continue;
		}

		if (looper->Lock() == false) {
			BLooper::sLooperList.MoveItem(index, BLooper::sLooperList.CountItems() - 1);
			hLocker->Unlock();
			ETK_DEBUG("[APP]: %s --- Lock looper failed, retry again...", __PRETTY_FUNCTION__);
			e_snooze(5000);
			continue;
		}

		if (!force) {
			if (looper->QuitRequested() == false) {
				looper->Unlock();
				hLocker->Unlock();
				return false;
			}
		}

		BLooper::sLooperList.RemoveItem(looper);

		hLocker->Unlock();

		looper->Quit();
	}

	return true;
}


const char*
BApplication::Signature() const
{
	return fSignature;
}


#ifndef ETK_GRAPHICS_NONE_BUILT_IN
extern BGraphicsEngine* get_built_in_graphics_engine();
#endif

void
BApplication::InitGraphicsEngine()
{
	bool hasEngine = false;

	do {
		BAutolock <BLooper>autolock(this);

		if (fGraphicsEngine != NULL) ETK_ERROR("[APP]: %s --- This function must run only one time!", __PRETTY_FUNCTION__);

		BPath aPath;

		for (int8 i = 0; i < 3; i++) {
			if (i < 2) {
				if (e_find_directory(i == 0 ?B_USER_ADDONS_DIRECTORY :B_ADDONS_DIRECTORY, &aPath) != B_OK) {
					ETK_DEBUG("[APP]: Unable to find %s.", i == 0 ? "B_USER_ADDONS_DIRECTORY" : "B_ADDONS_DIRECTORY");
					continue;
				}

				aPath.Append("etkxx/graphics");
				BDirectory directory(aPath.Path());
				if (directory.InitCheck() != B_OK) {
					ETK_DEBUG("[APP]: Unable to read directory(%s).", aPath.Path());
					continue;
				}

				BEntry aEntry;
				while (directory.GetNextEntry(&aEntry, false) == B_OK) {
					if (aEntry.GetPath(&aPath) != B_OK) continue;
					void *addon = load_addon(aPath.Path());
					if (addon == NULL) {
						ETK_WARNING("[APP]: Unable to load addon(%s).", aPath.Path());
						continue;
					}

					BGraphicsEngine* (*instantiate_func)() = NULL;
					if (get_image_symbol(addon, "instantiate_graphics_engine", (void**)&instantiate_func) != B_OK) {
						ETK_WARNING("[APP]: Unable to get symbol of image(%s).", aPath.Path());
						unload_addon(addon);
						continue;
					}

					BGraphicsEngine *engine = (*instantiate_func)();
					if (engine == NULL || engine->Initalize() != B_OK) {
						ETK_DEBUG("[APP]: Unable to initalize engine(%s).", aPath.Path());
						unload_addon(addon);
						continue;
					}

					fGraphicsEngine = engine;
					fGraphicsEngineAddon = addon;
					break;
				}

				if (fGraphicsEngine != NULL) break;
			} else {
#ifndef ETK_GRAPHICS_NONE_BUILT_IN
				BGraphicsEngine *engine = get_built_in_graphics_engine();
				if (engine == NULL || engine->Initalize() != B_OK) {
					ETK_WARNING("[APP]: Unable to initalize built-in engine.");
					break;
				}

				fGraphicsEngine = engine;
				fGraphicsEngineAddon = NULL;
#else
				break;
#endif
			}
		}

		if (fGraphicsEngine != NULL) {
			hasEngine = true;
			if (fGraphicsEngine->GetDefaultCursor(&_B_CURSOR_SYSTEM_DEFAULT) != B_OK)
				_B_CURSOR_SYSTEM_DEFAULT = *B_CURSOR_HAND;
			fCursor = _B_CURSOR_SYSTEM_DEFAULT;
			fGraphicsEngine->SetCursor(fCursorHidden ? NULL : fCursor.Data());
		}
	} while (false);

	if (hasEngine) {
		font_lock();
		fGraphicsEngine->InitalizeFonts();
		font_unlock();
		font_init();
	} else {
		ETK_WARNING("[APP]: No graphics engine found.");
	}
}


bool
BApplication::AddModalWindow(BMessenger &msgr)
{
	BMessenger *aMsgr = new BMessenger(msgr);
	if (aMsgr == NULL || aMsgr->IsValid() == false) {
		if (aMsgr) delete aMsgr;
		return false;
	}

	BLocker *hLocker = get_handler_operator_locker();
	hLocker->Lock();
	for (int32 i = 0; i < fModalWindows.CountItems(); i++) {
		BMessenger *tMsgr = (BMessenger*)fModalWindows.ItemAt(i);
		if (*tMsgr == *aMsgr) {
			hLocker->Unlock();
			delete aMsgr;
			return false;
		}
	}
	if (fModalWindows.AddItem(aMsgr, 0) == false) {
		hLocker->Unlock();
		delete aMsgr;
		return false;
	}
	hLocker->Unlock();

	return true;
}


bool
BApplication::RemoveModalWindow(BMessenger &msgr)
{
	BLocker *hLocker = get_handler_operator_locker();
	hLocker->Lock();
	for (int32 i = 0; i < fModalWindows.CountItems(); i++) {
		BMessenger *tMsgr = (BMessenger*)fModalWindows.ItemAt(i);
		if (*tMsgr == msgr) {
			if (fModalWindows.RemoveItem(tMsgr) == false) break;
			hLocker->Unlock();
			delete tMsgr;
			return true;
		}
	}
	hLocker->Unlock();

	return false;
}


void
BApplication::SetCursor(const BCursor *cursor, bool sync)
{
	if (cursor == NULL || cursor->DataLength() == 0) return;

	if (get_current_thread_id() != Thread()) {
		BMessage msg(B_APP_CURSOR_REQUESTED);
		msg.AddData("etk:cursor_data", B_ANY_TYPE, cursor->Data(), (size_t)cursor->DataLength(), true);
		if (sync == false) app_messenger.SendMessage(&msg);
		else app_messenger.SendMessage(&msg, &msg);
	} else if (fCursor != *cursor) {
		fCursor = *cursor;
		if (fGraphicsEngine && !fCursorHidden) fGraphicsEngine->SetCursor(fCursor.Data());
	}
}


void
BApplication::HideCursor()
{
	if (get_current_thread_id() != Thread()) {
		BMessage msg(B_APP_CURSOR_REQUESTED);
		msg.AddBool("etk:show_cursor", false);
		app_messenger.SendMessage(&msg);
	} else if (!fCursorHidden) {
		fCursorHidden = true;
		fCursorObscure = false;
		if (fGraphicsEngine) fGraphicsEngine->SetCursor(NULL);
	}
}


void
BApplication::ShowCursor()
{
	if (get_current_thread_id() != Thread()) {
		BMessage msg(B_APP_CURSOR_REQUESTED);
		msg.AddBool("etk:show_cursor", true);
		app_messenger.SendMessage(&msg);
	} else if (fCursorHidden || fCursorObscure) {
		fCursorHidden = false;
		fCursorObscure = false;
		if (fGraphicsEngine) fGraphicsEngine->SetCursor(fCursor.Data());
	}
}


void
BApplication::ObscureCursor()
{
	if (get_current_thread_id() != Thread()) {
		BMessage msg(B_APP_CURSOR_REQUESTED);
		msg.AddBool("etk:obscure_cursor", true);
		app_messenger.SendMessage(&msg);
	} else if (!fCursorHidden && !fCursorObscure) {
		fCursorObscure = true;
		if (fGraphicsEngine) fGraphicsEngine->SetCursor(NULL);
	}
}


bool
BApplication::IsCursorHidden() const
{
	return fCursorHidden;
}

