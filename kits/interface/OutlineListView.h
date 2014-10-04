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
 * File: OutlineListView.h
 * Description: BOutlineListView --- Displays a list of items that can be structured like an outline
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_OUTLINB_LIST_VIEW_H__
#define __ETK_OUTLINB_LIST_VIEW_H__

#include <interface/ListView.h>

#ifdef __cplusplus /* Just for C++ */

class BOutlineListView : public BListView
{
	public:
		BOutlineListView(BRect frame,
		                 const char *name,
		                 list_view_type type = B_SINGLE_SELECTION_LIST,
		                 uint32 resizingMode = B_FOLLOW_LEFT |B_FOLLOW_TOP,
		                 uint32 flags = B_WILL_DRAW |B_NAVIGABLE |B_FRAME_EVENTS);
		virtual ~BOutlineListView();

		virtual bool		AddUnder(BListItem *item, BListItem *superitem);

		virtual bool		AddItem(BListItem *item);
		virtual bool		AddItem(BListItem *item, int32 fullListIndex);
		virtual bool		RemoveItem(BListItem *item, bool auto_destruct_item_and_subitems = true);
		virtual bool		RemoveItems(int32 fullListIndex, int32 count, bool auto_destruct_items = true);

		virtual BListItem	*RemoveItem(int32 fullListIndex, bool auto_destruct_subitems, int32 *count);
		virtual BListItem	*RemoveItem(int32 fullListIndex); // same as RemoveItem(fullListIndex, true, NULL)

		BListItem		*FullListItemAt(int32 fullListIndex) const;
		int32			FullListIndexOf(const BListItem *item) const;
		BListItem		*FullListFirstItem() const;
		BListItem		*FullListLastItem() const;
		bool			FullListHasItem(const BListItem *item) const;
		int32			FullListCountItems() const;
		int32			FullListCurrentSelection(int32 index = 0) const;
		virtual void		MakeEmpty();
		bool			FullListIsEmpty() const;

		void			FullListDoForEach(bool (*func)(BListItem *item));
		void			FullListDoForEach(bool (*func)(BListItem *item, void *user_data), void *user_data);

		BListItem		*Superitem(const BListItem *item) const;
		bool			HasSubitems(const BListItem *item) const;

		int32			CountItemsUnder(BListItem *item, bool oneLevelOnly) const;
		BListItem		*EachItemUnder(BListItem *item, bool oneLevelOnly,
		                          BListItem *(*eachFunc)(BListItem *item, void *user_data), void *user_data);
		BListItem		*ItemUnderAt(BListItem *item, bool oneLevelOnly, int32 index) const;

		void			Expand(BListItem *item);
		void			Collapse(BListItem *item);
		bool			IsExpanded(int32 fullListIndex) const;

		// FullListItems(): return the list, use it carefully please
		const BListItem		**FullListItems() const;

		virtual void		MouseDown(BPoint where);
		virtual void		MouseUp(BPoint where);
		virtual void		MouseMoved(BPoint where, uint32 code, const BMessage *a_message);
		virtual void		KeyDown(const char *bytes, int32 numBytes);
		virtual void		KeyUp(const char *bytes, int32 numBytes);

	private:
		BList fFullItems;
};

#endif /* __cplusplus */

#endif /* __ETK_OUTLINB_LIST_VIEW_H__ */

