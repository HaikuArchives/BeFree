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
 * File: ListView.h
 * Description: BListView --- Displays a list of items the user can select and invoke
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_LIST_VIEW_H__
#define __ETK_LIST_VIEW_H__

#include <support/List.h>
#include <app/Invoker.h>
#include <interface/ListItem.h>

typedef enum list_view_type {
	B_SINGLE_SELECTION_LIST,
	B_MULTIPLE_SELECTION_LIST,
} list_view_type;


#ifdef __cplusplus /* Just for C++ */

class BListView : public BView, public BInvoker
{
	public:
		BListView(BRect frame,
		          const char *name,
		          list_view_type type = B_SINGLE_SELECTION_LIST,
		          uint32 resizingMode = B_FOLLOW_LEFT |B_FOLLOW_TOP,
		          uint32 flags = B_WILL_DRAW |B_NAVIGABLE |B_FRAME_EVENTS);
		virtual ~BListView();

		virtual bool		AddItem(BListItem *item);
		virtual bool		AddItem(BListItem *item, int32 atIndex);
		virtual bool		RemoveItem(BListItem *item, bool auto_destruct_item = true);
		virtual BListItem	*RemoveItem(int32 index);
		virtual bool		RemoveItems(int32 index, int32 count, bool auto_destruct_items = true);

		virtual void		SetListType(list_view_type type);
		list_view_type	ListType() const;

		virtual void		SetSelectionMessage(BMessage *message);
		virtual void		SetInvocationMessage(BMessage *message);
		BMessage		*SelectionMessage() const;
		uint32			SelectionCommand() const;
		BMessage		*InvocationMessage() const;
		uint32			InvocationCommand() const;
		virtual status_t	Invoke(const BMessage *message = NULL);

		// Empty functions BEGIN --- just for derivative class
		virtual void		SelectionChanged();
		// Empty functions END

		void			Select(int32 index, bool extend = false);
		void			Select(int32 start, int32 finish, bool extend = false);
		bool			IsItemSelected(int32 index) const;
		int32			CurrentSelection(int32 index = 0) const;

		void			Deselect(int32 index);
		void			DeselectAll();
		void			DeselectExcept(int32 start, int32 finish);

		BRect			ItemFrame(int32 index) const;
		void			InvalidateItem(int32 index);
		void			ScrollToSelection();

		BListItem		*ItemAt(int32 index) const;
		BListItem		*FirstItem() const;
		BListItem		*LastItem() const;
		int32			IndexOf(const BListItem *item) const;
		int32			IndexOf(BPoint where, bool mustVisible = false) const;
		bool			HasItem(const BListItem *item) const;
		int32			CountItems() const;
		virtual void		MakeEmpty();
		bool			IsEmpty() const;
		bool			SwapItems(int32 indexA, int32 indexB);
		bool			MoveItem(int32 fromIndex, int32 toIndex);

		// ReplaceItem(): when "oldItem" assigned to NULL, the old item will be destructed automatically.
		bool			ReplaceItem(int32 index, BListItem *newItem, BListItem **oldItem = NULL);

		void			SortItems(int (*cmp)(const BListItem **a, const BListItem **b));
		void			DoForEach(bool (*func)(BListItem *item));
		void			DoForEach(bool (*func)(BListItem *item, void *user_data), void *user_data);

		// Items(): return the list, use it carefully please
		const BListItem		**Items() const;

		virtual void		Draw(BRect updateRect);
		virtual void		MouseDown(BPoint where);
		virtual void		MouseUp(BPoint where);
		virtual void		MouseMoved(BPoint where, uint32 code, const BMessage *a_message);
		virtual void		KeyDown(const char *bytes, int32 numBytes);
		virtual void		KeyUp(const char *bytes, int32 numBytes);
		virtual void		SetFont(const BFont *font, uint8 mask = B_FONT_ALL);
		virtual void		MakeFocus(bool focusState = true);
		virtual void		WindowActivated(bool state);
		virtual void		AttachedToWindow();
		virtual void		DetachedFromWindow();

	protected:
		void			SetPosition(int32 pos);
		int32			Position() const;
		void			ScrollToItem(int32 index);

	private:
		BList fItems;
		list_view_type fListType;
		int32 fFirstSelected;
		int32 fLastSelected;
		int32 fPos;

		BMessage *fSelectionMessage;
};

#endif /* __cplusplus */

#endif /* __ETK_LIST_VIEW_H__ */

