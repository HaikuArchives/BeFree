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
 * File: List.cpp
 * Description: BList --- ordered list of data pointers
 *
 * --------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>

#include "List.h"

#define MAX_LIST_COUNT	(B_MAXINT32 - 1)


bool
BList::_Resize(int32 count)
{
	if (count <= 0) {
		if (fMinimumCount > 0) {
			void **newObjects = (void**)realloc(fObjects, (size_t)(fMinimumCount + 1) * sizeof(void*));
			if (newObjects != NULL) {
				fObjects = newObjects;
				fItemReal = fMinimumCount + 1;
			}
		} else {
			if (fObjects) free(fObjects);
			fObjects = NULL;
			fItemReal = 0;
		}

		fItemCount = 0;
		if (fObjects) bzero(fObjects, sizeof(void*));

		return true;
	} else if (count > MAX_LIST_COUNT) {
		return false;
	} else if (count <= fMinimumCount || count + 1 == fItemReal) {
		fItemCount = count;
		bzero(fObjects + count, sizeof(void*));
		return true;
	}

	void **newObjects = (void**)realloc(fObjects, (size_t)(count + 1) * sizeof(void*));
	if (newObjects == NULL) {
		if (count + 1 < fItemReal) {
			bzero(fObjects + count, (size_t)(fItemReal - count) * sizeof(void*));
			fItemCount = count;
			return true;
		} else {
			return false;
		}
	}

	if (fItemReal < count + 1)
		bzero(newObjects + fItemReal, (size_t)(count + 1 - fItemReal) * sizeof(void*));
	else
		bzero(newObjects + count, sizeof(void*));

	fObjects = newObjects;
	fItemCount = count;
	fItemReal = count + 1;

	return true;
}


BList::BList(int32 initialAllocSize)
		: fObjects(NULL), fItemCount(0), fItemReal(0), fMinimumCount(0)
{
	if (initialAllocSize > 0 && initialAllocSize <= MAX_LIST_COUNT) {
		if (_Resize(initialAllocSize)) {
			fMinimumCount = initialAllocSize;
			fItemCount = 0;
		}
	}
}


BList::BList(int32 initialAllocSize, int32 nullItems)
		: fObjects(NULL), fItemCount(0), fItemReal(0), fMinimumCount(0)
{
	if (initialAllocSize > 0 && initialAllocSize <= MAX_LIST_COUNT) {
		if (_Resize(initialAllocSize)) {
			fMinimumCount = initialAllocSize;
			fItemCount = 0;
		}
	}

	if (nullItems > 0 && nullItems <= MAX_LIST_COUNT) _Resize(nullItems);
}


BList::BList(const BList& list)
		: fObjects(NULL), fItemCount(0), fItemReal(0), fMinimumCount(0)
{
	BList::operator=(list);
}


BList::~BList()
{
	if (fObjects) free(fObjects);
}


BList&
BList::operator=(const BList &from)
{
	if (fObjects) free(fObjects);
	fObjects = NULL;
	fItemCount = 0;
	fItemReal = 0;
	fMinimumCount = 0;

	if (from.fMinimumCount > 0 && from.fMinimumCount <= MAX_LIST_COUNT) {
		if (_Resize(from.fMinimumCount)) {
			fMinimumCount = from.fMinimumCount;
			fItemCount = 0;
		}
	}

	BList::AddList(&from);

	return *this;
}


bool
BList::AddItem(void *item)
{
	if (fItemCount >= MAX_LIST_COUNT) return false;

	if (!_Resize(fItemCount + 1)) return false;

	fObjects[fItemCount - 1] = item;

	return true;
}



bool
BList::AddItem(void *item, int32 atIndex)
{
	if (atIndex < 0 || atIndex > fItemCount) return false;
	if (atIndex == fItemCount) return AddItem(item);
	if (fItemCount >= MAX_LIST_COUNT) return false;

	if (!_Resize(fItemCount + 1)) return false;

	if (memmove(fObjects + atIndex + 1, fObjects + atIndex, (fItemCount - (atIndex + 1)) * sizeof(void*)) == NULL) {
		fItemCount--;
		return false;
	}

	fObjects[atIndex] = item;

	return true;
}


bool
BList::AddList(const BList *newItems)
{
	if (!newItems) return false;
	if (newItems->IsEmpty()) return false;

	if (MAX_LIST_COUNT - fItemCount < newItems->CountItems()) return false;

	if (!_Resize(fItemCount + newItems->CountItems())) return false;

	void *newData = (void*)(newItems->Items());

	if (memcpy(fObjects + fItemCount - newItems->CountItems(), newData, newItems->CountItems() * sizeof(void*)) == NULL) {
		fItemCount -= newItems->CountItems();
		return false;
	}

	return true;
}


bool
BList::AddList(const BList *newItems, int32 atIndex)
{
	if (fItemCount == 0 && atIndex == 0) return AddList(newItems);

	if (atIndex < 0 || atIndex >= fItemCount) return false;

	if (!newItems) return false;
	if (newItems->IsEmpty()) return false;

	if (MAX_LIST_COUNT - fItemCount < newItems->fItemCount) return false;

	if (!_Resize(fItemCount + newItems->fItemCount)) return false;

	void **newObjects = (void**)malloc((fItemCount - atIndex + newItems->fItemCount) * sizeof(void*));
	if (!newObjects) {
		fItemCount -= newItems->fItemCount;
		return false;
	}

	if (memcpy(newObjects, newItems->fObjects, newItems->fItemCount * sizeof(void*)) == NULL) {
		delete newObjects;
		fItemCount -= newItems->fItemCount;
		return false;
	}

	if (memcpy(newObjects + newItems->fItemCount, fObjects + atIndex, (fItemCount - atIndex) * sizeof(void*)) == NULL) {
		delete newObjects;
		fItemCount -= newItems->fItemCount;
		return false;
	}

	if (memcpy(fObjects + atIndex, newObjects,  (fItemCount - atIndex + newItems->fItemCount) * sizeof(void*)) == NULL) {
		delete newObjects;
		fItemCount -= newItems->fItemCount;
		return false;
	}

	delete newObjects;

	return true;
}


bool
BList::RemoveItem(void *item)
{
	if (item == NULL) return false;
	return(RemoveItem(IndexOf(item)) != NULL);
}


void*
BList::RemoveItem(int32 index)
{
	if (index < 0 || index >= fItemCount) return NULL;

	void *data = fObjects[index];

	if (index < fItemCount - 1) {
		if (memmove(fObjects + index, fObjects + index + 1, (fItemCount - index - 1) * sizeof(void*)) == NULL) return NULL;
	}

	if (!_Resize(fItemCount - 1)) {
		fItemCount--;
		bzero(fObjects + fItemCount, sizeof(void*));
	}

	return data;
}


bool
BList::RemoveItems(int32 index, int32 count)
{
	if (index < 0 || index >= fItemCount) return false;

	if (count < 0) count = fItemCount - index;
	else count = min_c(fItemCount - index, count);

	if (count == 0) return true;

	if (index < (fItemCount - 1) && count != (fItemCount - index)) {
		if (memmove(fObjects + index, fObjects + index + count, (fItemCount - index - count) * sizeof(void*)) == NULL) return false;
	}

	if (!_Resize(fItemCount - count)) {
		fItemCount -= count;
		bzero(fObjects + fItemCount, (fItemReal - fItemCount) * sizeof(void*));
	}

	return true;
}


bool
BList::ReplaceItem(int32 index, void *newItem, void **oldItem)
{
	if (index < 0 || index >= fItemCount) return false;

	if (oldItem) *oldItem = fObjects[index];
	fObjects[index] = newItem;

	return true;
}


void
BList::MakeEmpty()
{
	_Resize(0);
}


bool
BList::SwapItems(int32 indexA, int32 indexB)
{
	if (indexA < 0 || indexA >= fItemCount || indexB < 0 || indexB >= fItemCount) return false;

	void *dataA = fObjects[indexA];

	fObjects[indexA] = fObjects[indexB];
	fObjects[indexB] = dataA;

	return true;
}


bool
BList::MoveItem(int32 fromIndex, int32 toIndex)
{
	if (fromIndex < 0 || fromIndex >= fItemCount || toIndex < 0 || toIndex >= fItemCount) return false;
	if (fromIndex == toIndex) return true;

	void *fromData = fObjects[fromIndex];

	if (toIndex > fromIndex) {
		if (memmove(fObjects + fromIndex, fObjects + fromIndex + 1, (toIndex - fromIndex) * sizeof(void*)) == NULL) return false;
	} else {
		if (memmove(fObjects + toIndex + 1, fObjects + toIndex, (fromIndex - toIndex) * sizeof(void*)) == NULL) return false;
	}

	fObjects[toIndex] = fromData;

	return true;
}


void
BList::SortItems(int (*cmp)(const void *, const void *))
{
	if (cmp && fItemCount > 1) qsort((void*)fObjects, fItemCount, sizeof(void*), cmp);
}


void*
BList::ItemAt(int32 index) const
{
	if (index < 0 || index >= fItemCount) return NULL;

	return fObjects[index];
}


void*
BList::FirstItem() const
{
	if (fItemCount > 0) return fObjects[0];
	return NULL;
}


void*
BList::LastItem() const
{
	if (fItemCount > 0) return fObjects[fItemCount - 1];
	return NULL;
}


bool
BList::HasItem(void *item) const
{
	if (item == NULL) return false;
	return(IndexOf(item) >= 0);
}


int32
BList::IndexOf(void *item) const
{
	for (int32 i = 0; i < fItemCount; i++) {
		if (fObjects[i] == item) return i;
	}

	return -1;
}


int32
BList::CountItems() const
{
	return fItemCount;
}


bool
BList::IsEmpty() const
{
	return(fItemCount == 0);
}


void
BList::DoForEach(bool (*func)(void *))
{
	if (!func) return;

	for (int32 i = 0; i < fItemCount; i++) {
		if ((*func)(fObjects[i])) break;
	}
}


void
BList::DoForEach(bool (*func)(void *, void *), void *user_data)
{
	if (!func) return;

	for (int32 i = 0; i < fItemCount; i++) {
		if ((*func)(fObjects[i], user_data)) break;
	}
}


void**
BList::Items() const
{
	return(fObjects);
}

