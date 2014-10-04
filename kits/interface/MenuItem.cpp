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
 * File: MenuItem.cpp
 *
 * --------------------------------------------------------------------------*/

#include <ctype.h>
#include <string.h>

#include <support/String.h>
#include <support/ClassInfo.h>

#include "Window.h"
#include "Menu.h"
#include "MenuItem.h"


BMenuItem::BMenuItem(const char *label, BMessage *message, char shortcut, uint32 modifiers)
		: BArchivable(), BInvoker(message, NULL, NULL),
		fShortcut(0), fModifiers(0), fMarked(false), fEnabled(true),
		fLabel(NULL), fShortcuts(NULL), fSubmenu(NULL), fMenu(NULL)
{
	SetShortcut(shortcut, modifiers);
	if (label) fLabel = EStrdup(label);
}


BMenuItem::BMenuItem(BMenu *menu, BMessage *message)
		: BArchivable(), BInvoker(message, NULL, NULL),
		fShortcut(0), fModifiers(0), fMarked(false), fEnabled(true),
		fLabel(NULL), fShortcuts(NULL), fSubmenu(NULL), fMenu(NULL)
{
	if (menu) {
		if (menu->fSuperitem == NULL) {
			fSubmenu = menu;
			if (menu->Name() != NULL) fLabel = EStrdup(menu->Name());
			menu->fSuperitem = this;
			if (menu->BView::IsEnabled() == false) fEnabled = false;
		} else {
			ETK_ERROR("[INTERFACE]: %s --- The menu already attached to other item.", __PRETTY_FUNCTION__);
		}
	}
}


BMenuItem::~BMenuItem()
{
	if (fMenu) {
		ETK_WARNING("[INTERFACE]: %s --- Item still attach to menu, detaching from menu automatically.", __PRETTY_FUNCTION__);
		if (fMenu->RemoveItem(this) == false)
			ETK_ERROR("[INTERFACE]: %s --- Detaching from menu failed.", __PRETTY_FUNCTION__);
	}

	if (fLabel) delete[] fLabel;
	if (fShortcuts) delete[] fShortcuts;

	if (fSubmenu) {
		fSubmenu->fSuperitem = NULL;
		fSubmenu->ClosePopUp();
		delete fSubmenu;
	}
}


void
BMenuItem::SetLabel(const char *label)
{
	if ((fLabel == NULL || strlen(fLabel) == 0) && (label == NULL || strlen(label) == 0)) return;
	if (!((fLabel == NULL || strlen(fLabel) == 0) || (label == NULL || strlen(label) == 0) || strcmp(fLabel, label) != 0)) return;

	BRect oldFrame = Frame();

	if (fLabel) {
		delete[] fLabel;
		fLabel = NULL;
	}

	if (label) fLabel = EStrdup(label);

	if (fMenu == NULL) return;

	if (fMenu->fResizeToFit)
		fMenu->Refresh();
	else
		fMenu->Invalidate(oldFrame | Frame());
}


void
BMenuItem::SetEnabled(bool state)
{
	if (fEnabled != state) {
		fEnabled = state;
		if (!(fSubmenu == NULL || fSubmenu->BView::IsEnabled() != state)) fSubmenu->SetEnabled(state);
		if (fMenu) fMenu->Invalidate(Frame());
	}
}


void
BMenuItem::SetMarked(bool state)
{
	if (fMarked != state || !(fMenu == NULL || fMenu->fRadioMode == false)) {
		fMarked = state;
		if (fMenu) {
			int32 index = fMenu->IndexOf(this);

			if (fMenu->fRadioMode) {
				if (fMarked) {
					if (fMenu->fMarkedIndex != index) {
						int32 oldIndex = fMenu->fMarkedIndex;
						fMenu->fMarkedIndex = index;
						if (oldIndex >= 0) fMenu->Invalidate(fMenu->ItemFrame(oldIndex));
					}
					if (fMenu->fLabelFromMarked && fMenu->fSuperitem) fMenu->fSuperitem->SetLabel(Label());
				} else if (fMenu->fMarkedIndex == index) {
					fMenu->fMarkedIndex = -1;
					fMenu->Invalidate(fMenu->ItemFrame(index));
					if (fMenu->fLabelFromMarked && fMenu->fSuperitem) fMenu->fSuperitem->SetLabel(fMenu->Name());
				}
			}

			fMenu->Invalidate(fMenu->ItemFrame(index));
		}
	}
}


void
BMenuItem::SetShortcut(char ch, uint32 modifiers)
{
	if (fShortcut != ch || fModifiers != modifiers) {
		// TODO: update shortcut when window attached
		BRect oldFrame = Frame();

		fShortcut = ch;
		fModifiers = modifiers;

		if (fShortcuts) delete[] fShortcuts;
		fShortcuts = NULL;

		if (fShortcut != 0 && fModifiers != 0) {
			BString str;
			if (fModifiers &B_CONTROL_KEY) str << "Ctrl+";
			if (fModifiers &B_SHIFT_KEY) str << "Shift+";
			if (fModifiers &B_COMMAND_KEY) str << "Alt+";

			if (fModifiers &B_FUNCTIONS_KEY) {
				switch (fShortcut) {
					case B_F1_KEY:
						str << "F1";
						break;
					case B_F2_KEY:
						str << "F2";
						break;
					case B_F3_KEY:
						str << "F3";
						break;
					case B_F4_KEY:
						str << "F4";
						break;
					case B_F5_KEY:
						str << "F5";
						break;
					case B_F6_KEY:
						str << "F6";
						break;
					case B_F7_KEY:
						str << "F7";
						break;
					case B_F8_KEY:
						str << "F8";
						break;
					case B_F9_KEY:
						str << "F9";
						break;
					case B_F10_KEY:
						str << "F10";
						break;
					case B_F11_KEY:
						str << "F11";
						break;
					case B_F12_KEY:
						str << "F12";
						break;
					case B_PRINT_KEY:
						str << "Print";
						break;
					case B_SCROLL_KEY:
						str << "Scroll";
						break;
					case B_PAUSB_KEY:
						str << "Pause";
						break;
					default:
						str.MakeEmpty();
						break; // here don't support
				}
			} else switch (fShortcut) {
					case B_ENTER:
						str << "Enter";
						break;
					case B_BACKSPACE:
						str << "Backspace";
						break;
					case B_SPACE:
						str << "Space";
						break;
					case B_TAB:
						str << "Tab";
						break;
					case B_ESCAPE:
						str << "Esc";
						break;
					case B_LEFT_ARROW:
						str << "Left";
						break;
					case B_RIGHT_ARROW:
						str << "Right";
						break;
					case B_UP_ARROW:
						str << "Up";
						break;
					case B_DOWN_ARROW:
						str << "Down";
						break;
					case B_INSERT:
						str << "Insert";
						break;
					case B_DELETE:
						str << "Delete";
						break;
					case B_HOME:
						str << "Home";
						break;
					case B_END:
						str << "End";
						break;
					case B_PAGE_UP:
						str << "PageUp";
						break;
					case B_PAGE_DOWN:
						str << "PageDown";
						break;
					default:
						str.Append((char)toupper(fShortcut), 1);
				}

			if (str.Length() > 0) fShortcuts = EStrdup(str.String());
		}

		if (fMenu == NULL) return;

		if (fMenu->fResizeToFit)
			fMenu->Refresh();
		else
			fMenu->Invalidate(oldFrame | Frame());
	}
}


const char*
BMenuItem::Label() const
{
	return fLabel;
}


bool
BMenuItem::IsEnabled() const
{
	return fEnabled;
}


bool
BMenuItem::IsMarked() const
{
	return fMarked;
}


char
BMenuItem::Shortcut(uint32 *modifiers) const
{
	if (modifiers) *modifiers = fModifiers;
	return fShortcut;
}


BMenu*
BMenuItem::Submenu() const
{
	return fSubmenu;
}


BMenu*
BMenuItem::Menu() const
{
	return fMenu;
}


BRect
BMenuItem::Frame() const
{
	if (!fMenu) return BRect();
	int32 index = fMenu->IndexOf(this);
	return fMenu->ItemFrame(index);
}


bool
BMenuItem::IsSelected() const
{
	if (!fMenu) return false;
	return(fMenu->CurrentSelection() == this);
}


void
BMenuItem::GetContentSize(float *width, float *height) const
{
	if (!width && !height || !fMenu) return;

	BFont font(be_plain_font);

	if (width) {
		*width = Label() ? (float)ceil((double)font.StringWidth(Label())) : 0;

		if (fMenu->Layout() == B_ITEMS_IN_COLUMN) {
			if (fSubmenu) {
				font_height fontHeight;
				font.GetHeight(&fontHeight);
				*width += (float)ceil((double)(fontHeight.ascent + fontHeight.descent) / 2.f) + 10;
			} else if (fShortcuts)
				*width += (float)ceil((double)font.StringWidth(fShortcuts)) + 30;

			*width += 20;
		} else {
			*width += 10;
		}
	}

	if (height) {
		font_height fontHeight;
		font.GetHeight(&fontHeight);
		*height = (float)ceil((double)(fontHeight.ascent + fontHeight.descent));
		*height += 4;
	}
}


void
BMenuItem::DrawContent()
{
	if (fMenu == NULL || fMenu->Window() == NULL) return;

	fMenu->PushState();

	BFont font(be_plain_font);
	font_height fontHeight;
	font.GetHeight(&fontHeight);
	fMenu->SetFont(&font, B_FONT_ALL);

	BRect rect = Frame().InsetByCopy(2, 2);
	if (fMenu->Layout() == B_ITEMS_IN_COLUMN) rect.left += 16;
	if (fMenu->PenLocation().x > rect.left && fMenu->PenLocation().x < rect.right) rect.left = fMenu->PenLocation().x;

	float sHeight = fontHeight.ascent + fontHeight.descent;
	BPoint location;
	if (fMenu->Layout() == B_ITEMS_IN_COLUMN)
		location.x = rect.left;
	else
		location.x = rect.Center().x - font.StringWidth(Label()) / 2.f;
	location.y = rect.Center().y - sHeight / 2.f;
	location.y += fontHeight.ascent + 1;

	rgb_color bkColor, textColor;

	if (fEnabled && fMenu->IsEnabled()) {
		if (IsSelected()) {
			bkColor = ui_color(B_MENU_SELECTED_BACKGROUND_COLOR);
			textColor = ui_color(B_MENU_SELECTED_ITEM_TEXT_COLOR);
		} else {
			bkColor = ui_color(B_MENU_BACKGROUND_COLOR);
			textColor = ui_color(B_MENU_ITEM_TEXT_COLOR);
		}
	} else {
		bkColor = ui_color(B_MENU_BACKGROUND_COLOR);
		if (fMenu->IsEnabled() == false) bkColor.mix(0, 0, 0, 20);

		textColor = ui_color(B_MENU_ITEM_TEXT_COLOR);
		rgb_color color = bkColor;
		color.alpha = 127;
		textColor.mix(color);
	}

	fMenu->SetDrawingMode(B_OP_COPY);
	fMenu->SetHighColor(textColor);
	fMenu->SetLowColor(bkColor);
	fMenu->DrawString(Label(), location);

	if (fMenu->Layout() == B_ITEMS_IN_COLUMN) {
		if (fSubmenu != NULL) {
			BRect r = Frame().InsetByCopy(5, 2);
			BPoint pt1, pt2, pt3;
			pt1.x = r.right;
			pt1.y = r.Center().y;
			pt2.x = pt3.x = pt1.x - (float)ceil((double)(fontHeight.ascent + fontHeight.descent) / 2.f);
			pt2.y = pt1.y - (float)ceil((double)(fontHeight.ascent + fontHeight.descent) / 3.5f);
			pt3.y = pt1.y + (float)ceil((double)(fontHeight.ascent + fontHeight.descent) / 3.5f);
			fMenu->FillTriangle(pt1, pt2, pt3);
		} else if (fShortcuts != NULL) {
			location.x = rect.right - font.StringWidth(fShortcuts);
			fMenu->DrawString(fShortcuts, location);
		}
	}

	fMenu->PopState();
}


void
BMenuItem::Draw()
{
	if (fMenu == NULL || fMenu->Window() == NULL || fMenu->IsVisible() == false) return;

	int32 index = fMenu->IndexOf(this);
	BRect frame = fMenu->ItemFrame(index);
	if (index < 0 || frame.IsValid() == false) return;

	rgb_color bkColor, textColor;

	if (fEnabled && fMenu->IsEnabled()) {
		if (IsSelected()) {
			bkColor = ui_color(B_MENU_SELECTED_BACKGROUND_COLOR);
			textColor = ui_color(B_MENU_SELECTED_ITEM_TEXT_COLOR);
		} else {
			bkColor = ui_color(B_MENU_BACKGROUND_COLOR);
			textColor = ui_color(B_MENU_ITEM_TEXT_COLOR);
		}
	} else {
		bkColor = ui_color(B_MENU_BACKGROUND_COLOR);
		if (fMenu->IsEnabled() == false) bkColor.mix(0, 0, 0, 20);

		textColor = ui_color(B_MENU_ITEM_TEXT_COLOR);
		rgb_color color = bkColor;
		color.alpha = 127;
		textColor.mix(color);
	}

	fMenu->PushState();
	fMenu->SetDrawingMode(B_OP_COPY);
	fMenu->SetPenSize(1);
	fMenu->SetHighColor(bkColor);
	fMenu->FillRect(frame);

	if (fEnabled && fMenu->IsEnabled() && IsSelected()) {
		fMenu->SetHighColor(ui_color(B_MENU_SELECTED_BORDER_COLOR));
		fMenu->StrokeRect(frame);
	}

	frame.InsetBy(2, 2);

	bool drawMarked = (fMenu->fRadioMode ? fMenu->fMarkedIndex == index : fMarked);

	if (fMenu->Layout() == B_ITEMS_IN_COLUMN) {
		if (drawMarked) {
			fMenu->SetHighColor(textColor);
			BRect rect = frame;
			rect.right = rect.left + 15;
			rect.InsetBy(3, 3);
			fMenu->MovePenTo(rect.LeftTop() + BPoint(0, rect.Height() / 2.f));
			fMenu->StrokeLine(rect.LeftBottom() + BPoint(rect.Width() / 2.f, 0));
			fMenu->StrokeLine(rect.RightTop());
		}

		frame.left += 16;
	} else {
		// TODO: draw marked
	}

	fMenu->MovePenTo(frame.LeftTop());
	fMenu->ConstrainClippingRegion(frame);

	DrawContent();

	fMenu->PopState();

	Highlight(IsSelected());
}


void
BMenuItem::Highlight(bool on)
{
}


void
BMenuItem::ShowSubmenu(bool selectFirstItem)
{
	if (fSubmenu == NULL) return;
	if (fSubmenu->Window() != NULL) {
		if (!(fMenu == NULL || fMenu->Window() == NULL)) fMenu->Window()->SendBehind(fSubmenu->Window());
		return;
	}

	BPoint where;
	if (fSubmenu->GetPopUpWhere(&where) == false) return;

	if (fSubmenu->PopUp(where, selectFirstItem) && Message() != NULL) {
		BMessage aMsg = *(Message());

		aMsg.AddInt64("when", e_real_time_clock_usecs());
		aMsg.AddPointer("source", this);
		if (fMenu) aMsg.AddInt32("index", fMenu->IndexOf(this));

		BInvoker::Invoke(&aMsg);
	}
}


bool
BMenuItem::SelectChanged()
{
	if (fMenu == NULL) return false;

	if (fMenu->Window()) {
		if (fEnabled) fMenu->Invalidate(Frame());
		if (fSubmenu) {
			if (IsSelected() &&
			        !(fMenu->Window()->CurrentMessage() == NULL ||
			          fMenu->Window()->CurrentMessage()->what != B_MOUSE_MOVED))
				ShowSubmenu(true);
			else if (!IsSelected())
				fSubmenu->ClosePopUp();
		}
	}

	return fEnabled;
}


status_t
BMenuItem::Invoke(const BMessage *msg)
{
	if (!msg && Message() == NULL) return B_ERROR;

	BMessage aMsg = (msg ? *msg : *(Message()));

	aMsg.AddInt64("when", e_real_time_clock_usecs());
	aMsg.AddPointer("source", this);
	if (fMenu) aMsg.AddInt32("index", fMenu->IndexOf(this));

	return BInvoker::Invoke(&aMsg);
}


BMenuSeparatorItem::BMenuSeparatorItem()
		: BMenuItem(NULL, NULL, 0, 0)
{
}


BMenuSeparatorItem::~BMenuSeparatorItem()
{
}


void
BMenuSeparatorItem::GetContentSize(float *width, float *height) const
{
	if (Menu() == NULL || (!width && !height)) return;

	if (width) *width = 4;
	if (height) *height = 4;
}


bool
BMenuSeparatorItem::SelectChanged()
{
	return false;
}


void
BMenuSeparatorItem::Draw()
{
	BRect frame = Frame();
	if (Menu() == NULL || Menu()->Window() == NULL || !frame.IsValid()) return;

	rgb_color bkColor;

	if (IsEnabled() && Menu()->IsEnabled()) {
		bkColor = ui_color(B_MENU_BACKGROUND_COLOR);
	} else {
		bkColor = ui_color(B_MENU_BACKGROUND_COLOR);
		if (Menu()->IsEnabled() == false) bkColor.mix(0, 0, 0, 20);
	}

	Menu()->PushState();

	Menu()->SetDrawingMode(B_OP_COPY);
	Menu()->SetPenSize(1);
	Menu()->SetHighColor(bkColor);
	Menu()->FillRect(frame);

	rgb_color shinerColor = Menu()->ViewColor();
	rgb_color darkerColor = shinerColor;
	shinerColor.mix(255, 255, 255, 100);
	darkerColor.mix(0, 0, 0, 100);

	if (Menu()->Layout() == B_ITEMS_IN_ROW || (Menu()->Layout() == B_ITEMS_IN_MATRIX && frame.Height() > frame.Width())) {
		Menu()->PushState();
		Menu()->ConstrainClippingRegion(frame);
		Menu()->SetDrawingMode(B_OP_COPY);
		Menu()->SetPenSize(1);
		Menu()->SetHighColor(shinerColor);
		frame.InsetBy(0, 2);
		BPoint pt1 = frame.Center();
		pt1.y = frame.top + 1;
		BPoint pt2 = pt1;
		pt2.y = frame.bottom - 1;
		Menu()->StrokeLine(pt1, pt2);
		pt1.x += 1;
		pt2.x += 1;
		Menu()->SetHighColor(darkerColor);
		Menu()->StrokeLine(pt1, pt2);
		Menu()->PopState();
	} else if (Menu()->Layout() == B_ITEMS_IN_COLUMN || (Menu()->Layout() == B_ITEMS_IN_MATRIX && frame.Width() >= frame.Height())) {
		Menu()->PushState();
		Menu()->ConstrainClippingRegion(frame);
		Menu()->SetDrawingMode(B_OP_COPY);
		Menu()->SetPenSize(1);
		Menu()->SetHighColor(shinerColor);
		frame.InsetBy(2, 0);
		BPoint pt1 = frame.Center();
		pt1.x = frame.left + 1;
		BPoint pt2 = pt1;
		pt2.x = frame.right - 1;
		Menu()->StrokeLine(pt1, pt2);
		pt1.y += 1;
		pt2.y += 1;
		Menu()->SetHighColor(darkerColor);
		Menu()->StrokeLine(pt1, pt2);
		Menu()->PopState();
	}

	Menu()->PopState();
}

