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
 * File: Menu.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_MENU_H__
#define __ETK_MENU_H__

#include <support/List.h>
#include <interface/View.h>
#include <interface/MenuItem.h>

#ifdef __cplusplus /* Just for C++ */

typedef enum e_menu_layout {
	B_ITEMS_IN_ROW = 0,
	B_ITEMS_IN_COLUMN,
	B_ITEMS_IN_MATRIX
} e_menu_layout;

class BSubmenuWindow;
class BPopUpMenu;

class BMenu : public BView
{
	public:
		BMenu(const char *title, e_menu_layout layout = B_ITEMS_IN_COLUMN);
		BMenu(const char *title, float width, float height);
		virtual ~BMenu();

		e_menu_layout		Layout() const;

		bool			AddItem(BMenuItem *item);
		bool			AddItem(BMenuItem *item, int32 index);
		bool			AddItem(BMenuItem *item, BRect frame);
		bool			AddItem(BMenu *menu);
		bool			AddItem(BMenu *menu, int32 index);
		bool			AddItem(BMenu *menu, BRect frame);
		bool			AddSeparatorItem();
		bool			RemoveItem(BMenuItem *item);
		BMenuItem*		RemoveItem(int32 index);
		bool			RemoveItem(BMenu *menu);

		BMenuItem*		ItemAt(int32 index) const;
		BMenu*			SubmenuAt(int32 index) const;
		int32			CountItems() const;
		int32			IndexOf(const BMenuItem *item) const;
		int32			IndexOf(const BMenu *menu) const;

		BMenuItem*		FindItem(uint32 command) const;
		BMenuItem*		FindItem(const char *name) const;
		BMenuItem*		FindMarked(int32 *index = NULL) const;

		BMenu*			Supermenu() const;
		BMenuItem*		Superitem() const;

		virtual status_t	SetTargetForItems(BHandler *target);
		virtual status_t	SetTargetForItems(BMessenger messenger);

		virtual void		SetEnabled(bool state);
		bool			IsEnabled() const;

		virtual void		SetRadioMode(bool state);
		bool			IsRadioMode() const;

		virtual void		SetLabelFromMarked(bool state);
		bool			IsLabelFromMarked() const;

		virtual void		AttachedToWindow();
		virtual void		DetachedFromWindow();

		virtual void		MessageReceived(BMessage *msg);
		virtual void		Draw(BRect updateRect);
		virtual void		MouseDown(BPoint where);
		virtual void		MouseUp(BPoint where);
		virtual void		MouseMoved(BPoint where, uint32 code, const BMessage *a_message);
		virtual void		KeyDown(const char *bytes, int32 numBytes);
		virtual void		KeyUp(const char *bytes, int32 numBytes);
		virtual void		Hide();

		virtual void		GetPreferredSize(float *width, float *height);

		BMenuItem*		CurrentSelection() const;
		void			SelectItem(BMenuItem *item, bool showSubmenu = false, bool selectFirstItem = false);

	protected:
		BMenu(BRect frame, const char *title, uint32 resizeMode, uint32 flags, e_menu_layout layout, bool resizeToFit);

		void			SetItemMargins(float left, float top, float right, float bottom);
		void			GetItemMargins(float *left, float *top, float *right, float *bottom) const;

		int32			FindItem(BPoint where);
		BRect			ItemFrame(int32 index) const;

		virtual void		ItemInvoked(BMenuItem *item);

	private:
		friend class BSubmenuWindow;
		friend class BMenuItem;
		friend class BPopUpMenu;

		BRect fMargins;
		bool fResizeToFit;
		e_menu_layout fLayout;
		BList fMenuItems;

		BMenuItem *fSuperitem;

		bool fRadioMode;
		bool fLabelFromMarked;
		int32 fSelectedIndex;
		int32 fTrackingIndex;
		int32 fMarkedIndex;
		bool fShowSubmenuByKeyDown;

		void Refresh();
		void SetLayout(e_menu_layout layout, float width, float height, bool resizeToFit);

		bool GetPopUpWhere(BPoint *where);
		bool PopUp(BPoint where, bool selectFirstItem);
		void ClosePopUp();
};

#endif /* __cplusplus */

#endif /* __ETK_MENU_H__ */

