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
 * File: View.cpp
 * Description: BView --- drawing/layout/control within BWindow
 *
 * --------------------------------------------------------------------------*/

#include <math.h>

#include <support/ClassInfo.h>
#include <add-ons/font/FontEngine.h>
#include <support/List.h>
#include <add-ons/graphics/GraphicsEngine.h>
#include <app/Application.h>

#include "GraphicsDefs.h"
#include "Window.h"
#include "ScrollView.h"
#include "Box.h"
#include "Bitmap.h"
#include "ViewPrivate.h"

class _LOCAL BViewState
{
	public:
		e_drawing_mode		DrawingMode;
		BPoint			PenLocation;
		float			PenSize;
		rgb_color		HighColor;
		rgb_color		LowColor;
		BFont			Font;
		BRegion			*Clipping;
		bool			SquarePointStyle;
		BViewState		*PreviousState;

		inline BViewState(BViewState *prev) {
			Clipping = NULL;
			PreviousState = prev;

			if (prev != NULL) {
				DrawingMode = prev->DrawingMode;
				PenLocation = prev->PenLocation;
				PenSize = prev->PenSize;
				HighColor = prev->HighColor;
				LowColor = prev->LowColor;
				Font = prev->Font;
				if (prev->Clipping != NULL) Clipping = new BRegion(*(prev->Clipping));
				SquarePointStyle = prev->SquarePointStyle;
			}
		};

		inline ~BViewState() {
			if (Clipping != NULL) delete Clipping;
		}
};


void
BView::InitSelf(BRect frame, uint32 resizingMode, uint32 flags)
{
	if (app == NULL || app->fGraphicsEngine == NULL)
		ETK_ERROR("[INTERFACE]: %s --- View must created within a application which has graphics-engine!", __PRETTY_FUNCTION__);

	if ((fDC = app->fGraphicsEngine->CreateContext()) == NULL)
		ETK_ERROR("[INTERFACE]: %s --- Unable to create draw context!", __PRETTY_FUNCTION__);

	BViewState *viewState = new BViewState(NULL);

	viewState->DrawingMode = B_OP_COPY;
	viewState->PenLocation.Set(0, 0);
	viewState->PenSize = 0;
	viewState->HighColor.set_to(0, 0, 0);
	viewState->LowColor.set_to(255, 255, 255);
	viewState->Font = *be_plain_font;
	viewState->Clipping = NULL;
	viewState->SquarePointStyle = false;

	fStates = (void*)viewState;

	fViewFlags = flags;
	fViewColor = ui_color(B_PANEL_BACKGROUND_COLOR);
	fForceFontAliasing = false;
	fMouseInside = false;
	fScrollTimeStamp = 0;

	fMouseGrabbed = false;
	fKeyboardGrabbed = false;
	fEventStored = false;
	fEventMaskStored = 0;
	fEventOptionsStored = 0;
	fEventMask = 0;
	fEventOptions = 0;

	fLayout = new BViewLayout(this, frame, resizingMode);
}


BView::BView(BRect frame, const char *name, uint32 resizingMode, uint32 flags)
		: BHandler(name)
{
	InitSelf(frame, resizingMode, flags);
}


BView::~BView()
{
	if (Parent() != NULL || Window() != NULL) {
		ETK_WARNING("[INTERFACE]: %s --- It's recommended that remove self from parent before delete.", __PRETTY_FUNCTION__);
		RemoveSelf();
	}

	for (BView *child = ChildAt(0); child != NULL; child = ChildAt(0)) {
		RemoveChild(child);
		delete child;
	}

	delete fDC;
	delete fLayout;

	BViewState *states = ((BViewState*)fStates);
	while (states != NULL) {
		BViewState *viewState = states;
		states = viewState->PreviousState;
		delete viewState;
	}
}


void
BView::MessageReceived(BMessage *msg)
{
	switch (msg->what) {
		case B_PULSE:
			Pulse();
			break;

		case B_VIEW_MOVED: {
			BPoint where;
			if (msg->FindPoint("where", &where) == false) break;
			FrameMoved(where);
		}
		break;

		case B_VIEW_RESIZED: {
			float width, height;
			if (msg->FindFloat("width", &width) == false) break;
			if (msg->FindFloat("height", &height) == false) break;
			FrameResized(width, height);
		}
		break;

		case B_MOUSE_DOWN:
		case B_MOUSE_UP:
		case B_MOUSE_MOVED: {
			BWindow *win = Window();
			if (win == NULL) break;

			BPoint where;
			if (msg->FindPoint("where", &where) == false) break;

			for (BView *child = ChildAt(0); child != NULL; child = child->NextSibling()) {
				BPoint pt = child->fLayout->ConvertFromContainer(where);

				if (child->fLayout->VisibleRegion()->Contains(pt) == false) continue;
				if (!(child->EventMask() &B_POINTER_EVENTS)) {
					BMessage aMsg(*msg);
					aMsg.ReplacePoint("where", pt);
					win->PostMessage(&aMsg, child);
				}

				return; // just one child can receive the message
			}

			if (msg->what == B_MOUSE_DOWN) {
				MouseDown(where);
			} else if (msg->what == B_MOUSE_UP) {
				if (fEventStored) {
					if (fMouseGrabbed) {
						win->_UngrabMouse();
						fMouseGrabbed = false;
					}

					uint32 eventMask = fEventMask & ~B_POINTER_EVENTS;

					if (eventMask == 0) {
						_SetEventMask(fEventMaskStored, fEventOptionsStored);
						fEventStored = false;
					} else {
						_SetEventMask(eventMask, fEventOptions);
						if (!fKeyboardGrabbed) fEventOptions &= ~B_LOCK_WINDOW_FOCUS;
					}
				}

				MouseUp(where);
			} else { //B_MOUSE_MOVED
				bool insided = fLayout->VisibleRegion()->Contains(where);
				uint32 transit = (insided ?B_INSIDB_VIEW :B_OUTSIDE_VIEW);
				if (fMouseInside != insided) {
					if (!insided) {
						fMouseInside = false;
						transit = B_EXITED_VIEW;
						win->fMouseInsideViews.RemoveItem(this);
					} else {
						fMouseInside = true;
						transit = B_ENTERED_VIEW;
						win->fMouseInsideViews.AddItem(this);
					}
				}
				// TODO:B_NO_POINTER_HISTORY, drag info
				MouseMoved(where, transit, NULL);
			}
		}
		break;

		case B_UNMAPPED_KEY_DOWN:
		case B_UNMAPPED_KEY_UP:
		case B_KEY_DOWN:
		case B_KEY_UP: {
			BWindow *win = Window();
			if (win == NULL) break;

			if (msg->what == B_UNMAPPED_KEY_UP || msg->what == B_KEY_UP) {
				if (fEventStored) {
					if (fKeyboardGrabbed) {
						win->_UngrabKeyboard();
						fKeyboardGrabbed = false;
					}

					uint32 eventMask = fEventMask & ~B_KEYBOARD_EVENTS;

					if (eventMask == 0) {
						_SetEventMask(fEventMaskStored, fEventOptionsStored);
						fEventStored = false;
					} else {
						_SetEventMask(eventMask, fEventOptions);
						if (!fMouseGrabbed) fEventOptions &= ~B_LOCK_WINDOW_FOCUS;
					}
				}
			}

			char _bytes[4];
			bzero(_bytes, 4);
			const char *bytes = NULL;

			if (msg->FindString("bytes", &bytes) != true || bytes == NULL) {
				bytes = _bytes;
				if ((msg->what == B_KEY_DOWN || msg->what == B_KEY_UP)) {
					for (int32 i = 0; i < 3; i++) {
						int8 val;
						if (!msg->FindInt8("byte", i, &val)) break;
						_bytes[i] = val;
					}
				}
			}

			if (msg->what == B_KEY_DOWN || msg->what == B_UNMAPPED_KEY_DOWN)
				KeyDown(bytes, strlen(bytes));
			else
				KeyUp(bytes, strlen(bytes));
		}
		break;

		case B_WINDOW_ACTIVATED:
			if (Window() == NULL) break;
			WindowActivated(Window()->IsActivate());
			for (BView *child = ChildAt(0); child != NULL; child = child->NextSibling()) {
				Looper()->PostMessage(msg, child);
			}
			break;

		default:
			BHandler::MessageReceived(msg);
	}
}


void
BView::Show()
{
	if (fLayout->IsHidden(false) == false) return;

	fLayout->Show();

	if (!is_kind_of(this, BScrollBar) ||
	        !is_kind_of(Parent(), BScrollView) ||
	        cast_as(Parent(), BScrollView)->fTarget == NULL) return;

	if (cast_as(Parent(), BScrollView)->fHSB == this ||
	        cast_as(Parent(), BScrollView)->fVSB == this)
		cast_as(Parent(), BScrollView)->fTarget->fLayout->UpdateVisibleRegion();
}


void
BView::Hide()
{
	if (fLayout->IsHidden(false)) return;

	BView::MakeFocus(false);

	fLayout->Hide();

	if (!is_kind_of(this, BScrollBar) ||
	        !is_kind_of(Parent(), BScrollView) ||
	        cast_as(Parent(), BScrollView)->fTarget == NULL) return;

	if (cast_as(Parent(), BScrollView)->fHSB == this ||
	        cast_as(Parent(), BScrollView)->fVSB == this)
		cast_as(Parent(), BScrollView)->fTarget->fLayout->UpdateVisibleRegion();
}


bool
BView::IsHidden() const
{
	if (Window() == NULL) return true;
	return fLayout->IsHidden();
}


void
BView::AttachedToWindow()
{
}


void
BView::AllAttached()
{
}


void
BView::DetachedFromWindow()
{
}


void
BView::AllDetached()
{
}


void
BView::ChildRemoving(BView *child)
{
}


void
BView::AddChild(BView *child, BView *nextSibling)
{
	if (child == NULL || child->Looper() != NULL || child->Parent() != NULL ||
	        (nextSibling == NULL ? false : nextSibling->Parent() != this)) {
		ETK_WARNING("[INTERFACE]: %s --- Unable to add child.", __PRETTY_FUNCTION__);
		return;
	}

	BWindow *win = Window();
	if (win != NULL) win->AddHandler(child);

	if (fLayout->AddItem(child->fLayout, nextSibling == NULL ? -1 : fLayout->IndexOf(nextSibling->fLayout)) == false) {
		if (win != NULL) win->RemoveHandler(child);
		ETK_WARNING("[INTERFACE]: %s --- Unable to add child.", __PRETTY_FUNCTION__);
		return;
	}

	if (win != NULL) {
		if (child->Window() == win) {
			child->AttachToWindow();
			child->AttachedToWindow();

			win->AddViewChildrenToHandlersList(win, child);
			child->AllAttached();
		} else {
			ETK_OUTPUT("Warning: [INTERFACE]: %s --- Unable to attch child to window, but child added.\n", __PRETTY_FUNCTION__);
		}
	}
}


bool
BView::IsSibling(const BView *sibling) const
{
	if (sibling == NULL || sibling == this) return false;
	if (fLayout->Container() != sibling->fLayout->Container()) return false;
	return(fLayout->Container() != NULL);
}


bool
BView::RemoveChild(BView *child)
{
	if (child == NULL || child->Parent() != this) return false;

	if (child->fScrollBar.IsEmpty() == false) {
		for (int32 i = 0; i < child->fScrollBar.CountItems(); i++) {
			BScrollBar *scrollbar = (BScrollBar*)child->fScrollBar.ItemAt(i);
			scrollbar->fTarget = NULL;
		}
		child->fScrollBar.MakeEmpty();
	}

	if (is_kind_of(child, BScrollBar)) {
		BScrollBar *scrollbar = cast_as(child, BScrollBar);
		if (scrollbar->fTarget != NULL) {
			scrollbar->fTarget->fScrollBar.RemoveItem(scrollbar);
			scrollbar->fTarget = NULL;
		}
	}

	ChildRemoving(child);

	BWindow *win = Window();
	if (win != NULL) {
		win->RemoveViewChildrenFromHandlersList(win, child);
		child->AllDetached();

		child->DetachedFromWindow();

		child->DetachFromWindow();
		win->RemoveHandler(child);
	}

	fLayout->RemoveItem(child->fLayout);

	return true;
}


bool
BView::RemoveSelf()
{
	if (Parent() != NULL) return Parent()->RemoveChild(this);
	if (Window() != NULL) return Window()->RemoveChild(this);
	return false;
}


int32
BView::CountChildren() const
{
	return fLayout->CountItems();
}


BView*
BView::ChildAt(int32 index) const
{
	return(fLayout->ItemAt(index) != NULL ? (BView*)fLayout->ItemAt(index)->PrivateData() : NULL);
}


BView*
BView::NextSibling() const
{
	return(fLayout->NextSibling() != NULL ? (BView*)fLayout->NextSibling()->PrivateData() : NULL);
}


BView*
BView::PreviousSibling() const
{
	return(fLayout->PreviousSibling() != NULL ? (BView*)fLayout->PreviousSibling()->PrivateData() : NULL);
}


BWindow*
BView::Window() const
{
	return cast_as(Looper(), BWindow);
}


BView*
BView::Parent() const
{
	return(is_kind_of(fLayout->Container(), BViewLayout) ? (BView*)fLayout->Container()->PrivateData() : NULL);
}


BView*
BView::Ancestor() const
{
	if (Parent() == NULL) return (BView*)this;

	BView *ancestor = Parent();
	while (ancestor->Parent() != NULL) ancestor = ancestor->Parent();

	return ancestor;
}


BRect
BView::Bounds() const
{
	return fLayout->Bounds();
}


BPoint
BView::LeftTop() const
{
	return fLayout->LeftTop();
}


BRect
BView::Frame() const
{
	return fLayout->Frame();
}


BRect
BView::VisibleBounds() const
{
	return fLayout->VisibleRegion()->Frame();
}


BRect
BView::VisibleFrame() const
{
	return ConvertToParent(fLayout->VisibleRegion()->Frame());
}


BRegion
BView::VisibleBoundsRegion() const
{
	BRegion region(*(fLayout->VisibleRegion()));
	return region;
}


BRegion
BView::VisibleFrameRegion() const
{
	BRegion region(*(fLayout->VisibleRegion()));
	ConvertToParent(&region);
	return region;
}


bool
BView::IsVisible() const
{
	return(fLayout->VisibleRegion()->CountRects() > 0);
}


void
BView::MouseDown(BPoint where)
{
}


void
BView::MouseUp(BPoint where)
{
}


void
BView::MouseMoved(BPoint where, uint32 code, const BMessage *a_message)
{
}


void
BView::WindowActivated(bool state)
{
}


BView*
BView::FindView(const char *name) const
{
	BString srcStr(name);

	for (BView *child = ChildAt(0); child != NULL; child = child->NextSibling()) {
		BString destStr(child->Name());

		if (srcStr == destStr) return child;

		BView *view = child->FindView(name);
		if (view != NULL) return view;
	}

	return NULL;
}


void
BView::KeyDown(const char *bytes, int32 numBytes)
{
}


void
BView::KeyUp(const char *bytes, int32 numBytes)
{
}


void
BView::Pulse()
{
}


void
BView::FrameMoved(BPoint new_position)
{
}


void
BView::FrameResized(float new_width, float new_height)
{
}


void
BView::TargetedByScrollView(BScrollView *scroll_view)
{
}


void
BView::Draw(BRect updateRect)
{
}


void
BView::DrawAfterChildren(BRect updateRect)
{
}


void
BView::ConvertToScreen(BPoint* pt) const
{
	if (!pt) return;

	BWindow *win = Window();
	if (win == NULL) return;

	ConvertToWindow(pt);
	win->ConvertToScreen(pt);
}


BPoint
BView::ConvertToScreen(BPoint pt) const
{
	BPoint pt1 = pt;
	ConvertToScreen(&pt1);
	return pt1;
}


void
BView::ConvertFromScreen(BPoint* pt) const
{
	if (pt == NULL || Window() == NULL) return;
	*pt -= ConvertToScreen(B_ORIGIN);
}


BPoint
BView::ConvertFromScreen(BPoint pt) const
{
	BPoint pt1 = pt;
	ConvertFromScreen(&pt1);
	return pt1;
}


void
BView::ConvertToScreen(BRect *r) const
{
	if (r == NULL) return;
	r->OffsetTo(ConvertToScreen(r->LeftTop()));
}


BRect
BView::ConvertToScreen(BRect r) const
{
	BRect rect = r;
	ConvertToScreen(&rect);
	return rect;
}


void
BView::ConvertFromScreen(BRect *r) const
{
	if (r == NULL) return;
	r->OffsetBy(ConvertFromScreen(B_ORIGIN));
}


BRect
BView::ConvertFromScreen(BRect r) const
{
	BRect rect = r;
	ConvertFromScreen(&rect);
	return rect;
}


void
BView::ConvertToScreen(BRegion *region) const
{
	if (region == NULL || region->CountRects() <= 0) return;
	BPoint pt = ConvertToScreen(region->Frame().LeftTop());
	region->OffsetBy(pt - region->Frame().LeftTop());
}


BRegion
BView::ConvertToScreen(const BRegion &region) const
{
	BRegion aRegion(region);
	ConvertToScreen(&aRegion);
	return aRegion;
}


void
BView::ConvertFromScreen(BRegion *region) const
{
	if (region == NULL || region->CountRects() <= 0) return;
	region->OffsetBy(ConvertFromScreen(B_ORIGIN));
}


BRegion
BView::ConvertFromScreen(const BRegion &region) const
{
	BRegion aRegion(region);
	ConvertFromScreen(&aRegion);
	return aRegion;
}


void
BView::ConvertToParent(BPoint *pt) const
{
	fLayout->ConvertToContainer(pt);
}


BPoint
BView::ConvertToParent(BPoint pt) const
{
	BPoint pt1 = pt;
	ConvertToParent(&pt1);
	return pt1;
}


void
BView::ConvertFromParent(BPoint *pt) const
{
	fLayout->ConvertFromContainer(pt);
}


BPoint
BView::ConvertFromParent(BPoint pt) const
{
	BPoint pt1 = pt;
	ConvertFromParent(&pt1);
	return pt1;
}


void
BView::ConvertToParent(BRect *r) const
{
	if (r == NULL) return;
	r->OffsetTo(ConvertToParent(r->LeftTop()));
}


BRect
BView::ConvertToParent(BRect r) const
{
	BRect rect = r;
	ConvertToParent(&rect);
	return rect;
}


void
BView::ConvertFromParent(BRect *r) const
{
	if (r == NULL) return;
	r->OffsetBy(ConvertFromParent(B_ORIGIN));
}


BRect
BView::ConvertFromParent(BRect r) const
{
	BRect rect = r;
	ConvertFromParent(&rect);
	return rect;
}


void
BView::ConvertToParent(BRegion *region) const
{
	if (region == NULL || region->CountRects() <= 0) return;
	BPoint pt = ConvertToParent(region->Frame().LeftTop());
	region->OffsetBy(pt - region->Frame().LeftTop());
}


BRegion
BView::ConvertToParent(const BRegion &region) const
{
	BRegion aRegion(region);
	ConvertToParent(&aRegion);
	return aRegion;
}


void
BView::ConvertFromParent(BRegion *region) const
{
	if (region == NULL || region->CountRects() <= 0) return;
	region->OffsetBy(ConvertFromParent(B_ORIGIN));
}


BRegion
BView::ConvertFromParent(const BRegion &region) const
{
	BRegion aRegion(region);
	ConvertFromParent(&aRegion);
	return aRegion;
}


void
BView::ConvertToWindow(BPoint* pt) const
{
	if (pt == NULL) return;

	BWindow *win = Window();
	if (win == NULL) return;

	for (BLayoutItem *container = fLayout;
	        container->Container() != win->fLayout;
	        container = cast_as(container->Container(), BLayoutItem)) {
		container->ConvertToContainer(pt);
	}
}


BPoint
BView::ConvertToWindow(BPoint pt) const
{
	BPoint pt1 = pt;
	ConvertToWindow(&pt1);
	return pt1;
}


void
BView::ConvertFromWindow(BPoint* pt) const
{
	if (pt == NULL || Window() == NULL) return;
	*pt -= ConvertToWindow(B_ORIGIN);
}


BPoint
BView::ConvertFromWindow(BPoint pt) const
{
	BPoint pt1 = pt;
	ConvertFromWindow(&pt1);
	return pt1;
}


void
BView::ConvertToWindow(BRect *r) const
{
	if (r == NULL) return;
	r->OffsetTo(ConvertToWindow(r->LeftTop()));
}


BRect
BView::ConvertToWindow(BRect r) const
{
	BRect rect = r;
	ConvertToWindow(&rect);
	return rect;
}


void
BView::ConvertFromWindow(BRect *r) const
{
	if (r == NULL) return;
	r->OffsetBy(ConvertFromWindow(B_ORIGIN));
}


BRect
BView::ConvertFromWindow(BRect r) const
{
	BRect rect = r;
	ConvertFromWindow(&rect);
	return rect;
}


void
BView::ConvertToWindow(BRegion *region) const
{
	if (region == NULL || region->CountRects() <= 0) return;
	BPoint pt = ConvertToWindow(region->Frame().LeftTop());
	region->OffsetBy(pt - region->Frame().LeftTop());
}


BRegion
BView::ConvertToWindow(const BRegion &region) const
{
	BRegion aRegion(region);
	ConvertToWindow(&aRegion);
	return aRegion;
}


void
BView::ConvertFromWindow(BRegion *region) const
{
	if (region == NULL || region->CountRects() <= 0) return;
	region->OffsetBy(ConvertFromWindow(B_ORIGIN));
}


BRegion
BView::ConvertFromWindow(const BRegion &region) const
{
	BRegion aRegion(region);
	ConvertFromWindow(&aRegion);
	return aRegion;
}


void
BView::SetFlags(uint32 flags)
{
	BWindow *win = Window();
	if (win != NULL) {
		uint32 oldPulseNeeded = fViewFlags &B_PULSE_NEEDED;
		uint32 newPulseNeeded = flags &B_PULSE_NEEDED;

		if (oldPulseNeeded != newPulseNeeded) {
			if (newPulseNeeded == B_PULSE_NEEDED)
				win->fNeededToPulseViews.AddItem(this);
			else
				win->fNeededToPulseViews.RemoveItem(this);

			if (win->fPulseRunner)
				win->fPulseRunner->SetCount((win->fPulseRate >B_INT64_CONSTANT(0) &&
				                             win->fNeededToPulseViews.CountItems() > 0 &&
				                             win->IsHidden() == false) ? -1 : 0);
		}
	}

	fViewFlags = flags;
}


uint32
BView::Flags() const
{
	return fViewFlags;
}


void
BView::SetResizingMode(uint32 mode)
{
	fLayout->SetResizingMode(mode);
}


uint32
BView::ResizingMode() const
{
	return fLayout->ResizingMode();
}


void
BView::_FrameChanged(BRect oldFrame, BRect newFrame)
{
	if (oldFrame == newFrame) return;

	BWindow *win = Window();

	if (fViewFlags &B_FRAME_EVENTS) {
		BMessage aMsg(B_VIEW_MOVED);
		aMsg.AddInt64("when", e_real_time_clock_usecs());
		aMsg.AddFloat("width", newFrame.Width());
		aMsg.AddFloat("height", newFrame.Height());
		aMsg.AddPoint("where", newFrame.LeftTop());

		if (oldFrame.LeftTop() != newFrame.LeftTop()) {
			if (win)
				win->PostMessage(&aMsg, this);
			else
				MessageReceived(&aMsg);
		}

		if (oldFrame.Width() != newFrame.Width() || oldFrame.Height() != newFrame.Height()) {
			aMsg.what = B_VIEW_RESIZED;
			if (win)
				win->PostMessage(&aMsg, this);
			else
				MessageReceived(&aMsg);
		}
	}
}


void
BView::MoveBy(float dh, float dv)
{
	MoveTo(fLayout->Frame().LeftTop() + BPoint(dh, dv));
}


void
BView::MoveTo(BPoint where)
{
	fLayout->MoveTo(where);
}


void
BView::MoveTo(float x, float y)
{
	MoveTo(BPoint(x, y));
}


void
BView::ResizeBy(float dh, float dv)
{
	ResizeTo(fLayout->Width() + dh, fLayout->Height() + dv);
}


void
BView::ResizeTo(float width, float height)
{
	fLayout->ResizeTo(width, height);
}


void
BView::AttachToWindow()
{
	BWindow *win = Window();

	if (fEventMask &B_POINTER_EVENTS)
		win->fMouseInterestedViews.AddItem(this);

	if (fEventMask &B_KEYBOARD_EVENTS)
		win->fKeyboardInterestedViews.AddItem(this);

	Invalidate();

	if (Flags() &B_PULSE_NEEDED) {
		win->fNeededToPulseViews.AddItem(this);
		if (win->fPulseRunner)
			win->fPulseRunner->SetCount((win->fPulseRate >B_INT64_CONSTANT(0) &&
			                             win->fNeededToPulseViews.CountItems() > 0 &&
			                             win->IsHidden() == false) ? -1 : 0);
	}
}


void
BView::DetachFromWindow()
{
	BWindow *win = Window();

	if (fEventStored) {
		if (fMouseGrabbed) {
			win->_UngrabMouse();
			fMouseGrabbed = false;
		}

		if (fKeyboardGrabbed) {
			win->_UngrabKeyboard();
			fKeyboardGrabbed = false;
		}

		_SetEventMask(fEventMaskStored, fEventOptionsStored);

		fEventStored = false;
	}

	if (fMouseInside)
		win->fMouseInsideViews.RemoveItem(this);

	if (fEventMask &B_POINTER_EVENTS)
		win->fMouseInterestedViews.RemoveItem(this);

	if (fEventMask &B_KEYBOARD_EVENTS)
		win->fKeyboardInterestedViews.RemoveItem(this);

	BView::MakeFocus(false);

	Invalidate();

	if (Flags() &B_PULSE_NEEDED) {
		win->fNeededToPulseViews.RemoveItem(this);
		if (win->fPulseRunner)
			win->fPulseRunner->SetCount((win->fPulseRate >B_INT64_CONSTANT(0) &&
			                             win->fNeededToPulseViews.CountItems() > 0 &&
			                             win->IsHidden() == false) ? -1 : 0);
	}
}


void
BView::MovePenTo(BPoint pt)
{
	((BViewState*)fStates)->PenLocation = pt;
}


void
BView::MovePenTo(float x, float y)
{
	((BViewState*)fStates)->PenLocation.x = x;
	((BViewState*)fStates)->PenLocation.y = y;
}


void
BView::MovePenBy(float dx, float dy)
{
	((BViewState*)fStates)->PenLocation.x += dx;
	((BViewState*)fStates)->PenLocation.y += dy;
}


BPoint
BView::PenLocation() const
{
	return ((BViewState*)fStates)->PenLocation;
}


void
BView::SetPenSize(float size)
{
	if (size < 0) return;
	if (((BViewState*)fStates)->PenSize != size) {
		((BViewState*)fStates)->PenSize = size;
		fDC->SetPenSize((uint32)ceil((double)size));
	}
}


float
BView::PenSize() const
{
	return ((BViewState*)fStates)->PenSize;
}


void
BView::SetViewColor(rgb_color c)
{
	if (fViewColor != c) {
		fViewColor = c;
		Invalidate();
	}
}


void
BView::SetViewColor(uint8 r, uint8 g, uint8 b, uint8 a)
{
	rgb_color c;
	c.set_to(r, g, b, a);
	SetViewColor(c);
}


rgb_color
BView::ViewColor() const
{
	return fViewColor;
}


void
BView::SetHighColor(rgb_color c)
{
	((BViewState*)fStates)->HighColor = c;
}


void
BView::SetHighColor(uint8 r, uint8 g, uint8 b, uint8 a)
{
	rgb_color c;
	c.set_to(r, g, b, a);
	SetHighColor(c);
}


rgb_color
BView::HighColor() const
{
	return ((BViewState*)fStates)->HighColor;
}


void
BView::SetLowColor(rgb_color c)
{
	((BViewState*)fStates)->LowColor = c;
}


void
BView::SetLowColor(uint8 r, uint8 g, uint8 b, uint8 a)
{
	rgb_color c;
	c.set_to(r, g, b, a);
	SetLowColor(c);
}


rgb_color
BView::LowColor() const
{
	return ((BViewState*)fStates)->LowColor;
}


void
BView::StrokePoint(BPoint pt, pattern p)
{
	MovePenTo(pt);

	if (IsVisible() == false) return;

	ConvertToWindow(&pt);

	BPoint _pt_ = BPoint(PenSize() / 2.f, PenSize() / 2.f);
	BRect updateRect(pt - _pt_, pt + _pt_);

	fDC->SetHighColor(HighColor());
	fDC->SetLowColor(LowColor());
	fDC->SetPattern(p);

	pt.Floor();
	if (Window()->fPixmap->StrokePoint(fDC, (int32)pt.x, (int32)pt.y) == B_OK) Window()->_Update(updateRect, false);
}


void
BView::StrokePoints(const BPoint *_pts, int32 count, const uint8 *alpha, pattern p)
{
	if (_pts == NULL || count <= 0) return;

	MovePenTo(_pts[count - 1]);

	if (IsVisible() == false) return;

	int32 *pts = new int32[2 * count];
	int32 *tmp = pts;

	BPoint pmin, pmax;

	for (int32 i = 0; i < count; i++) {
		BPoint pt = ConvertToWindow(_pts[i]);

		if (i == 0)
			pmin = pmax = pt;
		else {
			pmin.x = min_c(pmin.x, pt.x);
			pmin.y = min_c(pmin.y, pt.y);
			pmax.x = max_c(pmax.x, pt.x);
			pmax.y = max_c(pmax.y, pt.y);
		}

		pt.Floor();

		*tmp++ = (int32)pt.x;
		*tmp++ = (int32)pt.y;
	}

	fDC->SetHighColor(HighColor());
	fDC->SetLowColor(LowColor());
	fDC->SetPattern(p);

	status_t status;
	if (alpha) status = Window()->fPixmap->StrokePoints_Alphas(fDC, pts, alpha, count);
	else status = Window()->fPixmap->StrokePoints(fDC, pts, count);

	delete[] pts;

	if (status == B_OK) {
		BRect updateRect(BRect(pmin, pmax).InsetByCopy(PenSize() / -2.f, PenSize() / -2.f));
		Window()->_Update(updateRect, false);
	}
}


void
BView::StrokeLine(BPoint pt, pattern p)
{
	StrokeLine(PenLocation(), pt, p);
}


void
BView::StrokeLine(BPoint pt0, BPoint pt1, pattern p)
{
	MovePenTo(pt1);

	if (IsVisible() == false) return;

	ConvertToWindow(&pt0);
	ConvertToWindow(&pt1);

	BRect updateRect;
	updateRect.left = min_c(pt0.x, pt1.x) - PenSize() / 2.f;
	updateRect.top = min_c(pt0.y, pt1.y) - PenSize() / 2.f;
	updateRect.right = max_c(pt0.x, pt1.x) + PenSize() / 2.f;
	updateRect.bottom = max_c(pt0.y, pt1.y) + PenSize() / 2.f;

	fDC->SetHighColor(HighColor());
	fDC->SetLowColor(LowColor());
	fDC->SetPattern(p);

	pt0.Floor();
	pt1.Floor();
	if (Window()->fPixmap->StrokeLine(fDC,
	                                  (int32)pt0.x, (int32)pt0.y,
	                                  (int32)pt1.x, (int32)pt1.y) == B_OK) Window()->_Update(updateRect, false);
}


void
BView::StrokeRect(BRect r, pattern p)
{
	if (r.IsValid() == false || IsVisible() == false) return;

	ConvertToWindow(&r);
	BRect updateRect(r.InsetByCopy(PenSize() / -2.f, PenSize() / -2.f));

	fDC->SetHighColor(HighColor());
	fDC->SetLowColor(LowColor());
	fDC->SetPattern(p);

	r.Floor();
	if (Window()->fPixmap->StrokeRect(fDC,
	                                  (int32)r.left, (int32)r.top,
	                                  (uint32)r.Width(), (uint32)r.Height()) == B_OK) Window()->_Update(updateRect, false);
}


void
BView::StrokePolygon(const BPolygon *aPolygon, bool closed, pattern p)
{
	if (aPolygon == NULL || aPolygon->CountPoints() <= 0 || IsVisible() == false) return;

	int32 *pts = new int32[2 * aPolygon->CountPoints()];
	int32 *tmp = pts;
	const BPoint *polyPts = aPolygon->Points();

	for (int32 i = 0; i < aPolygon->CountPoints(); i++) {
		BPoint pt = ConvertToWindow(*polyPts++).FloorSelf();
		*tmp++ = (int32)pt.x;
		*tmp++ = (int32)pt.y;
	}

	fDC->SetHighColor(HighColor());
	fDC->SetLowColor(LowColor());
	fDC->SetPattern(p);

	if (Window()->fPixmap->StrokePolygon(fDC, pts, aPolygon->CountPoints(), closed) == B_OK) {
		BRect r = ConvertToWindow(aPolygon->Frame());
		BRect updateRect(r.InsetByCopy(PenSize() / -2.f, PenSize() / -2.f));
		Window()->_Update(updateRect, false);
	}

	delete[] pts;
}


void
BView::StrokePolygon(const BPoint *ptArray, int32 numPts, bool closed, pattern p)
{
	BPolygon aPolygon(ptArray, numPts);
	if (aPolygon.CountPoints() <= 0) return;
	StrokePolygon(&aPolygon, closed, p);
}


void
BView::FillPolygon(const BPolygon *aPolygon, pattern p)
{
	if (aPolygon == NULL || aPolygon->CountPoints() <= 0 || IsVisible() == false) return;

	int32 *pts = new int32[2 * aPolygon->CountPoints()];
	int32 *tmp = pts;
	const BPoint *polyPts = aPolygon->Points();

	for (int32 i = 0; i < aPolygon->CountPoints(); i++) {
		BPoint pt = ConvertToWindow(*polyPts++).FloorSelf();
		*tmp++ = (int32)pt.x;
		*tmp++ = (int32)pt.y;
	}

	fDC->SetHighColor(HighColor());
	fDC->SetLowColor(LowColor());
	fDC->SetPattern(p);

	if (Window()->fPixmap->FillPolygon(fDC, pts, aPolygon->CountPoints()) == B_OK) {
		BRect r = ConvertToWindow(aPolygon->Frame());
		BRect updateRect(r.InsetByCopy(PenSize() / -2.f, PenSize() / -2.f));
		Window()->_Update(updateRect, false);
	}

	delete[] pts;
}


void
BView::FillPolygon(const BPoint *ptArray, int32 numPts, pattern p)
{
	BPolygon aPolygon(ptArray, numPts);
	if (aPolygon.CountPoints() <= 0) return;
	FillPolygon(&aPolygon, p);
}


void
BView::StrokeTriangle(BPoint pt1, BPoint pt2, BPoint pt3, pattern p)
{
	BPoint pts[3] = {pt1, pt2, pt3};
	StrokePolygon(pts, 3, true, p);
}


void
BView::FillTriangle(BPoint pt1, BPoint pt2, BPoint pt3, pattern p)
{
	BPoint pts[3] = {pt1, pt2, pt3};
	FillPolygon(pts, 3, p);
}


void
BView::StrokeRects(const BRect *rs, int32 count, pattern p)
{
	if (rs == NULL || count <= 0 || IsVisible() == false) return;

	int32 *rects = new int32[4 * count];

	BRect updateRect;
	int32 _count_ = 0;

	int32 *tRects = rects;
	for (int32 i = 0; i < count; i++) {
		BRect r = *rs++;
		ConvertToWindow(&r);
		if (r.IsValid() == false) continue;
		updateRect |= r;
		r.Floor();
		*tRects++ = (int32)r.left;
		*tRects++ = (int32)r.top;
		*tRects++ = (int32)r.Width();
		*tRects++ = (int32)r.Height();
		_count_++;
	}

	if (_count_ > 0) {
		fDC->SetHighColor(HighColor());
		fDC->SetLowColor(LowColor());
		fDC->SetPattern(p);

		if (Window()->fPixmap->StrokeRects(fDC, rects, _count_) == B_OK) {
			updateRect.InsetBy(PenSize() / -2.f, PenSize() / -2.f);
			Window()->_Update(updateRect, false);
		}
	}

	delete[] rects;
}


void
BView::FillRect(BRect r, pattern p)
{
	if (r.IsValid() == false || IsVisible() == false) return;

	ConvertToWindow(&r);
	BRect updateRect(r.InsetByCopy(PenSize() / -2.f, PenSize() / -2.f));

	fDC->SetHighColor(HighColor());
	fDC->SetLowColor(LowColor());
	fDC->SetPattern(p);

	r.Floor();
	if (Window()->fPixmap->FillRect(fDC,
	                                (int32)r.left, (int32)r.top,
	                                (uint32)r.Width(), (uint32)r.Height()) == B_OK) Window()->_Update(updateRect, false);
}


void
BView::FillRects(const BRect *rs, int32 count, pattern p)
{
	if (rs == NULL || count <= 0 || IsVisible() == false) return;

	int32 *rects = new int32[4 * count];

	BRect updateRect;
	int32 _count_ = 0;

	int32 *tRects = rects;
	for (int32 i = 0; i < count; i++) {
		BRect r = *rs++;
		ConvertToWindow(&r);
		if (r.IsValid() == false) continue;
		updateRect |= r;
		r.Floor();
		*tRects++ = (int32)r.left;
		*tRects++ = (int32)r.top;
		*tRects++ = (int32)r.Width();
		*tRects++ = (int32)r.Height();
		_count_++;
	}

	if (_count_ > 0) {
		fDC->SetHighColor(HighColor());
		fDC->SetLowColor(LowColor());
		fDC->SetPattern(p);

		if (Window()->fPixmap->FillRects(fDC, rects, _count_) == B_OK) {
			updateRect.InsetBy(PenSize() / -2.f, PenSize() / -2.f);
			Window()->_Update(updateRect, false);
		}
	}

	delete[] rects;
}


void
BView::FillRegion(const BRegion *region, pattern p)
{
	if (region == NULL || IsVisible() == false) return;

	BRect updateRect = region->Frame().InsetByCopy(PenSize() / -2.f, PenSize() / -2.f);

	BRegion aRegion(*region);
	ConvertToWindow(&aRegion);
	if (aRegion.CountRects() <= 0) return;

	fDC->SetHighColor(HighColor());
	fDC->SetLowColor(LowColor());
	fDC->SetPattern(p);

	if (Window()->fPixmap->FillRegion(fDC, aRegion) == B_OK) Window()->_Update(updateRect, false);
}


void
BView::StrokeRoundRect(BRect r, float xRadius, float yRadius, pattern p)
{
	if (r.IsValid() == false || xRadius < 0 || yRadius < 0 || IsVisible() == false) return;
	if (r.Width() == 0 || r.Height() == 0 || (xRadius == 0 && yRadius == 0)) {
		StrokeRect(r, p);
		return;
	}

	ConvertToWindow(&r);
	BRect updateRect(r.InsetByCopy(PenSize() / -2.f, PenSize() / -2.f));

	fDC->SetHighColor(HighColor());
	fDC->SetLowColor(LowColor());
	fDC->SetPattern(p);

	r.Floor();
	if (Window()->fPixmap->StrokeRoundRect(fDC,
	                                       (int32)r.left, (int32)r.top,
	                                       (uint32)r.Width(), (uint32)r.Height(),
	                                       (uint32)ceil((double)xRadius),
	                                       (uint32)ceil((double)yRadius)) == B_OK) Window()->_Update(updateRect, false);
}


void
BView::FillRoundRect(BRect r, float xRadius, float yRadius, pattern p)
{
	if (r.IsValid() == false || xRadius < 0 || yRadius < 0 || IsVisible() == false) return;
	if (r.Width() == 0 || r.Height() == 0 || (xRadius == 0 && yRadius == 0)) {
		FillRect(r, p);
		return;
	}

	ConvertToWindow(&r);
	BRect updateRect(r.InsetByCopy(PenSize() / -2.f, PenSize() / -2.f));

	fDC->SetHighColor(HighColor());
	fDC->SetLowColor(LowColor());
	fDC->SetPattern(p);

	r.Floor();
	if (Window()->fPixmap->FillRoundRect(fDC,
	                                     (int32)r.left, (int32)r.top,
	                                     (uint32)r.Width(), (uint32)r.Height(),
	                                     (uint32)ceil((double)xRadius),
	                                     (uint32)ceil((double)yRadius)) == B_OK) Window()->_Update(updateRect, false);
}


void
BView::StrokeArc(BPoint center, float xRadius, float yRadius, float start_angle, float arc_angle, pattern p)
{
	if (xRadius <= 0 || yRadius <= 0) return;

	BRect r;
	r.left = center.x - xRadius;
	r.top = center.y - yRadius;
	r.right = center.x + xRadius;
	r.bottom = center.y + yRadius;

	StrokeArc(r, start_angle, arc_angle, p);
}


void
BView::StrokeArc(BRect r, float start_angle, float arc_angle, pattern p)
{
	if (r.IsValid() == false || IsVisible() == false) return;

	ConvertToWindow(&r);
	BRect updateRect(r.InsetByCopy(PenSize() / -2.f, PenSize() / -2.f));

	fDC->SetHighColor(HighColor());
	fDC->SetLowColor(LowColor());
	fDC->SetPattern(p);

	r.Floor();
	if (Window()->fPixmap->StrokeArc(fDC,
	                                 (int32)r.left, (int32)r.top,
	                                 (uint32)r.Width(), (uint32)r.Height(),
	                                 start_angle, start_angle + arc_angle) == B_OK) Window()->_Update(updateRect, false);
}


void
BView::FillArc(BPoint center, float xRadius, float yRadius, float start_angle, float arc_angle, pattern p)
{
	if (xRadius <= 0 || yRadius <= 0) return;

	BRect r;
	r.left = center.x - xRadius;
	r.top = center.y - yRadius;
	r.right = center.x + xRadius;
	r.bottom = center.y + yRadius;

	FillArc(r, start_angle, arc_angle, p);
}


void
BView::FillArc(BRect r, float start_angle, float arc_angle, pattern p)
{
	if (r.IsValid() == false || IsVisible() == false) return;

	ConvertToWindow(&r);
	BRect updateRect(r.InsetByCopy(PenSize() / -2.f, PenSize() / -2.f));

	fDC->SetHighColor(HighColor());
	fDC->SetLowColor(LowColor());
	fDC->SetPattern(p);

	r.Floor();
	if (Window()->fPixmap->FillArc(fDC,
	                               (int32)r.left, (int32)r.top,
	                               (uint32)r.Width(), (uint32)r.Height(),
	                               start_angle, start_angle + arc_angle) == B_OK) Window()->_Update(updateRect, false);
}


void
BView::StrokeEllipse(BPoint center, float xRadius, float yRadius, pattern p)
{
	StrokeArc(center, xRadius, yRadius, 0, 360, p);
}


void
BView::StrokeEllipse(BRect rect, pattern p)
{
	StrokeArc(rect, 0, 360, p);
}


void
BView::FillEllipse(BPoint center, float xRadius, float yRadius, pattern p)
{
	FillArc(center, xRadius, yRadius, 0, 360, p);
}


void
BView::FillEllipse(BRect rect, pattern p)
{
	FillArc(rect, 0, 360, p);
}


void
BView::PushState()
{
	if (Window() == NULL)
		ETK_ERROR("[INTERFACE]: %s --- View isn't attached to a window!", __PRETTY_FUNCTION__);
	fStates = (void*)(new BViewState((BViewState*)fStates));
}


void
BView::PopState()
{
	if (Window() == NULL)
		ETK_ERROR("[INTERFACE]: %s --- View isn't attached to a window!", __PRETTY_FUNCTION__);

	BViewState *viewStates = (BViewState*)fStates;
	if (viewStates->PreviousState == NULL) {
		ETK_WARNING("[INTERFACE]: %s --- Maybe you don't call \"PushState\" and \"PopState\" at same times.", __PRETTY_FUNCTION__);
		return;
	}
	fStates = (void*)viewStates->PreviousState;
	delete viewStates;

	fDC->SetDrawingMode(((BViewState*)fStates)->DrawingMode);
	fDC->SetSquarePointStyle(((BViewState*)fStates)->SquarePointStyle);
	fDC->SetPenSize((uint32)ceil((double)((BViewState*)fStates)->PenSize));

	BRegion clipping(*(fLayout->VisibleRegion()));
	if (((BViewState*)fStates)->Clipping != NULL) clipping &= *(((BViewState*)fStates)->Clipping);
	if (fClippingTemp.CountRects() > 0) clipping &= fClippingTemp;
	ConvertToWindow(&clipping);
	fDC->SetClipping(clipping);
}


void
BView::Invalidate(BRect invalRect, bool redraw)
{
	if (invalRect.IsValid() == false || IsVisible() == false) return;
	if (fLayout->VisibleRegion()->Intersects(invalRect) == false) return;
	if (Window() != NULL) Window()->Invalidate(ConvertToWindow(invalRect), redraw);
}


void
BView::Invalidate(bool redraw)
{
	Invalidate(Bounds(), redraw);
}


void
BView::_Expose(BRegion region, bigtime_t when)
{
	if (IsVisible() == false) return;

	BWindow *win = Window();

	if (when < fScrollTimeStamp) {
		win->fBrokeOnExpose = true;
		return;
	}

	if (!(fViewFlags &B_UPDATE_WITH_REGION)) region.Set(region.Frame());

	BRegion clipping(*(fLayout->VisibleRegion()));

	region &= clipping;
	if (region.CountRects() <= 0) return;

	if (fViewColor != B_TRANSPARENT_COLOR) {
		fDC->SetHighColor(fViewColor);
		fDC->SetLowColor(fViewColor);
		fDC->SetPattern(B_SOLID_HIGH);

		fDC->SetPenSize(0);
		fDC->SetDrawingMode(B_OP_COPY);

		BRegion tmpClipping(region);
		ConvertToWindow(&tmpClipping);
		fDC->SetClipping(tmpClipping);

		BRect rect = ConvertToWindow(region.Frame());
		rect.Floor();
		win->fPixmap->FillRect(fDC, (int32)rect.left, (int32)rect.top, (uint32)rect.Width(), (uint32)rect.Height());
	}

	if (((BViewState*)fStates)->Clipping != NULL) clipping &= *(((BViewState*)fStates)->Clipping);
	fClippingTemp = region;
	clipping &= fClippingTemp;
	ConvertToWindow(&clipping);
	fDC->SetClipping(clipping);

	fDC->SetDrawingMode(((BViewState*)fStates)->DrawingMode);
	fDC->SetPenSize((uint32)ceil((double)((BViewState*)fStates)->PenSize));

	if (fViewFlags &B_WILL_DRAW) {
		if (fViewFlags &B_UPDATE_WITH_REGION)
			for (int32 i = 0; i < region.CountRects(); i++) Draw(region.RectAt(i));
		else
			Draw(region.Frame());
	}

	bool doQuit = false;
	for (BView *child = ChildAt(0); child != NULL; child = child->NextSibling()) {
		if (win->fBrokeOnExpose || win->_HasResizeMessage(true)) {
			doQuit = true;
			break;
		}
		if (child->VisibleFrameRegion().Intersects(&region) == false) continue;
		child->_Expose(child->ConvertFromParent(region), when);
	}

	if (!doQuit && (fViewFlags &B_DRAW_ON_CHILDREN)) {
		if (fViewFlags &B_UPDATE_WITH_REGION)
			for (int32 i = 0; i < region.CountRects(); i++) DrawAfterChildren(region.RectAt(i));
		else
			DrawAfterChildren(region.Frame());
	}

	fClippingTemp.MakeEmpty();

	clipping = *(fLayout->VisibleRegion());
	if (((BViewState*)fStates)->Clipping != NULL) clipping &= *(((BViewState*)fStates)->Clipping);
	ConvertToWindow(&clipping);
	fDC->SetClipping(clipping);
}


void
BView::SetDrawingMode(e_drawing_mode mode)
{
	if (((BViewState*)fStates)->DrawingMode != mode) {
		((BViewState*)fStates)->DrawingMode = mode;
		fDC->SetDrawingMode(mode);
	}
}


e_drawing_mode
BView::DrawingMode() const
{
	return ((BViewState*)fStates)->DrawingMode;
}


void
BView::SetFont(const BFont *font, uint8 mask)
{
	if (font == NULL) return;

	if (mask &B_FONT_ALL) {
		((BViewState*)fStates)->Font = *font;
	} else {
		if (mask &B_FONT_FAMILY_AND_STYLE)
			((BViewState*)fStates)->Font.SetFamilyAndStyle(font->FamilyAndStyle());

		if (mask &B_FONT_SIZE)
			((BViewState*)fStates)->Font.SetSize(font->Size());

		if (mask &B_FONT_SHEAR)
			((BViewState*)fStates)->Font.SetShear(font->Shear());

		if (mask &B_FONT_SPACING)
			((BViewState*)fStates)->Font.SetSpacing(font->Spacing());
	}
}


void
BView::SetFont(const e_font_desc *fontDesc, uint8 mask)
{
	if (fontDesc == NULL) return;
	BFont font(*fontDesc);
	SetFont(&font, mask);
}


void
BView::GetFont(BFont *font) const
{
	if (font != NULL) *font = ((BViewState*)fStates)->Font;
}


void
BView::SetFontSize(float size)
{
	if (size <= 0) return;

	BFont font(((BViewState*)fStates)->Font);
	font.SetSize(size);

	SetFont(&font, B_FONT_SIZE);
}


void
BView::GetFontHeight(font_height *height) const
{
	((BViewState*)fStates)->Font.GetHeight(height);
}


void
BView::ForceFontAliasing(bool enable)
{
	fForceFontAliasing = enable;
}


void
BView::DrawString(const char *aString, int32 length, float tabWidth)
{
	DrawString(aString, PenLocation(), length, tabWidth);
}


void
BView::DrawString(const char *aString, BPoint location, int32 length, float tabWidth)
{
	if (aString == NULL || *aString == 0 || length == 0 || IsVisible() == false) return;

	BFontEngine *engine = ((BViewState*)fStates)->Font.Engine();
	if (engine == NULL) return;

	float size = ((BViewState*)fStates)->Font.Size();
	float spacing = ((BViewState*)fStates)->Font.Spacing();
	float shear = ((BViewState*)fStates)->Font.Shear();
	bool bold = ((BViewState*)fStates)->Font.IsBoldStyle();

	engine->Lock();
	e_font_render_mode renderMode = engine->RenderMode();
	float spaceWidth = engine->StringWidth(" ", size, spacing, shear, bold, 1);
	engine->Unlock();

	if (!(renderMode & (B_FONT_RENDER_DIRECTLY |B_FONT_RENDER_PIXMAP))) return;

	if (tabWidth == 0) {
		if (renderMode &B_FONT_RENDER_DIRECTLY)
			DrawStringInDirectlyMode(aString, location, length);
		else //B_FONT_RENDER_PIXMAP
			DrawStringInPixmapMode(aString, location, length);
	} else {
		BString aStr(aString, length);
		float spacing_width = spacing * size;

		if (tabWidth < 0) {
			if (tabWidth <B_FONT_MIN_TAB_WIDTH) tabWidth = -B_FONT_MIN_TAB_WIDTH;
			else tabWidth = (float)(ceil((double)(-tabWidth)));
			tabWidth = (tabWidth * spaceWidth) + (tabWidth - 1.f) * spacing_width;
		}

		MovePenTo(location);

		for (int32 aOffset = 0; aOffset < aStr.Length(); aOffset++) {
			int32 oldOffset = aOffset, len;
			aOffset = aStr.FindFirst("\t", aOffset);

			len = (aOffset < 0 ? aStr.Length() : aOffset) - oldOffset;
			if (len > 0) {
				if (renderMode &B_FONT_RENDER_DIRECTLY)
					DrawStringInDirectlyMode(aStr.String() + oldOffset, PenLocation(), len);
				else //B_FONT_RENDER_PIXMAP
					DrawStringInPixmapMode(aStr.String() + oldOffset, PenLocation(), len);
			}

			if (aOffset < 0) break;

			MovePenBy((aOffset > 0 ? spacing_width : 0) + tabWidth + UnitsPerPixel(), 0);
		}
	}
}


void
BView::DrawStringInDirectlyMode(const char *aString, BPoint location, int32 length)
{
	BFontEngine *engine = ((BViewState*)fStates)->Font.Engine();

	fDC->SetHighColor(HighColor());
	fDC->SetLowColor(LowColor());
	fDC->SetPattern(B_SOLID_HIGH);

	float size = ((BViewState*)fStates)->Font.Size();
	float spacing = ((BViewState*)fStates)->Font.Spacing();
	float shear = ((BViewState*)fStates)->Font.Shear();
	bool bold = ((BViewState*)fStates)->Font.IsBoldStyle();
	bool force_mono = fForceFontAliasing;

	MovePenTo(location);

	engine->Lock();
	engine->ForceFontAliasing(force_mono);
	float width = engine->StringWidth(aString, size, spacing, shear, bold, length);
	BRect updateRect = engine->RenderString(this, aString, size, spacing, shear, bold, length);
	engine->Unlock();

	if (updateRect.IsValid()) {
		ConvertToWindow(&updateRect);
		Window()->_Update(updateRect, false);
	}

	location.x += width + UnitsPerPixel();
	MovePenTo(location);
}


void
BView::DrawStringInPixmapMode(const char *aString, BPoint location, int32 length)
{
	BRect rect = VisibleBounds();
	BFontEngine *engine = ((BViewState*)fStates)->Font.Engine();

	float size = ((BViewState*)fStates)->Font.Size();
	float spacing = ((BViewState*)fStates)->Font.Spacing();
	float shear = ((BViewState*)fStates)->Font.Shear();
	bool bold = ((BViewState*)fStates)->Font.IsBoldStyle();
	bool is_mono = fForceFontAliasing;

	int32 w = 0, h = 0;

	engine->Lock();

	font_height fheight;
	engine->GetHeight(&fheight, size);

	BPoint startPoint = location;
	startPoint.y -= fheight.ascent + 1;

	engine->ForceFontAliasing(is_mono);

	float width = engine->StringWidth(aString, size, spacing, shear, bold, length);
	uint8 *bitmap = engine->RenderString(aString, &w, &h, &is_mono, size, spacing, shear, bold, length);

	engine->Unlock();

	rect &= BRect(startPoint, startPoint + BPoint((float)w, (float)h));
	if (rect.IsValid() == false || startPoint.x > rect.right || startPoint.y > rect.bottom ||
	        startPoint.x + (float)w < rect.left || startPoint.y + (float)h < rect.top) {
		if (bitmap) delete[] bitmap;
		return;
	}

	int32 _w_ = w;
	int32 _h_ = h;
	w = min_c(w, (int32)(ceil(rect.Width())));
	h = min_c(h, (int32)(ceil(rect.Height())));

	if (!bitmap || w <= 0 || h <= 0) {
		if (bitmap) delete[] bitmap;
		return;
	}

	int32 *pts = new int32[2 * w * h];
	uint8 *alpha = is_mono ? NULL : new uint8[w * h];
	int32 pointCount = 0;

	int32 i_start = max_c((int32)(ceil(rect.top) - ceil(startPoint.y)), 0);
	int32 j_start = max_c((int32)(ceil(rect.left) - ceil(startPoint.x)), 0);

	ConvertToWindow(&startPoint);
	BPoint pmin, pmax;

	int32 *tmp = pts;
	for (int32 i = i_start; i < min_c(i_start + h, _h_); i++) {
		for (int32 j = j_start; j < min_c(j_start + w, _w_); j++) {
			uint8 c = bitmap[i * _w_ + j];
			if (c != 0) {
				BPoint pt = startPoint + BPoint((float)j, (float)i);
				if (alpha) alpha[pointCount] = c;

				if (pointCount == 0)
					pmin = pmax = pt;
				else {
					pmin.x = min_c(pmin.x, pt.x);
					pmin.y = min_c(pmin.y, pt.y);
					pmax.x = max_c(pmax.x, pt.x);
					pmax.y = max_c(pmax.y, pt.y);
				}

				pt.Floor();

				*tmp++ = (int32)pt.x;
				*tmp++ = (int32)pt.y;

				pointCount++;
			}
		}
	}

	status_t status = B_ERROR;

	if (pointCount > 0) {
		fDC->SetHighColor(HighColor());
		fDC->SetLowColor(LowColor());
		fDC->SetPattern(B_SOLID_HIGH);

		if (alpha && DrawingMode() == B_OP_ALPHA) {
			status = Window()->fPixmap->StrokePoints_Alphas(fDC, pts, alpha, pointCount);
		} else {
			if (alpha) {
				// here we just generate 16 gray
				const uint8 *_alpha_ = alpha;
				BList ptsLists[16];
				for (int32 i = 0; i < pointCount; i++) {
					int32 a = (int32)(*_alpha_++);
					ptsLists[a / 16].AddItem((void*)&pts[i * 2]);
				}

				rgb_color ptColors[16];
				rgb_color lcolor = LowColor();
				for (int32 i = 0; i < 16; i++) {
					ptColors[i].set_to(HighColor());
					lcolor.alpha = 255 - (uint8)i * 17;
					ptColors[i].mix(lcolor);
				}

				status = Window()->fPixmap->StrokePoints_Colors(fDC, ptsLists, 16, ptColors);
			} else {
				status = Window()->fPixmap->StrokePoints(fDC, pts, pointCount);
			}
		}
	}

	delete[] pts;
	if (alpha) delete[] alpha;

	delete[] bitmap;

	if (status == B_OK) {
		BPoint _pt_ = BPoint(PenSize() / 2.f, PenSize() / 2.f);
		BRect updateRect(pmin - _pt_, pmax + _pt_);
		Window()->_Update(updateRect, false);
	}

	location.x += width + UnitsPerPixel();
	MovePenTo(location);
}


void
BView::DrawString(const char *aString, int32 length, BPoint location, float tabWidth)
{
	DrawString(aString, location, length, tabWidth);
}


void
BView::MakeFocus(bool focusState)
{
	BWindow *win = Window();
	if (win == NULL) return;

	if (focusState) {
		if (win->fFocus != this) {
			if (win->fFocus) win->fFocus->MakeFocus(false);
			win->fFocus = this;
			win->SetPreferredHandler(this);
		}
	} else {
		if (win->fFocus == this) {
			win->fFocus = NULL;
			if (win->PreferredHandler() == this) win->SetPreferredHandler(NULL);
		}
	}
}


bool
BView::IsFocus() const
{
	BWindow *win = Window();
	if (win == NULL) return false;
	return(win->fFocus == this);
}


status_t
BView::SetEventMask(uint32 mask, uint32 options)
{
	if (fEventStored == false) return _SetEventMask(mask, options);

	fEventMaskStored = mask;
	fEventOptionsStored = options;

	return B_OK;
}


status_t
BView::_SetEventMask(uint32 mask, uint32 options)
{
	BWindow *win = Window();

	if (fEventMask != mask) {
		if (win == NULL) {
			fEventMask = mask;
		} else if (fEventMask != 0) {
			uint32 mask1 = fEventMask & ~mask;
			uint32 mask2 = mask & ~fEventMask;

			if (mask2 &B_POINTER_EVENTS) {
				if (win->fMouseInterestedViews.AddItem(this) == false) return B_ERROR;
			} else if (mask2 &B_KEYBOARD_EVENTS) {
				if (win->fKeyboardInterestedViews.AddItem(this) == false) return B_ERROR;
			}

			if (mask1 &B_POINTER_EVENTS)
				win->fMouseInterestedViews.RemoveItem(this);
			else if (mask1 &B_KEYBOARD_EVENTS)
				win->fKeyboardInterestedViews.RemoveItem(this);

			fEventMask = mask;
		} else {
			if (mask &B_POINTER_EVENTS) {
				if (win->fMouseInterestedViews.AddItem(this) == false) return B_ERROR;
			}

			if (mask &B_KEYBOARD_EVENTS) {
				if (win->fKeyboardInterestedViews.AddItem(this) == false) {
					if (mask &B_POINTER_EVENTS)
						win->fMouseInterestedViews.RemoveItem(this);
					return B_ERROR;
				}
			}

			fEventMask = mask;
		}
	}

	// TODO:B_NO_POINTER_HISTORY and action implement
	fEventOptions = (options &B_NO_POINTER_HISTORY ?B_NO_POINTER_HISTORY : 0);

	return B_OK;
}


uint32
BView::EventMask() const
{
	return fEventMask;
}


status_t
BView::SetPrivateEventMask(uint32 mask, uint32 options)
{
	// TODO: suspend etc...
	if (mask == 0) return B_ERROR;

	BWindow *win = Window();
	if (win == NULL) return B_ERROR;

	BMessage *msg = win->CurrentMessage();
	if (msg == NULL) return B_ERROR;

	if (mask == B_KEYBOARD_EVENTS) {
		if (!(msg->what != B_KEY_DOWN || msg->what != B_UNMAPPED_KEY_DOWN)) return B_ERROR;
	} else if (mask == B_POINTER_EVENTS) {
		if (msg->what != B_MOUSE_DOWN) return B_ERROR;
	} else {
		return B_ERROR;
	}

	uint32 eventMask, eventNewMask;
	uint32 eventOptions, eventNewOptions;

	eventMask = eventNewMask = fEventMask;
	eventOptions = eventNewOptions = fEventOptions;

	if (fEventStored == false) {
		fEventStored = true;

		fEventMaskStored = fEventMask;
		fEventOptionsStored = fEventOptions;

		eventNewMask = mask;
		eventNewOptions = options;
	} else {
		uint32 _mask_ = mask & ~eventNewMask;
		uint32 _options_ = options & ~eventNewOptions;

		if (_mask_ != 0) {
			if (_mask_ &B_POINTER_EVENTS) eventNewMask &= B_POINTER_EVENTS;
			if (_mask_ &B_KEYBOARD_EVENTS) eventNewMask &= B_KEYBOARD_EVENTS;
		}

		if (_options_ != 0) {
			if (_options_ &B_LOCK_WINDOW_FOCUS) eventNewOptions &= B_LOCK_WINDOW_FOCUS;
			if (_options_ &B_SUSPEND_VIEW_FOCUS) eventNewOptions &= B_SUSPEND_VIEW_FOCUS;
			if (_options_ &B_NO_POINTER_HISTORY) eventNewOptions &= B_NO_POINTER_HISTORY;
		}
	}

	if (eventMask != eventNewMask || eventOptions != eventNewOptions) {
		status_t status = _SetEventMask(eventNewMask, eventNewOptions);
		if (status != B_OK) return status;
		fEventOptions = eventNewOptions;
	}

	if (options &B_LOCK_WINDOW_FOCUS) {
		if (mask == B_POINTER_EVENTS) {
			if (fMouseGrabbed == false) {
				if (win->_GrabMouse() == false) {
					_SetEventMask(eventMask, eventOptions);
					fEventOptions = eventOptions;
					return B_ERROR;
				}
				fMouseGrabbed = true;
			}
		} else if (mask == B_KEYBOARD_EVENTS) {
			if (fKeyboardGrabbed == false) {
				if (win->_GrabKeyboard() == false) {
					_SetEventMask(eventMask, eventOptions);
					fEventOptions = eventOptions;
					return B_ERROR;
				}
				fKeyboardGrabbed = true;
			}
		}
	}

	return B_OK;
}


void
BView::GetPreferredSize(float *width, float *height)
{
	fLayout->BLayoutItem::GetPreferredSize(width, height);
}


void
BView::ResizeToPreferred()
{
	fLayout->BLayoutItem::ResizeToPreferred();
}


void
BView::GetClippingRegion(BRegion *clipping) const
{
	if (clipping == NULL) return;

	*clipping = *(fLayout->VisibleRegion());
	if (((BViewState*)fStates)->Clipping != NULL) *clipping &= *(((BViewState*)fStates)->Clipping);
}


void
BView::ConstrainClippingRegion(const BRegion *_clipping)
{
	if (_clipping == NULL) {
		if (((BViewState*)fStates)->Clipping == NULL) return;

		delete ((BViewState*)fStates)->Clipping;
		((BViewState*)fStates)->Clipping = NULL;

		BRegion clipping(*(fLayout->VisibleRegion()));
		if (fClippingTemp.CountRects() > 0) clipping &= fClippingTemp;
		ConvertToWindow(&clipping);
		fDC->SetClipping(clipping);
	} else {
		if (((BViewState*)fStates)->Clipping == NULL)
			((BViewState*)fStates)->Clipping = new BRegion(*_clipping);
		else
			*(((BViewState*)fStates)->Clipping) = *_clipping;

		BRegion clipping(*(fLayout->VisibleRegion()));
		clipping &= *_clipping;
		if (fClippingTemp.CountRects() > 0) clipping &= fClippingTemp;
		ConvertToWindow(&clipping);
		fDC->SetClipping(clipping);
	}
}


void
BView::ConstrainClippingRegion(BRect clipping)
{
	BRegion aRegion(clipping);
	ConstrainClippingRegion(&aRegion);
}


status_t
BView::GetMouse(BPoint *location, int32 *buttons, bool checkMessageQueue)
{
	BWindow *win = Window();
	if (win == NULL || location == NULL || buttons == NULL) return B_ERROR;

	if (checkMessageQueue) {
		// TODO
		return B_ERROR;
	}

	if (win->fWindow == NULL) {
		// TODO
		return B_ERROR;
	}

	int32 mx, my;
	if (win->fWindow->QueryMouse(&mx, &my, buttons) != B_OK) return B_ERROR;
	location->x = (float)mx;
	location->y = (float)my;
	ConvertFromWindow(location);

	return B_OK;
}


bool
BView::QueryCurrentMouse(bool pushed, int32 buttons, bool btnsAlone, int32 *clicks) const
{
	BWindow *win = Window();
	if (win == NULL) return false;

	BMessage *msg = win->CurrentMessage();
	if (msg == NULL) return false;
	if (pushed && msg->what != B_MOUSE_DOWN) return false;
	if (pushed == false && msg->what != B_MOUSE_UP) return false;

	int32 btns;
	if (msg->FindInt32("buttons", &btns) == false) return false;

	if (clicks) {
		if (msg->FindInt32("clicks", clicks) == false) *clicks = 1;
	}

	return(btnsAlone ? (buttons == btns) : (btns >= buttons));
}


void
BView::_UpdateVisibleRegion()
{
	if (!(is_kind_of(Parent(), BScrollView) == false || cast_as(Parent(), BScrollView)->fTarget != this)) {
		BRegion *region;
		cast_as(fLayout, BViewLayout)->_GetVisibleRegion(&region);
		*region &= ConvertFromParent(cast_as(Parent(), BScrollView)->TargetValidFrame());
	}

	BRegion clipping(*(fLayout->VisibleRegion()));
	if (((BViewState*)fStates)->Clipping != NULL) clipping &= *(((BViewState*)fStates)->Clipping);
	if (fClippingTemp.CountRects() > 0) clipping &= fClippingTemp;
	ConvertToWindow(&clipping);
	fDC->SetClipping(clipping);
}


bool
BView::IsPrinting() const
{
	// TODO
	return false;
}


float
BView::UnitsPerPixel() const
{
	return(fLayout->Container() != NULL ? fLayout->Container()->UnitsPerPixel() : 1);
}


void
BView::SetEnabled(bool state)
{
	if (cast_as(fLayout, BViewLayout)->IsEnabled() != state) {
		cast_as(fLayout, BViewLayout)->SetEnabled(state);
		if (Flags() &B_WILL_DRAW) Invalidate();
	}
}


bool
BView::IsEnabled() const
{
	return cast_as(fLayout, BViewLayout)->IsEnabled();
}


void
BView::ScrollBy(float dh, float dv)
{
	ScrollTo(BPoint(dh, dv) + fLayout->LeftTop());
}


void
BView::ScrollTo(float x, float y)
{
	ScrollTo(BPoint(x, y));
}


void
BView::ScrollTo(BPoint where)
{
	if (LeftTop() != where) {
		fLayout->ScrollTo(where);

		fScrollTimeStamp = e_real_time_clock_usecs();
		Invalidate(Bounds(), true);

		for (int32 i = 0; i < fScrollBar.CountItems(); i++) {
			BScrollBar *scrollbar = (BScrollBar*)fScrollBar.ItemAt(i);
			scrollbar->_SetValue(scrollbar->Orientation() == B_HORIZONTAL ? LeftTop().x : LeftTop().y, false);
		}
	}
}


void
BView::SetSquarePointStyle(bool state)
{
	if (((BViewState*)fStates)->SquarePointStyle != state) {
		if (fDC->SetSquarePointStyle(state) == B_OK) ((BViewState*)fStates)->SquarePointStyle = state;
	}
}


bool
BView::IsSquarePointStyle() const
{
	return ((BViewState*)fStates)->SquarePointStyle;
}


void
BView::DrawBitmap(const BBitmap *bitmap)
{
	DrawBitmap(bitmap, PenLocation());
}


void
BView::DrawBitmap(const BBitmap *bitmap, BPoint where)
{
	if (bitmap == NULL) return;

	BRect r = bitmap->Bounds();
	DrawBitmap(bitmap, r, r.OffsetToCopy(where));
}


void
BView::DrawBitmap(const BBitmap *bitmap, BRect destRect)
{
	if (bitmap == NULL) return;

	BRect r = bitmap->Bounds();
	DrawBitmap(bitmap, r, destRect);
}


void
BView::DrawBitmap(const BBitmap *bitmap, BRect srcRect, BRect destRect)
{
	if (bitmap == NULL || bitmap->fPixmap == NULL ||
	        srcRect.IsValid() == false || bitmap->Bounds().Intersects(srcRect) == false || destRect.IsValid() == false ||
	        fLayout->VisibleRegion()->Intersects(destRect) == false) return;

	ConvertToWindow(&destRect);
	BRect updateRect(destRect.InsetByCopy(PenSize() / -2.f, PenSize() / -2.f));

	srcRect.Floor();
	destRect.Floor();
	if (bitmap->fPixmap->CopyTo(fDC, Window()->fPixmap,
	                            (int32)srcRect.left, (int32)srcRect.top,
	                            (uint32)srcRect.Width(), (uint32)srcRect.Height(),
	                            (int32)destRect.left, (int32)destRect.top,
	                            (uint32)destRect.Width(), (uint32)destRect.Height()) == B_OK) Window()->_Update(updateRect, false);
}


void
BView::CopyBits(BRect srcRect, BRect destRect)
{
	if (srcRect.IsValid() == false || destRect.IsValid() == false ||
	        fLayout->VisibleRegion()->Intersects(destRect) == false) return;

	ConvertToWindow(&srcRect);
	ConvertToWindow(&destRect);
	BRect updateRect(destRect.InsetByCopy(PenSize() / -2.f, PenSize() / -2.f));

	srcRect.Floor();
	destRect.Floor();
	if (Window()->fPixmap->CopyTo(fDC, Window()->fPixmap,
	                              (int32)srcRect.left, (int32)srcRect.top,
	                              (uint32)srcRect.Width(), (uint32)srcRect.Height(),
	                              (int32)destRect.left, (int32)destRect.top,
	                              (uint32)destRect.Width(), (uint32)destRect.Height()) == B_OK) Window()->_Update(updateRect, false);
}


void
BView::Flush() const
{
	// nothing yet
}


void
BView::Sync() const
{
	// nothing yet
}

