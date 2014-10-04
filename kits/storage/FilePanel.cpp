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
 * File: FilePanel.cpp
 *
 * --------------------------------------------------------------------------*/

#include <time.h>

#include <kernel/Kernel.h>
#include <support/Autolock.h>
#include <support/ClassInfo.h>
#include <app/Application.h>
#include <interface/Button.h>
#include <interface/MenuField.h>
#include <interface/ListView.h>
#include <interface/ScrollView.h>
#include <interface/TextControl.h>
#include <interface/StringView.h>
#include <interface/Bitmap.h>
#include <render/Pixmap.h>

#include "VolumeRoster.h"
#include "FindDirectory.h"
#include "FilePanel.h"

#define ICON_WIDTH	16
#define ICON_HEIGHT	16

#include "icons/volume.xpm"
#include "icons/folder.xpm"
#include "icons/file.xpm"

#define MSG_PANEL_GET_DIR	'getd'
#define MSG_PANEL_SELECTED	'sele'
#define MSG_PANEL_SET_DIR	'setd'
#define MSG_PANEL_DONE		'done'

#define MSG_PANEL_SELECT_ALL	'sela'
#define MSG_PANEL_DESELECT_ALL	'seln'

#define MSG_PANEL_TOGGLE_HIDDEN	'thid'

#define TEXT_OPEN_FILE		"Open File"
#define TEXT_SAVE_FILE		"Save File"
#define TEXT_FILE		"File"
#define TEXT_OPEN		"Open"
#define TEXT_SAVE		"Save"
#define TEXT_SELECT_ALL		"Select all"
#define TEXT_DESELECT_ALL	"Deselect all"
#define TEXT_SHOW_HIDDEN_FILES	"Show hidden files"
#define TEXT_CANCEL		"Cancel"
#define TEXT_LOCATION		"Location:"
#define TEXT_NAME		"Name"
#define TEXT_SIZE		"Size"
#define TEXT_MODIFIED		"Modified"
#define TEXT_ITEMS		"items"
#define TEXT_ALL_VOLUMES	"All volumes"
#define TEXT_NO_NAME		"No name"
#define TEXT_OTHER_DIRECTORYIES	"Other directories"
#define TEXT_BYTES		"bytes"
#define TEXT_HOME		"Home"

#define TEXT_NOTICE_EMPTY	"[Empty]"
#define TEXT_NOTICE_VOLUME	"[Volume]"

class BFilePanelView;
class BFilePanelWindow;


class _LOCAL BFilePanelLabel : public BStringView
{
	public:
		BFilePanelLabel(BRect frame, const char *name, const char *text, uint32 resizeMode);

		virtual void	Draw(BRect updateRect);
		virtual void	GetPreferredSize(float *width, float *height);
};


class _LOCAL BFilePanelListItem : public BListItem
{
	public:
		BFilePanelListItem(const char *path, BFilePanelView *panel_view, e_dev_t dev = -1);
		virtual ~BFilePanelListItem();

		const char	*Path() const;
		const char	*Leaf() const;

		bool		IsVolume() const;
		bool		IsDirectory() const;

		int64		Size() const;
		bigtime_t	ModifiedTime() const;
		BFilePanelView	*PanelView() const;

	private:
		BPath fPath;
		char *fLeaf;

		int32 fFlags;
		int64 fSize;
		bigtime_t fModifiedTime;

		BFilePanelView *fPanelView;

		virtual void	DrawItem(BView *owner, BRect itemRect, bool drawEverything);
		virtual void	Update(BView *owner, const BFont *font);
};


class _LOCAL BFilePanelListView : public BListView
{
	public:
		BFilePanelListView(BRect frame, const char *name, list_view_type type);

		void			SortItems(int (*sort_func)(const BFilePanelListItem**, const BFilePanelListItem**),
		                 bool clear_position);

		virtual void		SelectionChanged();
		virtual status_t	Invoke(const BMessage *msg);
		virtual void		KeyDown(const char *bytes, int32 numBytes);
};


class _LOCAL BFilePanelTitleView : public BView
{
	public:
		BFilePanelTitleView(BRect parent_bounds);

		virtual void	Draw(BRect updateRect);
		virtual void	GetPreferredSize(float *width, float *height);
		virtual void	ScrollTo(BPoint where);
		virtual void	MouseUp(BPoint where);
};


class _LOCAL BFilePanelView : public BView
{
	public:
		BFilePanelView(BRect frame, bool allow_multiple_selection);
		virtual ~BFilePanelView();

		void		AddColumn(const char *name, float width,
		                void (*draw_func)(BView*, BRect, BFilePanelListItem*),
		                int (*sort_func)(const BFilePanelListItem**, const BFilePanelListItem**));
		void		RemoveColumn(int32 index);
		void		SwapColumns(int32 indexA, int32 indexB);

		int32		CountColumns() const;
		const char	*GetNameOfColumn(int32 index) const;
		float		GetWidthOfColumn(int32 index) const;

		void		DrawItem(BView *owner, BRect itemRect, BFilePanelListItem*);
		void		SortItems(int32 nColumn);
		int		GetSortIndex(int32 *index) const;

		virtual void	FrameResized(float new_width, float new_height);

	private:
		struct column_data {
			char *name;
			float width;
			void (*draw_func)(BView*, BRect, BFilePanelListItem*);
			int (*sort_func)(const BFilePanelListItem**, const BFilePanelListItem**);
		};

		BList fColumns;
		BFilePanelTitleView *fTitleView;
		BFilePanelListView *fListView;
		BScrollBar *fHSB;
		BScrollBar *fVSB;
		int32 fSort;
};


class _LOCAL BFilePanelWindow : public BWindow
{
	public:
		BFilePanelWindow(BFilePanel *panel,
		                 e_file_panel_mode mode,
		                 uint32 node_flavors,
		                 bool modal,
		                 bool allow_multiple_selection);
		virtual ~BFilePanelWindow();

		virtual void		MessageReceived(BMessage *msg);
		virtual bool		QuitRequested();

		BFilePanel		*Panel() const;
		BMessenger		*Target() const;

		e_file_panel_mode	PanelMode() const;

		void			SetTarget(const BMessenger *target);
		void			SetMessage(const BMessage *msg);

		void			Refresh();
		void			SetPanelDirectory(const char *path);
		const char		*PanelDirectory() const;

		void			SetFilter(BFilePanelFilter *filter);
		void			SetSaveText(const char *text);
		void			SetButtonLabel(e_file_panel_button btn, const char *label);
		void			SetHideWhenDone(bool state);

		void			Rewind();
		status_t		GetNextSelected(BEntry *entry);

		void			RefreshDirMenu();

	private:
		friend class BFilePanel;
		friend class BFilePanelListView;

		BFilePanel *fPanel;

		BPath fPath;

		e_file_panel_mode fMode;
		uint32 fNodeFlavors;
		BMessenger *fTarget;
		BMessage *fMessage;
		BFilePanelFilter *fFilter;
		bool fHidesWhenDone;

		BFilePanelView *fPanelView;
		BMenu *fDirMenu;

		int32 fSelIndex;
		bool fShowHidden;

		static bool	RefreshCallback(const char *path, void *data);
		virtual bool	IsDependsOnOthersWhenQuitRequested() const;
};


BFilePanelLabel::BFilePanelLabel(BRect frame, const char *name, const char *text, uint32 resizeMode)
		: BStringView(frame, name, text, resizeMode, B_WILL_DRAW)
{
	SetAlignment(B_ALIGN_CENTER);
	SetVerticalAlignment(B_ALIGN_MIDDLE);
}


void
BFilePanelLabel::Draw(BRect updateRect)
{
	BStringView::Draw(updateRect);

	PushState();
	SetDrawingMode(B_OP_COPY);
	SetPenSize(0);
	SetHighColor(ui_color(B_SHINE_COLOR));
	StrokeRect(Bounds());
	SetHighColor(ui_color(B_SHADOW_COLOR));
	StrokeLine(Bounds().LeftBottom(), Bounds().RightBottom());
	StrokeLine(Bounds().RightTop());
	PopState();
}


void
BFilePanelLabel::GetPreferredSize(float *width, float *height)
{
	BStringView::GetPreferredSize(width, height);
	if (width != NULL) *width += 2;
	if (height != NULL) *height += 2;
}


BFilePanelListItem::BFilePanelListItem(const char *path, BFilePanelView *panel_view, e_dev_t dev)
		: BListItem(), fPath(path), fSize(0), fModifiedTime(0), fPanelView(panel_view)
{
	if (dev >= 0) {
		BVolume vol(dev);
		BString name;
		vol.GetName(&name);
		fFlags = 2;
		fLeaf = b_strdup(name.String());
	} else {
		BEntry aEntry(path);
		fFlags = (aEntry.IsDirectory() ? 1 : 0);

		fLeaf = b_strdup(fPath.Leaf());

		if (fFlags == 0) aEntry.GetSize(&fSize);
		aEntry.GetModificationTime(&fModifiedTime);
	}
}


BFilePanelListItem::~BFilePanelListItem()
{
	if (fLeaf) free(fLeaf);
}


void
BFilePanelListItem::DrawItem(BView *owner, BRect itemRect, bool drawEverything)
{
	rgb_color bkColor = (IsSelected() ? ui_color(B_DOCUMENT_HIGHLIGHT_COLOR): owner->ViewColor());
	rgb_color fgColor = ui_color(B_DOCUMENT_TEXT_COLOR);

	if (!IsEnabled()) {
		bkColor.disable(owner->ViewColor());
		fgColor.disable(owner->ViewColor());
	}

	if (IsSelected() || !IsEnabled()) {
		owner->SetHighColor(bkColor);
		owner->FillRect(itemRect);

		owner->SetHighColor(fgColor);
		owner->SetLowColor(bkColor);
	} else {
		owner->SetHighColor(fgColor);
		owner->SetLowColor(owner->ViewColor());
	}

	DrawLeader(owner, &itemRect);
	if (itemRect.IsValid() == false) return;

	fPanelView->DrawItem(owner, itemRect, this);
}


void
BFilePanelListItem::Update(BView *owner, const BFont *font)
{
	font_height fontHeight;
	font->GetHeight(&fontHeight);
	SetHeight(max_c(fontHeight.ascent + fontHeight.descent, ICON_HEIGHT) + 4);

	float width = 0;
	GetLeaderSize(&width, NULL);
	for (int32 i = 0; i < fPanelView->CountColumns(); i++) width += fPanelView->GetWidthOfColumn(i);
	SetWidth(width);
}


const char*
BFilePanelListItem::Path() const
{
	return fPath.Path();
}


const char*
BFilePanelListItem::Leaf() const
{
	return fLeaf;
}


bool
BFilePanelListItem::IsDirectory() const
{
	return(fFlags == 1);
}


bool
BFilePanelListItem::IsVolume() const
{
	return(fFlags == 2);
}


int64
BFilePanelListItem::Size() const
{
	return fSize;
}


bigtime_t
BFilePanelListItem::ModifiedTime() const
{
	return fModifiedTime;
}


BFilePanelView*
BFilePanelListItem::PanelView() const
{
	return fPanelView;
}


BFilePanelListView::BFilePanelListView(BRect frame, const char *name, list_view_type type)
		: BListView(frame, name, type, B_FOLLOW_ALL)
{
}


void
BFilePanelListView::SelectionChanged()
{
	BFilePanelWindow *win = (BFilePanelWindow*)Window();
	if (win) {
		BButton *btn = (BButton*)win->FindView("default button");
		BMenuBar *menuBar = (BMenuBar*)win->FindView("MenuBar");
		BMenu *menu = menuBar->FindItem(TEXT_FILE)->Submenu();
		BMenuItem *menuItem = menu->FindItem(win->PanelMode() == B_OPEN_PANEL ? TEXT_OPEN : TEXT_SAVE);
		BFilePanelListItem *item;

		for (int32 i = 0; (item = (BFilePanelListItem*)ItemAt(CurrentSelection(i))) != NULL; i++) {
			if (win->fNodeFlavors == B_DIRECTORY_NODE && !item->IsDirectory()) break;
			if (win->fNodeFlavors == B_FILB_NODE && item->IsDirectory()) break;
		}

		btn->SetEnabled(item == NULL);
		menuItem->SetEnabled(item == NULL);

		win->Panel()->SelectionChanged();
	}
}


status_t
BFilePanelListView::Invoke(const BMessage *msg)
{
	const BMessage *message = (msg ? msg : Message());
	if (!message) return B_BAD_VALUE;

	BMessage aMsg(*message);
	aMsg.AddInt32("index", Position());

	return BInvoker::Invoke(&aMsg);
}


void
BFilePanelListView::KeyDown(const char *bytes, int32 numBytes)
{
	int32 oldPos = Position();
	BListView::KeyDown(bytes, numBytes);
	if (oldPos != Position()) ScrollToItem(Position());
}


void
BFilePanelListView::SortItems(int (*sort_func)(const BFilePanelListItem**, const BFilePanelListItem**),
                              bool clear_position)
{
	if (clear_position) SetPosition(-1);
	BListView::SortItems((int (*)(const BListItem**, const BListItem**))sort_func);
}


static int column_name_sort_callback(const BFilePanelListItem **_itemA, const BFilePanelListItem **_itemB)
{
	const BFilePanelListItem *itemA = *_itemA;
	const BFilePanelListItem *itemB = *_itemB;

	if (itemA->IsVolume() != itemB->IsVolume() &&
	        (itemA->IsVolume() || itemB->IsVolume())) {
		return(itemA->IsVolume() ? -1 : 1);
	} else if (itemA->IsDirectory() != itemB->IsDirectory() &&
	           (itemA->IsDirectory() || itemB->IsDirectory())) {
		return(itemA->IsDirectory() ? -1 : 1);
	}

	BString strA(itemA->Leaf()), strB(itemB->Leaf());
	if (strA == strB) return 0;

	return(itemA->PanelView()->GetSortIndex(NULL) * (strA < strB ? -1 : 1));
}


static void column_name_drawing_callback(BView *owner, BRect rect, BFilePanelListItem *item)
{
	if (!rect.IsValid()) return;

	const char **xpm_data = (const char**)file_xpm;

	if (item->IsVolume()) xpm_data = (const char**)volume_xpm;
	else if (item->IsDirectory()) xpm_data = (const char**)folder_xpm;

	if (xpm_data != NULL) {
		BPixmap *pixmap = new BPixmap(ICON_WIDTH, ICON_HEIGHT, B_RGB24);
		pixmap->SetDrawingMode(B_OP_COPY);
		pixmap->SetHighColor(owner->LowColor());
		pixmap->FillRect(0, 0, ICON_WIDTH, ICON_HEIGHT);
		pixmap->DrawXPM(xpm_data, 0, 0, 0, 0);

		BBitmap *bitmap = new BBitmap(pixmap);
		delete pixmap;

		owner->DrawBitmap(bitmap, BPoint(rect.left, rect.Center().y - ICON_HEIGHT / 2));
		delete bitmap;
	}

	if (item->Leaf()) {
		font_height fontHeight;
		owner->GetFontHeight(&fontHeight);

		float sHeight = fontHeight.ascent + fontHeight.descent;

		BPoint penLocation;
		penLocation.x = rect.left + ICON_WIDTH + 5;
		penLocation.y = rect.Center().y - sHeight / 2.f;
		penLocation.y += fontHeight.ascent + 1;

		owner->DrawString(item->Leaf(), penLocation);
	}
}


static int column_size_sort_callback(const BFilePanelListItem **_itemA, const BFilePanelListItem **_itemB)
{
	const BFilePanelListItem *itemA = *_itemA;
	const BFilePanelListItem *itemB = *_itemB;

	if (itemA->IsVolume() != itemB->IsVolume() &&
	        (itemA->IsVolume() || itemB->IsVolume())) {
		return(itemA->IsVolume() ? -1 : 1);
	} else if (itemA->IsDirectory() != itemB->IsDirectory() &&
	           (itemA->IsDirectory() || itemB->IsDirectory())) {
		return(itemA->IsDirectory() ? -1 : 1);
	}

	if (itemA->Size() == itemB->Size()) return 0;

	return(itemA->PanelView()->GetSortIndex(NULL) * (itemA->Size() < itemB->Size() ? -1 : 1));
}


static void column_size_drawing_callback(BView *owner, BRect rect, BFilePanelListItem *item)
{
	if (!rect.IsValid() || item->IsVolume() || item->IsDirectory()) return;

	BString str;
	if (item->Size() >= 0x40000000) str.AppendFormat("%.2gG", (float)item->Size() / (float)0x40000000);
	else if (item->Size() >= 0x100000) str.AppendFormat("%.2gM", (float)item->Size() / (float)0x100000);
	else if (item->Size() >= 0x400) str.AppendFormat("%.2gK", (float)item->Size() / (float)0x400);
	else str.AppendFormat("%I64i %s", item->Size(), TEXT_BYTES);

	BFont font;
	font_height fontHeight;
	owner->GetFont(&font);
	font.GetHeight(&fontHeight);

	float sHeight = fontHeight.ascent + fontHeight.descent;

	BPoint penLocation;
	penLocation.x = rect.right - font.StringWidth(str.String());
	penLocation.y = rect.Center().y - sHeight / 2.f;
	penLocation.y += fontHeight.ascent + 1;

	owner->DrawString(str.String(), penLocation);
}


static int column_modified_sort_callback(const BFilePanelListItem **_itemA, const BFilePanelListItem **_itemB)
{
	const BFilePanelListItem *itemA = *_itemA;
	const BFilePanelListItem *itemB = *_itemB;

	if (itemA->IsVolume() != itemB->IsVolume() &&
	        (itemA->IsVolume() || itemB->IsVolume())) {
		return(itemA->IsVolume() ? -1 : 1);
	} else if (itemA->IsDirectory() != itemB->IsDirectory() &&
	           (itemA->IsDirectory() || itemB->IsDirectory())) {
		return(itemA->IsDirectory() ? -1 : 1);
	}

	if (itemA->ModifiedTime() == itemB->ModifiedTime()) return 0;

	return(itemA->PanelView()->GetSortIndex(NULL) * (itemA->ModifiedTime() < itemB->ModifiedTime() ? -1 : 1));
}


static void column_modified_drawing_callback(BView *owner, BRect rect, BFilePanelListItem *item)
{
	if (!rect.IsValid() || item->IsVolume()) return;

	time_t timer = (time_t)(item->ModifiedTime() /B_INT64_CONSTANT(1000000));
	struct tm *tmTime = NULL;

#ifndef HAVE_LOCALTIME_R
	tmTime = localtime(&timer);
#else
	struct tm _tmTime;
	tmTime = localtime_r(&timer, &_tmTime);
#endif

	if (tmTime == NULL) return;

	BString str;
	str.AppendFormat("%d-%02d-%02d, %02d:%02d",
	                 1900 + tmTime->tm_year, 1 + tmTime->tm_mon, tmTime->tm_mday,
	                 tmTime->tm_hour, tmTime->tm_min);

	font_height fontHeight;
	owner->GetFontHeight(&fontHeight);

	float sHeight = fontHeight.ascent + fontHeight.descent;

	BPoint penLocation;
	penLocation.x = rect.left + 5;
	penLocation.y = rect.Center().y - sHeight / 2.f;
	penLocation.y += fontHeight.ascent + 1;

	owner->DrawString(str.String(), penLocation);
}


BFilePanelView::BFilePanelView(BRect frame, bool allow_multiple_selection)
		: BView(frame, NULL, B_FOLLOW_ALL, B_FRAME_EVENTS), fSort(1)
{
	BFilePanelLabel *label;

	BRect rect = Bounds();

	fTitleView = new BFilePanelTitleView(rect);
	AddChild(fTitleView);

	fHSB = new BScrollBar(rect, "HScrollBar", 0, 0, 0, B_HORIZONTAL);
	fHSB->ResizeBy(-(105 +B_V_SCROLL_BAR_WIDTH), B_H_SCROLL_BAR_HEIGHT - rect.Height());
	fHSB->MoveTo(105, rect.bottom -B_H_SCROLL_BAR_HEIGHT);
	AddChild(fHSB);

	label = new BFilePanelLabel(BRect(0, rect.bottom -B_H_SCROLL_BAR_HEIGHT, 100, rect.bottom),
	                            "CountVw", NULL,
	                            B_FOLLOW_LEFT |B_FOLLOW_BOTTOM);
	AddChild(label);

	fVSB = new BScrollBar(rect, "VScrollBar", 0, 0, 0, B_VERTICAL);
	fVSB->ResizeBy(B_V_SCROLL_BAR_WIDTH - rect.Width(), -B_H_SCROLL_BAR_HEIGHT);
	fVSB->MoveTo(rect.right -B_V_SCROLL_BAR_WIDTH, 0);
	AddChild(fVSB);

	rect.right -= B_V_SCROLL_BAR_WIDTH + 1;
	rect.top += fTitleView->Frame().Height() + 1;
	rect.bottom -= B_H_SCROLL_BAR_HEIGHT + 1;
	fListView = new BFilePanelListView(rect, "PoseView",
	                                   allow_multiple_selection ?B_MULTIPLE_SELECTION_LIST :B_SINGLE_SELECTION_LIST);
	fListView->SetMessage(new BMessage(MSG_PANEL_SELECTED));
	AddChild(fListView);

	fHSB->SetTarget(fTitleView);
	fHSB->SetEnabled(false);

	fVSB->SetTarget(fListView);
	fVSB->SetEnabled(false);

	AddColumn(TEXT_NAME, 250, column_name_drawing_callback, column_name_sort_callback);
	AddColumn(TEXT_SIZE, 100, column_size_drawing_callback, column_size_sort_callback);
	AddColumn(TEXT_MODIFIED, 200, column_modified_drawing_callback, column_modified_sort_callback);
}


BFilePanelView::~BFilePanelView()
{
	struct column_data *data;
	while ((data = (struct column_data*)fColumns.RemoveItem(0)) != NULL) {
		if (data->name != NULL) delete[] data->name;
		delete data;
	}
}


void
BFilePanelView::AddColumn(const char *name, float width,
                          void (*draw_func)(BView*, BRect, BFilePanelListItem*),
                          int (*sort_func)(const BFilePanelListItem**, const BFilePanelListItem**))
{
	struct column_data *data = new struct column_data;

	data->name = EStrdup(name);
	data->width = width;
	data->draw_func = draw_func;
	data->sort_func = sort_func;

	fColumns.AddItem(data);

	FrameResized(Bounds().Width(), Bounds().Height());

	Invalidate();
}


void
BFilePanelView::RemoveColumn(int32 index)
{
	struct column_data *data = (struct column_data*)fColumns.RemoveItem(index);
	if (data != NULL) {
		if (data->name != NULL) delete[] data->name;
		delete data;
	}

	FrameResized(Bounds().Width(), Bounds().Height());

	Invalidate();
}


void
BFilePanelView::SwapColumns(int32 indexA, int32 indexB)
{
	fColumns.SwapItems(indexA, indexB);
	Invalidate();
}


int32
BFilePanelView::CountColumns() const
{
	return fColumns.CountItems();
}


const char*
BFilePanelView::GetNameOfColumn(int32 index) const
{
	struct column_data *data = (struct column_data*)fColumns.ItemAt(index);
	return(data != NULL ? data->name : NULL);
}


float
BFilePanelView::GetWidthOfColumn(int32 index) const
{
	struct column_data *data = (struct column_data*)fColumns.ItemAt(index);
	return(data != NULL ? data->width : 0);
}


void
BFilePanelView::DrawItem(BView *owner, BRect itemRect, BFilePanelListItem *item)
{
	BRect rect = itemRect;
	rect.right = rect.left;

	for (int32 i = 0; i < fColumns.CountItems(); i++) {
		struct column_data *data = (struct column_data*)fColumns.ItemAt(i);

		rect.left = rect.right + (i == 0 ? 0.f : 1.f);
		rect.right = rect.left + data->width;

		if (data->draw_func == NULL) continue;

		owner->PushState();
		BRect aRect = rect.InsetByCopy(2, 2) & itemRect;
		owner->ConstrainClippingRegion(aRect);
		data->draw_func(owner, aRect, item);
		owner->PopState();
	}
}


void
BFilePanelView::SortItems(int32 nColumn)
{
	if (nColumn > 0) fSort = ((fSort < 0 ? -fSort : fSort) == nColumn ? -fSort : nColumn);

	struct column_data *data = (struct column_data*)fColumns.ItemAt((fSort < 0 ? -fSort : fSort) - 1);
	if (data == NULL || data->sort_func == NULL) return;

	fListView->SortItems(data->sort_func, nColumn != 0);
	fListView->Invalidate();
}


int
BFilePanelView::GetSortIndex(int32 *index) const
{
	if (fSort == 0) return 0;
	if (index) *index = (fSort < 0 ? -fSort : fSort) - 1;
	return(fSort < 0 ? -1 : 1);
}


void
BFilePanelView::FrameResized(float new_width, float new_height)
{
	float w = 0;
	BRect rect;
	for (int32 i = 0; i < CountColumns(); i++) w += GetWidthOfColumn(i);
	for (int32 i = 0; i < fListView->CountItems(); i++) rect |= fListView->ItemFrame(i);

	fHSB->SetRange(0, max_c(w - new_width, 0));
	fHSB->SetEnabled(new_width < w);

	fVSB->SetRange(0, max_c(rect.Height() - fListView->Frame().Height(), 0));
	fVSB->SetEnabled(fListView->Frame().Height() < rect.Height());
}


BFilePanelTitleView::BFilePanelTitleView(BRect parent_bounds)
		: BView(parent_bounds, "TitleView", B_FOLLOW_LEFT_RIGHT |B_FOLLOW_TOP, B_WILL_DRAW)
{
	font_height fontHeight;
	GetFontHeight(&fontHeight);

	ResizeTo(parent_bounds.Width() -B_V_SCROLL_BAR_WIDTH - 1,
	         fontHeight.ascent + fontHeight.descent + 4);
	MoveTo(0, 0);
}


void
BFilePanelTitleView::Draw(BRect updateRect)
{
	BFilePanelView *parent = (BFilePanelView*)Parent();
	int32 sortIndex = -1;

	if (parent == NULL) return;
	parent->GetSortIndex(&sortIndex);

	rgb_color textColor = ui_color(B_PANEL_TEXT_COLOR);
	rgb_color shineColor = ui_color(B_SHINE_COLOR);
	rgb_color shadowColor = ui_color(B_SHADOW_COLOR);
	if (!IsEnabled()) {
		textColor.disable(ViewColor());
		shineColor.disable(ViewColor());
		shadowColor.disable(ViewColor());
	}

	BFont font;
	font_height fontHeight;
	GetFont(&font);
	font.GetHeight(&fontHeight);

	PushState();

	SetDrawingMode(B_OP_COPY);
	SetPenSize(0);

	BRect bounds = Frame().OffsetToSelf(B_ORIGIN);
	SetHighColor(ViewColor());
	FillRect(bounds);

	BRect rect = bounds;
	rect.right = rect.left;
	for (int32 i = 0; i <= parent->CountColumns(); i++) {
		if (i == parent->CountColumns()) {
			if (rect.right >= bounds.right) break;
			rect.left = rect.right + (i == 0 ? 0.f : 1.f);
			rect.right = bounds.right + 1;
		} else {
			const char *name = parent->GetNameOfColumn(i);

			rect.left = rect.right + (i == 0 ? 0.f : 1.f);
			rect.right = rect.left + parent->GetWidthOfColumn(i);

			if (name) {
				BPoint penLocation;
				float strWidth = font.StringWidth(name);

				penLocation.x = rect.Center().x - strWidth / 2.f;
				penLocation.y = rect.Center().y - (fontHeight.ascent + fontHeight.descent) / 2.f;
				penLocation.y += fontHeight.ascent + 1;

				SetHighColor(textColor);
				DrawString(name, penLocation);

				if (i == sortIndex)
					StrokeLine(penLocation, penLocation + BPoint(strWidth, 0));
			}
		}

		SetHighColor(shineColor);
		StrokeRect(rect);
		SetHighColor(shadowColor);
		StrokeLine(rect.LeftBottom(), rect.RightBottom());
		StrokeLine(rect.RightTop());
	}

	PopState();
}


void
BFilePanelTitleView::GetPreferredSize(float *width, float *height)
{
	if (width) {
		*width = 0;

		BFilePanelView *parent = (BFilePanelView*)Parent();
		for (int32 i = 0; i < parent->CountColumns(); i++) *width += parent->GetWidthOfColumn(i);
	}

	if (height) {
		font_height fontHeight;
		GetFontHeight(&fontHeight);
		*height = fontHeight.ascent + fontHeight.descent + 4;
	}
}


void
BFilePanelTitleView::ScrollTo(BPoint where)
{
	BListView *listView = (BListView*)Parent()->FindView("PoseView");

	BView::ScrollTo(where);
	listView->ScrollTo(where.x, listView->Frame().top - listView->ConvertToParent(BPoint(0, 0)).y);
}


void
BFilePanelTitleView::MouseUp(BPoint where)
{
	BFilePanelView *parent = (BFilePanelView*)Parent();

	float left = 0, right = 0;
	for (int32 i = 0; i < parent->CountColumns(); i++) {
		left = right + (i == 0 ? 0.f : 1.f);
		right = left + parent->GetWidthOfColumn(i);
		if (where.x < left || where.x > right) continue;
		parent->SortItems(i + 1);
		Invalidate();
		break;
	}
}


static e_filter_result filter_key_down_hook(BMessage *message, BHandler **target, BMessageFilter *filter)
{
	int32 modifiers;
	const char *bytes;

	BFilePanelWindow *panelWindow = (BFilePanelWindow*)filter->Looper();

	if (message->FindInt32("modifiers", &modifiers) == false) return B_DISPATCH_MESSAGE;
	if (message->FindString("bytes", &bytes) == false || bytes == NULL) return B_DISPATCH_MESSAGE;

	if ((modifiers & (B_COMMAND_KEY |B_CONTROL_KEY)) && !(bytes[0] != B_UP_ARROW || bytes[1] != 0)) {
		BPath aPath(panelWindow->PanelDirectory());
		if (aPath.GetParent(&aPath) != B_OK) aPath.Unset();
		panelWindow->SetPanelDirectory(aPath.Path());
		return B_SKIP_MESSAGE;
	}

	return B_DISPATCH_MESSAGE;
}


BFilePanelWindow::BFilePanelWindow(BFilePanel *panel,
                                   e_file_panel_mode mode,
                                   uint32 node_flavors,
                                   bool modal,
                                   bool allow_multiple_selection)
		: BWindow(BRect(0, 0, 600, 400),
		          mode == B_OPEN_PANEL ? TEXT_OPEN_FILE : TEXT_SAVE_FILE,
		          modal ?B_MODAL_WINDOW :B_TITLED_WINDOW, 0),
		fPanel(panel), fMode(mode), fNodeFlavors(node_flavors == 0 ?B_FILB_NODE : node_flavors),
		fTarget(NULL), fMessage(NULL), fFilter(NULL), fHidesWhenDone(true),
		fSelIndex(0), fShowHidden(false)
{
	BView *topView, *aView;
	BMenuBar *menuBar;
	BMenu *menu;
	BMenuItem *menuItem;
	BMenuField *menuField;
	BButton *button;

	BRect rect = Bounds();

	topView = new BView(rect, NULL, B_FOLLOW_ALL, 0);
	topView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(topView);

	menu = new BMenu(TEXT_FILE, B_ITEMS_IN_COLUMN);
	menu->AddItem(new BMenuItem(mode == B_OPEN_PANEL ? TEXT_OPEN : TEXT_SAVE, new BMessage(MSG_PANEL_DONE)));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem(TEXT_SELECT_ALL, new BMessage(MSG_PANEL_SELECT_ALL)));
	menu->AddItem(new BMenuItem(TEXT_DESELECT_ALL, new BMessage(MSG_PANEL_DESELECT_ALL)));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem(TEXT_SHOW_HIDDEN_FILES, new BMessage(MSG_PANEL_TOGGLE_HIDDEN)));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem(TEXT_CANCEL, new BMessage(B_CANCEL)));
	for (int32 i = 0; (menuItem = menu->ItemAt(i)) != NULL; i++) menuItem->SetTarget(this);

	menuBar = new BMenuBar(rect, NULL,
	                       B_FOLLOW_LEFT_RIGHT |B_FOLLOW_TOP,
	                       B_ITEMS_IN_ROW, false);
	menuBar->SetName("MenuBar");
	menuBar->AddItem(menu);
	menuBar->GetPreferredSize(NULL, &rect.bottom);
	menuBar->ResizeTo(rect.Width(), rect.Height());
	topView->AddChild(menuBar);

	rect.top = rect.bottom + 1;
	rect.bottom = Bounds().bottom;
	rect.InsetBy(5, 5);
	aView = new BView(rect, NULL, B_FOLLOW_ALL, 0);
	topView->AddChild(aView);

	rect.OffsetTo(B_ORIGIN);

	fDirMenu = new BMenu(NULL, B_ITEMS_IN_COLUMN);
	menuField = new BMenuField(rect, "DirMenuField", TEXT_LOCATION, fDirMenu, false);
	menuField->GetPreferredSize(NULL, &rect.bottom);
	menuField->ResizeTo(rect.Width(), rect.Height());
	menuField->MoveTo(0, 0);
	aView->AddChild(menuField);

	rect.bottom = aView->Bounds().bottom;
	rect.top = rect.bottom - 20;

	if (mode == B_SAVE_PANEL) {
		BTextControl *textControl = new BTextControl(rect, "text control",
		        NULL, NULL, NULL,
		        B_FOLLOW_LEFT |B_FOLLOW_BOTTOM);
		textControl->ResizeTo(200, rect.Height());
		aView->AddChild(textControl);
	}

	rect.left = rect.right - 100;
	button = new BButton(rect, "default button",
	                     mode == B_OPEN_PANEL ? TEXT_OPEN : TEXT_SAVE,
	                     new BMessage(MSG_PANEL_DONE),
	                     B_FOLLOW_RIGHT |B_FOLLOW_BOTTOM);
	aView->AddChild(button);

	rect.OffsetBy(-110, 0);
	button = new BButton(rect, "cancel button",
	                     TEXT_CANCEL,
	                     new BMessage(B_CANCEL),
	                     B_FOLLOW_RIGHT |B_FOLLOW_BOTTOM);
	aView->AddChild(button);

	rect = aView->Bounds();
	rect.top = menuField->Frame().bottom + 5;
	rect.bottom -= 25;
	fPanelView = new BFilePanelView(rect, allow_multiple_selection);
	aView->AddChild(fPanelView);

	MoveToCenter();

	e_find_directory(B_USER_DIRECTORY, &fPath);
	AddCommonFilter(new BMessageFilter(B_KEY_DOWN, filter_key_down_hook));

	Run();
}


BFilePanelWindow::~BFilePanelWindow()
{
	if (fTarget) delete fTarget;
	if (fMessage) delete fMessage;
}


bool
BFilePanelWindow::QuitRequested()
{
	if (!IsHidden()) {
		Hide();
		fPanel->WasHidden();
	}
	return false;
}


void
BFilePanelWindow::MessageReceived(BMessage *msg)
{
	int32 index;
	const char *dir;
	const BMessenger *msgr;
	BMessage *aMsg = NULL;
	BListView *listView;
	BFilePanelListItem *item;

	BMenu *menu;
	BMenuItem *menuItem;

	switch (msg->what) {
		case MSG_PANEL_GET_DIR:
			msg->AddString("PanelDirectory", fPath.Path());
			msg->SendReply(msg);
			break;

		case MSG_PANEL_SET_DIR:
			if (msg->FindString("PanelDirectory", &dir) == false || dir == NULL) break;
			fPath.SetTo(dir);
			Refresh();
			break;

		case MSG_PANEL_TOGGLE_HIDDEN:
			menu = ((BMenuBar*)FindView("MenuBar"))->FindItem(TEXT_FILE)->Submenu();
			menuItem = menu->FindItem(TEXT_SHOW_HIDDEN_FILES);
			menuItem->SetMarked(fShowHidden = !menuItem->IsMarked());
			Refresh();
			break;

		case MSG_PANEL_SELECT_ALL:
			listView = (BListView*)FindView("PoseView");
			listView->Select(0, -1);
			listView->Invalidate();
			break;

		case MSG_PANEL_DESELECT_ALL:
			listView = (BListView*)FindView("PoseView");
			listView->DeselectAll();
			listView->Invalidate();
			break;

		case MSG_PANEL_SELECTED:
			if (msg->FindInt32("index", &index)) {
				listView = (BListView*)FindView("PoseView");
				if ((item = (BFilePanelListItem*)listView->ItemAt(index)) == NULL) break;
				if (item->IsVolume() || item->IsDirectory()) {
					fPath.SetTo(item->Path());
					Refresh();
				} else if (listView->ListType() == B_SINGLE_SELECTION_LIST) {
					PostMessage(MSG_PANEL_DONE);
				}
			}
			break;

		case MSG_PANEL_DONE:
			if (FindView("default button")->IsEnabled() == false) break;
			listView = (BListView*)FindView("PoseView");
			if (fMode == B_OPEN_PANEL && listView->CurrentSelection() < 0) break;
			if (fMode == B_SAVE_PANEL) {
				if (listView->CurrentSelection() < 0) {
					BTextControl *textControl = (BTextControl*)FindView("text control");
					if (textControl->Text() == NULL) break;
				}
			}

			msgr = (fTarget == NULL ? &app_messenger : fTarget);
			if (fMessage) aMsg = new BMessage(*fMessage);
			else aMsg = new BMessage(fMode == B_OPEN_PANEL ? (uint32)E_REFS_RECEIVED : (uint32)E_SAVE_REQUESTED);
			aMsg->AddString("directory", fPath.Path());
			for (int32 i = 0; (index = listView->CurrentSelection(i)) > 0; i++) {
				item = (BFilePanelListItem*)listView->ItemAt(index);
				aMsg->AddString("name", BPath(item->Path()).Leaf());
			}
			if (fMode == B_SAVE_PANEL)
				aMsg->AddString("name",	((BTextControl*)FindView("text control"))->Text());
			fPanel->SendMessage(msgr, aMsg);
			delete aMsg;

			if (fHidesWhenDone && !IsHidden()) {
				Hide();
				fPanel->WasHidden();
			}
			break;

		case B_CANCEL:
			msgr = (fTarget == NULL ? &app_messenger : fTarget);
			if (fMessage) {
				aMsg = new BMessage(*fMessage);
				aMsg->AddInt32("old_what", aMsg->what);
				aMsg->what = B_CANCEL;
			} else {
				aMsg = new BMessage(B_CANCEL);
			}
			aMsg->AddPointer("source", fPanel);
			fPanel->SendMessage(msgr, aMsg);
			delete aMsg;

			PostMessage(B_QUIT_REQUESTED);
			break;

		default:
			BWindow::MessageReceived(msg);
			break;
	}
}


BFilePanel*
BFilePanelWindow::Panel() const
{
	return fPanel;
}


BMessenger*
BFilePanelWindow::Target() const
{
	return fTarget;
}


e_file_panel_mode
BFilePanelWindow::PanelMode() const
{
	return fMode;
}


void
BFilePanelWindow::SetTarget(const BMessenger *target)
{
	if (fTarget) delete fTarget;
	fTarget = (target != NULL ? new BMessenger(*target) : NULL);
}


void
BFilePanelWindow::SetMessage(const BMessage *msg)
{
	if (fMessage) delete fMessage;
	fMessage = (msg != NULL ? new BMessage(*msg) : NULL);
}


bool
BFilePanelWindow::RefreshCallback(const char *path, void *data)
{
	BListView *listView = (BListView*)data;
	BFilePanelWindow *self = (BFilePanelWindow*)listView->Window();

	BEntry aEntry(path);
	if (self->fShowHidden == false && aEntry.IsHidden()) return false;
	if (!(self->fFilter == NULL || self->fFilter->Filter(&aEntry))) return false;

	listView->AddItem(new BFilePanelListItem(path, self->fPanelView));

	return false;
}


bool
BFilePanelWindow::IsDependsOnOthersWhenQuitRequested() const
{
	return true;
}


void
BFilePanelWindow::Refresh()
{
	BListView *listView = (BListView*)FindView("PoseView");

	listView->RemoveItems(0, -1, true);

	if (fPath.Path() != NULL) {
		BDirectory dir(fPath.Path());
		dir.DoForEach(RefreshCallback, (void*)listView);
	} else {
		BVolumeRoster volRoster;
		BVolume vol;

		while (volRoster.GetNextVolume(&vol) == B_NO_ERROR) {
			BDirectory volRootDir;
			BEntry aEntry;
			BPath aPath;

			vol.GetRootDirectory(&volRootDir);
			volRootDir.GetEntry(&aEntry);
			aEntry.GetPath(&aPath);

			if (aPath.Path() == NULL) continue;

			listView->AddItem(new BFilePanelListItem(aPath.Path(), fPanelView, vol.Device()));
		}
	}

	BFilePanelLabel *label = (BFilePanelLabel*)FindView("CountVw");
	BString str;
	str << listView->CountItems() << " " << TEXT_ITEMS;
	label->SetText(str.String());

	fPanelView->FrameResized(fPanelView->Frame().Width(), fPanelView->Frame().Height());
	fPanelView->SortItems(0);

	RefreshDirMenu();
}


void
BFilePanelWindow::SetPanelDirectory(const char *path)
{
	fPath.Unset();
	fPath.SetTo(path);
	Refresh();
}


const char*
BFilePanelWindow::PanelDirectory() const
{
	return fPath.Path();
}


void
BFilePanelWindow::SetFilter(BFilePanelFilter *filter)
{
	if (fFilter) delete fFilter;
	fFilter = filter;
}


void
BFilePanelWindow::SetSaveText(const char *text)
{
	BTextControl *textControl = (BTextControl*)FindView("text control");
	if (textControl) textControl->SetText(text);
}


void
BFilePanelWindow::SetButtonLabel(e_file_panel_button btn, const char *label)
{
	BButton *button = (BButton*)FindView(btn == B_CANCEL_BUTTON ? "cancel button" : "default button");
	if (button) button->SetLabel(label);
}


void
BFilePanelWindow::SetHideWhenDone(bool state)
{
	fHidesWhenDone = state;
}


void
BFilePanelWindow::Rewind()
{
	fSelIndex = 0;
}


status_t
BFilePanelWindow::GetNextSelected(BEntry *entry)
{
	BListView *listView = (BListView*)FindView("PoseView");
	BFilePanelListItem *item = (BFilePanelListItem*)listView->ItemAt(listView->CurrentSelection(fSelIndex));

	if (item == NULL) {
		if (fMode == B_SAVE_PANEL && fSelIndex++ == 0) {
			BTextControl *textControl = (BTextControl*)FindView("text control");
			if (textControl->Text() != NULL) {
				BPath aPath = fPath;
				aPath.Append(textControl->Text());
				entry->SetTo(aPath.Path());
				return B_OK;
			}
		}

		return B_ERROR;
	}

	entry->SetTo(item->Path());

	fSelIndex++;

	return B_OK;
}


void
BFilePanelWindow::RefreshDirMenu()
{
	BString str;
	BMessage *msg;
	BMenuItem *menuItem;

	if (fDirMenu->CountItems() == 0) {
		BVolumeRoster volRoster;
		BVolume vol;
		BPath aPath;

		while (volRoster.GetNextVolume(&vol) == B_NO_ERROR) {
			BDirectory volRootDir;
			BEntry aEntry;

			vol.GetRootDirectory(&volRootDir);
			volRootDir.GetEntry(&aEntry);
			if (aEntry.GetPath(&aPath) != B_OK || aPath.Path() == NULL) continue;

			msg = new BMessage(MSG_PANEL_SET_DIR);
			msg->AddString("PanelDirectory", aPath.Path());

			str.MakeEmpty();
			vol.GetName(&str);
			if (str.Length() == 0) str = TEXT_NO_NAME;
			str.PrependFormat("%s ", TEXT_NOTICE_VOLUME);

			menuItem = new BMenuItem(str.String(), msg);
			menuItem->SetTarget(this);
			fDirMenu->AddItem(menuItem);
		}

		if (fDirMenu->CountItems() > 0) fDirMenu->AddItem(new BMenuSeparatorItem(), 0);

		if (e_find_directory(B_USER_DIRECTORY, &aPath) == B_OK) {
			msg = new BMessage(MSG_PANEL_SET_DIR);
			msg->AddString("PanelDirectory", aPath.Path());

			menuItem = new BMenuItem(TEXT_HOME, msg);
			menuItem->SetTarget(this);
			fDirMenu->AddItem(menuItem, 0);

			fDirMenu->AddItem(new BMenuSeparatorItem(), 0);
		}
	} else {
		while ((menuItem = fDirMenu->ItemAt(0)) != NULL) {
			if (is_instance_of(menuItem, BMenuSeparatorItem)) break;
			fDirMenu->RemoveItem(0);
			delete menuItem;
		}
	}

	BMenu *menu = new BMenu(TEXT_OTHER_DIRECTORYIES, B_ITEMS_IN_COLUMN);
	fDirMenu->AddItem(menu, 0);

	BPath aPath = fPath;
	while (aPath.GetParent(&aPath) == B_OK) {
		msg = new BMessage(MSG_PANEL_SET_DIR);
		msg->AddString("PanelDirectory", aPath.Path());

		menuItem = new BMenuItem(aPath.Path(), msg);
		menuItem->SetTarget(this);
		menu->AddItem(menuItem);
	}

	if (menu->CountItems() == 0) {
		menuItem = new BMenuItem(TEXT_NOTICE_EMPTY, NULL);
		menuItem->SetEnabled(false);
		menu->AddItem(menuItem);
	}

	fDirMenu->Superitem()->SetLabel(fPath.Path() == NULL ? TEXT_ALL_VOLUMES : fPath.Path());
}


BFilePanel::BFilePanel(e_file_panel_mode mode,
                       const BMessenger *target,
                       const char *panel_directory,
                       uint32 node_flavors,
                       bool allow_multiple_selection,
                       const BMessage *message,
                       BFilePanelFilter *filter,
                       bool modal,
                       bool hide_when_done)
{
	fWindow = new BFilePanelWindow(this, mode, node_flavors, modal, allow_multiple_selection);
	if (panel_directory) SetPanelDirectory(panel_directory);
	SetTarget(target);
	SetMessage(message);
	SetFilter(filter);
	SetHideWhenDone(hide_when_done);
}


BFilePanel::~BFilePanel()
{
	fWindow->Lock();
	fWindow->Quit();
}


void
BFilePanel::Show()
{
	BFilePanelWindow *win = (BFilePanelWindow*)fWindow;
	BAutolock <BFilePanelWindow> autolock(win);

	if (win->IsHidden()) {
		win->Show();
		win->Refresh();
	}
}


void
BFilePanel::Hide()
{
	BAutolock <BWindow> autolock(fWindow);

	fWindow->Hide();
}


bool
BFilePanel::IsShowing() const
{
	return(!fWindow->IsHidden());
}


void
BFilePanel::WasHidden()
{
}


void
BFilePanel::SelectionChanged()
{
}


void
BFilePanel::SendMessage(const BMessenger *msgr, BMessage *msg)
{
	if (msgr && msg) msgr->SendMessage(msg);
}


BWindow*
BFilePanel::Window() const
{
	return fWindow;
}


BMessenger*
BFilePanel::Target() const
{
	BFilePanelWindow *win = (BFilePanelWindow*)fWindow;
	return win->Target();
}


BFilePanelFilter*
BFilePanel::Filter() const
{
	return ((BFilePanelWindow*)fWindow)->fFilter;
}


e_file_panel_mode
BFilePanel::PanelMode() const
{
	BFilePanelWindow *win = (BFilePanelWindow*)fWindow;
	return win->PanelMode();
}


void
BFilePanel::SetTarget(const BMessenger *target)
{
	BFilePanelWindow *win = (BFilePanelWindow*)fWindow;
	BAutolock <BFilePanelWindow> autolock(win);

	win->SetTarget(target);
}


void
BFilePanel::SetMessage(const BMessage *msg)
{
	BFilePanelWindow *win = (BFilePanelWindow*)fWindow;
	BAutolock <BFilePanelWindow> autolock(win);

	win->SetMessage(msg);
}


void
BFilePanel::SetFilter(BFilePanelFilter *filter)
{
	BFilePanelWindow *win = (BFilePanelWindow*)fWindow;
	BAutolock <BFilePanelWindow> autolock(win);

	win->SetFilter(filter);
}


void
BFilePanel::SetSaveText(const char *text)
{
	BFilePanelWindow *win = (BFilePanelWindow*)fWindow;
	BAutolock <BFilePanelWindow> autolock(win);

	win->SetSaveText(text);
}


void
BFilePanel::SetButtonLabel(e_file_panel_button btn, const char *label)
{
	BFilePanelWindow *win = (BFilePanelWindow*)fWindow;
	BAutolock <BFilePanelWindow> autolock(win);

	win->SetButtonLabel(btn, label);
}


void
BFilePanel::SetHideWhenDone(bool state)
{
	BFilePanelWindow *win = (BFilePanelWindow*)fWindow;
	BAutolock <BFilePanelWindow> autolock(win);

	win->SetHideWhenDone(state);
}


bool
BFilePanel::HidesWhenDone() const
{
	return ((BFilePanelWindow*)fWindow)->fHidesWhenDone;
}


void
BFilePanel::GetPanelDirectory(BEntry *entry) const
{
	if (entry == NULL) return;

	if (fWindow->Thread() != get_current_thread_id()) {
		BMessenger msgr(fWindow, fWindow);
		BMessage msg(MSG_PANEL_GET_DIR);
		const char *path = NULL;

		entry->Unset();
		if (msgr.SendMessage(&msg, &msg) != B_OK) return;
		if (msg.FindString("PanelDirectory", &path) == false) return;
		entry->SetTo(path);
	} else {
		entry->Unset();
		entry->SetTo(((BFilePanelWindow*)fWindow)->fPath.Path());
	}
}


void
BFilePanel::GetPanelDirectory(BPath *path) const
{
	BEntry aEntry;

	if (path == NULL) return;

	path->Unset();
	GetPanelDirectory(&aEntry);
	aEntry.GetPath(path);
}


void
BFilePanel::GetPanelDirectory(BDirectory *directory) const
{
	BPath aPath;

	if (directory == NULL) return;

	directory->Unset();
	GetPanelDirectory(&aPath);
	directory->SetTo(aPath.Path());
}


void
BFilePanel::SetPanelDirectory(const BEntry *entry)
{
	BPath path;

	if (entry) entry->GetPath(&path);
	SetPanelDirectory(path.Path());
}


void
BFilePanel::SetPanelDirectory(const BDirectory *directory)
{
	BEntry entry;

	if (directory) directory->GetEntry(&entry);
	SetPanelDirectory(&entry);
}


void
BFilePanel::SetPanelDirectory(const char *directory)
{
	BFilePanelWindow *win = (BFilePanelWindow*)fWindow;
	BAutolock <BFilePanelWindow> autolock(win);

	win->SetPanelDirectory(directory);
}


void
BFilePanel::Refresh()
{
	BFilePanelWindow *win = (BFilePanelWindow*)fWindow;
	BAutolock <BFilePanelWindow> autolock(win);

	win->Refresh();
}


void
BFilePanel::Rewind()
{
	BFilePanelWindow *win = (BFilePanelWindow*)fWindow;
	BAutolock <BFilePanelWindow> autolock(win);

	win->Rewind();
}


status_t
BFilePanel::GetNextSelected(BEntry *entry)
{
	if (entry == NULL) return B_ERROR;

	BFilePanelWindow *win = (BFilePanelWindow*)fWindow;
	BAutolock <BFilePanelWindow> autolock(win);

	return win->GetNextSelected(entry);
}

