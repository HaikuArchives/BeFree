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
 * File: LayoutItem.cpp
 *
 * --------------------------------------------------------------------------*/

#include <support/ClassInfo.h>

#include "Layout.h"


BLayoutItem::BLayoutItem(BRect frame, uint32 resizingMode)
		: BLayoutContainer(), fContainer(NULL),
		fIndex(-1),
		fLocalOrigin(B_ORIGIN),
		fFrame(frame), fResizingMode(resizingMode),
		fHidden(false), fUpdating(false)
{
}


BLayoutItem::~BLayoutItem()
{
	RemoveSelf();
}


BLayoutContainer*
BLayoutItem::Container() const
{
	return fContainer;
}


BLayoutContainer*
BLayoutItem::Ancestor() const
{
	if (fContainer == NULL) return cast_as((BLayoutItem*)this, BLayoutContainer);
	if (is_kind_of(fContainer, BLayoutItem) == false) return fContainer;
	return cast_as(fContainer, BLayoutItem)->Ancestor();
}


BLayoutItem*
BLayoutItem::PreviousSibling() const
{
	return(fContainer == NULL ? NULL : (BLayoutItem*)fContainer->ItemAt(fIndex - 1));
}


BLayoutItem*
BLayoutItem::NextSibling() const
{
	return(fContainer == NULL ? NULL : (BLayoutItem*)fContainer->ItemAt(fIndex + 1));
}


bool
BLayoutItem::RemoveSelf()
{
	if (fContainer == NULL) return false;
	return fContainer->RemoveItem(this);
}


void
BLayoutItem::SetResizingMode(uint32 mode)
{
	fResizingMode = mode;
}


uint32
BLayoutItem::ResizingMode() const
{
	return fResizingMode;
}


void
BLayoutItem::Show()
{
	if (fHidden == false) return;

	fHidden = false;

	if (fUpdating || fContainer == NULL || fFrame.IsValid() == false) return;
	for (BLayoutItem *item = this; item != NULL; item = item->NextSibling()) item->UpdateVisibleRegion();
	fContainer->Invalidate(fFrame);
}


void
BLayoutItem::Hide()
{
	if (fHidden == true) return;

	fHidden = true;

	if (fUpdating || fContainer == NULL || fFrame.IsValid() == false) return;
	for (BLayoutItem *item = this; item != NULL; item = item->NextSibling()) item->UpdateVisibleRegion();
	fContainer->Invalidate(fFrame);
}


bool
BLayoutItem::IsHidden(bool check_containers) const
{
	if (check_containers == false) return fHidden;
	if (fHidden || fContainer == NULL) return true;
	if (is_kind_of(fContainer, BLayoutItem) == false) return false;
	return cast_as(fContainer, BLayoutItem)->IsHidden(true);
}


void
BLayoutItem::SendBehind(BLayoutItem *item)
{
	if (fContainer == NULL || item == NULL || !(item->fContainer == fContainer && item->fIndex > fIndex)) return;

	int32 index = (item == NULL ? 0 : item->fIndex);
	if (fContainer->fItems.MoveItem(fIndex, index) == false) return;

	for (int32 i = min_c(index, fIndex); i < fContainer->fItems.CountItems(); i++)
		((BLayoutItem*)fContainer->fItems.ItemAt(i))->fIndex = i;

	if (fUpdating || fHidden || fFrame.IsValid() == false) return;
	BRect updateRect = fFrame | item->fFrame;
	for (int32 i = min_c(index, fIndex); i < fContainer->fItems.CountItems(); i++)
		((BLayoutItem*)fContainer->fItems.ItemAt(i))->UpdateVisibleRegion();
	fContainer->Invalidate(updateRect);
}


void
BLayoutItem::MoveTo(BPoint where)
{
	if (fFrame.LeftTop() == where) return;

	BRect oldFrame = fFrame;
	fFrame.OffsetTo(where);

	if (fUpdating || fContainer == NULL || fHidden || fFrame.IsValid() == false) return;
	for (BLayoutItem *item = this; item != NULL; item = item->NextSibling()) item->UpdateVisibleRegion();
	fContainer->Invalidate(oldFrame | fFrame);
}


void
BLayoutItem::ScrollTo(BPoint where)
{
	if (where.x < 0) where.x = 0;
	if (where.y < 0) where.y = 0;

	if (fLocalOrigin == where) return;

	fLocalOrigin = where;

	if (fUpdating || fContainer == NULL || fHidden || fFrame.IsValid() == false) return;
	UpdateVisibleRegion();
	fContainer->Invalidate(fFrame);
}


void
BLayoutItem::ResizeTo(float width, float height)
{
	if (fFrame.Width() == width && fFrame.Height() == height) return;

	float width_ext = width - fFrame.Width();
	float height_ext = height - fFrame.Height();
	BRect oldFrame = fFrame;
	BPoint center_offset = fFrame.Center() - oldFrame.Center();

	fFrame.right += width_ext;
	fFrame.bottom += height_ext;

	for (BLayoutItem *item = ItemAt(0); item != NULL; item = item->NextSibling()) {
		uint32 iMode = item->fResizingMode;
		BRect iFrame = item->fFrame;

		if (iMode == B_FOLLOW_NONE || iMode == (B_FOLLOW_LEFT |B_FOLLOW_TOP)) continue;

		if ((iMode & B_FOLLOW_H_CENTER) && (iMode &B_FOLLOW_LEFT_RIGHT) != B_FOLLOW_LEFT_RIGHT) {
			float newCenterX = iFrame.Center().x + center_offset.x;
			if (iMode & B_FOLLOW_RIGHT) {
				iFrame.right += width_ext;
				iFrame.left = iFrame.right - 2.f * (iFrame.right - newCenterX);
			} else if (iMode & B_FOLLOW_LEFT) {
				iFrame.right = iFrame.left + 2.f * (newCenterX - iFrame.left);
			} else {
				float iWidth = iFrame.Width();
				iFrame.left = newCenterX - iWidth / 2.f;
				iFrame.right = newCenterX + iWidth / 2.f;
			}
		} else if (iMode & B_FOLLOW_RIGHT) {
			iFrame.right += width_ext;
			if (!(iMode & B_FOLLOW_LEFT)) iFrame.left += width_ext;
		}

		if ((iMode & B_FOLLOW_V_CENTER) && (iMode &B_FOLLOW_TOP_BOTTOM) != B_FOLLOW_TOP_BOTTOM) {
			float newCenterY = iFrame.Center().y + center_offset.y;
			if (iMode & B_FOLLOW_TOP_BOTTOM) {
				iFrame.bottom += height_ext;
				iFrame.top = iFrame.bottom - 2.f * (iFrame.bottom - newCenterY);
			} else if (iMode & B_FOLLOW_TOP) {
				iFrame.bottom = iFrame.top + 2.f * (newCenterY - iFrame.top);
			} else {
				float iHeight = iFrame.Height();
				iFrame.top = newCenterY - iHeight / 2.f;
				iFrame.bottom = newCenterY + iHeight / 2.f;
			}
		} else if (iMode & B_FOLLOW_BOTTOM) {
			iFrame.bottom += height_ext;
			if (!(iMode & B_FOLLOW_TOP)) iFrame.top += height_ext;
		}

		item->fFrame.OffsetTo(iFrame.LeftTop());
		item->fUpdating = true;
		item->ResizeTo(iFrame.Width(), iFrame.Height());
		item->fUpdating = false;
	}

	if (fUpdating || fContainer == NULL || fHidden || (oldFrame.IsValid() == false && fFrame.IsValid() == false)) return;
	for (BLayoutItem *item = this; item != NULL; item = item->NextSibling()) item->UpdateVisibleRegion();
	fContainer->Invalidate(oldFrame | fFrame);
}


void
BLayoutItem::MoveAndResizeTo(BPoint where, float width, float height)
{
	BRect oldFrame = fFrame;

	bool saveUpdating = fUpdating;
	fUpdating = true;
	MoveTo(where);
	ResizeTo(width, height);
	fUpdating = saveUpdating;

	if (oldFrame == fFrame) return;
	if (fUpdating || fContainer == NULL || fHidden || (oldFrame.IsValid() == false && fFrame.IsValid() == false)) return;
	for (BLayoutItem *item = this; item != NULL; item = item->NextSibling()) item->UpdateVisibleRegion();
	fContainer->Invalidate(oldFrame | fFrame);
}


void
BLayoutItem::GetPreferredSize(float *width, float *height)
{
	if (width == NULL && height == NULL) return;

	float w = 0, h = 0;
	BRect rect = fFrame.OffsetToCopy(B_ORIGIN);

	for (BLayoutItem *item = ItemAt(0); item != NULL; item = item->NextSibling()) {
		if (item->fHidden) continue;

		float iW = 0, iH = 0;
		item->GetPreferredSize(&iW, &iH);

		uint32 iMode = item->fResizingMode;
		if ((iMode & B_FOLLOW_LEFT) || (iMode &B_FOLLOW_NONE)) iW += item->fFrame.left;
		if (iMode & B_FOLLOW_RIGHT) iW += rect.right - item->fFrame.right;
		if ((iMode & B_FOLLOW_TOP) || (iMode &B_FOLLOW_NONE)) iH += item->fFrame.top;
		if (iMode & B_FOLLOW_BOTTOM) iH += rect.bottom - item->fFrame.bottom;

		w = max_c(w, iW);
		h = max_c(h, iH);
	}

	if (width) *width = w;
	if (height) *height = h;
}


void
BLayoutItem::ResizeToPreferred()
{
	float w = -1, h = -1;
	GetPreferredSize(&w, &h);
	if (w < 0) w = fFrame.Width();
	if (h < 0) h = fFrame.Height();
	if (w == fFrame.Width() && h == fFrame.Height()) return;

	BRect iFrame = fFrame;
	uint32 iMode = fResizingMode;

	if ((iMode & B_FOLLOW_H_CENTER) && (iMode &B_FOLLOW_LEFT_RIGHT) != B_FOLLOW_LEFT_RIGHT) {
		float centerX = fFrame.Center().x;
		iFrame.left = centerX - w / 2.f;
		iFrame.right = centerX + w / 2.f;
	} else if ((iMode & B_FOLLOW_LEFT_RIGHT) != B_FOLLOW_LEFT_RIGHT) {
		if (iMode & B_FOLLOW_RIGHT)
			iFrame.left = iFrame.right - w;
		else
			iFrame.right = iFrame.left + w;
	}

	if ((iMode & B_FOLLOW_V_CENTER) && (iMode &B_FOLLOW_TOP_BOTTOM) != B_FOLLOW_TOP_BOTTOM) {
		float centerY = fFrame.Center().y;
		iFrame.top = centerY - h / 2.f;
		iFrame.bottom = centerY + h / 2.f;
	} else if ((iMode & B_FOLLOW_TOP_BOTTOM) != B_FOLLOW_TOP_BOTTOM) {
		if (iMode & B_FOLLOW_BOTTOM)
			iFrame.top = iFrame.bottom - h;
		else
			iFrame.bottom = iFrame.top + h;
	}

	if (iFrame == fFrame) return;

	bool saveUpdating = fUpdating;
	BRect oldFrame = fFrame;

	fUpdating = true;
	MoveAndResizeTo(iFrame.LeftTop(), iFrame.Width(), iFrame.Height());
	fUpdating = saveUpdating;

	if (fUpdating || fContainer == NULL || fHidden || (oldFrame.IsValid() == false && fFrame.IsValid() == false)) return;
	for (BLayoutItem *item = this; item != NULL; item = item->NextSibling()) item->UpdateVisibleRegion();
	fContainer->Invalidate(oldFrame | fFrame);
}


BRect
BLayoutItem::Bounds() const
{
	return fFrame.OffsetToCopy(fLocalOrigin);
}


BRect
BLayoutItem::Frame() const
{
	return fFrame;
}


const BRegion*
BLayoutItem::VisibleRegion() const
{
	return &fVisibleRegion;
}


void
BLayoutItem::GetVisibleRegion(BRegion **region)
{
	if (region) *region = &fVisibleRegion;
}


BPoint
BLayoutItem::LeftTop() const
{
	return fLocalOrigin;
}


float
BLayoutItem::Width() const
{
	return fFrame.Width();
}


float
BLayoutItem::Height() const
{
	return fFrame.Height();
}


void
BLayoutItem::ConvertToContainer(BPoint *pt) const
{
	if (pt == NULL) return;

	*pt -= fLocalOrigin;
	*pt += fFrame.LeftTop();
}


BPoint
BLayoutItem::ConvertToContainer(BPoint pt) const
{
	ConvertToContainer(&pt);
	return pt;
}


void
BLayoutItem::ConvertFromContainer(BPoint *pt) const
{
	if (pt == NULL) return;

	*pt -= fFrame.LeftTop();
	*pt += fLocalOrigin;
}


BPoint
BLayoutItem::ConvertFromContainer(BPoint pt) const
{
	ConvertFromContainer(&pt);
	return pt;
}


void
BLayoutItem::UpdateVisibleRegion()
{
	BLayoutItem *item;

	if (fUpdating) return;

	if (fContainer == NULL || fHidden || fFrame.IsValid() == false) {
		fVisibleRegion.MakeEmpty();
	} else {
		if (is_kind_of(fContainer, BLayoutItem) == false) {
			fVisibleRegion = fFrame;
		} else {
			fVisibleRegion = cast_as(fContainer, BLayoutItem)->fVisibleRegion;
			fVisibleRegion &= fFrame;
		}

		for (item = fContainer->ItemAt(0);
		        item != NULL && item != this && fVisibleRegion.CountRects() > 0;
		        item = item->NextSibling()) {
			if (item->fHidden || item->fFrame.IsValid() == false) continue;
			fVisibleRegion.Exclude(item->fFrame.InsetByCopy(-fContainer->UnitsPerPixel(), -fContainer->UnitsPerPixel()));
		}

		fVisibleRegion.OffsetBy(B_ORIGIN - fFrame.LeftTop() + fLocalOrigin);
	}

	for (item = ItemAt(0); item != NULL; item = item->NextSibling()) item->UpdateVisibleRegion();
}

