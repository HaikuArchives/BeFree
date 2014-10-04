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
 * File: Layout.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_LAYOUT_H__
#define __ETK_LAYOUT_H__

#include <support/List.h>
#include <interface/InterfaceDefs.h>
#include <interface/Region.h>

#ifdef __cplusplus /* Just for C++ */

class BLayoutItem;
class BLayoutForm;


class BLayoutContainer {
public:
	BLayoutContainer();
	virtual ~BLayoutContainer();

	virtual bool		AddItem(BLayoutItem *item, int32 index = -1);
	virtual bool		RemoveItem(BLayoutItem *item);
	BLayoutItem		*RemoveItem(int32 index);

	BLayoutItem		*ItemAt(int32 index) const;
	int32			IndexOf(const BLayoutItem *item) const;
	int32			CountItems() const;

	float			UnitsPerPixel() const;
	void			SetUnitsPerPixel(float value, bool deep = true);

	virtual void		Invalidate(BRect rect);

	void			SetPrivateData(void *data, void (*destroy_func)(void*) = NULL);
	void			*PrivateData() const;

private:
	friend class BLayoutItem;

	float fUnitsPerPixel;
	BList fItems;
	void *fPrivate[2];
};


class BLayoutItem :
public BLayoutContainer {
public:
	BLayoutItem(BRect frame, uint32 resizingMode);
	virtual ~BLayoutItem();

	BLayoutContainer	*Container() const;
	BLayoutContainer	*Ancestor() const;

	bool			RemoveSelf();

	BLayoutItem		*PreviousSibling() const;
	BLayoutItem		*NextSibling() const;

	virtual void		SetResizingMode(uint32 mode);
	uint32			ResizingMode() const;

	virtual void		Show();
	virtual void		Hide();
	bool			IsHidden(bool check_containers = true) const;

	virtual void		SendBehind(BLayoutItem *item);
	virtual void		MoveTo(BPoint where);
	virtual void		ScrollTo(BPoint where);
	virtual void		ResizeTo(float width, float height);
	void			MoveAndResizeTo(BPoint where, float width, float height);

	virtual void		GetPreferredSize(float *width, float *height);
	virtual void		ResizeToPreferred();

	BRect			Bounds() const; // in it's coordinate system
	BRect			Frame() const; // in container's coordinate system
	const BRegion		*VisibleRegion() const; // in it's coordinate system

	BPoint			LeftTop() const;
	float			Width() const;
	float			Height() const;

	void			ConvertToContainer(BPoint *pt) const;
	BPoint			ConvertToContainer(BPoint pt) const;
	void			ConvertFromContainer(BPoint *pt) const;
	BPoint			ConvertFromContainer(BPoint pt) const;

	virtual void		UpdateVisibleRegion();

protected:
	void			GetVisibleRegion(BRegion **region);

private:
	friend class BLayoutContainer;
	friend class BLayoutForm;

	BLayoutContainer *fContainer;
	int32 fIndex;

	BPoint fLocalOrigin;
	BRegion fVisibleRegion;

	BRect fFrame;
	uint32 fResizingMode;
	bool fHidden;
	bool fUpdating;
};


class BLayoutForm :
public BLayoutItem {
public:
	BLayoutForm(BRect frame, uint32 resizingMode, int32 rows, int32 columns);
	virtual ~BLayoutForm();

private:
	void *fData;
};

#endif /* __cplusplus */

#endif /* __ETK_LAYOUT_H__ */

