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
 * File: MenuField.cpp
 * Description: BMenuField --- display a labeled pop-up menu
 *
 * --------------------------------------------------------------------------*/

#include <support/String.h>
#include <support/ClassInfo.h>

#include "Window.h"
#include "MenuField.h"


BMenuField::BMenuField(BRect frame, const char *name,
                       const char *label, BMenu *menu, bool fixedSize,
                       uint32 resizeMode, uint32 flags)
		: BView(frame, name, resizeMode, flags),
		fAlignment(B_ALIGN_LEFT), fDivider(-1),
		fLabel(NULL), fMenu(NULL)
{
	fFixedSize = fixedSize;
	if (label != NULL) fLabel = EStrdup(label);
	AddChild(fMenuBar = new BMenuBar(frame, NULL, B_FOLLOW_NONE, B_ITEMS_IN_ROW, !fFixedSize));
	fMenuBar->SetEventMask(0);
	fMenuBar->SetBorder(B_BORDER_FRAME);
	SetMenu(menu);
}


BMenuField::~BMenuField()
{
	if (fLabel != NULL) delete[] fLabel;
}


void
BMenuField::ChildRemoving(BView *child)
{
	if (child == fMenuBar) {
		fMenuBar = NULL;
		fMenu = NULL;
	}
	BView::ChildRemoving(child);
}


void
BMenuField::SetLabel(const char *label)
{
	if (fLabel != NULL) delete[] fLabel;
	fLabel = (label == NULL ? NULL : EStrdup(label));
	Invalidate();
}


const char*
BMenuField::Label() const
{
	return fLabel;
}


void
BMenuField::SetAlignment(e_alignment alignment)
{
	if (alignment != fAlignment) {
		fAlignment = alignment;
		if (fLabel != NULL) Invalidate();
	}
}


e_alignment
BMenuField::Alignment() const
{
	return fAlignment;
}


void
BMenuField::SetDivider(float divider)
{
	if (fDivider < 0 && divider < 0) return;

	if (fDivider != divider) {
		fDivider = divider;
		BMenuField::FrameResized(Frame().Width(), Frame().Height());
		Invalidate();
	}
}


float
BMenuField::Divider() const
{
	return fDivider;
}


bool
BMenuField::SetMenu(BMenu *menu)
{
	if (fMenuBar == NULL || (menu == NULL ? false : (fMenuBar->AddItem(menu) == false))) return false;

	if (fMenu) {
		fMenuBar->RemoveItem(fMenu);
		delete fMenu;
	}

	fMenu = menu;

	BMenuField::FrameResized(Frame().Width(), Frame().Height());

	Invalidate();

	return true;
}


BMenu*
BMenuField::Menu() const
{
	return fMenu;
}


BMenuBar*
BMenuField::MenuBar() const
{
	return fMenuBar;
}


BMenuItem*
BMenuField::MenuItem() const
{
	return(fMenu == NULL ? NULL : fMenu->Superitem());
}


void
BMenuField::Draw(BRect updateRect)
{
	if (Window() == NULL) return;

	if (fLabel != NULL && fDivider != 0) {
		BFont font;
		GetFont(&font);
		font_height fontHeight;
		font.GetHeight(&fontHeight);
		float sHeight = fontHeight.ascent + fontHeight.descent;

		BRect rect = Frame().OffsetToSelf(B_ORIGIN);
		rect.right = (fDivider >= 0 ? fDivider : max_c(font.StringWidth(fLabel), 0));

		BPoint penLocation;

		if (fAlignment == B_ALIGN_LEFT || fDivider < 0)
			penLocation.x = 0;
		else if (fAlignment == B_ALIGN_RIGHT)
			penLocation.x = rect.right - max_c(font.StringWidth(fLabel), 0);
		else
			penLocation.x = (rect.right - max_c(font.StringWidth(fLabel), 0)) / 2;

		penLocation.y = rect.Center().y - sHeight / 2.f;
		penLocation.y += fontHeight.ascent + 1;

		PushState();
		ConstrainClippingRegion(rect);
		SetHighColor(IsEnabled() ? ui_color(B_PANEL_TEXT_COLOR) : ui_color(B_SHINE_COLOR).disable(ViewColor()));
		SetLowColor(ViewColor());
		DrawString(Label(), penLocation);
		if (!IsEnabled()) {
			SetHighColor(ui_color(B_SHADOW_COLOR).disable(ViewColor()));
			DrawString(Label(), penLocation - BPoint(1, 1));
		}
		PopState();
	}

#if 0
	if (IsFocus() && Window()->IsActivate()) {
		PushState();
		SetHighColor(ui_color(B_NAVIGATION_BASE_COLOR));
		StrokeRect(Frame().OffsetToSelf(B_ORIGIN));
		PopState();
	}
#endif
}


void
BMenuField::GetPreferredSize(float *width, float *height)
{
	if (width) *width = 0;
	if (height) *height = 0;

	if (fMenuBar != NULL) fMenuBar->GetPreferredSize(width, height);

	if (width != NULL) {
		BFont font;
		GetFont(&font);
		*width += (fDivider >= 0 ? fDivider : max_c(font.StringWidth(fLabel), 0)) + UnitsPerPixel() * 2;
	}

	if (height != NULL) {
		font_height fontHeight;
		GetFontHeight(&fontHeight);
		float sHeight = fontHeight.ascent + fontHeight.descent;
		if (sHeight + 10 > *height) *height = sHeight + 10;
	}
}


void
BMenuField::FrameResized(float new_width, float new_height)
{
	if (fMenuBar == NULL) return;

	BFont font;
	GetFont(&font);

	fMenuBar->ResizeToPreferred();
	fMenuBar->MoveTo((fDivider >= 0 ? fDivider : max_c(font.StringWidth(fLabel), 0)) + UnitsPerPixel(),
	                 (new_height - fMenuBar->Frame().Height()) / 2);
	if (fFixedSize)
		fMenuBar->ResizeTo(new_width - fMenuBar->Frame().left, fMenuBar->Frame().Height());
}


void
BMenuField::FrameMoved(BPoint new_position)
{
	BMenuField::FrameResized(Frame().Width(), Frame().Height());
}


void
BMenuField::WindowActivated(bool state)
{
	if (!state && fMenuBar) fMenuBar->SelectItem(NULL);

#if 0
	if (!(IsFocus() && (Flags() &B_WILL_DRAW))) return;
	PushState();
	SetHighColor(state ? ui_color(B_NAVIGATION_BASE_COLOR) : ViewColor());
	StrokeRect(Frame().OffsetToSelf(B_ORIGIN));
	PopState();
#endif
}


void
BMenuField::MakeFocus(bool focusState)
{
	if (IsFocus() != focusState) {
		BView::MakeFocus(focusState);

#if 0
		if (IsVisible() && (Flags() &B_WILL_DRAW)) {
			PushState();
			SetHighColor(IsFocus() ? ui_color(B_NAVIGATION_BASE_COLOR) : ViewColor());
			StrokeRect(Frame().OffsetToSelf(B_ORIGIN));
			PopState();
		}
#endif

		if (!IsFocus() && fMenuBar) fMenuBar->SelectItem(NULL);
	}
}


void
BMenuField::SetFont(const BFont *font, uint8 mask)
{
	BView::SetFont(font, mask);
	BMenuField::FrameResized(Frame().Width(), Frame().Height());
	Invalidate();
}


void
BMenuField::MouseDown(BPoint where)
{
	if ((Flags() &B_NAVIGABLE) && !IsFocus()) MakeFocus();

	BMenuItem *item = NULL;
	if (fMenuBar == NULL || (item = fMenuBar->ItemAt(0)) == NULL) return;

	BRect itemFrame = item->Frame();
	if (itemFrame.Contains(fMenuBar->ConvertFromParent(where)) == false &&
	        fMenuBar->CurrentSelection() == NULL) where = fMenuBar->ConvertToParent(itemFrame.Center());

	fMenuBar->MouseDown(fMenuBar->ConvertFromParent(where));
}


void
BMenuField::MouseUp(BPoint where)
{
	BMenuItem *item = NULL;
	if (fMenuBar == NULL || (item = fMenuBar->ItemAt(0)) == NULL) return;

	BRect itemFrame = item->Frame();
	if (itemFrame.Contains(fMenuBar->ConvertFromParent(where)) == false) where = fMenuBar->ConvertToParent(itemFrame.Center());

	fMenuBar->MouseUp(fMenuBar->ConvertFromParent(where));
}


void
BMenuField::MouseMoved(BPoint where, uint32 code, const BMessage *a_message)
{
	BMenuItem *item = NULL;
	if (fMenuBar == NULL || (item = fMenuBar->ItemAt(0)) == NULL) return;

	BRect itemFrame = item->Frame();
	if (itemFrame.Contains(fMenuBar->ConvertFromParent(where)) == false &&
	        fMenuBar->CurrentSelection() == NULL) where = fMenuBar->ConvertToParent(itemFrame.Center());

	fMenuBar->MouseMoved(fMenuBar->ConvertFromParent(where), code, a_message);
}


void
BMenuField::KeyDown(const char *bytes, int32 numBytes)
{
	BMenuItem *item = NULL;
	if (bytes == NULL || fMenuBar == NULL || (item = fMenuBar->ItemAt(0)) == NULL) return;

	if (numBytes == 1 && bytes[0] == B_DOWN_ARROW && fMenuBar->CurrentSelection() == NULL) fMenuBar->SelectItem(item);
	fMenuBar->KeyDown(bytes, numBytes);
}


void
BMenuField::KeyUp(const char *bytes, int32 numBytes)
{
	BMenuItem *item = NULL;
	if (bytes == NULL || fMenuBar == NULL || (item = fMenuBar->ItemAt(0)) == NULL) return;

	fMenuBar->KeyUp(bytes, numBytes);
}

