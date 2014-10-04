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
 * File: List.h
 * Description: BList --- ordered list of data pointers
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_LIST_H__
#define __ETK_LIST_H__

#include <support/SupportDefs.h>

#ifdef __cplusplus /* Just for C++ */

class BList
{
	public:
		// BList(int32), BList(int32, int32):
		// 	The argument "initialAllocSize" is the minimum count to hold in memory.
		// 		Valid range: 1 ~ (B_MAXINT32 - 1)
		// 		When you pass invalid value to "initialAllocSize", the minimum count just equal to 0.
		// 	The argument "nullItems" is the count to preallocate NULL items for ReplaceItem().
		BList(int32 initialAllocSize = 0);
		BList(int32 initialAllocSize, int32 nullItems);

		// BList(const BList&),operator=(const BList&):
		// 	The minimum count to hold same as "list" when possible.
		BList(const BList &list);
		BList &operator=(const BList &from);

		virtual ~BList();

		bool	AddItem(void *item);
		bool	AddItem(void *item, int32 atIndex);
		bool	AddList(const BList *newItems);
		bool	AddList(const BList *newItems, int32 atIndex);

		// RemoveItem(),RemoveItems(): the item WOULD NOT be destructed yet.
		bool	RemoveItem(void *item);
		void	*RemoveItem(int32 index);
		bool	RemoveItems(int32 index, int32 count);

		// ReplaceItem(): the old item WOULD NOT be destructed yet.
		bool	ReplaceItem(int32 index, void *newItem, void **oldItem = NULL);

		void	MakeEmpty();

		bool	SwapItems(int32 indexA, int32 indexB);
		bool	MoveItem(int32 fromIndex, int32 toIndex);

		void	SortItems(int (*cmp)(const void *a, const void *b));

		void	*ItemAt(int32 index) const;
		void	*FirstItem() const;
		void	*LastItem() const;

		bool	HasItem(void *item) const;
		int32	IndexOf(void *item) const;
		int32	CountItems() const;
		bool	IsEmpty() const;

		void	DoForEach(bool (*func)(void *data));
		void	DoForEach(bool (*func)(void *data, void *user_data), void *user_data);

		// Items(): return the list, use it carefully please
		void	**Items() const;

	private:
		void **fObjects;

		int32 fItemCount;
		int32 fItemReal;
		int32 fMinimumCount;

		bool _Resize(int32 count);
};

#endif /* __cplusplus */

#endif /* __ETK_LIST_H__ */

