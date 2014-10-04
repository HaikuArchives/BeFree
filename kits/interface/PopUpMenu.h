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
 * File: PopUpMenu.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_POP_UP_MENU_H__
#define __ETK_POP_UP_MENU_H__

#include <support/Locker.h>
#include <interface/Menu.h>
#include <interface/Window.h>

#ifdef __cplusplus /* Just for C++ */

class BPopUpMenu : public BMenu
{
	public:
		BPopUpMenu(const char *title,
		           bool radioMode = true,
		           bool labelFromMarked = true,
		           e_menu_layout layout = B_ITEMS_IN_COLUMN);
		virtual ~BPopUpMenu();

		// "could_proxy_when_sync" must be "true" when it called synchronously from looper of BApplication!
		BMenuItem*	Go(BPoint where,
		              bool delivers_message_when_sync = false,
		              bool open_anyway = false,
		              bool asynchronous = false,
		              bool could_proxy_when_sync = false);

		void		SetAsyncAutoDestruct(bool state);
		bool		AsyncAutoDestruct() const;

		virtual void	MessageReceived(BMessage *msg);
		virtual void	MouseUp(BPoint where);

	private:
		friend class BMenu;

		BMenuItem *fSelectedItem;
		bool fAutoDestruct;
		bool IsPopUpByGo() const;
};

#endif /* __cplusplus */

#endif /* __ETK_POP_UP_MENU_H__ */

