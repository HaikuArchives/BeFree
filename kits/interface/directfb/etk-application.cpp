/* --------------------------------------------------------------------------
 *
 * DirectFB Graphics Add-on for ETK++
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
 * File: etk-application.cpp
 *
 * --------------------------------------------------------------------------*/

#include <string.h>

#include "etk-dfb.h"

#include <kernel/Kernel.h>
#include <support/Autolock.h>
#include <support/ClassInfo.h>
#include <support/StringArray.h>
#include <app/Clipboard.h>
#include <app/Application.h>

extern bool get_prog_argc_argv_linux(BString &progName, BStringArray &progArgv);


static void dfb_clipboard_changed(BDFBGraphicsEngine *dfbEngine)
{
	BString aStr;

	char *mimetype = NULL;
	void *data = NULL;
	unsigned int dataLen = 0;

	dfbEngine->Lock();
	dfbEngine->dfbDisplay->GetClipboardTimeStamp(dfbEngine->dfbDisplay, &dfbEngine->dfbClipboardTimeStamp);
	dfbEngine->dfbDisplay->GetClipboardData(dfbEngine->dfbDisplay, &mimetype, &data, &dataLen);
	dfbEngine->Unlock();

	if (mimetype == NULL || strcmp(mimetype, "text/plain") != 0 || data == NULL || dataLen == 0) {
		if (mimetype) free(mimetype);
		if (data) free(data);
		return;
	}

	aStr.Append((char*)data, (int32)dataLen);
	free(mimetype);
	free(data);

	BMessage *clipMsg = NULL;
	if (clipboard.Lock()) {
		if ((clipMsg = clipboard.Data()) != NULL) {
			const char *text = NULL;
			ssize_t textLen = 0;
			if (clipMsg->FindData("text/plain", B_MIME_TYPE, (const void**)&text, &textLen) == false ||
			        text == NULL || textLen != (ssize_t)aStr.Length() || aStr.Compare(text, (int32)textLen) != 0) {
				clipboard.Clear();
				clipMsg->AddBool("etk:msg_from_gui", true);
				clipMsg->AddData("text/plain", B_MIME_TYPE, aStr.String(), aStr.Length());
				clipboard.Commit();
			}
		}
		clipboard.Unlock();
	}
}


class _LOCAL EDFBClipboardMessageFilter : public BMessageFilter
{
	public:
		BDFBGraphicsEngine *fEngine;

		EDFBClipboardMessageFilter(BDFBGraphicsEngine *dfbEngine)
				: BMessageFilter(B_CLIPBOARD_CHANGED, NULL) {
			fEngine = dfbEngine;
		}

		virtual e_filter_result Filter(BMessage *message, BHandler **target) {
			if (fEngine == NULL || message->what != B_CLIPBOARD_CHANGED) return B_DISPATCH_MESSAGE;

			do {
				const char *text = NULL;
				ssize_t textLen = 0;
				BString aStr;

				BMessage *msg;

				clipboard.Lock();
				if (!((msg = clipboard.Data()) == NULL || msg->HasBool("etk:msg_from_gui"))) {
					msg->FindData("text/plain", B_MIME_TYPE, (const void**)&text, &textLen);
					if (textLen > 0) aStr.Append(text, textLen);
				}
				clipboard.Unlock();

				if (aStr.Length() <= 0) break;

				fEngine->Lock();
				fEngine->dfbDisplay->SetClipboardData(fEngine->dfbDisplay, "text/plain",
				                                      (void*)aStr.String(), aStr.Length(),
				                                      &fEngine->dfbClipboardTimeStamp);
				fEngine->Unlock();
			} while (false);

			return B_DISPATCH_MESSAGE;
		}
};


BGraphicsEngine* get_built_in_graphics_engine()
{
	return(new BDFBGraphicsEngine());
}


BDFBGraphicsEngine::BDFBGraphicsEngine()
		: BGraphicsEngine(),
		dfbDisplay(NULL), dfbDisplayLayer(NULL), dfbEventBuffer(NULL),
		dfbDisplayWidth(0), dfbDisplayHeight(0), dfbCursor(NULL),
		dfbDoQuit(false), fDFBThread(NULL), fClipboardFilter(NULL)
{
}


BDFBGraphicsEngine::~BDFBGraphicsEngine()
{
}


bool
BDFBGraphicsEngine::Lock()
{
	return fLocker.Lock();
}


void
BDFBGraphicsEngine::Unlock()
{
	fLocker.Unlock();
}


status_t
BDFBGraphicsEngine::InitCheck()
{
	BAutolock <BDFBGraphicsEngine> autolock(this);
	if (autolock.IsLocked() == false || dfbDisplay == NULL || dfbDoQuit) return B_NO_INIT;
	return B_OK;
}


static void process_dfb_event(BDFBGraphicsEngine *dfbEngine, DFBEvent *evt)
{
	if (dfbEngine == NULL || evt == NULL) return;

	bigtime_t currentTime = e_real_time_clock_usecs();
//	ETK_DEBUG("[GRAPHICS]: %s --- Something DFB Event received.", __PRETTY_FUNCTION__);

	BMessenger etkWinMsgr;
	BMessage message;

	message.AddBool("etk:msg_from_gui", true);
	message.AddInt64("when", currentTime);

	if (evt->clazz == DFEC_WINDOW) {
		dfbEngine->Lock();
		DFBWindowEvent *event = &(evt->window);
		BDFBGraphicsWindow *win = (BDFBGraphicsWindow*)dfbEngine->GetDFBWindowData(event->window_id);

		if (win == NULL || win->GetContactor(&etkWinMsgr) != B_OK || etkWinMsgr.IsValid() == false) {
//			ETK_DEBUG("[GRAPHICS]: %s --- Can't find window for the event (win:%p).", __PRETTY_FUNCTION__, win);
			dfbEngine->Unlock();
			return;
		}

		IDirectFBWindow *dfbWindow = win->dfbWindow;

		switch (event->type) {
			case DWET_SIZE: {
				int originX, originY;
				BRect margins = win->fMargins;
				dfbWindow->GetPosition(dfbWindow, &originX, &originY);
				if (win->fWidth != (uint32)event->w || win->fHeight != (uint32)event->h) {
					win->fWidth = event->w;
					win->fHeight = event->h;
					dfbWindow->SetOpaqueRegion(dfbWindow, (int)margins.left, (int)margins.top,
					                           (int)event->w - 1 - (int)margins.right,
					                           (int)event->h - 1 - (int)margins.bottom);
					rgb_color c = win->BackgroundColor();
					win->dfbSurface->Clear(win->dfbSurface, c.red, c.green, c.blue, 255);
				}
				if (win->fHidden == false && (win->fOriginX != originX || win->fOriginY != originY)) {
					win->fOriginX = originX;
					win->fOriginY = originY;
				}
				dfbEngine->Unlock();

				message.AddPoint("where", BPoint((float)originX + margins.left, (float)originY + margins.top));
				message.AddFloat("width", (float)(event->w - 1) - margins.left - margins.right);
				message.AddFloat("height", (float)(event->h - 1) - margins.top - margins.bottom);

				message.what = B_WINDOW_RESIZED;
				etkWinMsgr.SendMessage(&message);
				message.what = B_WINDOW_MOVED;
				etkWinMsgr.SendMessage(&message);

				dfbEngine->Lock();
			}
			break;

			case DWET_POSITION: {
				BRect margins = win->fMargins;
				if (win->fHidden == false && (win->fOriginX != event->x || win->fOriginY != event->y)) {
					win->fOriginX = event->x;
					win->fOriginY = event->y;
				}
				dfbEngine->Unlock();

				message.AddPoint("where", BPoint((float)event->x + margins.left, (float)event->y + margins.top));

				message.what = B_WINDOW_MOVED;
				etkWinMsgr.SendMessage(&message);

				dfbEngine->Lock();
			}
			break;

			case DWET_POSITION_SIZE: {
				BRect margins = win->fMargins;
				if (win->fWidth != (uint32)event->w || win->fHeight != (uint32)event->h) {
					win->fWidth = event->w;
					win->fHeight = event->h;
					dfbWindow->SetOpaqueRegion(dfbWindow, (int)margins.left, (int)margins.top,
					                           (int)event->w - 1 - (int)margins.right,
					                           (int)event->h - 1 - (int)margins.bottom);
					rgb_color c = win->BackgroundColor();
					win->dfbSurface->Clear(win->dfbSurface, c.red, c.green, c.blue, 255);
				}
				if (win->fHidden == false && (win->fOriginX != event->x || win->fOriginY != event->y)) {
					win->fOriginX = event->x;
					win->fOriginY = event->y;
				}
				dfbEngine->Unlock();

				message.AddPoint("where", BPoint((float)event->x + margins.left, (float)event->y + margins.top));
				message.AddFloat("width", (float)(event->w - 1) - margins.left - margins.right);
				message.AddFloat("height", (float)(event->h - 1) - margins.top - margins.bottom);

				message.what = B_WINDOW_RESIZED;
				etkWinMsgr.SendMessage(&message);
				message.what = B_WINDOW_MOVED;
				etkWinMsgr.SendMessage(&message);

				dfbEngine->Lock();
			}
			break;

			case DWET_CLOSE: {
				if (dfbEngine->dfbCurFocusWin == event->window_id)
					dfbEngine->dfbCurFocusWin = B_MAXUINT;

				if (dfbEngine->dfbCurPointerGrabbed == event->window_id)
					dfbEngine->dfbCurPointerGrabbed = B_MAXUINT;

				dfbEngine->Unlock();

				message.what = B_QUIT_REQUESTED;
				etkWinMsgr.SendMessage(&message);

				dfbEngine->Lock();
			}
			break;

			case DWET_GOTFOCUS: {
				if (win->fFlags & B_AVOID_FOCUS) break;

				dfbEngine->dfbCurFocusWin = event->window_id;
				dfbWindow->SetOpacity(dfbWindow, 0xff);
				dfbEngine->Unlock();

				message.what = B_WINDOW_ACTIVATED;
				etkWinMsgr.SendMessage(&message);

				dfbEngine->Lock();
			}
			break;

			case DWET_LOSTFOCUS:
#if 0
				if (dfbEngine->dfbCurFocusWin == event->window_id) {
					dfbEngine->dfbCurFocusWin = B_MAXUINT;
					dfbWindow->SetOpacity(dfbWindow, 0xaf);
					dfbEngine->Unlock();

					message.what = B_WINDOW_ACTIVATED;
					etkWinMsgr.SendMessage(&message);

					dfbEngine->Lock();
				}
#else
				dfbWindow->SetOpacity(dfbWindow, 0xaf);
#endif
				break;

			case DWET_WHEEL: {
				dfbEngine->Unlock();

				// TODO: delta_x
				message.what = B_MOUSE_WHEEL_CHANGED;
				float delta_x = 0;
				float delta_y = 0;
				delta_y = (event->step < 0 ? -1.0f : 1.0f);

				message.AddFloat("etk:wheel_delta_x", delta_x);
				message.AddFloat("etk:wheel_delta_y", delta_y);

				message.AddMessenger("etk:msg_for_target", etkWinMsgr);
				etkWinMsgr = BMessenger(app);
				etkWinMsgr.SendMessage(&message);

				dfbEngine->Lock();
			}
			break;

			case DWET_BUTTONDOWN:
			case DWET_BUTTONUP: {
				int originX, originY;
				BRect margins = win->fMargins;
				dfbWindow->GetPosition(dfbWindow, &originX, &originY);
				dfbEngine->Unlock();

				message.what = (event->type == DWET_BUTTONDOWN ? B_MOUSE_DOWN :B_MOUSE_UP);

				int32 button = 0;
				if (event->button == DIBI_LEFT) button = 1;
				else if (event->button == DIBI_MIDDLE) button = 2;
				else if (event->button == DIBI_RIGHT) button = 3;

				int32 buttons = button;
				DFBInputDeviceButtonMask state = event->buttons;
				if ((state & DIBM_LEFT) && button != 1) buttons += 1;
				if ((state & DIBM_MIDDLE) && button != 2) buttons += 2;
				if ((state & DIBM_RIGHT) && button != 3) buttons += 3;
				message.AddInt32("button", button);
				message.AddInt32("buttons", buttons);

				message.AddPoint("where", BPoint((float)(event->cx - originX) - margins.left,
				                                 (float)(event->cy - originY) - margins.top));
				message.AddPoint("screen_where", BPoint((float)event->cx, (float)event->cy));

				// TODO: modifiers, clicks
				message.AddMessenger("etk:msg_for_target", etkWinMsgr);
				etkWinMsgr = BMessenger(app);
				etkWinMsgr.SendMessage(&message);

				dfbEngine->Lock();
			}
			break;

			case DWET_MOTION: {
				int originX, originY;
				BRect margins = win->fMargins;
				dfbWindow->GetPosition(dfbWindow, &originX, &originY);
				dfbEngine->Unlock();

				message.what = B_MOUSE_MOVED;

				int32 buttons = 0;
				DFBInputDeviceButtonMask state = event->buttons;
				if (state & DIBM_LEFT) buttons += 1;
				if (state & DIBM_MIDDLE) buttons += 2;
				if (state & DIBM_RIGHT) buttons += 3;
				message.AddInt32("buttons", buttons);

				message.AddPoint("where", BPoint((float)(event->cx - originX) - margins.left,
				                                 (float)(event->cy - originY) - margins.top));
				message.AddPoint("screen_where", BPoint((float)event->cx, (float)event->cy));

				message.AddMessenger("etk:msg_for_target", etkWinMsgr);
				etkWinMsgr = BMessenger(app);
				etkWinMsgr.SendMessage(&message);

				dfbEngine->Lock();
			}
			break;

			case DWET_KEYDOWN:
			case DWET_KEYUP: {
				dfbEngine->Unlock();

				message.AddInt32("key", (int32)(event->key_code));

				// TODO: etk:key_repeat, modifiers, states, raw_char
				if (DFB_KEY_TYPE(event->key_symbol) == DIKT_UNICODE) {
					uint16 symbol = (uint16)event->key_symbol;
					if (symbol == DIKS_ENTER) symbol = B_ENTER;
					char *keybuffer = e_unicode_convert_to_utf8((const uint16*)&symbol, 1);
					int32 keynum = (keybuffer ? (int32)strlen(keybuffer) : 0);

					if (keybuffer) {
						for (int32 i = 0; i < keynum; i++) message.AddInt8("byte", (int8)keybuffer[i]);
						message.AddString("bytes", keybuffer);
						free(keybuffer);
					}
				} else {
					char byte[2];
					bzero(byte, 2);

					if (event->key_symbol == DIKS_CURSOR_LEFT) byte[0] = B_LEFT_ARROW;
					else if (event->key_symbol == DIKS_CURSOR_RIGHT) byte[0] = B_RIGHT_ARROW;
					else if (event->key_symbol == DIKS_CURSOR_UP) byte[0] = B_UP_ARROW;
					else if (event->key_symbol == DIKS_CURSOR_DOWN) byte[0] = B_DOWN_ARROW;
					else if (event->key_symbol == DIKS_INSERT) byte[0] = B_INSERT;
					else if (event->key_symbol == DIKS_HOME) byte[0] = B_HOME;
					else if (event->key_symbol == DIKS_END) byte[0] = B_END;
					else if (event->key_symbol == DIKS_PAGE_UP) byte[0] = B_PAGE_UP;
					else if (event->key_symbol == DIKS_PAGE_DOWN) byte[0] = B_PAGE_DOWN;

					if (byte[0] != 0) {
						message.AddInt8("byte", byte[0]);
						message.AddString("bytes", byte);
					}
				}

				if (message.HasString("bytes"))
					message.what = (event->type == DWET_KEYDOWN ? B_KEY_DOWN :B_KEY_UP);
				else
					message.what = (event->type == DWET_KEYDOWN ? B_UNMAPPED_KEY_DOWN :B_UNMAPPED_KEY_UP);

				int32 modifiers = 0;

				if (event->modifiers & DIMM_SHIFT) modifiers |= B_SHIFT_KEY;
				if (event->modifiers & DIMM_CONTROL) modifiers |= B_CONTROL_KEY;
				if (event->modifiers & DIMM_ALT) modifiers |= B_COMMAND_KEY;
				if (event->modifiers & DIMM_SUPER) modifiers |= B_MENU_KEY;
				if (event->modifiers & DIMM_HYPER) modifiers |= B_OPTION_KEY;

				if (event->locks & DILS_SCROLL) modifiers |= B_SCROLL_LOCK;
				if (event->locks & DILS_NUM) modifiers |= B_NUM_LOCK;
				if (event->locks & DILS_CAPS) modifiers |= B_CAPS_LOCK;

				message.AddInt32("modifiers", modifiers);

				message.AddMessenger("etk:msg_for_target", etkWinMsgr);
				etkWinMsgr = BMessenger(app);
				etkWinMsgr.SendMessage(&message);

				dfbEngine->Lock();
			}
			break;

			default:
				break;
		}

		dfbEngine->Unlock();
	} else if (evt->clazz == DFEC_USER && evt->user.type == DUET_WINDOWREDRAWALL) {
		dfbEngine->Lock();
		DFBUserEvent *event = &(evt->user);
		BDFBGraphicsWindow *win = (BDFBGraphicsWindow*)dfbEngine->GetDFBWindowData((DFBWindowID)event->data);

		if (win == NULL || win->GetContactor(&etkWinMsgr) != B_OK || etkWinMsgr.IsValid() == false) {
			dfbEngine->Unlock();
			return;
		}

		IDirectFBWindow *dfbWindow = win->dfbWindow;

		int width, height;
		dfbWindow->GetSize(dfbWindow, &width, &height);
		dfbEngine->Unlock();

		message.what = _UPDATE_;

		message.AddRect("etk:frame", BRect(0, 0, (float)width - 1.f, (float)height - 1.f));
		message.AddBool("etk:expose", true);

		etkWinMsgr.SendMessage(&message);
	}
}


static status_t dfb_task(void *arg)
{
	BDFBGraphicsEngine *dfbEngine = (BDFBGraphicsEngine*)arg;

	ETK_DEBUG("[GRAPHICS]: Enter DirectFB task...");

	dfbEngine->Lock();

	while (!(dfbEngine->dfbDoQuit || dfbEngine->dfbDisplay == NULL)) {
		dfbEngine->Unlock();

		if (dfbEngine->dfbEventBuffer->WaitForEvent(dfbEngine->dfbEventBuffer) != DFB_OK) {
			ETK_WARNING("[GRAPHICS]: %s --- DirectFB operate error!", __PRETTY_FUNCTION__);
			break;
		}

		dfbEngine->Lock();
		if (dfbEngine->dfbDoQuit) break;
		while (dfbEngine->dfbEventBuffer->HasEvent(dfbEngine->dfbEventBuffer) == DFB_OK) {
			/* Read Event */
			DFBEvent evt;
			if (dfbEngine->dfbEventBuffer->GetEvent(dfbEngine->dfbEventBuffer, &evt) != DFB_OK) break;

			dfbEngine->Unlock();
			process_dfb_event(dfbEngine, &evt); // Process DFB Event
			dfbEngine->Lock();
			if (dfbEngine->dfbDoQuit) break;
		}

		struct timeval timestamp;
		bzero(&timestamp, sizeof(struct timeval));
		if (dfbEngine->dfbDisplay->GetClipboardTimeStamp(dfbEngine->dfbDisplay, &timestamp) != DFB_OK) continue;
		if (memcmp((void*)&dfbEngine->dfbClipboardTimeStamp, (void*)&timestamp, sizeof(struct timeval)) != 0) {
			dfbEngine->Unlock();
			dfb_clipboard_changed(dfbEngine);
			dfbEngine->Lock();
		}
	}

	/* Do some clean */

	dfbEngine->dfbEventBuffer->Reset(dfbEngine->dfbEventBuffer);
	dfbEngine->dfbEventBuffer->Release(dfbEngine->dfbEventBuffer);
	dfbEngine->dfbEventBuffer= NULL;

	dfbEngine->dfbDisplayLayer = NULL;

	if (dfbEngine->dfbCursor) dfbEngine->dfbCursor->Release(dfbEngine->dfbCursor);
	dfbEngine->dfbCursor = NULL;

	dfbEngine->dfbDisplay->Release(dfbEngine->dfbDisplay);
	dfbEngine->dfbDisplay = NULL;

	dfbEngine->Unlock();

	ETK_DEBUG("[GRAPHICS]: DirectFB task quited.");

	return B_OK;
}


status_t
BDFBGraphicsEngine::Initalize()
{
	BMessageFilter *clipboardFilter = new EDFBClipboardMessageFilter(this);
	app->Lock();
	app->AddFilter(clipboardFilter);
	app->Unlock();

	Lock();

	if (fDFBThread != NULL) {
		Unlock();

		app->Lock();
		app->RemoveFilter(clipboardFilter);
		app->Unlock();
		delete clipboardFilter;
		return B_ERROR;
	}

	/* Do DFB Initalize */
	int eArgc = 0;
	char **eArgv = NULL;
	bool argvGotFromSystem = false;

	BString progName;
	BStringArray progArgv;

	DFBDisplayLayerConfig layer_config;

	argvGotFromSystem = (get_prog_argc_argv_linux(progName, progArgv) ? progArgv.CountItems() > 0 : false);

	if (!argvGotFromSystem || progArgv.CountItems() <= 1) {
		if (progName.Length() <= 0) progName.SetTo("dfb_app");
		if (progArgv.CountItems() <= 0) progArgv.AddItem(progName);

		char *options = getenv("DFBARGS");
		if (options == NULL) {
			if (getenv("DISPLAY") != NULL) // here have X11 and we want SDL only because the X11 module need "depth="
				options = "--dfb:system=SDL,mode=800x600,no-sighandler";
			else
				options = "--dfb:system=FBDev,mode=1024x768,vt-switching,no-sighandler,hardware";
		}
		progArgv.AddItem(options);
	}

	if (progName.Length() > 0 && progArgv.CountItems() > 0) {
		if (progArgv.FindString("--dfb-help") >= 0) {
			ETK_OUTPUT("%s\n", DirectFBUsageString());

			Unlock();

			app->Lock();
			app->RemoveFilter(clipboardFilter);
			app->Unlock();
			delete clipboardFilter;
			return B_ERROR;
		}

		eArgc = (int)progArgv.CountItems();
		eArgv = new char*[progArgv.CountItems() + 1];
		for (int32 i = 0; i < progArgv.CountItems(); i++)
			eArgv[i] = (char*)progArgv.ItemAt(i)->String();
		eArgv[progArgv.CountItems()] = NULL;
	}

	if (DirectFBInit(&eArgc, (char***)&eArgv) != DFB_OK || DirectFBCreate(&dfbDisplay) != DFB_OK) {
		if (eArgv) delete[] eArgv;
		ETK_WARNING("[GRAPHICS]: %s --- Initalize DirectFB (DirectFBCreate) failed!", __PRETTY_FUNCTION__);

		Unlock();

		app->Lock();
		app->RemoveFilter(clipboardFilter);
		app->Unlock();
		delete clipboardFilter;
		return B_ERROR;
	}
	if (eArgv) delete[] eArgv;

	if (dfbDisplay->GetDisplayLayer(dfbDisplay, DLID_PRIMARY, &dfbDisplayLayer) != DFB_OK) {
		dfbDisplay->Release(dfbDisplay);
		dfbDisplay = NULL;

		ETK_WARNING("[GRAPHICS]: %s --- Initalize DirectFB (GetDisplayLayer) failed!", __PRETTY_FUNCTION__);

		Unlock();

		app->Lock();
		app->RemoveFilter(clipboardFilter);
		app->Unlock();
		delete clipboardFilter;
		return B_ERROR;
	}

	dfbDisplayLayer->GetConfiguration(dfbDisplayLayer, &layer_config);
	dfbDisplayLayer->SetCooperativeLevel(dfbDisplayLayer, DLSCL_ADMINISTRATIVE);
	dfbDisplayLayer->EnableCursor(dfbDisplayLayer, 1);

	// Set default desktop color
	// TODO: Read app_server configuration
	rgb_color desktopColor = ui_color(B_DESKTOP_COLOR);
	if (dfbDisplayLayer->SetBackgroundColor(dfbDisplayLayer, desktopColor.red,
	                                        desktopColor.green, desktopColor.blue, 255) != DFB_OK) {
		ETK_WARNING("[GRAPHICS]: %s --- Set desktop color failed!", __PRETTY_FUNCTION__);
	}

	dfbDisplayWidth = layer_config.width;
	dfbDisplayHeight = layer_config.height;

	if (dfbDisplay->CreateEventBuffer(dfbDisplay, &dfbEventBuffer) != DFB_OK) {
		dfbDisplayLayer = NULL;

		dfbDisplay->Release(dfbDisplay);
		dfbDisplay = NULL;

		ETK_WARNING("[GRAPHICS]: %s --- Initalize DirectFB (CreateEventBuffer) failed!", __PRETTY_FUNCTION__);

		Unlock();

		app->Lock();
		app->RemoveFilter(clipboardFilter);
		app->Unlock();
		delete clipboardFilter;
		return B_ERROR;
	}

	dfbDoQuit = false;
	dfbCurFocusWin = B_MAXUINT;
	dfbCurPointerGrabbed = B_MAXUINT;
	bzero(&dfbClipboardTimeStamp, sizeof(struct timeval));
	dfbDisplay->GetClipboardTimeStamp(dfbDisplay, &dfbClipboardTimeStamp);

	if ((fDFBThread = create_thread(dfb_task, B_URGENT_DISPLAY_PRIORITY, this, NULL)) == NULL ||
	        resume_thread(fDFBThread) != B_OK) {
		if (fDFBThread) {
			delete_thread(fDFBThread);
			fDFBThread = NULL;
		}

		dfbEventBuffer->Release(dfbEventBuffer);
		dfbEventBuffer= NULL;

		dfbDisplayLayer = NULL;

		dfbDisplay->Release(dfbDisplay);
		dfbDisplay = NULL;

		Unlock();

		app->Lock();
		app->RemoveFilter(clipboardFilter);
		app->Unlock();
		delete clipboardFilter;
		return B_ERROR;
	}

	fClipboardFilter = clipboardFilter;

	Unlock();

	dfb_clipboard_changed(this);

	return B_OK;
}


void
BDFBGraphicsEngine::Cancel()
{
	BMessageFilter *clipboardFilter = NULL;

	Lock();

	if (fDFBThread != NULL) {
		void *dfbThread = open_thread(get_thread_id(fDFBThread));
		if (dfbThread == NULL) {
			Unlock();
			return;
		}

		dfbDoQuit = true;

		/* Send a pending event to EventLoop in order to quit */
		DFBUserEvent evt;
		evt.clazz = DFEC_USER;
		evt.type = DUET_EVENTPENDING;
		evt.data = NULL;

		dfbEventBuffer->PostEvent(dfbEventBuffer, DFB_EVENT(&evt));

		Unlock();

		status_t status;
		wait_for_thread(dfbThread, &status);

		Lock();

		if (fDFBThread != NULL && get_thread_id(fDFBThread) == get_thread_id(dfbThread)) {
			delete_thread(fDFBThread);
			fDFBThread = NULL;

			struct dfb_data *item;
			while ((item = (struct dfb_data*)fDFBDataList.RemoveItem((int32)0)) != NULL) free(item);
		}

		delete_thread(dfbThread);

		clipboardFilter = fClipboardFilter;
		fClipboardFilter = NULL;
	}

	Unlock();

	if (clipboardFilter != NULL) {
		app->Lock();
		app->RemoveFilter(clipboardFilter);
		app->Unlock();
		delete clipboardFilter;
	}
}


BGraphicsContext*
BDFBGraphicsEngine::CreateContext()
{
	return(new BGraphicsContext());
}


BGraphicsDrawable*
BDFBGraphicsEngine::CreatePixmap(uint32 w, uint32 h)
{
	return(new BDFBGraphicsDrawable(this, w, h));
}


BGraphicsWindow*
BDFBGraphicsEngine::CreateWindow(int32 x, int32 y, uint32 w, uint32 h)
{
	return(new BDFBGraphicsWindow(this, x, y, w, h));
}


status_t
BDFBGraphicsEngine::GetDesktopBounds(uint32 *w, uint32 *h)
{
	BAutolock <BDFBGraphicsEngine> autolock(this);
	if (autolock.IsLocked() == false || InitCheck() != B_OK) return B_ERROR;

	if (w) *w = dfbDisplayWidth;
	if (h) *h = dfbDisplayHeight;

	return B_OK;
}


status_t
BDFBGraphicsEngine::GetCurrentWorkspace(uint32 *workspace)
{
	// don't support workspace
	if (workspace != NULL) *workspace = 0;
	return B_ERROR;
}


status_t
BDFBGraphicsEngine::SetCursor(const void *cursor_data)
{
	BAutolock <BDFBGraphicsEngine> autolock(this);
	if (autolock.IsLocked() == false || InitCheck() != B_OK) return B_ERROR;

	if (cursor_data) {
		BCursor cursor(cursor_data);
		if (cursor.ColorDepth() != 1) return B_ERROR;

		DFBSurfaceDescription desc;
		desc.flags = (DFBSurfaceDescriptionFlags)(DSDESC_CAPS | DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT);
		desc.caps = DSCAPS_SYSTEMONLY;
		desc.pixelformat = DSPF_ARGB;
		desc.width = (int)cursor.Width();
		desc.height = (int)cursor.Height();

		IDirectFBSurface *newCursor;
		if (dfbDisplay->CreateSurface(dfbDisplay, &desc, &newCursor) != DFB_OK) return B_ERROR;

		const uint8 *bits = (const uint8*)cursor.Bits();
		const uint8 *mask = (const uint8*)cursor.Mask();

		newCursor->SetDrawingFlags(newCursor, DSDRAW_NOFX);
		newCursor->SetColor(newCursor, 0, 0, 0, 0);
		newCursor->FillRectangle(newCursor, 0, 0, desc.width, desc.height);

		for (uint8 j = 0; j < cursor.Height(); j++)
			for (uint8 i = 0; i < cursor.Width(); i += 8, bits++, mask++)
				for (uint8 k = 0; k < 8 && k + i < cursor.Width(); k++) {
					if (!(*mask & (1 << (7 - k)))) continue;
					if (*bits & (1 << (7 - k))) newCursor->SetColor(newCursor, 0, 0, 0, 255);
					else newCursor->SetColor(newCursor, 255, 255, 255, 255);
					newCursor->FillRectangle(newCursor, k + i, j, 1, 1);
				}

		dfbDisplayLayer->SetCursorShape(dfbDisplayLayer, newCursor, (int)cursor.SpotX(), (int)cursor.SpotY());
		dfbDisplayLayer->SetCursorOpacity(dfbDisplayLayer, 255);
		dfbDisplayLayer->WaitForSync(dfbDisplayLayer);

		if (dfbCursor) dfbCursor->Release(dfbCursor);
		dfbCursor = newCursor;
	} else {
		dfbDisplayLayer->SetCursorOpacity(dfbDisplayLayer, 0);
	}

	return B_OK;
}


status_t
BDFBGraphicsEngine::GetDefaultCursor(BCursor *cursor)
{
	return B_ERROR;
}


bool
BDFBGraphicsEngine::SetDFBWindowData(IDirectFBWindow *dfbWin, void *data, void **old_data)
{
	if (dfbWin == NULL) return false;

	BAutolock <BDFBGraphicsEngine> autolock(this);
	if (autolock.IsLocked() == false || InitCheck() != B_OK) return false;

	bool found = false;
	for (int32 i = 0; i < fDFBDataList.CountItems(); i++) {
		struct dfb_data *item = (struct dfb_data*)fDFBDataList.ItemAt(i);
		if (item->win == dfbWin) {
			if (old_data) *old_data = item->data;

			if (data) {
				item->data = data;
			} else {
				fDFBDataList.RemoveItem(item);
				free(item);
			}

			found = true;
			break;
		}
	}

	if (!found && data) {
		struct dfb_data *item = (struct dfb_data*)malloc(sizeof(struct dfb_data));
		if (!(item == NULL || fDFBDataList.AddItem(item) == false)) {
			item->win = dfbWin;
			item->data = data;
			if (old_data) *old_data = NULL;
			return true;
		}
		if (item) free(item);
	}

	return found;
}


void*
BDFBGraphicsEngine::GetDFBWindowData(IDirectFBWindow *dfbWin)
{
	if (dfbWin == NULL) return NULL;

	BAutolock <BDFBGraphicsEngine> autolock(this);
	if (autolock.IsLocked() == false || InitCheck() != B_OK) return NULL;

	for (int32 i = 0; i < fDFBDataList.CountItems(); i++) {
		struct dfb_data *item = (struct dfb_data*)fDFBDataList.ItemAt(i);
		if (item->win == dfbWin) return item->data;
	}

	return NULL;
}


void*
BDFBGraphicsEngine::GetDFBWindowData(DFBWindowID dfbWinID)
{
	if (dfbWinID == B_MAXUINT) return NULL;

	BAutolock <BDFBGraphicsEngine> autolock(this);
	if (autolock.IsLocked() == false || InitCheck() != B_OK) return NULL;

	for (int32 i = 0; i < fDFBDataList.CountItems(); i++) {
		struct dfb_data *item = (struct dfb_data*)fDFBDataList.ItemAt(i);
		DFBWindowID id;
		if (item->win->GetID(item->win, &id) != DFB_OK) continue;
		if (id == dfbWinID) return item->data;
	}

	return NULL;
}


bool
BDFBGraphicsEngine::ConvertRegion(const BRegion *region, DFBRegion **dfbRegions, int *nRegions)
{
	if (dfbRegions == NULL || nRegions == NULL) return false;

	int32 nrectsNeeded = max_c((region ? region->CountRects() : 0), 1);
	if ((*dfbRegions = (DFBRegion*)malloc(sizeof(DFBRegion) * (size_t)nrectsNeeded)) == NULL) return false;
	*nRegions = 0;

	if (region != NULL) {
		for (int32 i = 0; i < region->CountRects(); i++) {
			BRect r = region->RectAt(i).FloorCopy();

			(*dfbRegions)[*nRegions].x1 = (int)r.left;
			(*dfbRegions)[*nRegions].y1 = (int)r.top;
			(*dfbRegions)[*nRegions].x2 = (int)r.right;
			(*dfbRegions)[*nRegions].y2 = (int)r.bottom;

			*nRegions += 1;
		}
	}

	if (*nRegions == 0) {
		if (region == NULL) {
			(*dfbRegions)->x1 = 0;
			(*dfbRegions)->y1 = 0;
			(*dfbRegions)->x2 = B_MAXINT;
			(*dfbRegions)->y2 = B_MAXINT;
			*nRegions = 1;
		} else {
			free(*dfbRegions);
			*dfbRegions = NULL;
		}
	}

	return(*nRegions != 0);
}

