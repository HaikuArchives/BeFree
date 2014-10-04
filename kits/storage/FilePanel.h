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
 * File: FilePanel.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_FILE_PANEL_H__
#define __ETK_FILE_PANEL_H__

#include <interface/Window.h>
#include <storage/Directory.h>

typedef enum e_file_panel_mode {
	B_OPEN_PANEL,
	B_SAVE_PANEL
} e_file_panel_mode;

typedef enum e_file_panel_button {
	B_CANCEL_BUTTON,
	E_DEFAULT_BUTTON
} e_file_panel_button;

#ifdef __cplusplus /* Just for C++ */

class BFilePanelFilter
{
	public:
		virtual bool		Filter(const BEntry *entry) = 0;
};


class BFilePanel
{
	public:
		BFilePanel(e_file_panel_mode mode = B_OPEN_PANEL,
		           const BMessenger *target = NULL,
		           const char *panel_directory = NULL,
		           uint32 node_flavors = 0,
		           bool allow_multiple_selection = true,
		           const BMessage *message = NULL,
		           BFilePanelFilter *filter = NULL,
		           bool modal = false,
		           bool hide_when_done = true);
		virtual ~BFilePanel();

		void			Show();
		void			Hide();
		bool			IsShowing() const;

		// Empty functions BEGIN --- just for derivative class
		virtual void		WasHidden();
		virtual void		SelectionChanged();
		// Empty functions END

		virtual void		SendMessage(const BMessenger *msgr, BMessage *msg);

		BWindow			*Window() const;
		BMessenger		*Target() const;
		BFilePanelFilter	*Filter() const;

		e_file_panel_mode	PanelMode() const;

		void			SetTarget(const BMessenger *target);
		void			SetMessage(const BMessage *msg);

		void			SetFilter(BFilePanelFilter *filter);
		void			SetSaveText(const char *text);
		void			SetButtonLabel(e_file_panel_button btn, const char *label);

		void			SetHideWhenDone(bool state);
		bool			HidesWhenDone() const;

		void			GetPanelDirectory(BEntry *entry) const;
		void			GetPanelDirectory(BPath *path) const;
		void			GetPanelDirectory(BDirectory *directory) const;

		void			SetPanelDirectory(const BEntry *entry);
		void			SetPanelDirectory(const BDirectory *directory);
		void			SetPanelDirectory(const char *directory);

		void			Refresh();
		void			Rewind();
		status_t		GetNextSelected(BEntry *entry);

	private:
		BWindow *fWindow;
};

#endif /* __cplusplus */

#endif /* __ETK_FILE_PANEL_H__ */

