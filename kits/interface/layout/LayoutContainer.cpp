/* --------------------------------------------------------------------------
 *
 * ETK++ --- The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2007, Anthony Lee, All Rights Reserved
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
 * File: LayoutContainer.cpp
 *
 * --------------------------------------------------------------------------*/

#include <support/ClassInfo.h>

#include "Layout.h"


BLayoutContainer::BLayoutContainer()
		: fUnitsPerPixel(1)
{
	fPrivate[0] = fPrivate[1] = NULL;
}


BLayoutContainer::~BLayoutContainer()
{
	BLayoutItem *item;
	while ((item = (BLayoutItem*)fItems.RemoveItem(0)) != NULL) {
		item->fContainer = NULL;
		delete item;
	}

	if (fPrivate[0] != NULL && fPrivate[1] != NULL)
		((void (*)(void*))fPrivate[1])(fPrivate[0]);
}


bool
BLayoutContainer::AddItem(BLayoutItem *item, int32 index)
{
	if (item == NULL || item->fContainer != NULL) return false;
	if (index < 0 || index > fItems.CountItems()) index = fItems.CountItems();

	if (fItems.AddItem((void*)item, index) == false) return false;

	item->fContainer = this;
	for (int32 i = index; i < fItems.CountItems(); i++) ((BLayoutItem*)fItems.ItemAt(i))->fIndex = i;
	if (!(item->fHidden || item->fFrame.IsValid() == false)) {
		BRect updateRect = item->fFrame;
		for (; item != NULL; item = item->NextSibling()) item->UpdateVisibleRegion();
		Invalidate(updateRect);
	}

	return true;
}


bool
BLayoutContainer::RemoveItem(BLayoutItem *item)
{
	if (item == NULL || item->fContainer != this) return false;

	int32 index = item->fIndex;
	if (fItems.RemoveItem(index) == false) return false;

	item->fContainer = NULL;
	item->fIndex = -1;
	item->UpdateVisibleRegion();

	for (int32 i = index; i < fItems.CountItems(); i++) ((BLayoutItem*)fItems.ItemAt(i))->fIndex = i;
	if (!(item->fHidden || item->fFrame.IsValid() == false)) {
		BRect updateRect = item->fFrame;
		for (item = (BLayoutItem*)fItems.ItemAt(index); item != NULL; item = item->NextSibling()) item->UpdateVisibleRegion();
		Invalidate(updateRect);
	}

	return true;
}


BLayoutItem*
BLayoutContainer::RemoveItem(int32 index)
{
	BLayoutItem *item = ItemAt(index);
	return(RemoveItem(item) ? item : NULL);
}


BLayoutItem*
BLayoutContainer::ItemAt(int32 index) const
{
	return (BLayoutItem*)fItems.ItemAt(index);
}


int32
BLayoutContainer::IndexOf(const BLayoutItem *item) const
{
	return((item == NULL || item->fContainer != this) ? -1 : item->fIndex);
}


int32
BLayoutContainer::CountItems() const
{
	return fItems.CountItems();
}


float
BLayoutContainer::UnitsPerPixel() const
{
	return fUnitsPerPixel;
}


void
BLayoutContainer::SetUnitsPerPixel(float value, bool deep)
{
	if (value <= 0) return;

	fUnitsPerPixel = value;

	BLayoutItem *item;
	if (deep) {
		item = (BLayoutItem*)fItems.ItemAt(0);
		while (item != NULL) {
			cast_as(item, BLayoutContainer)->fUnitsPerPixel = value;

			if (cast_as(item, BLayoutContainer)->fItems.CountItems() > 0) {
				item = (BLayoutItem*)cast_as(item, BLayoutContainer)->fItems.ItemAt(0);
			} else if (item->fContainer == this) {
				item = item->NextSibling();
			} else {
				if (item->NextSibling() != NULL) {
					item = item->NextSibling();
					continue;
				}

				while (cast_as(item->fContainer, BLayoutItem)->NextSibling() == NULL) {
					item = cast_as(item->fContainer, BLayoutItem);
					if (item->fContainer == this) break;
				}
				item = cast_as(item->fContainer, BLayoutItem)->NextSibling();
			}
		}
	}

	BRect updateRect;
	for (item = (BLayoutItem*)fItems.ItemAt(0); item != NULL; item = item->NextSibling()) {
		if (item->fHidden || item->fFrame.IsValid() == false) continue;
		updateRect |= item->fFrame;
		item->UpdateVisibleRegion();
	}
	Invalidate(updateRect);
}


void
BLayoutContainer::Invalidate(BRect rect)
{
}


void
BLayoutContainer::SetPrivateData(void *data, void (*destroy_func)(void*))
{
	if (fPrivate[0] != NULL && fPrivate[1] != NULL)
		((void (*)(void*))fPrivate[1])(fPrivate[0]);
	fPrivate[0] = data;
	fPrivate[1] = (void*)destroy_func;
}


void*
BLayoutContainer::PrivateData() const
{
	return fPrivate[0];
}

