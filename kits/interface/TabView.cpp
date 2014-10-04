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
 * File: TabView.cpp
 *
 * --------------------------------------------------------------------------*/

#include <support/String.h>
#include "TabView.h"


BTab::BTab(BView *targetView)
		: fLabel(NULL), fEnabled(true), fFocus(false), fOwner(NULL)
{
	fView = targetView;
}


BTab::~BTab()
{
	if (fOwner != NULL) fOwner->BTabView::RemoveTab(fOwner->TabIndexOf(this));

	if (fLabel) delete[] fLabel;
	if (fView != NULL) {
		fView->RemoveSelf();
		delete fView;
	}
}


void
BTab::SetLabel(const char *label)
{
	if (fLabel) delete[] fLabel;
	fLabel = (label == NULL ? NULL : EStrdup(label));
}


const char*
BTab::Label() const
{
	return fLabel;
}


void
BTab::Select()
{
	if (fOwner == NULL) return;

	int32 index = fOwner->TabIndexOf(this);
	if (index == fOwner->fSelection) return;

	BTab *oldTab = fOwner->TabAt(fOwner->fSelection);
	if (oldTab) oldTab->Deselect();

	fOwner->fSelection = index;

	if (fView) fView->Show();
}


void
BTab::Deselect()
{
	if (IsSelected() == false) return;
	fOwner->fSelection = -1;
	if (fView) fView->Hide();
}


bool
BTab::IsSelected() const
{
	if (fOwner == NULL) return false;
	return(fOwner->fSelection == fOwner->TabIndexOf(this));
}


void
BTab::SetEnabled(bool state)
{
	fEnabled = state;
}


bool
BTab::IsEnabled() const
{
	return fEnabled;
}


void
BTab::MakeFocus(bool state)
{
	fFocus = state;
}


bool
BTab::IsFocus() const
{
	return fFocus;
}


bool
BTab::SetView(BView *targetView, BView **oldTargetView)
{
	if (targetView == NULL ? false : (targetView->Parent() != NULL || targetView->Window() != NULL)) return false;

	if (fView) fView->RemoveSelf();

	if (oldTargetView) *oldTargetView = fView;
	else if (fView) delete fView;

	fView = targetView;

	if (fView == NULL || fOwner == NULL || fOwner->fContainer == NULL) return true;

	if (IsSelected()) fView->Show();
	else fView->Hide();
	fOwner->fContainer->AddChild(fView);

	return true;
}


BView*
BTab::View() const
{
	return fView;
}


BTabView*
BTab::TabView() const
{
	return fOwner;
}


void
BTab::DrawFocusMark(BView* owner, BRect frame)
{
	if (!fFocus || !fEnabled) return;

	// TODO
}


void
BTab::DrawLabel(BView* owner, BRect frame)
{
	if (fLabel == NULL) return;

	rgb_color textColor = ui_color(B_PANEL_TEXT_COLOR);
	if (!fEnabled) textColor.disable(owner->ViewColor());

	BFont font;
	font_height fontHeight;
	owner->GetFont(&font);
	font.GetHeight(&fontHeight);

	owner->SetHighColor(textColor);
	BPoint penLocation = frame.LeftTop();
	penLocation.x += (frame.Width() - font.StringWidth(fLabel)) / 2.f;
	penLocation.y += fontHeight.ascent + 1.f;

	owner->DrawString(fLabel, penLocation);
}


void
BTab::DrawTab(BView* owner, BRect frame, e_tab_position position, bool full)
{
	rgb_color shineColor = ui_color(B_SHINE_COLOR);
	rgb_color shadowColor = ui_color(B_SHADOW_COLOR);

	owner->PushState();

	rgb_color bgColor = owner->ViewColor();
	if (position == B_TAB_FRONT) bgColor.set_to(235, 220, 30);

	if (!fEnabled) {
		shineColor.disable(bgColor);
		shadowColor.disable(bgColor);
	}

	owner->SetPenSize(0);
	owner->SetDrawingMode(B_OP_COPY);

	if (position == B_TAB_FRONT) {
		owner->SetHighColor(bgColor);
		owner->FillRect(frame);
	}

	owner->SetHighColor(shineColor);
	owner->StrokeLine(frame.LeftBottom(), frame.LeftTop());
	owner->StrokeLine(frame.RightTop());
	owner->SetHighColor(shadowColor);
	owner->StrokeLine(frame.RightBottom());

	owner->ConstrainClippingRegion(frame.InsetByCopy(1, 1));

	DrawLabel(owner, frame);
	DrawFocusMark(owner, frame);

	owner->PopState();
}


BTabView::BTabView(BRect frame, const char *name,
                   e_button_width tabWidth, uint32 resizeMode, uint32 flags)
		: BView(frame, name, resizeMode, flags), fSelection(-1)
{
	fTabWidth = tabWidth;

	font_height fontHeight;
	GetFontHeight(&fontHeight);
	fTabHeight = fontHeight.ascent + fontHeight.descent + 2.f;

	BRect rect = frame;
	rect.OffsetTo(BPoint(0, 0));
	rect.top += fTabHeight;
	rect.InsetBy(2.f, 2.f);
	fContainer = new BView(rect, NULL, B_FOLLOW_NONE, 0);
	AddChild(fContainer);
}


BTabView::~BTabView()
{
	BTab *tab;
	while ((tab = (BTab*)fTabs.RemoveItem((int32)0)) != NULL) {
		tab->fOwner = NULL;
		delete tab;
	}
}


void
BTabView::Select(int32 tabIndex)
{
	if ((tabIndex >= 0 ? tabIndex == fSelection : fSelection < 0) || tabIndex >= fTabs.CountItems()) return;

	if (tabIndex >= 0) {
		BTab *tab = (BTab*)fTabs.ItemAt(tabIndex);
		tab->Select();
	} else if (fSelection >= 0) {
		BTab *tab = (BTab*)fTabs.ItemAt(fSelection);
		tab->Deselect();
	}
}


int32
BTabView::Selection() const
{
	return fSelection;
}


bool
BTabView::AddTab(BView *tabTargetView, BTab *tab)
{
	if (tabTargetView == NULL && tab == NULL) return false;

	if (tabTargetView == NULL ? tab->fOwner != NULL :
	        (tabTargetView->Parent() != NULL || tabTargetView->Window() != NULL)) return false;

	BView *oldTargetView = NULL;
	BTab *newTab = NULL;

	if (tab == NULL) {
		newTab = (tab = new BTab(tabTargetView));
		tab->SetLabel(tabTargetView->Name());
	} else if (tabTargetView != NULL) {
		if (tab->SetView(tabTargetView, &oldTargetView) == false) return false;
	}

	if (fTabs.AddItem(tab) == false) {
		if (newTab != NULL) delete newTab;
		if (tabTargetView != NULL && tab != NULL) tab->SetView(oldTargetView, &oldTargetView);
		return false;
	}

	tab->fOwner = this;
	if (tab->fView != NULL && fContainer != NULL) {
		tab->fView->Hide();
		fContainer->AddChild(tab->fView);
		if (fSelection < 0) tab->Select();
	}

	return true;
}


BTab*
BTabView::RemoveTab(int32 tabIndex)
{
	BTab *tab = (BTab*)fTabs.RemoveItem(tabIndex);
	if (tab == NULL) return NULL;

	tab->fOwner = NULL;
	if (tab->fView != NULL) tab->fView->RemoveSelf();
	if (tabIndex == fSelection) fSelection = -1;

	return tab;
}


int32
BTabView::CountTabs() const
{
	return fTabs.CountItems();
}


BTab*
BTabView::TabAt(int32 tabIndex) const
{
	return (BTab*)fTabs.ItemAt(tabIndex);
}


int32
BTabView::TabIndexOf(const BTab *tab) const
{
	if (tab == NULL || tab->fOwner != this) return -1;
	return fTabs.IndexOf((void*)tab);
}


BView*
BTabView::ViewForTab(int32 tabIndex) const
{
	BTab *tab = (BTab*)fTabs.ItemAt(tabIndex);
	return(tab == NULL ? NULL : tab->View());
}


BView*
BTabView::ContainerView() const
{
	return fContainer;
}


void
BTabView::SetTabWidth(e_button_width tabWidth)
{
	if (fTabWidth != tabWidth) {
		fTabWidth = tabWidth;

		BRect r = Frame().OffsetToSelf(B_ORIGIN);
		r.bottom = r.top + fTabHeight;
		Invalidate(r);
	}
}


e_button_width
BTabView::TabWidth() const
{
	return fTabWidth;
}


void
BTabView::SetTabHeight(float tabHeight)
{
	if (tabHeight > 0 && tabHeight != fTabHeight) {
		fTabHeight = tabHeight;

		if (fContainer != NULL) {
			BRect frame = Frame().OffsetToSelf(B_ORIGIN);
			frame.top += fTabHeight;
			frame.InsetBy(2, 2);

			fContainer->ResizeTo(frame.Width(), frame.Height());
			fContainer->MoveTo(frame.LeftTop());
		}
		Invalidate();
	}
}


float
BTabView::TabHeight() const
{
	return fTabHeight;
}


void
BTabView::ChildRemoving(BView *child)
{
	if (child == fContainer) {
		if (fSelection >= 0) {
			BView *tabView = ViewForTab(fSelection);
			if (tabView != NULL) tabView->RemoveSelf();
			fSelection = -1;
		}
		fContainer = NULL;
	}
}


BRect
BTabView::TabFrame(int32 tabIndex) const
{
	if (tabIndex < 0 || tabIndex >= fTabs.CountItems()) return BRect();

	BFont font;
	GetFont(&font);

	BRect r = Frame().OffsetToSelf(B_ORIGIN);
	r.bottom = r.top + fTabHeight;
	r.right = r.left;

	for (int32 i = 0; i < fTabs.CountItems(); i++) {
		BTab *tab = (BTab*)fTabs.ItemAt(i);
		if (fTabWidth == B_WIDTH_FROM_LABEL) {
			if (i > 0) r.left = r.right + 5.f;
			r.right = r.left + max_c(font.StringWidth(tab->Label()) + 2.f, 10.f);
			if (i == tabIndex) break;
		} else { /* fTabWidth == B_WIDTH_AS_USUAL */
			r.right = r.left + max_c(r.Width(), max_c(font.StringWidth(tab->Label()) + 2.f, 10.f));
		}
	}

	if (fTabWidth == B_WIDTH_AS_USUAL) {
		float maxWidth = r.Width();
		r.left += (maxWidth + 5.f) * (float)tabIndex;
		r.right = r.left + maxWidth;
	}

	return r;
}


BRect
BTabView::DrawTabs()
{
	BRect selTabRect;

	for (int32 i = 0; i < fTabs.CountItems(); i++) {
		if (i == fSelection) continue;

		BTab *tab = (BTab*)fTabs.ItemAt(i);
		BRect tabRect = TabFrame(i);
		tab->DrawTab(this, tabRect, (i == 0 ?B_TAB_FIRST :B_TAB_ANY), true);
	}

	if (fSelection >= 0) {
		BTab *tab = (BTab*)fTabs.ItemAt(fSelection);
		selTabRect = TabFrame(fSelection);
		tab->DrawTab(this, selTabRect, B_TAB_FRONT, true);
	}

	return selTabRect;
}


void
BTabView::DrawBox(BRect selTabRect)
{
	BRect rect = Frame().OffsetToSelf(B_ORIGIN);
	rect.top += fTabHeight;

	rgb_color shineColor = ui_color(B_SHINE_COLOR);
	rgb_color shadowColor = ui_color(B_SHADOW_COLOR);

	if (!IsEnabled()) {
		shineColor.disable(ViewColor());
		shadowColor.disable(ViewColor());
	}

	PushState();

	BRegion clipping(rect);
	clipping.Exclude(selTabRect.InsetByCopy(0, -1));
	ConstrainClippingRegion(&clipping);

	SetPenSize(0);
	SetDrawingMode(B_OP_COPY);
	SetHighColor(shineColor);
	StrokeRect(rect);
	SetHighColor(shadowColor);
	StrokeLine(rect.LeftBottom(), rect.RightBottom());
	StrokeLine(rect.RightTop());

	PopState();
}


void
BTabView::Draw(BRect updateRect)
{
	BRect selTabRect = DrawTabs();
	DrawBox(selTabRect);
}


void
BTabView::MouseDown(BPoint where)
{
	int32 btnClicks = 1;
	if (where.y > fTabHeight + 1.f || !IsEnabled() ||
	        !QueryCurrentMouse(true, B_PRIMARY_MOUSE_BUTTON, true, &btnClicks) || btnClicks > 1) return;

	// TODO
	int32 index = -1;
	for (int32 i = 0; i < fTabs.CountItems(); i++) {
		if (TabFrame(i).Contains(where)) index = i;
	}

	if (index < 0 || fSelection == index) return;

	BTab *tab = (BTab*)fTabs.ItemAt(index);
	if (tab->IsEnabled() == false) return;
	tab->Select();

	BRect r = Frame().OffsetToSelf(B_ORIGIN);
	r.bottom = r.top + fTabHeight;
	Invalidate(r);
}

