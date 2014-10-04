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
 * File: MenuBar.cpp
 *
 * --------------------------------------------------------------------------*/

#include <support/ClassInfo.h>

#include "Window.h"
#include "MenuBar.h"
#include "MenuField.h"


BMenuBar::BMenuBar(BRect frame, const char *title, uint32 resizeMode, e_menu_layout layout, bool resizeToFit)
		: BMenu(frame, title, resizeMode, B_WILL_DRAW, layout, resizeToFit), fBorder(B_BORDER_FRAME)
{
	SetEventMask(B_POINTER_EVENTS);
}


BMenuBar::~BMenuBar()
{
}


void
BMenuBar::MakeFocus(bool state)
{
	BMenuField *parent = cast_as(Parent(), BMenuField);
	if (!(state == false || parent == NULL || parent->MenuBar() != this)) return;

	BMenu::MakeFocus(state);
	if (!IsFocus() && CurrentSelection() != NULL) SelectItem(NULL);
}


void
BMenuBar::MouseDown(BPoint where)
{
	BMenu::MouseDown(where);

	if (Window() == NULL) return;

	BMenuItem *item = CurrentSelection();
	if (item != NULL) {
		if (VisibleBounds().Contains(where) == false) {
			ConvertToScreen(&where);
			BMenu *submenu;
			while (true) {
				submenu = (item ? item->Submenu() : NULL);
				if (submenu == NULL || submenu->Window() == NULL || submenu->Window()->Frame().Contains(where)) {
					if (submenu == NULL || submenu->Window() == NULL) {
						SelectItem(NULL);
						MakeFocus(false);
					}
					break;
				}

				item = submenu->CurrentSelection();
			}
		} else {
			bool found = false;
			for (int32 i = 0; i < CountItems(); i++)
				if (ItemFrame(i).Contains(where)) found = true;
			if (found == false) {
				SelectItem(NULL);
				MakeFocus(false);
			}
		}
	}

	if (CurrentSelection() != NULL) MakeFocus(true);
}


void
BMenuBar::MouseUp(BPoint where)
{
	BMenu::MouseUp(where);

	if (Window() == NULL) return;

	BMenuItem *item = CurrentSelection();
	if (item != NULL) {
		if (VisibleBounds().Contains(where) == false) {
			ConvertToScreen(&where);
			BMenu *submenu;
			while (true) {
				submenu = (item ? item->Submenu() : NULL);
				if (submenu == NULL || submenu->Window() == NULL || submenu->Window()->Frame().Contains(where)) {
					if (submenu == NULL || submenu->Window() == NULL) {
						SelectItem(NULL);
						MakeFocus(false);
					}
					break;
				}

				item = submenu->CurrentSelection();
			}
		} else {
			bool found = false;
			for (int32 i = 0; i < CountItems(); i++)
				if (ItemFrame(i).Contains(where)) found = true;
			if (found == false) {
				SelectItem(NULL);
				MakeFocus(false);
			}
		}
	}

	if (CurrentSelection() != NULL) MakeFocus(true);
}


void
BMenuBar::ItemInvoked(BMenuItem *item)
{
	BMenu::ItemInvoked(item);
	SelectItem(NULL);
	MakeFocus(false);
}


void
BMenuBar::MouseMoved(BPoint where, uint32 code, const BMessage *a_message)
{
	if (CurrentSelection() == NULL) return;
	BMenu::MouseMoved(where, code, a_message);
}


void
BMenuBar::KeyDown(const char *bytes, int32 numBytes)
{
	if (CurrentSelection() == NULL) return;
	BMenu::KeyDown(bytes, numBytes);
}


void
BMenuBar::KeyUp(const char *bytes, int32 numBytes)
{
	if (CurrentSelection() == NULL) return;
	BMenu::KeyUp(bytes, numBytes);
}


void
BMenuBar::MessageReceived(BMessage *msg)
{
	BMenu::MessageReceived(msg);

	if (Window() == NULL) return;

	switch (msg->what) {
		case B_OBSERVER_NOTICE_CHANGE: {
			uint32 what;
			if (msg->FindInt32(B_OBSERVE_ORIGINAL_WHAT, (int32*)&what) == false ||
			        !(what == B_MINIMIZED || what == B_WINDOW_ACTIVATED)) break;

			if (Window()->IsActivate() == false || Window()->IsHidden() || Window()->IsMinimized()) {
				SelectItem(NULL);
				MakeFocus(false);
			}
		}
		break;

		case _MENU_EVENT_: {
			BMenuItem *item = NULL;
			if (msg->FindPointer("source", (void**)&item) == false || item == NULL) break;
			SelectItem(NULL);
			MakeFocus(false);
		}
		break;

		default:
			break;
	}
}


void
BMenuBar::Draw(BRect updateRect)
{
	if (!IsVisible()) return;

	BMenu::Draw(updateRect);
	if (fBorder == B_BORDER_NONE) return;

	rgb_color shineColor = ui_color(B_SHINE_COLOR);
	rgb_color shadowColor = ui_color(B_SHADOW_COLOR);

	if (!IsEnabled()) {
		shineColor.disable(ViewColor());
		shadowColor.disable(ViewColor());
	}

	switch (fBorder) {
		case B_BORDER_FRAME: {
			BRect rect = Frame().OffsetToSelf(B_ORIGIN);

			PushState();
			SetHighColor(shadowColor);
			StrokeRect(rect);
			SetHighColor(shineColor);
			StrokeLine(rect.LeftBottom(), rect.LeftTop());
			StrokeLine(rect.RightTop());
			PopState();
		}
		break;

		default:
			// TODO
			break;
	}
}


void
BMenuBar::SetBorder(e_menu_bar_border border)
{
	if (fBorder != border) {
		fBorder = border;
		Invalidate();
	}
}


e_menu_bar_border
BMenuBar::Border() const
{
	return fBorder;
}

