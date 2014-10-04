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
 * File: Menu.cpp
 *
 * --------------------------------------------------------------------------*/

#include <support/ClassInfo.h>
#include <kernel/Kernel.h>

#include "Window.h"
#include "PopUpMenu.h"
#include "Menu.h"

#define ETK_MENU_ROW_SPACING	5
#define ETK_MENU_COLUMN_SPACING	2


class BSubmenuView;

class BSubmenuWindow : public BWindow
{
	public:
		BSubmenuWindow(BPoint where, BMenu *menu);
		virtual ~BSubmenuWindow();

		virtual void DispatchMessage(BMessage *msg, BHandler *target);
		virtual bool QuitRequested();
		virtual void FrameMoved(BPoint new_position);

	private:
		friend class BMenu;
		friend class BSubmenuView;

		BMenu *fMenu;
};


BMenu::BMenu(BRect frame, const char *title, uint32 resizeMode, uint32 flags, e_menu_layout layout, bool resizeToFit)
		: BView(frame, title, resizeMode, flags), fSuperitem(NULL),
		fRadioMode(false), fLabelFromMarked(false),
		fSelectedIndex(-1), fTrackingIndex(-1), fMarkedIndex(-1), fShowSubmenuByKeyDown(false)
{
	fMargins = BRect(0, 0, 0, 0);

	SetViewColor(ui_color(B_MENU_BACKGROUND_COLOR));

	SetFlags(flags |B_WILL_DRAW);

	SetLayout(layout, frame.Width(), frame.Height(), resizeToFit);
}


BMenu::BMenu(const char *title, e_menu_layout layout)
		: BView(BRect(0, 0, 10, 10), title, B_FOLLOW_NONE, B_WILL_DRAW), fSuperitem(NULL),
		fRadioMode(false), fLabelFromMarked(false),
		fSelectedIndex(-1), fTrackingIndex(-1), fMarkedIndex(-1), fShowSubmenuByKeyDown(false)
{
	fMargins = BRect(0, 0, 0, 0);

	SetViewColor(ui_color(B_MENU_BACKGROUND_COLOR));

	SetLayout(layout, 10, 10, (layout != B_ITEMS_IN_MATRIX ? true : false));
}


BMenu::BMenu(const char *title, float width, float height)
		: BView(BRect(0, 0, width > 10 ? width : 10, height > 10 ? height : 10), title, B_FOLLOW_NONE, B_WILL_DRAW), fSuperitem(NULL),
		fRadioMode(false), fLabelFromMarked(false),
		fSelectedIndex(-1), fTrackingIndex(-1), fMarkedIndex(-1), fShowSubmenuByKeyDown(false)
{
	fMargins = BRect(0, 0, 0, 0);

	SetViewColor(ui_color(B_MENU_BACKGROUND_COLOR));

	SetLayout(B_ITEMS_IN_MATRIX, width, height, false);
}


BMenu::~BMenu()
{
	for (int32 i = 0; i < fMenuItems.CountItems(); i++) {
		BMenuItem *item = (BMenuItem*)fMenuItems.ItemAt(i);
		item->fMenu = NULL;
		delete item;
	}

	if (fSuperitem) {
		ETK_WARNING("[INTERFACE]: %s --- Menu still have super-item as deleting.", __PRETTY_FUNCTION__);
		fSuperitem->fSubmenu = NULL;
	}
}


bool
BMenu::AddItem(BMenuItem *item)
{
	return AddItem(item, fMenuItems.CountItems());
}


bool
BMenu::AddItem(BMenuItem *item, int32 index)
{
	if (!item || item->Menu() != NULL || fLayout == B_ITEMS_IN_MATRIX) return false;

	if (fMenuItems.AddItem((void*)item, index) == false) return false;

	item->fMenu = this;
	if (fSelectedIndex == index) fSelectedIndex++;
	if (fMarkedIndex == index) fMarkedIndex++;

	if (Window() != NULL) Refresh();

	return true;
}


bool
BMenu::AddItem(BMenuItem *item, BRect frame)
{
	if (!item || item->Menu() != NULL || !frame.IsValid() || fLayout != B_ITEMS_IN_MATRIX) return false;

	if (fMenuItems.AddItem((void*)item) == false) return false;

	item->fMenu = this;

	item->fFrame = frame;

	if (Window() != NULL) Refresh();

	return true;
}


bool
BMenu::AddItem(BMenu *menu)
{
	return AddItem(menu, fMenuItems.CountItems());
}


bool
BMenu::AddItem(BMenu *menu, int32 index)
{
	if (!menu || menu->Superitem() != NULL || fLayout == B_ITEMS_IN_MATRIX) return false;

	BMenuItem *item = new BMenuItem(menu, NULL);
	if (!item) return false;

	if (fMenuItems.AddItem((void*)item, index) == false) {
		item->fSubmenu = NULL;
		menu->fSuperitem = NULL;
		delete item;
		return false;
	}

	item->fMenu = this;

	if (fSelectedIndex == index) fSelectedIndex++;
	if (fMarkedIndex == index) fMarkedIndex++;

	if (Window() != NULL) Refresh();

	return true;
}


bool
BMenu::AddItem(BMenu *menu, BRect frame)
{
	if (!menu || menu->Superitem() != NULL || !frame.IsValid() || fLayout != B_ITEMS_IN_MATRIX) return false;

	BMenuItem *item = new BMenuItem(menu, NULL);
	if (!item) return false;

	if (fMenuItems.AddItem((void*)item) == false) {
		item->fSubmenu = NULL;
		menu->fSuperitem = NULL;
		delete item;
		return false;
	}

	item->fMenu = this;
	item->fFrame = frame;

	Invalidate(frame);

	return false;
}


bool
BMenu::AddSeparatorItem()
{
	if (fLayout == B_ITEMS_IN_MATRIX) return false;

	BMenuItem *item = new BMenuSeparatorItem();
	if (!item) return false;

	if (!AddItem(item)) {
		delete item;
		return false;
	}

	return true;
}


bool
BMenu::RemoveItem(BMenuItem *item)
{
	int32 index = IndexOf(item);
	return(RemoveItem(index) != NULL);
}


BMenuItem*
BMenu::RemoveItem(int32 index)
{
	if (index < 0 || index >= fMenuItems.CountItems()) return NULL;

	BMenuItem *item = (BMenuItem*)fMenuItems.RemoveItem(index);
	if (!item) return NULL;

	item->fMenu = NULL;
	if (item->fSubmenu != NULL) item->fSubmenu->ClosePopUp();

	if (fSelectedIndex == index) fSelectedIndex = -1;
	if (fMarkedIndex == index) {
		fMarkedIndex = -1;
		if (fRadioMode) FindMarked(&fMarkedIndex);
	}

	if (Window() != NULL) Refresh();

	return item;
}


bool
BMenu::RemoveItem(BMenu *menu)
{
	if (!menu || menu->fSuperitem == NULL || menu->fSuperitem->fMenu != this) return false;

	int32 index = IndexOf(menu->fSuperitem);
	BMenuItem *item = (BMenuItem*)fMenuItems.RemoveItem(index);
	if (!item) return false;

	menu->fSuperitem = NULL;
	menu->ClosePopUp();

	item->fMenu = NULL;
	item->fSubmenu = NULL;
	delete item;

	if (fSelectedIndex == index) fSelectedIndex = -1;
	if (fMarkedIndex == index) {
		fMarkedIndex = -1;
		if (fRadioMode) FindMarked(&fMarkedIndex);
	}

	if (Window() != NULL) Refresh();

	return true;
}


BMenuItem*
BMenu::ItemAt(int32 index) const
{
	return (BMenuItem*)fMenuItems.ItemAt(index);
}


BMenu*
BMenu::SubmenuAt(int32 index) const
{
	BMenuItem *item = (BMenuItem*)fMenuItems.ItemAt(index);
	if (!item) return NULL;

	return item->Submenu();
}


int32
BMenu::CountItems() const
{
	return fMenuItems.CountItems();
}


int32
BMenu::IndexOf(const BMenuItem *item) const
{
	if (!item || item->fMenu != this) return -1;
	return fMenuItems.IndexOf((void*)item);
}


int32
BMenu::IndexOf(const BMenu *menu) const
{
	if (!menu || menu->Superitem() == NULL) return -1;
	return fMenuItems.IndexOf((void*)menu->Superitem());
}


BMenuItem*
BMenu::FindItem(uint32 command) const
{
	for (int32 i = 0; i < fMenuItems.CountItems(); i++) {
		BMenuItem *item = (BMenuItem*)fMenuItems.ItemAt(i);

		if (item->Command() == command) return item;
		if (item->fSubmenu == NULL) continue;

		BMenuItem *found = item->fSubmenu->FindItem(command);
		if (found != NULL) return found;
	}

	return NULL;
}


inline bool comapre_menuitem_name(const char *name1, const char *name2)
{
	if (!name1 && !name2) return true;
	if (!name1 || !name2) return false;
	if (strlen(name1) != strlen(name2)) return false;
	if (strcmp(name1, name2) != 0) return false;
	return true;
}


BMenuItem*
BMenu::FindItem(const char *name) const
{
	for (int32 i = 0; i < fMenuItems.CountItems(); i++) {
		BMenuItem *item = (BMenuItem*)fMenuItems.ItemAt(i);

		if (comapre_menuitem_name(item->Label(), name)) return item;
		if (item->fSubmenu == NULL) continue;

		BMenuItem *found = item->fSubmenu->FindItem(name);
		if (found != NULL) return found;
	}

	return NULL;
}


BMenu*
BMenu::Supermenu() const
{
	if (!fSuperitem) return NULL;
	return fSuperitem->Menu();
}


BMenuItem*
BMenu::Superitem() const
{
	return fSuperitem;
}


status_t
BMenu::SetTargetForItems(BHandler *target)
{
	status_t status = B_OK;
	BMessenger msgr(target, NULL, &status);
	if (status != B_OK) return status;

	return SetTargetForItems(msgr);
}


status_t
BMenu::SetTargetForItems(BMessenger messenger)
{
	for (int32 i = 0; i < fMenuItems.CountItems(); i++) {
		BMenuItem *item = (BMenuItem*)fMenuItems.ItemAt(i);
		status_t status = item->SetTarget(messenger);
		if (status != B_OK) return status;
	}

	return B_OK;
}


void
BMenu::SetEnabled(bool state)
{
	if (BView::IsEnabled() != state) {
		BView::SetEnabled(state);

		rgb_color vColor = ui_color(B_MENU_BACKGROUND_COLOR);

		if (!state) {
			fSelectedIndex = -1;
			fTrackingIndex = -1;

			vColor.mix(0, 0, 0, 20);
		}

		SetViewColor(vColor);

		if (!(fSuperitem  == NULL || fSuperitem->fEnabled == state)) fSuperitem->SetEnabled(state);

		if (is_instance_of(Window(), BSubmenuWindow)) Window()->SetBackgroundColor(vColor);
	}
}


bool
BMenu::IsEnabled() const
{
	if (BView::IsEnabled() == false) return false;
	if (fSuperitem == NULL || fSuperitem->fMenu == NULL) return true;
	return fSuperitem->fMenu->IsEnabled();
}


void
BMenu::AttachedToWindow()
{
	if (is_instance_of(Window(), BSubmenuWindow)) SetEventMask(B_POINTER_EVENTS |B_KEYBOARD_EVENTS);

	Window()->DisableUpdates();
	Refresh();
	Window()->EnableUpdates();

	if (!is_instance_of(Window(), BSubmenuWindow)) {
		Window()->StartWatching(this, B_MINIMIZED);
		Window()->StartWatching(this, B_WINDOW_ACTIVATED);
	}

	Window()->StartWatching(this, B_WINDOW_MOVED);
	Window()->StartWatching(this, B_WINDOW_RESIZED);

	for (int32 i = 0; i < fMenuItems.CountItems(); i++) {
		BMenu *menu = ((BMenuItem*)fMenuItems.ItemAt(i))->fSubmenu;
		if (menu == NULL) continue;
		if (menu->fRadioMode == false || menu->fLabelFromMarked == false || menu->fSuperitem == NULL) continue;

		BMenuItem *markedItem = (BMenuItem*)menu->fMenuItems.ItemAt(menu->fMarkedIndex);
		if (markedItem)
			menu->fSuperitem->SetLabel(markedItem->Label());
		else
			menu->fSuperitem->SetLabel(menu->Name());
	}
}


void
BMenu::DetachedFromWindow()
{
	Window()->StopWatchingAll(this);

	for (int32 i = 0; i < fMenuItems.CountItems(); i++) {
		BMenuItem *item = (BMenuItem*)fMenuItems.ItemAt(i);
		if (!(item == NULL || item->fSubmenu == NULL)) item->fSubmenu->ClosePopUp();
	}

	fTrackingIndex = -1;
}


void
BMenu::MessageReceived(BMessage *msg)
{
	bool processed = false;

	switch (msg->what) {
		case B_OBSERVER_NOTICE_CHANGE: {
			if (Window() == NULL) break;

			uint32 what;
			if (msg->FindInt32(B_OBSERVE_ORIGINAL_WHAT, (int32*)&what) == false ||
			        !(what == B_WINDOW_MOVED ||
			          what == B_WINDOW_RESIZED ||
			          what == B_MINIMIZED ||
			          what == B_WINDOW_ACTIVATED)) break;

			processed = true;

			if (fSelectedIndex < 0) break;
			BMenuItem *item = (BMenuItem*)fMenuItems.ItemAt(fSelectedIndex);
			if (item == NULL || item->fSubmenu == NULL || item->fSubmenu->Window() == NULL) break;

			if ((is_instance_of(Window(), BSubmenuWindow) == false &&
			        Window()->IsActivate() == false &&
			        item->fSubmenu->Window()->IsActivate() == false) ||
			        Window()->IsHidden() || Window()->IsMinimized()) {
				item->fSubmenu->ClosePopUp();
			} else if (is_instance_of(item->fSubmenu->Window(), BSubmenuWindow)) {
#if 0
				// notice submenu to move
				item->fSubmenu->Window()->PostMessage(_MENU_EVENT_, item->fSubmenu->Window());
#else
				item->fSubmenu->ClosePopUp();
#endif
			}
		}
		break;

		case _MENU_EVENT_: {
			if (Window() == NULL) break;

			BMenuItem *item = NULL;
			if (msg->FindPointer("source", (void**)&item) == false || item == NULL) break;

			processed = true;

			if (is_instance_of(Window(), BSubmenuWindow)) {
				if (!(Supermenu() == NULL || Supermenu()->Window() == NULL))
					Supermenu()->Window()->PostMessage(msg, Supermenu());
				Window()->PostMessage(B_QUIT_REQUESTED);
			} else {
				uint32 what;
				if (msg->FindInt32("etk:menu_orig_what", (int32*)&what)) {
					BMessage aMsg = *msg;
					aMsg.what = what;
					item->BInvoker::Invoke(&aMsg);
				}
			}
		}
		break;

		default:
			break;
	}

	if (!processed) BView::MessageReceived(msg);
}


void
BMenu::MouseDown(BPoint where)
{
	if (!IsEnabled() || Window() == NULL || fMenuItems.CountItems() <= 0 || !QueryCurrentMouse(true, B_PRIMARY_MOUSE_BUTTON)) return;

	BRect rect = VisibleBounds();
	if (!rect.Contains(where)) return;

	if (fTrackingIndex >= 0) return;

	int32 newIndex = FindItem(where);
	if (newIndex < 0)return;

	if (fSelectedIndex != newIndex) {
		BMenuItem *oldItem = (BMenuItem*)fMenuItems.ItemAt(fSelectedIndex);
		BMenuItem *newItem = (BMenuItem*)fMenuItems.ItemAt(newIndex);

		int32 oldSelectedIndex = fSelectedIndex;
		fSelectedIndex = newIndex;

		Window()->DisableUpdates();
		if (newItem->SelectChanged()) {
			if (oldItem) oldItem->SelectChanged();
		} else {
			fSelectedIndex = oldSelectedIndex;
		}
		Window()->EnableUpdates();
	}

	fShowSubmenuByKeyDown = false;

	fTrackingIndex = fSelectedIndex;
}


void
BMenu::MouseUp(BPoint where)
{
	if (fTrackingIndex >= 0) {
		int32 trackingIndex = fTrackingIndex;
		fTrackingIndex = -1;

		if (!IsEnabled() || Window() == NULL || fMenuItems.CountItems() <= 0) return;
		if (trackingIndex != fSelectedIndex || ItemFrame(trackingIndex).Contains(where) == false) return;

		BMenuItem *item = (BMenuItem*)fMenuItems.ItemAt(fSelectedIndex);
		if (item == NULL || item->IsEnabled() == false) return;

		if (item->fSubmenu) {
			item->ShowSubmenu(true);
		} else {
			if (is_instance_of(Window(), BSubmenuWindow)) {
				if (!(Supermenu() == NULL || Supermenu()->Window() == NULL)) {
					BMessage msg(_MENU_EVENT_);
					if (item->Message() != NULL) {
						msg = *(item->Message());
						msg.AddInt32("etk:menu_orig_what", msg.what);
						msg.what = _MENU_EVENT_;
					}
					msg.AddInt64("when", e_real_time_clock_usecs());
					msg.AddPointer("source", item);
					msg.AddInt32("index", fSelectedIndex);

					Supermenu()->Window()->PostMessage(&msg, Supermenu());
				}
				Window()->PostMessage(B_QUIT_REQUESTED);
			} else if (is_kind_of(this, BPopUpMenu)) {
				BMessage msg(_MENU_EVENT_);
				if (item->Message() != NULL) {
					msg = *(item->Message());
					msg.AddInt32("etk:menu_orig_what", msg.what);
					msg.what = _MENU_EVENT_;
				}
				msg.AddInt64("when", e_real_time_clock_usecs());
				msg.AddPointer("source", item);
				msg.AddInt32("index", fSelectedIndex);

				Window()->PostMessage(&msg, this);
			} else {
				item->Invoke();
			}

			ItemInvoked(item);
		}
	}
}


void
BMenu::MouseMoved(BPoint where, uint32 code, const BMessage *a_message)
{
	if (!IsEnabled() || Window() == NULL || fMenuItems.CountItems() <= 0 || !VisibleBounds().Contains(where)) return;

	int32 newIndex = FindItem(where);
	if (newIndex < 0) return;

	if (fSelectedIndex != newIndex) {
		BMenuItem *oldItem = (BMenuItem*)fMenuItems.ItemAt(fSelectedIndex);
		BMenuItem *newItem = (BMenuItem*)fMenuItems.ItemAt(newIndex);

		int32 oldSelectedIndex = fSelectedIndex;
		fSelectedIndex = newIndex;

		Window()->DisableUpdates();
		if (newItem->SelectChanged()) {
			if (oldItem) oldItem->SelectChanged();
		} else {
			fSelectedIndex = oldSelectedIndex;
		}
		Window()->EnableUpdates();
	}

	fShowSubmenuByKeyDown = false;
}


void
BMenu::KeyDown(const char *bytes, int32 numBytes)
{
	if (!IsEnabled() || Window() == NULL || fMenuItems.CountItems() <= 0 || numBytes != 1 || bytes == NULL) return;
	if (Window()->CurrentMessage() == NULL) return;
	if (!(Window()->CurrentMessage()->what == B_KEY_DOWN || Window()->CurrentMessage()->what == B_UNMAPPED_KEY_DOWN)) return;

	BMenuItem *selectedItem = (BMenuItem*)fMenuItems.ItemAt(fSelectedIndex);

	if (!(selectedItem == NULL || selectedItem->fSubmenu == NULL || selectedItem->fSubmenu->Window() == NULL ||
	        Window()->CurrentMessage()->HasBool("_MENU_EVENT_"))) {
		selectedItem->fSubmenu->Window()->PostMessage(Window()->CurrentMessage(), selectedItem->fSubmenu);
		return;
	}

	if (!fShowSubmenuByKeyDown)
		fShowSubmenuByKeyDown = ((selectedItem->fSubmenu == NULL || selectedItem->fSubmenu->Window() == NULL) ? false : true);

	int32 oldIndex = fSelectedIndex;
	bool doRewind = false;
	bool doInvert = false;

	char ch = *bytes;
	switch (ch) {
		case B_UP_ARROW: {
			if (fLayout != B_ITEMS_IN_ROW) {
				fSelectedIndex--;
				if (fSelectedIndex < 0) fSelectedIndex = fMenuItems.CountItems() - 1;
				doRewind = true;
				doInvert = true;
			}
		}
		break;

		case B_DOWN_ARROW: {
			if (fLayout != B_ITEMS_IN_ROW) {
				fSelectedIndex++;
				if (fSelectedIndex >= fMenuItems.CountItems()) fSelectedIndex = 0;
				doRewind = true;
				doInvert = false;
			}
		}
		break;

		case B_LEFT_ARROW: {
			if (fLayout != B_ITEMS_IN_COLUMN) {
				fSelectedIndex--;
				if (fSelectedIndex < 0) fSelectedIndex = fMenuItems.CountItems() - 1;
				doRewind = true;
				doInvert = true;
			}
		}
		break;

		case B_RIGHT_ARROW: {
			if (fLayout != B_ITEMS_IN_COLUMN) {
				fSelectedIndex++;
				if (fSelectedIndex >= fMenuItems.CountItems()) fSelectedIndex = 0;
				doRewind = true;
				doInvert = false;
			}
		}
		break;

		case B_ENTER:
		case B_SPACE: {
			if (fSelectedIndex < 0 || fTrackingIndex >= 0) break;
			fTrackingIndex = fSelectedIndex;
		}
		break;

		case B_ESCAPE:
			if (!is_instance_of(Window(), BSubmenuWindow)) fTrackingIndex = fSelectedIndex = -1;
			break;

		case B_HOME:
			fSelectedIndex = 0;
			doRewind = false;
			doInvert = false;
			break;

		case B_END:
			fSelectedIndex = fMenuItems.CountItems() - 1;
			doRewind = false;
			doInvert = true;
			break;

		default:
			break;
	}

	if (oldIndex != fSelectedIndex && fSelectedIndex >= 0) {
		BMenuItem *oldItem = (BMenuItem*)fMenuItems.ItemAt(oldIndex);

		Window()->DisableUpdates();

		if (oldItem) oldItem->SelectChanged();

		int32 tmp = fSelectedIndex;
		fSelectedIndex = -1;

		if (!doInvert) {
			for (int32 i = tmp; i < fMenuItems.CountItems(); i++) {
				BMenuItem *newItem = (BMenuItem*)fMenuItems.ItemAt(i);
				fSelectedIndex = i;
				if (newItem->SelectChanged()) break;
				fSelectedIndex = -1;
			}
			if (fSelectedIndex < 0 && doRewind) {
				for (int32 i = 0; i < tmp; i++) {
					BMenuItem *newItem = (BMenuItem*)fMenuItems.ItemAt(i);
					fSelectedIndex = i;
					if (newItem->SelectChanged()) break;
					fSelectedIndex = -1;
				}
			}
		} else {
			for (int32 i = tmp; i >= 0; i--) {
				BMenuItem *newItem = (BMenuItem*)fMenuItems.ItemAt(i);
				fSelectedIndex = i;
				if (newItem->SelectChanged()) break;
				fSelectedIndex = -1;
			}
			if (fSelectedIndex < 0 && doRewind) {
				for (int32 i = fMenuItems.CountItems() - 1; i > tmp; i--) {
					BMenuItem *newItem = (BMenuItem*)fMenuItems.ItemAt(i);
					fSelectedIndex = i;
					if (newItem->SelectChanged()) break;
					fSelectedIndex = -1;
				}
			}
		}

		Window()->EnableUpdates();
	} else if (fSelectedIndex < 0) {
		BMenuItem *oldItem = (BMenuItem*)fMenuItems.ItemAt(oldIndex);
		if (oldItem) oldItem->SelectChanged();
		fShowSubmenuByKeyDown = false;
	}
}


void
BMenu::KeyUp(const char *bytes, int32 numBytes)
{
	if (!IsEnabled() || Window() == NULL || fMenuItems.CountItems() <= 0 || numBytes != 1 || bytes == NULL) return;
	if (Window()->CurrentMessage() == NULL) return;
	if (!(Window()->CurrentMessage()->what == B_KEY_UP || Window()->CurrentMessage()->what == B_UNMAPPED_KEY_UP)) return;

	BMenuItem *selectedItem = (BMenuItem*)fMenuItems.ItemAt(fSelectedIndex);
	if (!(selectedItem == NULL || selectedItem->fSubmenu == NULL || selectedItem->fSubmenu->Window() == NULL ||
	        Window()->CurrentMessage()->HasBool("_MENU_EVENT_"))) {
		selectedItem->fSubmenu->Window()->PostMessage(Window()->CurrentMessage(), selectedItem->fSubmenu);
		return;
	}

	char ch = *bytes;
	switch (ch) {
		case B_ENTER:
		case B_SPACE: {
			if (fTrackingIndex < 0 || fTrackingIndex != fSelectedIndex) break;
			if (selectedItem == NULL || selectedItem->IsEnabled() == false) break;
			if (selectedItem->fSubmenu) {
				if (selectedItem->fSubmenu->Window() == NULL) selectedItem->ShowSubmenu(true);
				break;
			}

			if (is_instance_of(Window(), BSubmenuWindow)) {
				if (!(Supermenu() == NULL || Supermenu()->Window() == NULL)) {
					BMessage msg(_MENU_EVENT_);
					if (selectedItem->Message() != NULL) {
						msg = *(selectedItem->Message());
						msg.AddInt32("etk:menu_orig_what", msg.what);
						msg.what = _MENU_EVENT_;
					}
					msg.AddInt64("when", e_real_time_clock_usecs());
					msg.AddPointer("source", selectedItem);
					msg.AddInt32("index", fSelectedIndex);

					Supermenu()->Window()->PostMessage(&msg, Supermenu());
				}
				Window()->PostMessage(B_QUIT_REQUESTED);
			} else if (is_kind_of(this, BPopUpMenu)) {
				BMessage msg(_MENU_EVENT_);
				if (selectedItem->Message() != NULL) {
					msg = *(selectedItem->Message());
					msg.AddInt32("etk:menu_orig_what", msg.what);
					msg.what = _MENU_EVENT_;
				}
				msg.AddInt64("when", e_real_time_clock_usecs());
				msg.AddPointer("source", selectedItem);
				msg.AddInt32("index", fSelectedIndex);

				Window()->PostMessage(&msg, this);
			} else {
				selectedItem->Invoke();
			}

			ItemInvoked(selectedItem);
		}
		break;

		case B_UP_ARROW: {
			if (fLayout == B_ITEMS_IN_ROW) {
				if (!is_instance_of(Window(), BSubmenuWindow)) break;
				if (Supermenu()->Layout() != B_ITEMS_IN_ROW && Supermenu()->Window() != NULL) {
					BMessage aMsg = *(Window()->CurrentMessage());
					aMsg.AddBool("_MENU_EVENT_", true);
					aMsg.ReplaceInt64("when", e_real_time_clock_usecs());
					aMsg.what = B_KEY_DOWN;
					Supermenu()->Window()->PostMessage(&aMsg, Supermenu());
					aMsg.what = B_KEY_UP;
					Supermenu()->Window()->PostMessage(&aMsg, Supermenu());
				} else {
					Window()->PostMessage(B_QUIT_REQUESTED);
				}
			} else if (fShowSubmenuByKeyDown) {
				if (selectedItem == NULL || selectedItem->fSubmenu == NULL) break;
				if (selectedItem->fSubmenu->Window() == NULL) selectedItem->ShowSubmenu(true);
			}
		}
		break;

		case B_DOWN_ARROW: {
			if (fLayout == B_ITEMS_IN_ROW) {
				if (selectedItem) {
					if (is_instance_of(Window(), BSubmenuWindow)) {
						if (Supermenu()->Layout() != B_ITEMS_IN_ROW && selectedItem->fSubmenu == NULL) {
							if (Supermenu()->Window() != NULL) {
								BMessage aMsg = *(Window()->CurrentMessage());
								aMsg.AddBool("_MENU_EVENT_", true);
								aMsg.ReplaceInt64("when", e_real_time_clock_usecs());
								aMsg.what = B_KEY_DOWN;
								Supermenu()->Window()->PostMessage(&aMsg, Supermenu());
								aMsg.what = B_KEY_UP;
								Supermenu()->Window()->PostMessage(&aMsg, Supermenu());
							}
							break;
						}
					}

					if (selectedItem->fSubmenu != NULL) selectedItem->ShowSubmenu(true);
				}
			} else if (fShowSubmenuByKeyDown) {
				if (selectedItem == NULL || selectedItem->fSubmenu == NULL) break;
				if (selectedItem->fSubmenu->Window() == NULL) selectedItem->ShowSubmenu(true);
			}
		}
		break;

		case B_LEFT_ARROW: {
			if (fLayout == B_ITEMS_IN_COLUMN) {
				if (!is_instance_of(Window(), BSubmenuWindow)) break;
				if (Supermenu()->Layout() != B_ITEMS_IN_COLUMN && Supermenu()->Window() != NULL) {
					BMessage aMsg = *(Window()->CurrentMessage());
					aMsg.AddBool("_MENU_EVENT_", true);
					aMsg.ReplaceInt64("when", e_real_time_clock_usecs());
					aMsg.what = B_KEY_DOWN;
					Supermenu()->Window()->PostMessage(&aMsg, Supermenu());
					aMsg.what = B_KEY_UP;
					Supermenu()->Window()->PostMessage(&aMsg, Supermenu());
				} else {
					Window()->PostMessage(B_QUIT_REQUESTED);
				}
			} else if (fShowSubmenuByKeyDown) {
				if (selectedItem == NULL || selectedItem->fSubmenu == NULL) break;
				if (selectedItem->fSubmenu->Window() == NULL) selectedItem->ShowSubmenu(true);
			}
		}
		break;

		case B_RIGHT_ARROW: {
			if (fLayout == B_ITEMS_IN_COLUMN) {
				if (selectedItem) {
					if (is_instance_of(Window(), BSubmenuWindow)) {
						if (Supermenu()->Layout() != B_ITEMS_IN_COLUMN && selectedItem->fSubmenu == NULL) {
							if (Supermenu()->Window() != NULL) {
								BMessage aMsg = *(Window()->CurrentMessage());
								aMsg.AddBool("_MENU_EVENT_", true);
								aMsg.ReplaceInt64("when", e_real_time_clock_usecs());
								aMsg.what = B_KEY_DOWN;
								Supermenu()->Window()->PostMessage(&aMsg, Supermenu());
								aMsg.what = B_KEY_UP;
								Supermenu()->Window()->PostMessage(&aMsg, Supermenu());
							}
							break;
						}
					}

					if (selectedItem->fSubmenu != NULL) selectedItem->ShowSubmenu(true);
				}
			} else if (fShowSubmenuByKeyDown) {
				if (selectedItem == NULL || selectedItem->fSubmenu == NULL) break;
				if (selectedItem->fSubmenu->Window() == NULL) selectedItem->ShowSubmenu(true);
			}
		}
		break;

		case B_ESCAPE: {
			if (is_instance_of(Window(), BSubmenuWindow)) Window()->PostMessage(B_QUIT_REQUESTED);
			if (Supermenu()) Supermenu()->fShowSubmenuByKeyDown = false;
		}
		break;

		case B_HOME:
		case B_END: {
			if (!fShowSubmenuByKeyDown) break;
			if (selectedItem == NULL || selectedItem->fSubmenu == NULL) break;
			if (selectedItem->fSubmenu->Window() == NULL) selectedItem->ShowSubmenu(true);
		}
		break;

		default:
			break;
	}

	fTrackingIndex = -1;
}


BRect
BMenu::ItemFrame(int32 index) const
{
	if (index < 0 || index >= fMenuItems.CountItems()) return BRect();

	if (fLayout == B_ITEMS_IN_MATRIX) {
		BMenuItem *item = (BMenuItem*)fMenuItems.ItemAt(index);
		return item->fFrame;
	}

	float left = fMargins.left, top = fMargins.top, w, h;

	for (int32 i = 0; i <= index; i++) {
		BMenuItem *item = (BMenuItem*)fMenuItems.ItemAt(i);

		w = 0;
		h = 0;
		item->GetContentSize(&w, &h);
		if (w < 4) w = 4;
		if (h < 4) h = 4;

		if (i == index) break;

		if (fLayout == B_ITEMS_IN_ROW) left += w + ETK_MENU_ROW_SPACING;
		else top += h + ETK_MENU_COLUMN_SPACING;
	}

	if (fLayout == B_ITEMS_IN_ROW)
		return BRect(left, fMargins.top, left + w, Frame().Height() - fMargins.bottom);
	else
		return BRect(fMargins.left, top, Frame().Width() - fMargins.right, top + h);
}


int32
BMenu::FindItem(BPoint where)
{
	if (!VisibleBounds().Contains(where)) return false;

	if (fLayout == B_ITEMS_IN_MATRIX) {
		for (int32 i = 0; i < fMenuItems.CountItems(); i++) {
			BMenuItem *item = (BMenuItem*)fMenuItems.ItemAt(i);
			if (item->fFrame.Contains(where)) return i;
		}

		return -1;
	}

	float left = fMargins.left, top = fMargins.top, w, h;

	for (int32 i = 0; i < fMenuItems.CountItems(); i++) {
		BMenuItem *item = (BMenuItem*)fMenuItems.ItemAt(i);

		w = 0;
		h = 0;
		item->GetContentSize(&w, &h);
		if (w < 4) w = 4;
		if (h < 4) h = 4;

		BRect r;
		if (fLayout == B_ITEMS_IN_ROW)
			r = BRect(left, fMargins.top, left + w, Frame().Height() - fMargins.bottom);
		else
			r = BRect(fMargins.left, top, Frame().Width() - fMargins.right, top + h);

		if (r.Contains(where)) return i;

		if (fLayout == B_ITEMS_IN_ROW) left += w + ETK_MENU_ROW_SPACING;
		else top += h + ETK_MENU_COLUMN_SPACING;
	}

	return -1;
}


void
BMenu::Refresh()
{
	if (is_instance_of(Window(), BSubmenuWindow) || fResizeToFit) {
		BRect rect = Frame();
		ResizeToPreferred();
		if (rect != Frame() && is_instance_of(Window(), BSubmenuWindow)) {
			BMessage msg(_MENU_EVENT_);
			msg.AddInt64("when", e_real_time_clock_usecs());
			msg.AddRect("frame", Frame());
			Window()->PostMessage(&msg, Window());
		}
	}

	Invalidate();
}


void
BMenu::Draw(BRect updateRect)
{
	if (!IsVisible()) return;

	BRect bounds = Frame().OffsetToSelf(B_ORIGIN);
	bounds.left += fMargins.left;
	bounds.top += fMargins.top;
	bounds.right -= fMargins.right;
	bounds.bottom -= fMargins.bottom;
	bounds &= updateRect;
	if (!bounds.IsValid()) return;

	PushState();

	if (fLayout == B_ITEMS_IN_MATRIX) {
		for (int32 i = 0; i < fMenuItems.CountItems(); i++) {
			BMenuItem *item = (BMenuItem*)fMenuItems.ItemAt(i);
			BRect r = item->fFrame;
			r &= bounds;
			if (!r.IsValid()) continue;
			ConstrainClippingRegion(r);
			item->Draw();
		}
	} else {
		float left = fMargins.left, top = fMargins.top, w, h;

		for (int32 i = 0; i < fMenuItems.CountItems(); i++) {
			BMenuItem *item = (BMenuItem*)fMenuItems.ItemAt(i);

			w = 0;
			h = 0;
			item->GetContentSize(&w, &h);
			if (w < 4) w = 4;
			if (h < 4) h = 4;

			BRect r;
			if (fLayout == B_ITEMS_IN_ROW)
				r = BRect(left, fMargins.top, left + w, Frame().Height() - fMargins.bottom);
			else
				r = BRect(fMargins.left, top, Frame().Width() - fMargins.right, top + h);

			r &= bounds;
			if (r.IsValid()) {
				ConstrainClippingRegion(r);
				item->Draw();
			}

			if (fLayout == B_ITEMS_IN_ROW) left += w + ETK_MENU_ROW_SPACING;
			else top += h + ETK_MENU_COLUMN_SPACING;
		}
	}

	PopState();
}


e_menu_layout
BMenu::Layout() const
{
	return fLayout;
}


void
BMenu::SetLayout(e_menu_layout layout, float width, float height, bool resizeToFit)
{
	fLayout = layout;
	fResizeToFit = resizeToFit;

	if (layout == B_ITEMS_IN_MATRIX) {
		if (width < fMargins.left + fMargins.right + 10) width = fMargins.left + fMargins.right + 10;
		if (height < fMargins.top + fMargins.bottom + 10) height = fMargins.top + fMargins.bottom + 10;

		ResizeTo(width, height);
	}

	Refresh();
}


void
BMenu::SetRadioMode(bool state)
{
	if (fRadioMode != state) {
		fRadioMode = state;

		if (state) {
			FindMarked(&fMarkedIndex);

			if (fLabelFromMarked && fSuperitem != NULL) {
				BMenuItem *markedItem = ItemAt(fMarkedIndex);
				if (markedItem)
					fSuperitem->SetLabel(markedItem->Label());
				else
					fSuperitem->SetLabel(Name());
			}
		} else {
			fMarkedIndex = -1;
			SetLabelFromMarked(false);
			if (fLabelFromMarked) {
				fLabelFromMarked = false;
				if (fSuperitem) fSuperitem->SetLabel(Name());
			}
		}

		Invalidate();
	}
}


bool
BMenu::IsRadioMode() const
{
	return fRadioMode;
}


BMenuItem*
BMenu::FindMarked(int32 *index) const
{
	if (fRadioMode && fMarkedIndex >= 0) {
		BMenuItem *item = (BMenuItem*)fMenuItems.ItemAt(fMarkedIndex);
		if (!(item == NULL || item->IsMarked() == false)) {
			if (index) *index = fMarkedIndex;
			return item;
		}
	}

	for (int32 i = 0; i < fMenuItems.CountItems(); i++) {
		BMenuItem *item = (BMenuItem*)fMenuItems.ItemAt(i);

		if (item->IsMarked()) {
			if (index) *index = i;
			return item;
		}
	}

	if (index) *index = -1;
	return NULL;
}


void
BMenu::SetLabelFromMarked(bool state)
{
	if (fLabelFromMarked != state) {
		if (state) {
			SetRadioMode(true);
			if (!fRadioMode) return;
		}

		fLabelFromMarked = state;
		if (fSuperitem) {
			if (state) {
				BMenuItem *markedItem = FindMarked();
				if (markedItem)
					fSuperitem->SetLabel(markedItem->Label());
				else
					fSuperitem->SetLabel(Name());
			} else {
				fSuperitem->SetLabel(Name());
			}
		}
	}
}


bool
BMenu::IsLabelFromMarked() const
{
	return(fRadioMode ? fLabelFromMarked : false);
}


void
BMenu::GetPreferredSize(float *width, float *height)
{
	if (!width && !height) return;

	if (fLayout == B_ITEMS_IN_MATRIX) {
		BRect rect(0, 0, 0, 0);

		for (int32 i = 0; i < fMenuItems.CountItems(); i++) {
			BMenuItem *item = (BMenuItem*)fMenuItems.ItemAt(i);

			BRect r = item->fFrame;
			if (r.IsValid()) rect |= r;
		}

		if (width) *width = rect.Width();
		if (height) *height = rect.Height();
	} else {
		BRect rect(0, 0, 0, 0);

		for (int32 i = 0; i < fMenuItems.CountItems(); i++) {
			BMenuItem *item = (BMenuItem*)fMenuItems.ItemAt(i);

			float w = 0, h = 0;
			item->GetContentSize(&w, &h);
			if (w < 4) w = 4;
			if (h < 4) h = 4;

			if (fLayout == B_ITEMS_IN_ROW) {
				w += (i == fMenuItems.CountItems() - 1 ? 0 : ETK_MENU_ROW_SPACING);
				rect.right += w;
				rect.bottom = max_c(rect.bottom, h);
			} else {
				h += (i == fMenuItems.CountItems() - 1 ? 0 : ETK_MENU_COLUMN_SPACING);
				rect.bottom += h;
				rect.right = max_c(rect.right, w);
			}
		}

		if (width) *width = rect.Width();
		if (height) *height = rect.Height();
	}

	if (width) *width += fMargins.left + fMargins.right;
	if (height) *height += fMargins.top + fMargins.bottom;
}


class BSubmenuView : public BView
{
	public:
		BSubmenuView(BRect frame);
		virtual ~BSubmenuView();

		virtual void Draw(BRect updateRect);
};


BSubmenuView::BSubmenuView(BRect frame)
		: BView(frame, NULL, B_FOLLOW_ALL, B_WILL_DRAW)
{
}


BSubmenuView::~BSubmenuView()
{
}


void
BSubmenuView::Draw(BRect updateRect)
{
	if (!(Bounds().InsetByCopy(1, 1).Contains(updateRect))) {
		SetDrawingMode(B_OP_COPY);
		SetPenSize(1);
		rgb_color borderColor = ui_color(B_MENU_BORDER_COLOR);

		BSubmenuWindow *win = cast_as(Window(), BSubmenuWindow);
		if (win->fMenu == NULL || win->fMenu->IsEnabled() == false) borderColor.mix(0, 0, 0, 20);

		SetHighColor(borderColor);
		StrokeRect(Bounds());
	}
}


BSubmenuWindow::BSubmenuWindow(BPoint where, BMenu *menu)
		: BWindow(BRect(0, 0, 1, 1), NULL, B_NO_BORDER_WINDOW_LOOK, B_FLOATING_APP_WINDOW_FEEL, B_AVOID_FOCUS), fMenu(NULL)
{
	Lock();
	if (!(menu == NULL || menu->Supermenu() == NULL || menu->Supermenu()->Window() == NULL)) {
		if (ProxyBy(menu->Supermenu()->Window())) {
			BSubmenuView *topView = new BSubmenuView(Bounds());
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
		}
	}
}


BSubmenuWindow::~BSubmenuWindow()
{
}


void
BSubmenuWindow::DispatchMessage(BMessage *msg, BHandler *target)
{
	if (target == this && msg->what == _MENU_EVENT_) {
		if (fMenu) {
			BPoint where;
			BRect frame;
			if (!(msg->FindRect("frame", &frame) == false || frame.IsValid() == false))
				ResizeTo(frame.Width() + 4, frame.Height() + 4);
			if (fMenu->GetPopUpWhere(&where)) MoveTo(where);
			else PostMessage(B_QUIT_REQUESTED);
		}

		return;
	}

	BWindow::DispatchMessage(msg, target);
}


void
BSubmenuWindow::FrameMoved(BPoint new_position)
{
	if (IsHidden() || fMenu == NULL || fMenu->Supermenu() == NULL || fMenu->Supermenu()->Window() == NULL) return;
	fMenu->Supermenu()->Window()->SendBehind(this);
}


bool
BSubmenuWindow::QuitRequested()
{
	if (!(fMenu == NULL || fMenu->Window() != this)) {
		BMenu *menu = fMenu;

		Hide();
		fMenu = NULL;

		menu->RemoveSelf();
	}

	return true;
}


bool
BMenu::PopUp(BPoint where, bool selectFirstItem)
{
	if (Window() != NULL) return false;
	if (Supermenu() == NULL || Supermenu()->Window() == NULL) return false;

	BSubmenuWindow *win = new BSubmenuWindow(where, this);
	if (!win) return false;

	if (Window() != win) {
		win->Quit();
		return false;
	}

	fSelectedIndex = -1;
	if (selectFirstItem) {
		for (int32 i = 0; i < fMenuItems.CountItems(); i++) {
			BMenuItem *newItem = (BMenuItem*)fMenuItems.ItemAt(i);
			fSelectedIndex = i;
			if (newItem->SelectChanged()) break;
			fSelectedIndex = -1;
		}
	}

	// TODO: show after a while
	win->Show();
	Supermenu()->Window()->SendBehind(win);

	win->Unlock();

	return true;
}


void
BMenu::ClosePopUp()
{
	BSubmenuWindow *win = cast_as(Window(), BSubmenuWindow);
	if (!(win == NULL || win->fMenu != this)) {
		win->Hide();
		win->fMenu = NULL;

		RemoveSelf();

		win->PostMessage(B_QUIT_REQUESTED);
	}
}


//#define TEMP_DEBUG(Exp)	{if((Exp)) ETK_DEBUG("[INTERFACE]: %s --- %s", __PRETTY_FUNCTION__, #Exp);}

bool
BMenu::GetPopUpWhere(BPoint *where)
{
	if (!where) return false;

	if (Supermenu() == NULL || Supermenu()->Window() == NULL ||
	        Supermenu()->Window()->IsHidden() || Supermenu()->Window()->IsMinimized()) {
//		TEMP_DEBUG(Supermenu() == NULL || Supermenu()->Window() == NULL);
//		if(!(Supermenu() == NULL || Supermenu()->Window() == NULL))
//		{
//			TEMP_DEBUG(Supermenu()->Window()->IsHidden());
//			TEMP_DEBUG(Supermenu()->Window()->IsMinimized());
//			TEMP_DEBUG(!is_instance_of(Supermenu()->Window(), BSubmenuWindow));
//		}

		return false;
	}

	if (Supermenu()->Window()->IsActivate() == false) {
		if (!(is_instance_of(Supermenu()->Window(), BSubmenuWindow) || is_instance_of(Supermenu(), BPopUpMenu))) return false;
		if (is_instance_of(Supermenu(), BPopUpMenu)) {
			if (cast_as(Supermenu(), BPopUpMenu)->IsPopUpByGo() == false) return false;
		}
	}

	BRect frame = Superitem()->Frame();
	frame.left -= Supermenu()->fMargins.left;
	frame.top -= Supermenu()->fMargins.top;
	frame.right += Supermenu()->fMargins.right;
	frame.bottom += Supermenu()->fMargins.bottom;
	frame &= Supermenu()->VisibleBounds();

	if (frame.IsValid() == false) return false;

	if (Supermenu()->Layout() == B_ITEMS_IN_COLUMN)
		*where = frame.RightTop() + BPoint(1, 0);
	else
		*where = frame.LeftBottom() + BPoint(0, 1);
	Supermenu()->ConvertToScreen(where);

	return true;
}


void
BMenu::SetItemMargins(float left, float top, float right, float bottom)
{
	if (left < 0) left = 0;
	if (top < 0) top = 0;
	if (right < 0) right = 0;
	if (bottom < 0) bottom = 0;

	if (fMargins != BRect(left, top, right, bottom)) {
		fMargins = BRect(left, top, right, bottom);
		Refresh();
	}
}


void
BMenu::GetItemMargins(float *left, float *top, float *right, float *bottom) const
{
	if (left) *left = fMargins.left;
	if (top) *top = fMargins.top;
	if (right) *right = fMargins.right;
	if (bottom) *bottom = fMargins.bottom;
}


BMenuItem*
BMenu::CurrentSelection() const
{
	if (Window() == NULL) return NULL;
	return (BMenuItem*)fMenuItems.ItemAt(fSelectedIndex);
}


void
BMenu::ItemInvoked(BMenuItem *item)
{
	if (fRadioMode == false || item == NULL || item->fEnabled == false) return;
	item->SetMarked(true);
}


void
BMenu::SelectItem(BMenuItem *item, bool showSubmenu, bool selectFirstItem)
{
	if (!(item == NULL || item->fMenu == this)) return;
	int32 newIndex = (item ? fMenuItems.IndexOf((void*)item) : -1);

	if (fSelectedIndex != newIndex && newIndex >= 0) {
		BWindow *win = Window();

		BMenuItem *oldItem = (BMenuItem*)fMenuItems.ItemAt(fSelectedIndex);
		BMenuItem *newItem = (BMenuItem*)fMenuItems.ItemAt(newIndex);

		int32 oldSelectedIndex = fSelectedIndex;
		fSelectedIndex = newIndex;

		if (win) win->DisableUpdates();
		if (newItem->SelectChanged()) {
			if (oldItem) oldItem->SelectChanged();
		} else {
			fSelectedIndex = oldSelectedIndex;
		}
		if (win) win->EnableUpdates();
	} else {
		BMenuItem *oldItem = (BMenuItem*)fMenuItems.ItemAt(fSelectedIndex);
		fSelectedIndex = -1;
		if (oldItem) oldItem->SelectChanged();
	}

	fTrackingIndex = -1;
}


void
BMenu::Hide()
{
	BView::Hide();
	if (IsHidden()) {
		if (fSelectedIndex >= 0) {
			BMenuItem *item = (BMenuItem*)fMenuItems.ItemAt(fSelectedIndex);
			fSelectedIndex = -1;
			if (item) item->SelectChanged();
		}

		fTrackingIndex = -1;
	}
}

