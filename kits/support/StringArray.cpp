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
 * File: StringArray.cpp
 *
 * --------------------------------------------------------------------------*/

#include "StringArray.h"


typedef struct __string_node__ {
	BString *str;
	void *data;

	__string_node__() {
		str = new BString();
		data = NULL;
	}

	~__string_node__() {
		if (str) delete str;
	}
} __string_node__;


BStringArray::BStringArray()
{
}


BStringArray::BStringArray(const char *string, void *attach_data)
{
	AddItem(string, attach_data);
}


BStringArray::BStringArray(const BString &string, void *attach_data)
{
	AddItem(string, attach_data);
}


BStringArray::BStringArray(const char **array)
{
	operator=(array);
}


BStringArray::BStringArray(const BStringArray &array)
{
	operator=(array);
}


BStringArray::~BStringArray()
{
	MakeEmpty();
}


void
BStringArray::MakeEmpty()
{
	if (!list.IsEmpty()) {
		for (int32 i = 0; i < list.CountItems(); i++) delete (__string_node__*)list.ItemAt(i);
		list.MakeEmpty();
	}
}


bool
BStringArray::IsEmpty() const
{
	return list.IsEmpty();
}


int32
BStringArray::CountItems() const
{
	return list.CountItems();
}


BStringArray&
BStringArray::operator=(const BStringArray &array)
{
	MakeEmpty();
	AddArray(array);
	return *this;
}


BStringArray&
BStringArray::operator=(const char **array)
{
	MakeEmpty();

	if (array) {
		while (*array) {
			AddItem(*array);
			array++;
		}
	}

	return *this;
}


BStringArray&
BStringArray::operator+=(const char *string)
{
	AddItem(string);
	return *this;
}


BStringArray&
BStringArray::operator+=(const BString &string)
{
	AddItem(string);
	return *this;
}


BStringArray&
BStringArray::operator+=(const BStringArray &array)
{
	AddArray(array);
	return *this;
}


BStringArray&
BStringArray::operator+=(const char **array)
{
	if (array) {
		while (*array) {
			AddItem(*array);
			array++;
		}
	}

	return *this;
}


bool
BStringArray::AddItem(const char *item, void *attach_data)
{
	__string_node__ *data = new __string_node__;
	if (!data || !data->str) {
		if (data) delete data;
		return false;
	}

	data->str->SetTo(item);
	data->data = attach_data;

	if (!list.AddItem((void*)data)) {
		delete data;
		return false;
	}

	return true;
}


bool
BStringArray::AddItem(const char *item, int32 atIndex, void *attach_data)
{
	__string_node__ *data = new __string_node__;
	if (!data || !data->str) {
		if (data) delete data;
		return false;
	}

	data->str->SetTo(item);
	data->data = attach_data;

	if (!list.AddItem((void*)data, atIndex)) {
		delete data;
		return false;
	}

	return true;
}


bool
BStringArray::AddItem(const BString &item, void *attach_data)
{
	return AddItem(item.String(), attach_data);
}


bool
BStringArray::AddItem(const BString &item, int32 atIndex, void *attach_data)
{
	return AddItem(item.String(), atIndex, attach_data);
}


bool
BStringArray::AddArray(const BStringArray &array)
{
	if (array.list.IsEmpty()) return false;

	BStringArray _array;
	for (int32 i = 0; i < array.list.CountItems(); i++) {
		const __string_node__ *node = (const __string_node__*)array.list.ItemAt(i);
		if (!node || !node->str) continue;
		if (_array.AddItem(node->str->String(), node->data) == false) return false;
	}

	if (list.AddList(&_array.list)) {
		_array.list.MakeEmpty();
		return true;
	}

	return false;
}


bool
BStringArray::AddArray(const BStringArray &array, int32 atIndex)
{
	if (array.list.IsEmpty()) return false;
	if (atIndex < 0 || atIndex >= list.CountItems()) return false;

	BStringArray _array;
	for (int32 i = 0; i < array.list.CountItems(); i++) {
		const __string_node__ *node = (const __string_node__*)array.list.ItemAt(i);
		if (!node || !node->str) continue;
		if (_array.AddItem(node->str->String(), node->data) == false) return false;
	}

	if (list.AddList(&_array.list, atIndex)) {
		_array.list.MakeEmpty();
		return true;
	}

	return false;
}


const BString*
BStringArray::ItemAt(int32 index, void **attach_data) const
{
	const __string_node__ *node = (const __string_node__*)list.ItemAt(index);
	if (!node) return NULL;
	if (attach_data) *attach_data = node->data;
	return node->str;
}


const BString*
BStringArray::FirstItem(void **attach_data) const
{
	const __string_node__ *node = (const __string_node__*)list.FirstItem();
	if (!node) return NULL;
	if (attach_data) *attach_data = node->data;
	return node->str;
}


const BString*
BStringArray::LastItem(void **attach_data) const
{
	const __string_node__ *node = (const __string_node__*)list.LastItem();
	if (!node) return NULL;
	if (attach_data) *attach_data = node->data;
	return node->str;
}


bool
BStringArray::RemoveItem(int32 index)
{
	__string_node__ *node = (__string_node__*)list.RemoveItem(index);

	if (node) {
		delete node;
		return true;
	}

	return false;
}


bool
BStringArray::RemoveItems(int32 index, int32 count)
{
	if (index < 0 || index >= list.CountItems()) return false;

	if (count < 0) count = list.CountItems() - index;
	else count = min_c(list.CountItems() - index, count);

	if (count == 0) return true;

	BList list_store(list);
	if (list_store.RemoveItems(0, index) == false) return false;
	if (list_store.CountItems() < count) return false;
	if (list_store.CountItems() > count)
		if (list_store.RemoveItems(count, -1) == false) return false;

	if (list.RemoveItems(index, count)) {
		for (int32 i = 0; i < list_store.CountItems(); i++) delete (__string_node__*)list_store.ItemAt(i);
		list_store.MakeEmpty();
		return true;
	}

	return false;
}


bool
BStringArray::ReplaceItem(int32 index, const char *string, void *attach_data)
{
	__string_node__ *node = (__string_node__*)list.ItemAt(index);

	if (node && node->str) {
		node->str->SetTo(string);
		node->data = attach_data;
		return true;
	}

	return false;
}


bool
BStringArray::ReplaceItem(int32 index, const BString &string, void *attach_data)
{
	__string_node__ *node = (__string_node__*)list.ItemAt(index);

	if (node && node->str) {
		node->str->SetTo(string);
		node->data = attach_data;
		return true;
	}

	return false;
}


BStringArray&
BStringArray::SortItems(int (*cmp)(const BString**, const BString**))
{
	list.SortItems((int (*)(const void*, const void*))cmp);

	return *this;
}


bool
BStringArray::SwapItems(int32 indexA, int32 indexB)
{
	if (indexA != indexB) return list.SwapItems(indexA, indexB);

	return true;
}


bool
BStringArray::MoveItem(int32 fromIndex, int32 toIndex)
{
	if (fromIndex != toIndex) return list.MoveItem(fromIndex, toIndex);

	return true;
}


int32
BStringArray::FindString(const char *string, int32 startIndex, bool all_equal, bool invert) const
{
	if (startIndex < 0 || startIndex >= list.CountItems()) return -1;

	int32 i = startIndex;

	while (i >= 0 && i < list.CountItems()) {
		const BString *str = ItemAt(i);

		if (str) {
			if (string == NULL || *string == 0) {
				if (str->Length() <= 0) return i;
			} else if (!all_equal) {
				if (str->FindFirst(string) >= 0) return i;
			} else {
				if (*str == string) return i;
			}
		} else if (string == NULL || *string == 0) {
			return i;
		}

		i += (invert ? -1 : 1);
	}

	return -1;
}


int32
BStringArray::FindString(const BString &string, int32 startIndex, bool all_equal, bool invert) const
{
	return FindString(string.String(), startIndex, all_equal, invert);
}


int32
BStringArray::IFindString(const char *string, int32 startIndex, bool all_equal, bool invert) const
{
	if (startIndex < 0 || startIndex >= list.CountItems()) return -1;

	int32 i = startIndex;

	while (i >= 0 && i < list.CountItems()) {
		const BString *str = ItemAt(i);

		if (str) {
			if (string == NULL || *string == 0) {
				if (str->Length() == 0) return i;
			} else if (!all_equal) {
				if (str->IFindFirst(string) >= 0) return i;
			} else {
				if (str->ICompare(string) == 0) return i;
			}
		} else if (string == NULL || *string == 0) {
			return i;
		}

		i += (invert ? -1 : 1);
	}

	return -1;
}


int32
BStringArray::IFindString(const BString &string, int32 startIndex, bool all_equal, bool invert) const
{
	return IFindString(string.String(), startIndex, all_equal, invert);
}

