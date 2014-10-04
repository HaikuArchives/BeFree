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
 * File: ListView.cpp
 * Description: BListView --- Displays a list of items the user can select and invoke
 *
 * --------------------------------------------------------------------------*/

#include <support/ClassInfo.h>

#include "ListView.h"
#include "ScrollView.h"
#include "Window.h"


BListView::BListView(BRect frame,
                     const char *name,
                     list_view_type type,
                     uint32 resizingMode,
                     uint32 flags)
		: BView(frame, name, resizingMode, flags), BInvoker(),
		fFirstSelected(-1), fLastSelected(-1), fPos(-1),
		fSelectionMessage(NULL)
{
	fListType = type;
}


BListView::~BListView()
{
	BListView::MakeEmpty();
	if (fSelectionMessage != NULL) delete fSelectionMessage;
}


void
BListView::MakeFocus(bool focusState)
{
	if (IsFocus() != focusState) {
		BView::MakeFocus(focusState);

		if (IsVisible() && (Flags() &B_WILL_DRAW)) {
			PushState();
			SetHighColor(IsFocus() ? ui_color(B_NAVIGATION_BASE_COLOR) : ViewColor());
			StrokeRect(Bounds());
			PopState();

			InvalidateItem(fPos);
		}
	}
}


void
BListView::WindowActivated(bool state)
{
	InvalidateItem(fPos);
	if (!(IsFocus() && (Flags() &B_WILL_DRAW))) return;
	PushState();
	SetHighColor(state ? ui_color(B_NAVIGATION_BASE_COLOR) : ViewColor());
	StrokeRect(Bounds());
	PopState();
}


bool
BListView::AddItem(BListItem *item)
{
	return BListView::AddItem(item, fItems.CountItems());
}


bool
BListView::AddItem(BListItem *item, int32 atIndex)
{
	if (item == NULL || item->fOwner != NULL) return false;
	if (fItems.AddItem(item, atIndex) == false) return false;
	item->fOwner = this;
	item->fSelected = false;

	if (fFirstSelected >= 0 && atIndex <= fLastSelected) {
		if (atIndex <= fFirstSelected) fFirstSelected++;
		fLastSelected++;
		SelectionChanged();
	}

	BFont font;
	GetFont(&font);
	item->Update(this, &font);

	if (atIndex <= fPos) fPos++;

	return true;
}


bool
BListView::RemoveItem(BListItem *item, bool auto_destruct_item)
{
	if (BListView::RemoveItem(IndexOf(item)) == NULL) return false;
	if (auto_destruct_item) delete item;

	return true;
}


BListItem*
BListView::RemoveItem(int32 index)
{
	BListItem *item = (BListItem*)fItems.RemoveItem(index);
	if (item == NULL) return NULL;

	item->fOwner = NULL;
	if (item->fSelected) {
		if (index == fFirstSelected) {
			fFirstSelected = -1;
			fLastSelected--;

			while (fListType == B_MULTIPLE_SELECTION_LIST && index < min_c(fItems.CountItems(), fLastSelected + 1)) {
				item = (BListItem*)fItems.ItemAt(index);
				if (item->fSelected) {
					fFirstSelected = index;
					break;
				}
				index++;
			}

			if (fFirstSelected < 0) fLastSelected = -1;
		} else if (index == fLastSelected) {
			fLastSelected = -1;

			while (index >= max_c(0, fFirstSelected)) {
				item = (BListItem*)fItems.ItemAt(index);
				if (item->fSelected) {
					fLastSelected = index;
					break;
				}
				index--;
			}

			if (fLastSelected < 0) fFirstSelected = -1;
		} else {
			fLastSelected--;
		}

		SelectionChanged();
	} else if (index < fFirstSelected) {
		fFirstSelected--;
		fLastSelected--;
		SelectionChanged();
	}

	if (index == fPos) fPos = -1;
	else if (index < fPos) fPos--;

	return item;
}


bool
BListView::RemoveItems(int32 index, int32 count, bool auto_destruct_items)
{
	if (index < 0 || index >= fItems.CountItems()) return false;

	if (count < 0) count = fItems.CountItems() - index;
	else count = min_c(fItems.CountItems() - index, count);

	// TODO: remove at once
	while (count-- > 0) {
		BListItem *item = BListView::RemoveItem(index);
		if (item == NULL) return false;
		if (auto_destruct_items) delete item;
	}

	return true;
}


void
BListView::SetListType(list_view_type type)
{
	if (fListType != type) {
		fListType = type;
		if (fListType == B_SINGLE_SELECTION_LIST) Select(CurrentSelection(0), false);
	}
}


list_view_type
BListView::ListType() const
{
	return fListType;
}


BListItem*
BListView::ItemAt(int32 index) const
{
	return (BListItem*)fItems.ItemAt(index);
}


BListItem*
BListView::FirstItem() const
{
	return (BListItem*)fItems.FirstItem();
}


BListItem*
BListView::LastItem() const
{
	return (BListItem*)fItems.LastItem();
}


int32
BListView::IndexOf(const BListItem *item) const
{
	if (item == NULL || item->fOwner != this) return -1;
	return fItems.IndexOf((void*)item);
}


int32
BListView::IndexOf(BPoint where, bool mustVisible) const
{
	float boundsBottom = -1;

	if (mustVisible) {
		BRect vRect = VisibleBounds();
		if (vRect.Contains(where) == false) return -1;
		boundsBottom = vRect.bottom;
	}

	BRect rect(1, 1, Frame().Width() - 1, 1);

	int32 retVal = -1;

	for (int32 i = 0; i < fItems.CountItems(); i++) {
		BListItem *item = (BListItem*)fItems.ItemAt(i);
		if (item->Height() < 0) continue;

		rect.top = rect.bottom;
		rect.bottom = rect.top + item->Height();
		if (rect.top > 1) rect.OffsetBy(0, 1);

		if (rect.Contains(where)) {
			retVal = i;
			break;
		}

		if (boundsBottom < 0) continue;
		else if (rect.top > boundsBottom) break;
	}

	return retVal;
}


bool
BListView::HasItem(const BListItem *item) const
{
	return((item == NULL || item->fOwner != this) ? false : true);
}


int32
BListView::CountItems() const
{
	return fItems.CountItems();
}


void
BListView::MakeEmpty()
{
	while (fItems.CountItems() > 0) {
		BListItem *item = (BListItem*)fItems.RemoveItem((int32)0);
		item->fOwner = NULL;
		delete item;
	}

	fFirstSelected = fLastSelected = -1;
}


bool
BListView::IsEmpty() const
{
	return fItems.IsEmpty();
}


void
BListView::SelectionChanged()
{
}


void
BListView::Draw(BRect updateRect)
{
	if (Window() == NULL) return;
	BRect bounds = Bounds();
	bool winActivated = Window()->IsActivate();

	if (IsFocus() && winActivated) {
		PushState();
		SetPenSize(0);
		SetDrawingMode(B_OP_COPY);
		SetHighColor(ui_color(B_NAVIGATION_BASE_COLOR));
		StrokeRect(bounds);
		PopState();
	}

	bounds.InsetBy(1, 1);
	BRect rect(1, 1, bounds.right - 1, 1);

	for (int32 i = 0; i < fItems.CountItems(); i++) {
		BListItem *item = (BListItem*)fItems.ItemAt(i);
		if (item->Height() < 0) continue;

		rect.top = rect.bottom;
		rect.bottom = rect.top + item->Height();
		if (rect.top > 1) rect.OffsetBy(0, 1);
		if (rect.Intersects(updateRect) == false) continue;
		if (rect.top >= bounds.bottom) break;
		if (rect.Intersects(bounds) == false) continue;

		PushState();
		ConstrainClippingRegion(rect & bounds);
		item->DrawItem(this, rect, true);
		if (!(i != fPos || !IsEnabled() || !IsFocus() || !winActivated)) {
			SetPenSize(0);
			SetDrawingMode(B_OP_COPY);
			SetHighColor(0, 0, 0);
			StrokeRect(rect);
		}
		PopState();
	}
}


void
BListView::KeyDown(const char *bytes, int32 numBytes)
{
	if (!IsEnabled() || !IsFocus() || numBytes != 1) return;

	int32 oldPos = -1, newPos = -1;
	bool doDown = false;

	switch (bytes[0]) {
		case B_ENTER:
			if (fPos >= 0 && fPos >= fFirstSelected && fPos <= fLastSelected) {
				BListItem *item = (BListItem*)fItems.ItemAt(fPos);
				if (!(item == NULL || item->fSelected == false)) {
					Invoke();
					break;
				}
			}
		case B_SPACE:
			if (fPos >= 0 && fPos < fItems.CountItems()) {
				if (((BListItem*)fItems.ItemAt(fPos))->fEnabled == false) break;
				if (((BListItem*)fItems.ItemAt(fPos))->fSelected)
					Deselect(fPos);
				else
					Select(fPos, fListType != B_SINGLE_SELECTION_LIST);

				Invalidate();
			}
			break;

		case B_ESCAPE:
			if (fPos >= 0) {
				InvalidateItem(fPos);
				fPos = -1;
			} else if (fFirstSelected >= 0) {
				DeselectAll();
				Invalidate();
			}
			break;

		case B_UP_ARROW: {
			if (fPos <= 0) break;
			oldPos = fPos;
			newPos = --fPos;
		}
		break;

		case B_DOWN_ARROW: {
			if (fPos >= fItems.CountItems() - 1) break;
			oldPos = fPos;
			newPos = ++fPos;
			doDown = true;
		}
		break;

		case B_PAGE_UP:
		case B_PAGE_DOWN: {
			// TODO
		}
		break;

		case B_HOME: {
			if (fPos <= 0) break;
			oldPos = fPos;
			newPos = fPos = 0;
		}
		break;

		case B_END: {
			if (fPos >= fItems.CountItems() - 1) break;
			oldPos = fPos;
			newPos = fPos = fItems.CountItems() - 1;
			doDown = true;
		}
		break;

		default:
			break;
	}

	if (oldPos >= 0 || newPos >= 0) {
		BRect rect = ItemFrame(newPos);
		if (rect.IsValid() == false ||
		        is_kind_of(Parent(), BScrollView) == false || cast_as(Parent(), BScrollView)->Target() != this) {
			InvalidateItem(oldPos);
			if (rect.IsValid()) Invalidate(rect);
		} else {
			BRect vRect = ConvertFromParent(cast_as(Parent(), BScrollView)->TargetFrame());
			if (vRect.top <= rect.top && vRect.bottom >= rect.bottom) {
				InvalidateItem(oldPos);
				if (rect.IsValid()) Invalidate(rect);
			} else {
				float xOffset = Frame().left - ConvertToParent(BPoint(0, 0)).x;
				if (doDown == false)
					ScrollTo(xOffset, rect.top);
				else
					ScrollTo(xOffset, rect.bottom - vRect.Height());
			}
		}
	}

	if (!(!(bytes[0] == B_LEFT_ARROW || bytes[0] == B_RIGHT_ARROW) ||
	        is_kind_of(Parent(), BScrollView) == false ||
	        cast_as(Parent(), BScrollView)->Target() != this)) {
		float visibleWidth = cast_as(Parent(), BScrollView)->TargetFrame().Width();
		BRect frame = Frame();

		if (visibleWidth >= frame.Width()) return;

		BScrollBar *hsb = cast_as(Parent(), BScrollView)->ScrollBar(B_HORIZONTAL);
		if (hsb == NULL) return;

		BPoint originOffset = frame.LeftTop() - ConvertToParent(BPoint(0, 0));

		float step = 0;
		hsb->GetSteps(NULL, &step);

		if (bytes[0] == B_LEFT_ARROW) originOffset.x -= step;
		else originOffset.x += step;

		if (originOffset.x > frame.Width() - visibleWidth) originOffset.x = frame.Width() - visibleWidth;
		else if (originOffset.x < 0) originOffset.x = 0;

		ScrollTo(originOffset);
	}
}


void
BListView::KeyUp(const char *bytes, int32 numBytes)
{
}


void
BListView::MouseDown(BPoint where)
{
	int32 btnClicks = 1;

	if (!IsEnabled() || !QueryCurrentMouse(true, B_PRIMARY_MOUSE_BUTTON, true, &btnClicks) || btnClicks > 2) return;

	if ((Flags() &B_NAVIGABLE) && !IsFocus()) MakeFocus();

	if (btnClicks == 2) {
		if (fFirstSelected >= 0) Invoke();
		return;
	}

	int32 selectIndex = IndexOf(where, true);
	BListItem *item = ItemAt(selectIndex);
	if (item == NULL || item->fEnabled == false) return;

	if (item->fSelected)
		Deselect(selectIndex);
	else
		Select(selectIndex, fListType != B_SINGLE_SELECTION_LIST);

	fPos = selectIndex;
	Invalidate();
}


void
BListView::MouseUp(BPoint where)
{
}


void
BListView::MouseMoved(BPoint where, uint32 code, const BMessage *a_message)
{
}


void
BListView::SetFont(const BFont *font, uint8 mask)
{
	if (font == NULL) return;

	BView::SetFont(font, mask);

	BFont aFont;
	GetFont(&aFont);
	for (int32 i = 0; i < fItems.CountItems(); i++) {
		BListItem *item = (BListItem*)fItems.ItemAt(i);
		item->Update(this, &aFont);
	}
}


void
BListView::SetSelectionMessage(BMessage *message)
{
	if (message == fSelectionMessage) return;
	if (fSelectionMessage != NULL) delete fSelectionMessage;
	fSelectionMessage = message;
}


void
BListView::SetInvocationMessage(BMessage *message)
{
	SetMessage(message);
}


BMessage*
BListView::SelectionMessage() const
{
	return fSelectionMessage;
}


uint32
BListView::SelectionCommand() const
{
	return(fSelectionMessage ? fSelectionMessage->what : 0);
}


BMessage*
BListView::InvocationMessage() const
{
	return Message();
}


uint32
BListView::InvocationCommand() const
{
	return Command();
}


status_t
BListView::Invoke(const BMessage *msg)
{
	if (fFirstSelected < 0) {
		return BInvoker::Invoke(msg);
	} else {
		const BMessage *message = (msg ? msg : Message());
		if (!message) return B_BAD_VALUE;

		BMessage aMsg(*message);

		for (int32 i = fFirstSelected; i <= fLastSelected; i++) {
			BListItem *item = (BListItem*)fItems.ItemAt(i);
			if (item == NULL) continue;
			if (item->fSelected) aMsg.AddInt32("index", i);
		}

		return BInvoker::Invoke(&aMsg);
	}
}


void
BListView::Select(int32 index, bool extend)
{
	Select(index, index, extend);
}


void
BListView::Select(int32 start, int32 finish, bool extend)
{
	if (extend == false) {
		for (int32 i = fFirstSelected; i <= fLastSelected; i++) {
			BListItem *item = (BListItem*)fItems.ItemAt(i);
			if (item == NULL) continue;
			item->fSelected = false;
		}
		fFirstSelected = fLastSelected = -1;
	}

	if (start >= 0) {
		bool hasNewSelection = false;

		if (finish < 0 || finish >= fItems.CountItems()) finish = fItems.CountItems() - 1;

		for (int32 index = start; index <= finish; index++) {
			BListItem *item = (BListItem*)fItems.ItemAt(index);
			if (item == NULL || item->fSelected == true) continue;

			item->fSelected = true;
			hasNewSelection = true;

			fFirstSelected = (fFirstSelected < 0 ? index : min_c(fFirstSelected, index));
			fLastSelected = (fLastSelected < 0 ? index : max_c(fLastSelected, index));
		}

		if (hasNewSelection && fSelectionMessage != NULL) Invoke(fSelectionMessage);
	}

	SelectionChanged();
}


bool
BListView::IsItemSelected(int32 index) const
{
	BListItem *item = (BListItem*)fItems.ItemAt(index);
	return(item == NULL ? false : item->fSelected);
}


int32
BListView::CurrentSelection(int32 index) const
{
	if (fFirstSelected < 0 || index < 0) return -1;

	// TODO: speed up
	for (int32 i = fFirstSelected; i <= fLastSelected; i++) {
		BListItem *item = (BListItem*)fItems.ItemAt(i);
		if (item == NULL) continue;
		if (item->fSelected) index--;
		if (index < 0) return i;
	}

	return -1;
}


void
BListView::Deselect(int32 index)
{
	BListItem *item = (BListItem*)fItems.ItemAt(index);
	if (item == NULL || item->fSelected == false) return;

	item->fSelected = false;

	if (index == fFirstSelected) {
		fFirstSelected = -1;

		while ((++index) < min_c(fItems.CountItems(), fLastSelected + 1)) {
			item = (BListItem*)fItems.ItemAt(index);
			if (item->fSelected) {
				fFirstSelected = index;
				break;
			}
		}

		if (fFirstSelected < 0) fLastSelected = -1;
	} else if (index == fLastSelected) {
		fLastSelected = -1;

		while ((--index) >= max_c(0, fFirstSelected)) {
			item = (BListItem*)fItems.ItemAt(index);
			if (item->fSelected) {
				fLastSelected = index;
				break;
			}
		}

		if (fLastSelected < 0) fFirstSelected = -1;
	}

	SelectionChanged();
}


void
BListView::DeselectAll()
{
	if (fFirstSelected < 0) return;

	for (int32 i = fFirstSelected; i <= fLastSelected; i++) {
		BListItem *item = (BListItem*)fItems.ItemAt(i);
		if (item == NULL) continue;
		item->fSelected = false;
	}

	fFirstSelected = fLastSelected = -1;

	SelectionChanged();
}


void
BListView::DeselectExcept(int32 start, int32 finish)
{
	if (fFirstSelected < 0) return;

	if (start >= 0 && (finish < 0 || finish >= fItems.CountItems())) finish = fItems.CountItems() - 1;

	for (int32 i = fFirstSelected; i <= fLastSelected; i++) {
		if (start >= 0 && start <= finish && i >= start && i <= finish) continue;

		BListItem *item = (BListItem*)fItems.ItemAt(i);
		if (item == NULL) continue;
		item->fSelected = false;
	}

	if (start >= 0 && start <= finish && !(start > fLastSelected || finish < fFirstSelected)) {
		fFirstSelected = max_c(fFirstSelected, start);
		fLastSelected = min_c(fLastSelected, finish);
	} else {
		fFirstSelected = fLastSelected = -1;
	}

	SelectionChanged();
}


bool
BListView::SwapItems(int32 indexA, int32 indexB)
{
	bool retVal = false;

	do {
		if ((retVal = fItems.SwapItems(indexA, indexB)) == false) break;

		if (fFirstSelected < 0) break;
		if (indexA >= fFirstSelected && indexA <= fLastSelected &&
		        indexB >= fFirstSelected && indexB <= fLastSelected) break;

		int32 newIn = -1;

		if (indexA >= fFirstSelected && indexA <= fLastSelected) newIn = indexB;
		else if (indexB >= fFirstSelected && indexB <= fLastSelected) newIn = indexA;

		if (newIn < 0) break;

		fFirstSelected = min_c(fFirstSelected, newIn);
		fLastSelected = max_c(fLastSelected, newIn);

		for (int32 i = fFirstSelected; i <= fLastSelected; i++) {
			BListItem *item = (BListItem*)fItems.ItemAt(i);
			if (item->fSelected == true) break;
			fFirstSelected++;
		}

		for (int32 i = fLastSelected; i >= fFirstSelected; i--) {
			BListItem *item = (BListItem*)fItems.ItemAt(i);
			if (item->fSelected == true) break;
			fLastSelected--;
		}

		if (fLastSelected < fFirstSelected) fFirstSelected = fLastSelected = -1;

		SelectionChanged();
	} while (false);

	return retVal;
}


bool
BListView::MoveItem(int32 fromIndex, int32 toIndex)
{
	bool retVal = false;

	do {
		if ((retVal = fItems.MoveItem(fromIndex, toIndex)) == false) break;

		if (fFirstSelected < 0) break;
		if ((fromIndex < fFirstSelected && toIndex < fFirstSelected) ||
		        (fromIndex > fLastSelected && toIndex > fLastSelected)) break;

		// TODO: speed up
		fFirstSelected = fLastSelected = -1;
		for (int32 i = 0; i < fItems.CountItems(); i++) {
			BListItem *item = (BListItem*)fItems.ItemAt(i);
			if (item->fSelected == false) continue;
			fFirstSelected = (fFirstSelected < 0 ? i : min_c(fFirstSelected, i));
			fLastSelected = (fLastSelected < 0 ? i : max_c(fLastSelected, i));
		}
		SelectionChanged();
	} while (false);

	return retVal;
}


bool
BListView::ReplaceItem(int32 index, BListItem *newItem, BListItem **oldItem)
{
	bool retVal = false;
	BListItem *old_item = NULL;

	do {
		if (newItem == NULL) break;
		if ((retVal = fItems.ReplaceItem(index, (void*)newItem, (void**)old_item)) == false) break;

		bool oldItemSelected = false;
		if (old_item) {
			old_item->fOwner = NULL;
			oldItemSelected = old_item->fSelected;
		}
		if (oldItem != NULL) {
			*oldItem = old_item;
			old_item = NULL;
		}

		newItem->fSelected = false;
		newItem->fOwner = this;

		if (oldItemSelected == false || fFirstSelected < 0) break;

		for (int32 i = fFirstSelected; i <= fLastSelected; i++) {
			BListItem *item = (BListItem*)fItems.ItemAt(i);
			if (item->fSelected == true) break;
			fFirstSelected++;
		}

		for (int32 i = fLastSelected; i >= fFirstSelected; i--) {
			BListItem *item = (BListItem*)fItems.ItemAt(i);
			if (item->fSelected == true) break;
			fLastSelected--;
		}

		if (fLastSelected < fFirstSelected) fFirstSelected = fLastSelected = -1;

		SelectionChanged();
	} while (false);

	if (old_item != NULL && retVal == true) delete old_item;

	return retVal;
}


void
BListView::SortItems(int (*cmp)(const BListItem **a, const BListItem **b))
{
	if (cmp == NULL) return;

	fItems.SortItems((int (*)(const void*, const void*))cmp);

	if (fFirstSelected > 0) {
		fFirstSelected = fLastSelected = -1;
		for (int32 i = 0; i < fItems.CountItems(); i++) {
			BListItem *item = (BListItem*)fItems.ItemAt(i);
			if (item->fSelected == false) continue;
			fFirstSelected = (fFirstSelected < 0 ? i : min_c(fFirstSelected, i));
			fLastSelected = (fLastSelected < 0 ? i : max_c(fLastSelected, i));
		}
		SelectionChanged();
	}
}


void
BListView::DoForEach(bool (*func)(BListItem *item))
{
	fItems.DoForEach((bool (*)(void*))func);
}


void
BListView::DoForEach(bool (*func)(BListItem *item, void *user_data), void *user_data)
{
	fItems.DoForEach((bool (*)(void*, void*))func, user_data);
}


const BListItem**
BListView::Items() const
{
	return (const BListItem**)fItems.Items();
}


BRect
BListView::ItemFrame(int32 index) const
{
	BRect r;

	if (index >= 0 && index < fItems.CountItems()) {
		BListItem *item = (BListItem*)fItems.ItemAt(index);
		if (item->Height() >= 0) {
			r.Set(1, 1, Frame().Width() - 1, 1);

			for (int32 i = 0; i <= index; i++) {
				BListItem *item = (BListItem*)fItems.ItemAt(i);
				if (item->Height() < 0) continue;

				r.top = r.bottom;
				r.bottom = r.top + item->Height();
				if (r.top > 1) r.OffsetBy(0, 1);
			}
		}
	}

	return r;
}


void
BListView::InvalidateItem(int32 index)
{
	BRect r = ItemFrame(index);
	if (r.IsValid()) Invalidate(r, true);
}


void
BListView::ScrollToItem(int32 index)
{
	BRect rect = ItemFrame(index);
	if (rect.IsValid() == false) return;

	BRect vRect = Bounds();
	if (vRect.top <= rect.top && vRect.bottom >= rect.bottom) return;

	BPoint pt = rect.LeftTop();
	pt.ConstrainTo(vRect);

	ScrollTo(Frame().left - ConvertToParent(BPoint(0, 0)).x,
	         pt.y == vRect.top ? rect.top : rect.bottom - vRect.Height());
}


void
BListView::ScrollToSelection()
{
	BScrollView *scrollView = cast_as(Parent(), BScrollView);
	if (scrollView == NULL || scrollView->Target() != this) return;

	BRect rect = ItemFrame(fFirstSelected);
	if (rect.IsValid() == false) return;

	BRect vRect = ConvertFromParent(scrollView->TargetFrame());
	if (vRect.top <= rect.top && vRect.bottom >= rect.bottom) return;

	BPoint pt = rect.LeftTop();
	pt.ConstrainTo(vRect);

	ScrollTo(Frame().left - ConvertToParent(BPoint(0, 0)).x,
	         pt.y == vRect.top ? rect.top : rect.bottom - vRect.Height());
}


void
BListView::AttachedToWindow()
{
	if (Target() == NULL) SetTarget(Window());
}


void
BListView::DetachedFromWindow()
{
	if (Target() == Window()) SetTarget(NULL);
}


void
BListView::SetPosition(int32 pos)
{
	if (pos >= fItems.CountItems()) pos = -1;
	fPos = pos;
}


int32
BListView::Position() const
{
	return fPos;
}

