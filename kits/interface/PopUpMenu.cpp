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
 * File: PopUpMenu.cpp
 *
 * --------------------------------------------------------------------------*/

#include <app/Application.h>
#include <support/ClassInfo.h>
#include <kernel/Kernel.h>

#include "Window.h"
#include "PopUpMenu.h"


class BPopUpMenuView;

class BPopUpMenuWindow : public BWindow
{
	public:
		BPopUpMenuWindow(BPoint where, BPopUpMenu *menu, bool delivers_message, bool open_anyway, bool async, bool could_proxy);

		virtual bool QuitRequested();

		void WaitToClose();

	private:
		friend class BPopUpMenu;
		friend class BPopUpMenuView;
		BPopUpMenu *fMenu;
		bool fAsync;
		bool fOpenAnyway;
		bool fDeliversMessage;
};


class BPopUpMenuView : public BView
{
	public:
		BPopUpMenuView(BRect frame);
		virtual void Draw(BRect updateRect);
};


BPopUpMenuView::BPopUpMenuView(BRect frame)
		: BView(frame, NULL, B_FOLLOW_ALL, B_WILL_DRAW)
{
}


void
BPopUpMenuView::Draw(BRect updateRect)
{
	if (!(Bounds().InsetByCopy(1, 1).Contains(updateRect))) {
		SetDrawingMode(B_OP_COPY);
		SetPenSize(1);
		rgb_color borderColor = ui_color(B_MENU_BORDER_COLOR);

		BPopUpMenuWindow *win = cast_as(Window(), BPopUpMenuWindow);
		if (win->fMenu == NULL || win->fMenu->IsEnabled() == false) borderColor.mix(0, 0, 0, 20);

		SetHighColor(borderColor);
		StrokeRect(Bounds());
	}
}


BPopUpMenuWindow::BPopUpMenuWindow(BPoint where, BPopUpMenu *menu, bool delivers_message, bool open_anyway, bool async, bool could_proxy)
		: BWindow(BRect(0, 0, 1, 1), NULL, B_NO_BORDER_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL, B_AVOID_FOCUS), fMenu(NULL)
{
	Lock();

	fAsync = async;
	fOpenAnyway = open_anyway;
	fDeliversMessage = (async ? true : delivers_message);

	BPopUpMenuView *topView = new BPopUpMenuView(Bounds());
	AddChild(topView);
	if (topView->Window() != this) {
		delete topView;
		return;
	}
	topView->AddChild(menu);
	if (menu->Window() != this) return;

	fMenu = menu;

	uint32 oldResizingMode = fMenu->ResizingMode();

	fMenu->SetResizingMode(B_FOLLOW_NONE);
	fMenu->ResizeToPreferred();
	fMenu->MoveTo(BPoint(2, 2));
	ResizeTo(fMenu->Frame().Width() + 4, fMenu->Frame().Height() + 4);
	MoveTo(where);

	rgb_color bkColor = ui_color(B_MENU_BACKGROUND_COLOR);
	if (fMenu->IsEnabled() == false) bkColor.mix(0, 0, 0, 20);
	SetBackgroundColor(bkColor);

	fMenu->SetResizingMode(oldResizingMode);

	if (async == false && could_proxy) {
		BLooper *looper = BLooper::LooperForThread(get_current_thread_id());
		if (looper) {
			looper->Lock();
			ProxyBy(looper);
			looper->Unlock();
		}
	}

	if (Proxy() == this) Run();
}


bool
BPopUpMenuWindow::QuitRequested()
{
	if (!(fMenu == NULL || fMenu->Window() != this)) {
		BPopUpMenu *menu = fMenu;

		Hide();
		fMenu = NULL;

		menu->RemoveSelf();

		if (fAsync && menu->AsyncAutoDestruct()) delete menu;
	}
	return true;
}


void
BPopUpMenuWindow::WaitToClose()
{
	if (fAsync || Proxy() == this || !IsLockedByCurrentThread())
		ETK_ERROR("[INTERFACE]: %s --- Usage error!!!", __PRETTY_FUNCTION__);

	while (true) {
		BMessage *aMsg = NextLooperMessage(B_INFINITE_TIMEOUT);
		DispatchLooperMessage(aMsg);
		if (aMsg == NULL) break;
	}

	if (!(fMenu == NULL || fMenu->Window() != this)) {
		BPopUpMenu *menu = fMenu;

		Hide();
		fMenu = NULL;

		menu->RemoveSelf();
	}

	Quit();
}



BPopUpMenu::BPopUpMenu(const char *title, bool radioMode, bool labelFromMarked, e_menu_layout layout)
		: BMenu(title, layout), fAutoDestruct(false)
{
	SetEventMask(B_POINTER_EVENTS);
	SetRadioMode(radioMode);
	if (radioMode) SetLabelFromMarked(labelFromMarked);
}


BPopUpMenu::~BPopUpMenu()
{
}


void
BPopUpMenu::MessageReceived(BMessage *msg)
{
	switch (msg->what) {
		case B_MOUSE_MOVED: {
			BPopUpMenuWindow *win = cast_as(Window(), BPopUpMenuWindow);
			if (win == NULL || win->fOpenAnyway == true) break;

			int32 buttons;
			BPoint where;

			if (!(msg->FindInt32("buttons", &buttons) == false || buttons > 0)) {
				Window()->PostMessage(B_QUIT_REQUESTED);
				return;
			}
			if (msg->FindPoint("where", &where) == false || VisibleBounds().Contains(where)) break;
			ConvertToScreen(&where);

			BMenu *submenu = this;
			while (submenu != NULL) {
				BMenu *aMenu = submenu->SubmenuAt(submenu->fSelectedIndex);
				if (aMenu == NULL) break;
				if (aMenu->Window() == NULL) break;
				if (aMenu->Window()->Frame().Contains(where)) {
					msg->RemovePoint("where");
					BMessenger msgr(aMenu->Window());
					msgr.SendMessage(msg);
					break;
				}
				submenu = aMenu;
			}
		}
		break;

		case _MENU_EVENT_: {
			if (Window() == NULL || !is_instance_of(Window(), BPopUpMenuWindow)) break;

			BMenuItem *item = NULL;
			if (msg->FindPointer("source", (void**)&item) == false || item == NULL) break;

			BPopUpMenuWindow *win = cast_as(Window(), BPopUpMenuWindow);
			fSelectedItem = item;
			if (win->fDeliversMessage) {
				uint32 what;
				if (msg->FindInt32("etk:menu_orig_what", (int32*)&what)) {
					BMessage aMsg = *msg;
					aMsg.what = what;
					item->BInvoker::Invoke(&aMsg);
				}
			}

			win->PostMessage(B_QUIT_REQUESTED);

			return;
		}
		break;

		default:
			break;
	}

	BMenu::MessageReceived(msg);
}


void
BPopUpMenu::MouseUp(BPoint where)
{
	BPopUpMenuWindow *win = cast_as(Window(), BPopUpMenuWindow);
	BMessage *msg = Window()->CurrentMessage();

	if (!(win == NULL || win->fOpenAnyway == true || msg == NULL || msg->what != B_MOUSE_UP)) {
		BPoint mousePos = where;
		BMenu *submenu = this;

		ConvertToScreen(&mousePos);

		if (!VisibleBounds().Contains(where)) {
			while (submenu != NULL) {
				BMenu *aMenu = submenu->SubmenuAt(submenu->fSelectedIndex);
				if (aMenu == NULL) break;
				if (aMenu->Window() == NULL) break;
				if (aMenu->Window()->Frame().Contains(mousePos)) {
					aMenu->ConvertFromScreen(&mousePos);
					aMenu->fTrackingIndex = aMenu->fSelectedIndex;
					aMenu->MouseUp(mousePos);
					break;
				}
				submenu = aMenu;
			}
		} else {
			fTrackingIndex = fSelectedIndex;
		}
	}

	BMenu::MouseUp(where);

	if (!(win == NULL || win->fOpenAnyway == true || msg == NULL || msg->what != B_MOUSE_UP)) {
		int32 buttons;
		if (msg->FindInt32("buttons", &buttons) == false) return;
		if (buttons > 0) return;

		Window()->PostMessage(B_QUIT_REQUESTED);
	}
}


BMenuItem*
BPopUpMenu::Go(BPoint where, bool delivers_message, bool open_anyway, bool async, bool could_proxy)
{
	if (Window() != NULL || Supermenu() != NULL) {
		ETK_WARNING("[INTERFACE]: %s --- Menu already pop-up or attached to others.", __PRETTY_FUNCTION__);
		return NULL;
	}

	SelectItem(NULL);
	fSelectedItem = NULL;

	BPopUpMenuWindow *win = new BPopUpMenuWindow(where, this, delivers_message, open_anyway, async, could_proxy);
	void *trackingThread = NULL;

	if (win == NULL || win->IsRunning() == false || Window() != win ||
	        (win->Proxy() == win ? ((trackingThread = open_thread(win->Thread())) == NULL) : false)) {
		ETK_WARNING("[INTERFACE]: %s --- Unable to create pop-up window.", __PRETTY_FUNCTION__);
		if (win != NULL) {
			if (Window() == win) {
				win->fMenu = NULL;
				RemoveSelf();
			}
			win->Quit();
		}
		return NULL;
	}

	win->Show();
	win->SendBehind(NULL);

	if (win->Proxy() != win) {
		win->WaitToClose();
	} else {
		win->Unlock();

		if (trackingThread) {
			if (!async) {
				status_t status;
				wait_for_thread(trackingThread, &status);
			}
			delete_thread(trackingThread);
		}
	}

	return(async ? NULL : fSelectedItem);
}


void
BPopUpMenu::SetAsyncAutoDestruct(bool state)
{
	fAutoDestruct = state;
}


bool
BPopUpMenu::AsyncAutoDestruct() const
{
	return fAutoDestruct;
}


bool
BPopUpMenu::IsPopUpByGo() const
{
	return(is_instance_of(Window(), BPopUpMenuWindow) != 0);
}

