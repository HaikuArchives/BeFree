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
 * File: Window.cpp
 *
 * --------------------------------------------------------------------------*/

#include <math.h>

#include <support/ClassInfo.h>
#include <support/String.h>
#include <kernel/Kernel.h>
#include <app/Application.h>
#include <support/Autolock.h>
#include <add-ons/graphics/GraphicsEngine.h>

#include "InterfaceDefs.h"
#include "View.h"
#include "ScrollBar.h"
#include "Window.h"
#include "Screen.h"
#include "layout/Layout.h"


class _LOCAL BWindowLayoutItem : public BLayoutItem
{
	public:
		BWindowLayoutItem(BRect frame);
		virtual ~BWindowLayoutItem();

		virtual void	Invalidate(BRect rect);
};


class _LOCAL BWindowLayoutContainer : public BLayoutContainer
{
	public:
		BWindowLayoutContainer(BWindow *win, BRect frame);
		virtual ~BWindowLayoutContainer();

		void		MoveTo(BPoint where);
		void		ResizeTo(float width, float height);

		BPoint		Origin() const;
		BLayoutItem	*TopItem() const;

		virtual void	Invalidate(BRect rect);

	private:
		BWindow *fWindow;
		BPoint fOrigin;
		BLayoutItem *fTopItem;
};


BWindowLayoutItem::BWindowLayoutItem(BRect frame)
		: BLayoutItem(frame, B_FOLLOW_NONE)
{
}


BWindowLayoutItem::~BWindowLayoutItem()
{
}


void
BWindowLayoutItem::Invalidate(BRect rect)
{
	if (Container() == NULL) return;
	rect.OffsetTo(ConvertToContainer(rect.LeftTop()));
	Container()->Invalidate(rect);
}


BWindowLayoutContainer::BWindowLayoutContainer(BWindow *win, BRect frame)
		: BLayoutContainer(), fWindow(NULL)
{
	fOrigin = frame.LeftTop();
	fTopItem = new BWindowLayoutItem(frame.OffsetToSelf(B_ORIGIN));
	fTopItem->Hide();
	AddItem(fTopItem);

	fWindow = win;
}


BWindowLayoutContainer::~BWindowLayoutContainer()
{
}


void
BWindowLayoutContainer::MoveTo(BPoint where)
{
	fOrigin = where;
}


void
BWindowLayoutContainer::ResizeTo(float width, float height)
{
	fTopItem->ResizeTo(width, height);
}


BPoint
BWindowLayoutContainer::Origin() const
{
	return fOrigin;
}


BLayoutItem*
BWindowLayoutContainer::TopItem() const
{
	return fTopItem;
}


void
BWindowLayoutContainer::Invalidate(BRect rect)
{
	if (fWindow == NULL) return;
	rect.OffsetTo(B_ORIGIN);
	fWindow->Invalidate(rect, true);
}


void
BWindow::InitSelf(BRect frame, const char *title, window_look look, window_feel feel, uint32 flags, uint32 workspace)
{
	if (app == NULL || app->fGraphicsEngine == NULL)
		ETK_ERROR("[INTERFACE]: Window must created within a application which has graphics-engine!");

#ifdef ETK_ENABLE_DEBUG
	BString winLooperName;
	winLooperName << "Window " << get_handler_token(this);
	SetName(winLooperName.String());
#endif // ETK_ENABLE_DEBUG

	fLayout = new BWindowLayoutContainer(this, frame);

	frame.Floor();
	if ((fWindow = app->fGraphicsEngine->CreateWindow((int32)frame.left, (int32)frame.top,
	               (uint32)max_c(frame.Width(), 0),
	               (uint32)max_c(frame.Height(), 0))) == NULL)
		ETK_ERROR("[INTERFACE]: %s --- Unable to create window!", __PRETTY_FUNCTION__);
	else if ((fPixmap = app->fGraphicsEngine->CreatePixmap((uint32)max_c(frame.Width(), 0),
	                    (uint32)max_c(frame.Height(), 0))) == NULL)
		ETK_ERROR("[INTERFACE]: %s --- Unable to create pixmap!", __PRETTY_FUNCTION__);
	else if ((fDC = app->fGraphicsEngine->CreateContext()) == NULL)
		ETK_ERROR("[INTERFACE]: %s --- Unable to create graphics context!", __PRETTY_FUNCTION__);

	fDC->SetClipping(BRegion(frame.OffsetToCopy(B_ORIGIN)));
	fDC->SetDrawingMode(B_OP_COPY);
	fDC->SetPattern(B_SOLID_HIGH);
	fDC->SetHighColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	fDC->SetPenSize(0);

	fWindowFlags = flags;
	fWindowLook = look;
	fWindowFeel = feel;
	fWindowTitle = title ? EStrdup(title) : NULL;
	fWindow->SetFlags(fWindowFlags);
	fWindow->SetLook(fWindowLook);
	fWindow->SetFeel(fWindowFeel);
	fWindow->SetBackgroundColor(fDC->HighColor());
	fWindow->SetTitle(fWindowTitle);

	BMessenger msgrSelf(this);
	BMessage pulseMsg(B_PULSE);
	fPulseRate = 500000;
	fPulseRunner = new BMessageRunner(msgrSelf, &pulseMsg, fPulseRate, 0);

	fFocus = NULL;
	fUpdateHolderThreadId = 0;
	fUpdateHolderCount = B_INT64_CONSTANT(-1);
	fInUpdate = false;
	fMinimized = false;
	fActivated = false;
	fActivatedTimeStamp = 0;
	fPositionChangedTimeStamp = 0;
	fSizeChangedTimeStamp = 0;
	fMouseGrabCount = 0;
	fKeyboardGrabCount = 0;
	fBrokeOnExpose = false;
	fWindowWorkspaces = 0;

	fWindow->ContactTo(&msgrSelf);

	SetWorkspaces(workspace);
}


BWindow::BWindow(BRect frame, const char *title,
                 window_type type,
                 uint32 flags, uint32 workspace)
		: BLooper(NULL, B_DISPLAY_PRIORITY)
{
	window_look look;
	window_feel feel;

	switch (type) {
		case B_TITLED_WINDOW:
			look = B_TITLED_WINDOW_LOOK;
			feel = B_NORMAL_WINDOW_FEEL;
			break;

		case B_MODAL_WINDOW:
			look = B_MODAL_WINDOW_LOOK;
			feel = B_MODAL_APP_WINDOW_FEEL;
			break;

		case B_DOCUMENT_WINDOW:
			look = B_DOCUMENT_WINDOW_LOOK;
			feel = B_NORMAL_WINDOW_FEEL;
			break;

		case B_BORDERED_WINDOW:
			look = B_BORDERED_WINDOW_LOOK;
			feel = B_NORMAL_WINDOW_FEEL;
			break;

		case B_FLOATING_WINDOW:
			look = B_FLOATING_WINDOW_LOOK;
			feel = B_FLOATING_APP_WINDOW_FEEL;
			break;

		default:
			look = B_TITLED_WINDOW_LOOK;
			feel = B_NORMAL_WINDOW_FEEL;
	}

	InitSelf(frame, title, look, feel, flags, workspace);
}


BWindow::BWindow(BRect frame, const char *title,
                 window_look look, window_feel feel,
                 uint32 flags, uint32 workspace)
		: BLooper(NULL, B_DISPLAY_PRIORITY)
{
	InitSelf(frame, title, look, feel, flags, workspace);
}


BWindow::~BWindow()
{
	Hide();

	BView *child = NULL;
	while ((child = ChildAt(CountChildren() - 1)) != NULL) {
		RemoveChild(child);
		delete child;
	}

	if (fWindow) delete fWindow;

	delete fPixmap;
	delete fDC;
	delete fLayout;
	delete fPulseRunner;
	if (fWindowTitle) delete[] fWindowTitle;
}


void
BWindow::DispatchMessage(BMessage *msg, BHandler *target)
{
	if (target == NULL) target = PreferredHandler();
	if (target == NULL || target->Looper() != this) return;

	if (target != this) {
		BLooper::DispatchMessage(msg, target);
		return;
	}

	bool sendNotices = true;

	msg->RemoveBool("etk:msg_from_gui");
	switch (msg->what) {
		case B_PULSE:
			if (IsHidden()) break;
			for (int32 i = 0; i < fNeededToPulseViews.CountItems(); i++) {
				BView *view = (BView*)fNeededToPulseViews.ItemAt(i);
				if (view->IsHidden() == false) PostMessage(msg, view);
			}
			break;

		case B_MOUSE_DOWN:
		case B_MOUSE_UP:
		case B_MOUSE_MOVED:
		case B_MOUSE_WHEEL_CHANGED: {
			BPoint where;
			if (msg->FindPoint("where", &where) == false && msg->what != B_MOUSE_WHEEL_CHANGED) {
				if (msg->FindPoint("screen_where", &where) == false) {
					ETK_DEBUG("[INTERFACE]: %s --- Invalid message.", __PRETTY_FUNCTION__);
					break;
				}
				ConvertFromScreen(&where);
				msg->AddPoint("where", where);
			}

			BMessage aMsg(*msg);

			if (msg->what != B_MOUSE_WHEEL_CHANGED) {
				uint32 saveWhat = aMsg.what;
				aMsg.what = B_MOUSE_MOVED;
				for (int32 i = 0; i < fMouseInsideViews.CountItems(); i++) {
					BView *view = (BView*)fMouseInsideViews.ItemAt(i);

					BPoint pt = view->ConvertFromWindow(where);
					if (view->fLayout->VisibleRegion()->Contains(pt)) continue;
					if (view->EventMask() &B_POINTER_EVENTS) continue;

					aMsg.ReplacePoint("where", pt);
					PostMessage(&aMsg, view);
				}
				aMsg.what = saveWhat;
			}

			for (BView *view = ChildAt(0); view != NULL; view = view->NextSibling()) {
				BPoint pt = view->fLayout->ConvertFromContainer(where);
				if (view->fLayout->VisibleRegion()->Contains(pt) == false) continue;

				if (!(view->EventMask() &B_POINTER_EVENTS)) {
					aMsg.ReplacePoint("where", pt);
					PostMessage(&aMsg, view);
				}

				break; // just one child can receive the message
			}

			for (int32 i = 0; i < fMouseInterestedViews.CountItems(); i++) {
				BView *view = (BView*)fMouseInterestedViews.ItemAt(i);

				aMsg.ReplacePoint("where", view->ConvertFromWindow(where));
				PostMessage(&aMsg, view);
			}
		}
		break;

		case B_UNMAPPED_KEY_DOWN:
		case B_UNMAPPED_KEY_UP:
		case B_KEY_DOWN:
		case B_KEY_UP:
		case B_MODIFIERS_CHANGED: {
			// TODO: shortcuts
			for (int32 i = -1; i < fKeyboardInterestedViews.CountItems(); i++) {
				BView *view = i < 0 ? CurrentFocus() : (BView*)fKeyboardInterestedViews.ItemAt(i);
				if ((i < 0 && view == NULL) || (i >= 0 && view == CurrentFocus())) continue;
				PostMessage(msg, view);
			}
		}
		break;

		case B_WORKSPACES_CHANGED: {
			uint32 curWorkspace;

			if (msg->FindInt32("new", (int32*)&curWorkspace) == false) break;
			if (curWorkspace != 0 && fWindowWorkspaces != curWorkspace) {
				uint32 oldWorkspace = fWindowWorkspaces;
				fWindowWorkspaces = curWorkspace;
				if (oldWorkspace != 0) WorkspacesChanged(oldWorkspace, curWorkspace);
			}
		}
		break;

		case B_WINDOW_ACTIVATED: {
			bigtime_t when;
			bool active = fActivated;

			if (msg->FindInt64("when", (int64*)&when) == false) break;
			if (!(fWindow == NULL || fWindow->GetActivatedState(&active) == B_OK)) break;

			if (fActivated != active && fActivatedTimeStamp <= when) {
				fActivated = active;
				fActivatedTimeStamp = when;
				if ((active && !(fWindowFlags &B_AVOID_FRONT)) && fWindow) fWindow->Raise();
				WindowActivated(active);
				for (BView *view = ChildAt(0); view != NULL; view = view->NextSibling()) {
					PostMessage(msg, view);
				}
			}
		}
		break;

		case B_WINDOW_MOVED:
		case B_WINDOW_RESIZED: {
			bigtime_t when;
			if (msg->FindInt64("when", &when) == false) break;
			if (msg->what == B_WINDOW_MOVED && when < fPositionChangedTimeStamp) break;
			if (msg->what == B_WINDOW_RESIZED && when < fSizeChangedTimeStamp) break;

			BRect frame = Frame();
			BPoint where = frame.LeftTop();
			float w = frame.Width();
			float h = frame.Height();

			if (msg->what == B_WINDOW_RESIZED) {
				if (msg->FindFloat("width", &w) == false || msg->FindFloat("height", &h) == false) break;
				msg->FindPoint("where", &where);
			} else { //B_WINDOW_MOVED
				if (msg->FindPoint("where", &where) == false) break;
			}

			bool doMoved = frame.LeftTop() != where;
			bool doResized = (frame.Width() != w || frame.Height() != h);

			if (CurrentMessage() == msg) {
				MessageQueue()->Lock();
				while (MessageQueue()->IsEmpty() == false) {
					BMessage *aMsg = MessageQueue()->FindMessage((int32)0);
					if (aMsg == NULL) break;

					if (!(aMsg->what == B_WINDOW_RESIZED || aMsg->what == B_WINDOW_MOVED)) {
						if (aMsg->what == _UPDATE_ || aMsg->what == _UPDATE_IF_NEEDED_) {
							if (!doResized) break;
							MessageQueue()->RemoveMessage(aMsg);
							continue;
						}
						break;
					}

					if (aMsg->what == B_WINDOW_RESIZED) {
						float w1, h1;
						bigtime_t nextWhen;
						if (aMsg->FindFloat("width", &w1) == false ||
						        aMsg->FindFloat("height", &h1) == false ||
						        aMsg->FindInt64("when", &nextWhen) == false ||
						        nextWhen < when) {
							MessageQueue()->RemoveMessage(aMsg);
							continue;
						}
						w = w1;
						h = h1;
						when = nextWhen;
						aMsg->FindPoint("where", &where);
					} else { //B_WINDOW_MOVED
						bigtime_t nextWhen;
						if (aMsg->FindInt64("when", &nextWhen) == false ||
						        nextWhen < when ||
						        aMsg->FindPoint("where", &where) == false) {
							MessageQueue()->RemoveMessage(aMsg);
							continue;
						}
						when = nextWhen;
					}

					if (frame.LeftTop() != where) doMoved = true;
					if (frame.Width() != w || frame.Height() != h) doResized = true;

					MessageQueue()->RemoveMessage(aMsg);
				}
				MessageQueue()->Unlock();
			}

			if (doMoved) {
				fPositionChangedTimeStamp = when;
				cast_as(fLayout, BWindowLayoutContainer)->MoveTo(where);
			}

			if (doResized) {
				fSizeChangedTimeStamp = when;

				BRect rFrame = frame;
				rFrame.right = rFrame.left + w;
				rFrame.bottom = rFrame.top + h;
				rFrame.Floor();
				fPixmap->ResizeTo((uint32)max_c(rFrame.Width(), 0), (uint32)max_c(rFrame.Height(), 0));
				fDC->SetClipping(BRegion(rFrame.OffsetToCopy(B_ORIGIN)));

				fExposeRect = Bounds();
				fBrokeOnExpose = false;
				if (fInUpdate == false) PostMessage(_UPDATE_IF_NEEDED_, this);

				// for disable update
				bool saveInUpdate = fInUpdate;
				fInUpdate = true;
				cast_as(fLayout, BWindowLayoutContainer)->ResizeTo(w, h);
				fInUpdate = saveInUpdate;
			} else if (fBrokeOnExpose) {
				fBrokeOnExpose = false;
				if (fInUpdate == false) PostMessage(_UPDATE_IF_NEEDED_, this);
			}

			sendNotices = false;

			frame = Frame();
			if (doMoved) {
				FrameMoved(frame.LeftTop());
				if (IsWatched(B_WINDOW_MOVED)) {
					BMessage aMsg(B_WINDOW_MOVED);
					aMsg.AddInt64("when", when);
					aMsg.AddPoint("where", frame.LeftTop());
					SendNotices(B_WINDOW_MOVED, &aMsg);
				}
			}
			if (doResized) {
				FrameResized(frame.Width(), frame.Height());
				if (IsWatched(B_WINDOW_RESIZED)) {
					BMessage aMsg(B_WINDOW_RESIZED);
					aMsg.AddInt64("when", when);
					aMsg.AddFloat("width", frame.Width());
					aMsg.AddFloat("height", frame.Height());
					SendNotices(B_WINDOW_RESIZED, &aMsg);
				}
			}
		}
		break;

		case B_MINIMIZE:
		case B_MINIMIZED: {
			bool minimize;
			if (msg->FindBool("minimize", &minimize) == false) break;
			Minimize(minimize);
		}
		break;

		case _UPDATE_IF_NEEDED_: {
			sendNotices = false;

			bigtime_t when = e_real_time_clock_usecs();
			msg->FindInt64("when", (int64*)&when);

			if (CurrentMessage() == msg) {
				bool noNeededToUpdate = false;
				BMessage *aMsg = NULL;

				MessageQueue()->Lock();
				if ((aMsg = MessageQueue()->FindMessage((int32)0)) != NULL) {
					if (aMsg->what == _UPDATE_IF_NEEDED_) {
						// Here we don't need to update until the next event
						noNeededToUpdate = true;
					} else if (aMsg->what == _UPDATE_) {
						// Here we don't need to update because of
						// that it's a expose event next to handle, and
						// probably within the short time for switching
						// another or more expose events will need to be handle.
						noNeededToUpdate = true;
					}
				}
				MessageQueue()->Unlock();
				if (noNeededToUpdate) break;
			}

			_UpdateIfNeeded(when);
		}
		break;

		case _UPDATE_: { // TODO: speed up
			sendNotices = false;

			BRect rect;
			if (msg->FindRect("etk:frame", &rect)) {
				bool expose = false;
				msg->FindBool("etk:expose", &expose);

				rect &= Bounds();
				if (rect.IsValid()) {
					if (expose) fExposeRect |= rect;
					else fUpdateRect |= rect;
				}
			}

			if (CurrentMessage() == msg) {
				bool noNeededToSendUpdate = false;
				BMessage *aMsg = NULL;
				MessageQueue()->Lock();
				if ((aMsg = MessageQueue()->FindMessage((int32)0)) != NULL) {
					if (aMsg->what == _UPDATE_IF_NEEDED_) {
						// Here we don't need to post _UPDATE_IF_NEEDED_
						noNeededToSendUpdate = true;
					} else if (aMsg->what == _UPDATE_) {
						// Here we don't post _UPDATE_IF_NEEDED_ because of
						// that it's a expose event next to handle, and
						// probably within the short time for switching
						// another or more expose events will need to be handle.
						noNeededToSendUpdate = true;
					}
				}
				MessageQueue()->Unlock();
				if (noNeededToSendUpdate) break;
			}

			if (fInUpdate == false) PostMessage(_UPDATE_IF_NEEDED_, this);
		}
		break;

		default:
			sendNotices = false;
			BLooper::DispatchMessage(msg, target);
	}

	if (sendNotices && IsWatched(msg->what)) SendNotices(msg->what, msg);
}


void
BWindow::Quit()
{
	if (!IsLockedByCurrentThread())
		ETK_ERROR("[INTERFACE]: %s --- Window must LOCKED before \"Quit()\" call!", __PRETTY_FUNCTION__);

	if (fWindow) {
		fWindow->Hide();
		fWindow->ContactTo(NULL);
	}

	if (fWindowFlags &B_QUIT_ON_WINDOW_CLOSE) app->PostMessage(B_QUIT_REQUESTED);

	BLooper::Quit();
}


void
BWindow::Show()
{
	if (cast_as(fLayout, BWindowLayoutContainer)->TopItem()->IsHidden(false) == false) return;

	cast_as(fLayout, BWindowLayoutContainer)->TopItem()->Show();

	fMinimized = false;
	if (fWindow) fWindow->Show();

	if (fPulseRunner)
		fPulseRunner->SetCount((fPulseRate > 0 && fNeededToPulseViews.CountItems() > 0) ? -1 : 0);

	if (!(IsRunning() || Proxy() != this)) Run();

	if (fWindowFeel == B_MODAL_APP_WINDOW_FEEL) {
		BMessenger msgrSelf(this);
		app->AddModalWindow(msgrSelf);
	}
}


void
BWindow::Hide()
{
	if (cast_as(fLayout, BWindowLayoutContainer)->TopItem()->IsHidden(false)) return;

	if (fPulseRunner) fPulseRunner->SetCount(0);

	if (fWindowFeel == B_MODAL_APP_WINDOW_FEEL) {
		BMessenger msgrSelf(this);
		app->RemoveModalWindow(msgrSelf);
	}

	if (fMouseGrabCount > 0) {
		if (fWindow) fWindow->UngrabMouse();
		fMouseGrabCount = 0;

		for (int32 i = 0; i < fMouseInterestedViews.CountItems(); i++) {
			BView *view = (BView*)fMouseInterestedViews.ItemAt(i);
			view->fMouseGrabbed = false;
		}
	}

	if (fKeyboardGrabCount > 0) {
		if (fWindow) fWindow->UngrabKeyboard();
		fKeyboardGrabCount = 0;

		for (int32 i = 0; i < fKeyboardInterestedViews.CountItems(); i++) {
			BView *view = (BView*)fKeyboardInterestedViews.ItemAt(i);
			view->fKeyboardGrabbed = false;
		}
	}

	if (fWindow) fWindow->Hide();

	fMinimized = false;
	fBrokeOnExpose = false;

	if (IsWatched(B_MINIMIZED)) {
		BMessage aMsg(B_MINIMIZED);
		aMsg.AddInt64("when", e_real_time_clock_usecs());
		aMsg.AddBool("minimize", false);
		SendNotices(B_MINIMIZED, &aMsg);
	}

	cast_as(fLayout, BWindowLayoutContainer)->TopItem()->Hide();
}


bool
BWindow::IsHidden() const
{
	return cast_as(fLayout, BWindowLayoutContainer)->TopItem()->IsHidden();
}


bool
BWindow::IsMinimized() const
{
	if (IsHidden()) return false;
	if (fMinimized) return true;
	return false;
}


void
BWindow::AddViewChildrenToHandlersList(BWindow *win, BView *child)
{
	if (win == NULL || child == NULL) return;
	for (BView *view = child->ChildAt(0); view != NULL; view = view->NextSibling()) {
		win->AddHandler(view);

		if (view->Looper() != win) {
			ETK_WARNING("[INTERFACE]: %s --- Add child of the view added by \"AddChild()\" failed.", __PRETTY_FUNCTION__);
			continue;
		}

		view->AttachToWindow();
		view->AttachedToWindow();

		AddViewChildrenToHandlersList(win, view);
		view->AllAttached();
	}
}


void
BWindow::RemoveViewChildrenFromHandlersList(BWindow *win, BView *child)
{
	if (win == NULL || child == NULL || child->Looper() != win) return;
	for (BView *view = child->ChildAt(0); view != NULL; view = view->NextSibling()) {
		RemoveViewChildrenFromHandlersList(win, view);
		view->AllDetached();

		view->DetachedFromWindow();

		view->DetachFromWindow();
		win->RemoveHandler(view);
	}
}


void
BWindow::AddChild(BView *child, BView *nextSibling)
{
	if (child == NULL || child->Looper() != NULL || child->Parent() != NULL ||
	        (nextSibling == NULL ? false : (nextSibling->Looper() != this || nextSibling->Parent() != NULL))) {
		ETK_WARNING("[INTERFACE]: %s --- Unable to add child.", __PRETTY_FUNCTION__);
		return;
	}

	AddHandler(child);
	if (child->Looper() != this) {
		ETK_WARNING("[INTERFACE]: %s --- Unable to attach child to window, abort to add child.", __PRETTY_FUNCTION__);
		return;
	}

	BLayoutItem *topItem = cast_as(fLayout, BWindowLayoutContainer)->TopItem();
	if (topItem->AddItem(child->fLayout, nextSibling == NULL ? -1 : topItem->IndexOf(nextSibling->fLayout)) == false) {
		RemoveHandler(child);
		ETK_WARNING("[INTERFACE]: %s --- Unable to add child to layout.", __PRETTY_FUNCTION__);
		return;
	}

	child->AttachToWindow();
	child->AttachedToWindow();

	AddViewChildrenToHandlersList(this, child);
	child->AllAttached();
}


bool
BWindow::RemoveChild(BView *child)
{
	if (child == NULL || child->Looper() != this || child->Parent() != NULL) return false;

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

	RemoveViewChildrenFromHandlersList(this, child);
	child->AllDetached();

	child->DetachedFromWindow();

	child->DetachFromWindow();
	RemoveHandler(child);

	cast_as(fLayout, BWindowLayoutContainer)->TopItem()->RemoveItem(child->fLayout);

	return true;
}


int32
BWindow::CountChildren() const
{
	return cast_as(fLayout, BWindowLayoutContainer)->TopItem()->CountItems();
}


BView*
BWindow::ChildAt(int32 index) const
{
	BLayoutItem *topItem = cast_as(fLayout, BWindowLayoutContainer)->TopItem();
	return(topItem->ItemAt(index) != NULL ? (BView*)topItem->ItemAt(index)->PrivateData() : NULL);
}


void
BWindow::ConvertToScreen(BPoint* pt) const
{
	if (!pt) return;
	*pt += cast_as(fLayout, BWindowLayoutContainer)->Origin();
}


BPoint
BWindow::ConvertToScreen(BPoint pt) const
{
	BPoint pt1 = pt;
	ConvertToScreen(&pt1);
	return pt1;
}


void
BWindow::ConvertFromScreen(BPoint* pt) const
{
	if (!pt) return;
	*pt -= cast_as(fLayout, BWindowLayoutContainer)->Origin();
}


BPoint
BWindow::ConvertFromScreen(BPoint pt) const
{
	BPoint pt1 = pt;
	ConvertFromScreen(&pt1);
	return pt1;
}


void
BWindow::ConvertToScreen(BRect *r) const
{
	if (!r) return;
	BPoint pt = ConvertToScreen(r->LeftTop());
	r->OffsetTo(pt);
}


BRect
BWindow::ConvertToScreen(BRect r) const
{
	BRect rect = r;
	ConvertToScreen(&rect);
	return rect;
}


void
BWindow::ConvertFromScreen(BRect *r) const
{
	if (!r) return;
	BPoint pt = ConvertFromScreen(B_ORIGIN);
	r->OffsetBy(pt);
}


BRect
BWindow::ConvertFromScreen(BRect r) const
{
	BRect rect = r;
	ConvertFromScreen(&rect);
	return rect;
}


void
BWindow::ConvertToScreen(BRegion *region) const
{
	if (!region || region->CountRects() <= 0) return;
	BPoint pt = ConvertToScreen(region->Frame().LeftTop());
	region->OffsetBy(pt - region->Frame().LeftTop());
}


BRegion
BWindow::ConvertToScreen(const BRegion &region) const
{
	BRegion aRegion(region);
	ConvertToScreen(&aRegion);
	return aRegion;
}


void
BWindow::ConvertFromScreen(BRegion *region) const
{
	if (!region || region->CountRects() <= 0) return;
	BPoint pt = ConvertFromScreen(B_ORIGIN);
	region->OffsetBy(pt);
}


BRegion
BWindow::ConvertFromScreen(const BRegion &region) const
{
	BRegion aRegion(region);
	ConvertFromScreen(&aRegion);
	return aRegion;
}


void
BWindow::FrameMoved(BPoint new_position)
{
}


void
BWindow::WorkspacesChanged(uint32 old_ws, uint32 new_ws)
{
}


void
BWindow::WorkspaceActivated(int32 ws, bool state)
{
}


void
BWindow::FrameResized(float new_width, float new_height)
{
}


void
BWindow::Minimize(bool minimize)
{
	if (minimize) {
		if (fMouseGrabCount > 0) {
			if (fWindow) fWindow->UngrabMouse();
			fMouseGrabCount = 0;

			for (int32 i = 0; i < fMouseInterestedViews.CountItems(); i++) {
				BView *view = (BView*)fMouseInterestedViews.ItemAt(i);
				view->fMouseGrabbed = false;
			}
		}

		if (fKeyboardGrabCount > 0) {
			if (fWindow) fWindow->UngrabKeyboard();
			fKeyboardGrabCount = 0;

			for (int32 i = 0; i < fKeyboardInterestedViews.CountItems(); i++) {
				BView *view = (BView*)fKeyboardInterestedViews.ItemAt(i);
				view->fKeyboardGrabbed = false;
			}
		}
	}

	if (fMinimized == minimize) return;

	fMinimized = minimize;

	if (IsHidden() || fWindow == NULL) return;

	if (fMinimized)
		fWindow->Iconify();
	else
		fWindow->Show();
}


BRect
BWindow::Bounds() const
{
	return cast_as(fLayout, BWindowLayoutContainer)->TopItem()->Bounds();
}


BRect
BWindow::Frame() const
{
	BRect rect = cast_as(fLayout, BWindowLayoutContainer)->TopItem()->Frame();
	rect.OffsetTo(cast_as(fLayout, BWindowLayoutContainer)->Origin());
	return rect;
}


void
BWindow::Invalidate(BRect invalRect, bool redraw)
{
	if (IsHidden() || invalRect.IsValid() == false) return;

	if (redraw) fExposeRect |= invalRect;
	else fUpdateRect |= invalRect;

	if (fInUpdate == false) {
		if (fWindow == NULL) {
			// TODO
			UpdateIfNeeded();
		} else {
			PostMessage(_UPDATE_IF_NEEDED_, this);
		}
	}
}


void
BWindow::DisableUpdates()
{
	int64 currentThread = get_current_thread_id();

	if (fUpdateHolderThreadId != 0 && fUpdateHolderThreadId != currentThread)
		ETK_ERROR("[INTERFACE]: %s --- Invalid \"DisableUpdates()\" and \"EnableUpdates()\" call!", __PRETTY_FUNCTION__);

	if (fUpdateHolderThreadId == 0) {
		fUpdateHolderThreadId = currentThread;
		fUpdateHolderCount = 1;
	} else {
		if (B_MAXINT64 - 1 < fUpdateHolderCount)
			ETK_ERROR("[INTERFACE]: %s --- Call \"DisableUpdates()\" more than limited times!", __PRETTY_FUNCTION__);
		fUpdateHolderCount++;
	}
}


void
BWindow::EnableUpdates()
{
	int64 currentThread = get_current_thread_id();

	if (fUpdateHolderThreadId != 0 && fUpdateHolderThreadId != currentThread)
		ETK_ERROR("[INTERFACE]: %s --- Invalid \"DisableUpdates()\" and \"EnableUpdates()\" call!", __PRETTY_FUNCTION__);
	else if (fUpdateHolderThreadId == 0) {
		ETK_WARNING("[INTERFACE]: %s --- Please call \"DisableUpdates()\" before \"EnableUpdates()\"!", __PRETTY_FUNCTION__);
		return;
	}

	fUpdateHolderCount--;
	if (fUpdateHolderCount > 0) return;

	fUpdateHolderCount = 0;
	fUpdateHolderThreadId = 0;

	if (fWindow && fUpdateRect.IsValid() && !_HasResizeMessage(false)) {
		fUpdateRect.Floor();
		fPixmap->CopyTo(fDC, fWindow,
		                (int32)fUpdateRect.left, (int32)fUpdateRect.top,
		                (uint32)fUpdateRect.Width(), (uint32)fUpdateRect.Height(),
		                (int32)fUpdateRect.left, (int32)fUpdateRect.top,
		                (uint32)fUpdateRect.Width(), (uint32)fUpdateRect.Height());
	}

	fUpdateRect = BRect();
}


bool
BWindow::NeedsUpdate() const
{
	return(fExposeRect.IsValid() || fUpdateRect.IsValid());
}


void
BWindow::_UpdateIfNeeded(bigtime_t when)
{
	if (_HasResizeMessage(false) || NeedsUpdate() == false) return;

	fBrokeOnExpose = false;
	BRect r = fExposeRect;
	if (r.IsValid()) {
		bool saveInUpdate = fInUpdate;

		fExposeRect = BRect();

		fInUpdate = true;
		_Expose(r, when);
		if (fBrokeOnExpose) {
			fExposeRect |= r;
			fInUpdate = saveInUpdate;
			fBrokeOnExpose = false;
			if (fInUpdate == false) PostMessage(_UPDATE_IF_NEEDED_, this);
			return;
		} else if (fExposeRect.IsValid()) {
			fUpdateRect |= r;
			fInUpdate = saveInUpdate;
			_UpdateIfNeeded(e_real_time_clock_usecs());
			return;
		}

		r |= fUpdateRect;

		fInUpdate = saveInUpdate;
	} else {
		r = fUpdateRect;
	}

	fUpdateRect = BRect();

	r &= Bounds();

	if (r.IsValid() == false || fWindow == NULL) return;

	r.Floor();
	fPixmap->CopyTo(fDC, fWindow,
	                (int32)r.left, (int32)r.top, (uint32)r.Width(), (uint32)r.Height(),
	                (int32)r.left, (int32)r.top, (uint32)r.Width(), (uint32)r.Height());
}


void
BWindow::UpdateIfNeeded()
{
	_UpdateIfNeeded(e_real_time_clock_usecs());
}


void
BWindow::_Update(BRect rect, bool force_update)
{
	if (rect.IsValid() == false) return;
	fUpdateRect |= rect;
	if (fInUpdate) return;
	if (fUpdateRect.IsValid() == false) return;
	if (fWindow && (force_update || fUpdateHolderThreadId == 0)) {
		fUpdateRect.Floor();
		fPixmap->CopyTo(fDC, fWindow,
		                (int32)fUpdateRect.left, (int32)fUpdateRect.top,
		                (uint32)fUpdateRect.Width(), (uint32)fUpdateRect.Height(),
		                (int32)fUpdateRect.left, (int32)fUpdateRect.top,
		                (uint32)fUpdateRect.Width(), (uint32)fUpdateRect.Height());
	}
	fUpdateRect = BRect();
}


void
BWindow::SetBackgroundColor(rgb_color c)
{
	if (fDC->HighColor() != c) {
		if (fWindow) fWindow->SetBackgroundColor(c);
		fDC->SetHighColor(c);

		fExposeRect = Bounds();
		if (fInUpdate == false) PostMessage(_UPDATE_IF_NEEDED_, this);
	}
}


void
BWindow::SetBackgroundColor(uint8 r, uint8 g, uint8 b, uint8 a)
{
	rgb_color c;
	c.set_to(r, g, b, a);
	SetBackgroundColor(c);
}


rgb_color
BWindow::BackgroundColor() const
{
	return fDC->HighColor();
}


void
BWindow::_Expose(BRect rect, bigtime_t when)
{
	rect &= Bounds();
	if (rect.IsValid() == false) return;

	BRect r = rect.FloorCopy();
	fPixmap->FillRect(fDC, (int32)r.left, (int32)r.top, (uint32)r.Width(), (uint32)r.Height());

	BRegion region(rect);

	for (BView *child = ChildAt(0); child != NULL; child = child->NextSibling()) {
		if (fBrokeOnExpose || _HasResizeMessage(true)) break;
		if (child->fLayout->VisibleRegion()->Intersects(child->ConvertFromParent(rect)) == false) continue;
		child->_Expose(child->ConvertFromParent(region), when);
	}
}


bool
BWindow::InUpdate() const
{
	return fInUpdate;
}


bool
BWindow::_HasResizeMessage(bool setBrokeOnExpose)
{
	bool retVal = false;
	BRect frame = Frame();

	MessageQueue()->Lock();
	BMessage *msg;
	int32 fromIndex = 0;
	while (retVal == false && (msg = MessageQueue()->FindMessage(B_WINDOW_RESIZED, fromIndex, 20)) != NULL) {
		float w, h;
		if (msg->FindFloat("width", &w) == false || msg->FindFloat("height", &h) == false) break;
		fromIndex = MessageQueue()->IndexOfMessage(msg) + 1;
		retVal = frame.Width() != w || frame.Height() != h;
	}
	MessageQueue()->Unlock();

	if (retVal && setBrokeOnExpose) fBrokeOnExpose = true;

	return retVal;
}


void
BWindow::Activate(bool state)
{
	if (!(IsHidden() || fMinimized) || !state) {
		if (!(fWindow == NULL || fWindow->Activate(state) == B_OK)) {
			ETK_DEBUG("[INTERFACE]: %s --- Unable to %s window.", __PRETTY_FUNCTION__, state ? "activate" : "inactivate");
			return;
		}

		fActivatedTimeStamp = e_real_time_clock_usecs();
		fActivated = state;
		if ((state && !(fWindowFlags &B_AVOID_FRONT)) && fWindow) fWindow->Raise();
		WindowActivated(state);

		BMessage aMsg(B_WINDOW_ACTIVATED);
		aMsg.AddInt64("when", e_real_time_clock_usecs());
		for (BView *view = ChildAt(0); view != NULL; view = view->NextSibling()) PostMessage(&aMsg, view);
	}
}


bool
BWindow::IsActivate() const
{
	return fActivated;
}


void
BWindow::WindowActivated(bool state)
{
}


BView*
BWindow::FindView(const char *name) const
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


BView*
BWindow::FindView(BPoint where) const
{
	if (Bounds().Contains(where) == false) return NULL;

	for (BView *child = ChildAt(0); child != NULL; child = child->NextSibling()) {
		if (child->fLayout->VisibleRegion()->Contains(child->fLayout->ConvertFromContainer(where))) return child;
	}

	return NULL;
}


BView*
BWindow::CurrentFocus() const
{
	return fFocus;
}


status_t
BWindow::SetType(window_type type)
{
	window_look look;
	window_feel feel;

	switch (type) {
		case B_MODAL_WINDOW:
			look = B_MODAL_WINDOW_LOOK;
			feel = B_MODAL_APP_WINDOW_FEEL;
			break;

		case B_DOCUMENT_WINDOW:
			look = B_DOCUMENT_WINDOW_LOOK;
			feel = B_NORMAL_WINDOW_FEEL;
			break;

		case B_BORDERED_WINDOW:
			look = B_BORDERED_WINDOW_LOOK;
			feel = B_NORMAL_WINDOW_FEEL;
			break;

		case B_FLOATING_WINDOW:
			look = B_FLOATING_WINDOW_LOOK;
			feel = B_FLOATING_APP_WINDOW_FEEL;
			break;

		default:
			return B_ERROR;
	}

	status_t status;

	window_look saveLook = fWindowLook;
	if ((status = SetLook(look)) != B_OK) return status;
	if ((status = SetFeel(feel)) != B_OK) {
		SetLook(saveLook);
		return status;
	}

	return B_OK;
}


window_type
BWindow::Type() const
{
	if (fWindowLook == B_TITLED_WINDOW_LOOK && fWindowFeel == B_NORMAL_WINDOW_FEEL)
		return B_TITLED_WINDOW;
	else if (fWindowLook == B_MODAL_WINDOW_LOOK && fWindowFeel == B_MODAL_APP_WINDOW_FEEL)
		return B_MODAL_WINDOW;
	else if (fWindowLook == B_DOCUMENT_WINDOW_LOOK && fWindowFeel == B_NORMAL_WINDOW_FEEL)
		return B_DOCUMENT_WINDOW;
	else if (fWindowLook == B_BORDERED_WINDOW_LOOK && fWindowFeel == B_NORMAL_WINDOW_FEEL)
		return B_BORDERED_WINDOW;
	else if (fWindowLook == B_FLOATING_WINDOW_LOOK && fWindowFeel == B_FLOATING_APP_WINDOW_FEEL)
		return B_FLOATING_WINDOW;
	else return B_UNTYPED_WINDOW;
}


status_t
BWindow::SetLook(window_look look)
{
	if (fWindowLook != look) {
		status_t status = fWindow == NULL ?B_OK : fWindow->SetLook(look);
		if (status != B_OK) return status;
		fWindowLook = look;
	}

	return B_OK;
}


window_look
BWindow::Look() const
{
	return fWindowLook;
}


status_t
BWindow::SetFeel(window_feel feel)
{
	if (fWindowFeel != feel) {
		status_t status = fWindow == NULL ?B_OK : fWindow->SetFeel(feel);
		if (status != B_OK) return status;

		window_feel oldFeel = fWindowFeel;
		fWindowFeel = feel;

		if ((oldFeel == B_MODAL_APP_WINDOW_FEEL || feel == B_MODAL_APP_WINDOW_FEEL) && !IsHidden()) {
			BMessenger msgrSelf(this);
			if (oldFeel == B_MODAL_APP_WINDOW_FEEL)
				app->RemoveModalWindow(msgrSelf);
			else
				app->AddModalWindow(msgrSelf);
		}
	}

	return B_OK;
}


window_feel
BWindow::Feel() const
{
	return fWindowFeel;
}


status_t
BWindow::SetFlags(uint32 flags)
{
	if (fWindowFlags != flags) {
		status_t status = fWindow == NULL ?B_OK : fWindow->SetFlags(flags);
		if (status != B_OK) return status;
		fWindowFlags = flags;
		if (flags &B_AVOID_FOCUS) {
			if (fActivated != false) {
				if (fWindow == NULL ? true : (fWindow->Activate(false) == B_OK)) {
					fActivatedTimeStamp = e_real_time_clock_usecs();

					fActivated = false;
					WindowActivated(false);

					BMessage aMsg(B_WINDOW_ACTIVATED);
					aMsg.AddInt64("when", e_real_time_clock_usecs());
					for (BView *view = ChildAt(0); view != NULL; view = view->NextSibling()) PostMessage(&aMsg, view);
				}
			}
		}
	}

	return B_OK;
}


uint32
BWindow::Flags() const
{
	return fWindowFlags;
}


void
BWindow::SetWorkspaces(uint32 workspace)
{
	if (workspace == 0) {
		if (app->fGraphicsEngine->GetCurrentWorkspace(&workspace) != B_OK || workspace == 0) return;
	}

	if (fWindow == NULL) {
		// TODO
		return;
	}

	if (fWindowWorkspaces != workspace || fWindowWorkspaces == 0) {
		if (fWindow->SetWorkspaces(workspace) == B_OK) {
			uint32 oldWorkspace = fWindowWorkspaces;
			fWindowWorkspaces = workspace;
			if (oldWorkspace != 0) WorkspacesChanged(oldWorkspace, workspace);
		}
	}
}


uint32
BWindow::Workspaces() const
{
	return fWindowWorkspaces;
}


void
BWindow::MoveBy(float dx, float dy)
{
	MoveTo(Frame().LeftTop() + BPoint(dx, dy));
}


void
BWindow::ResizeBy(float dx, float dy)
{
	BRect frame = Frame();
	ResizeTo(frame.Width() + dx, frame.Height() + dy);
}


void
BWindow::MoveTo(BPoint where)
{
	if (fWindow == NULL) {
		// TODO
		return;
	}

	if (Frame().LeftTop() != where) {
		BPoint pt = where.FloorCopy();
		if (fWindow->MoveTo((int32)pt.x, (int32)pt.y) != B_OK) return;

		fPositionChangedTimeStamp = e_real_time_clock_usecs();
		cast_as(fLayout, BWindowLayoutContainer)->MoveTo(where);
		FrameMoved(where);

		if (IsWatched(B_WINDOW_MOVED)) {
			BMessage aMsg(B_WINDOW_MOVED);
			aMsg.AddInt64("when", fPositionChangedTimeStamp);
			aMsg.AddPoint("where", where);
			SendNotices(B_WINDOW_MOVED, &aMsg);
		}
	}
}


void
BWindow::MoveToCenter()
{
	BScreen scr(this);
	BRect r = scr.Frame();
	if (!r.IsValid()) return;
	MoveTo(BPoint((r.Width() - Frame().Width()) / 2, (r.Height() - Frame().Height()) / 2));
}


void
BWindow::ResizeTo(float w, float h)
{
	if (fWindow == NULL) {
		// TODO
		return;
	}

	uint32 min_h = B_MAXUINT32, max_h = B_MAXUINT32, min_v = B_MAXUINT32, max_v = B_MAXUINT32;
	fWindow->GetSizeLimits(&min_h, &max_h, &min_v, &max_v);

	if (w < (float)min_h && min_h != B_MAXUINT32) w = (float)min_h;
	else if (w > (float)max_h && max_h != B_MAXUINT32) w = (float)max_h;
	if (h < (float)min_v && min_v != B_MAXUINT32) h = (float)min_v;
	else if (h > (float)max_v && max_v != B_MAXUINT32) h = (float)max_v;

	BRect frame = Frame();
	if (frame.Width() != w || frame.Height() != h) {
		frame.right = frame.left + w;
		frame.bottom = frame.top + h;
		frame.Floor();
		if (fWindow->MoveAndResizeTo((int32)frame.left, (int32)frame.top,
		                             (uint32)max_c(frame.Width(), 0),
		                             (uint32)max_c(frame.Height(), 0)) != B_OK) return;
		fPixmap->ResizeTo((uint32)max_c(frame.Width(), 0), (uint32)max_c(frame.Height(), 0));
		fDC->SetClipping(BRegion(frame.OffsetToCopy(B_ORIGIN)));

		fSizeChangedTimeStamp = e_real_time_clock_usecs();

		fExposeRect = Bounds();
		if (fInUpdate == false) PostMessage(_UPDATE_IF_NEEDED_, this);

		// for disable update
		bool saveInUpdate = fInUpdate;
		fInUpdate = true;
		cast_as(fLayout, BWindowLayoutContainer)->ResizeTo(w, h);
		fInUpdate = saveInUpdate;

		FrameResized(w, h);

		if (IsWatched(B_WINDOW_RESIZED)) {
			BMessage aMsg(B_WINDOW_RESIZED);
			aMsg.AddInt64("when", fSizeChangedTimeStamp);
			aMsg.AddFloat("width", w);
			aMsg.AddFloat("height", h);
			SendNotices(B_WINDOW_RESIZED, &aMsg);
		}
	}
}


void
BWindow::SetSizeLimits(float min_h, float max_h, float min_v, float max_v)
{
	if (fWindow == NULL) {
		// TODO
		return;
	}

	uint32 minH = B_MAXUINT32, maxH = B_MAXUINT32, minV = B_MAXUINT32, maxV = B_MAXUINT32;
	if (min_h >= 0) minH = (uint32)ceil((double)min_h);
	if (max_h >= 0) maxH = (uint32)ceil((double)max_h);
	if (min_v >= 0) minV = (uint32)ceil((double)min_v);
	if (max_v >= 0) maxV = (uint32)ceil((double)max_v);

	if (fWindow->SetSizeLimits(minH, maxH, minV, maxV) == B_OK) {
		if (min_h >= 0 || min_v >= 0) {
			BRect r = Frame();
			if (r.Width() < min_h && min_h >= 0) r.right = r.left + min_h;
			if (r.Height() < min_v && min_v >= 0) r.bottom = r.top + min_v;

			if (r != Frame()) ResizeTo(r.Width(), r.Height());
		}
	}
}


void
BWindow::GetSizeLimits(float *min_h, float *max_h, float *min_v, float *max_v) const
{
	uint32 minH = B_MAXUINT32, maxH = B_MAXUINT32, minV = B_MAXUINT32, maxV = B_MAXUINT32;
	if (fWindow) fWindow->GetSizeLimits(&minH, &maxH, &minV, &maxV);

	if (min_h) *min_h = minH != B_MAXUINT32 ? (float)minH : -1.f;
	if (max_h) *max_h = maxH != B_MAXUINT32 ? (float)maxH : -1.f;
	if (min_v) *min_v = minV != B_MAXUINT32 ? (float)minV : -1.f;
	if (max_v) *max_v = maxV != B_MAXUINT32 ? (float)maxV : -1.f;
}


status_t
BWindow::SendBehind(const BWindow *win)
{
	if (fWindow == NULL) {
		// TODO
		return B_ERROR;
	}

	if (win) {
		if (win->fWindow == NULL) return B_ERROR;
		return fWindow->Lower(win->fWindow);
	}

	return fWindow->Raise();
}


bool
BWindow::_GrabMouse()
{
	if (fWindow == NULL) {
		// TODO
		return false;
	}

	if (fMouseGrabCount == 0) {
		if (fWindow->GrabMouse() != B_OK) {
			ETK_DEBUG("[INTERFACE]: Mouse grab failed.");
			return false;
		}
	}

	if (fMouseGrabCount <B_MAXUINT32) {
		fMouseGrabCount++;
//		ETK_DEBUG("[INTERFACE]: Mouse grabbed (%u).", fMouseGrabCount);
		return true;
	}

	return false;
}


bool
BWindow::_GrabKeyboard()
{
	if (fWindow == NULL) {
		// TODO
		return false;
	}

	if (fKeyboardGrabCount == 0) {
		if (fWindow->GrabKeyboard() != B_OK) {
			ETK_DEBUG("[INTERFACE]: Keyboard grab failed.");
			return false;
		}
	}

	if (fKeyboardGrabCount <B_MAXUINT32) {
		fKeyboardGrabCount++;
//		ETK_DEBUG("[INTERFACE]: Keyboard grabbed (%u).", fKeyboardGrabCount);
		return true;
	}

	return false;
}


void
BWindow::_UngrabMouse()
{
	if (fWindow == NULL) {
		// TODO
		return;
	}

	if (fMouseGrabCount == 0) return;
	fMouseGrabCount--;
	if (fMouseGrabCount == 0) fWindow->UngrabMouse();
//	ETK_DEBUG("[INTERFACE]: Mouse ungrabbed (%u).", fMouseGrabCount);
}


void
BWindow::_UngrabKeyboard()
{
	if (fWindow == NULL) {
		// TODO
		return;
	}

	if (fKeyboardGrabCount == 0) return;
	fKeyboardGrabCount--;
	if (fKeyboardGrabCount == 0) fWindow->UngrabKeyboard();
//	ETK_DEBUG("[INTERFACE]: Keyboard ungrabbed (%u).", fKeyboardGrabCount);
}


bool
BWindow::GrabMouse()
{
	return _GrabMouse();
}


bool
BWindow::GrabKeyboard()
{
	return _GrabKeyboard();
}


void
BWindow::UngrabMouse()
{
	_UngrabMouse();
}


void
BWindow::UngrabKeyboard()
{
	_UngrabKeyboard();
}


bool
BWindow::IsMouseGrabbed() const
{
	return(fMouseGrabCount > 0);
}


bool
BWindow::IsKeyboardGrabbed() const
{
	return(fKeyboardGrabCount > 0);
}


void
BWindow::SetPulseRate(bigtime_t rate)
{
	if (fPulseRunner->SetInterval(rate) == B_OK) {
		fPulseRate = rate;
		fPulseRunner->SetCount((rate > 0 && fNeededToPulseViews.CountItems() > 0 && !IsHidden()) ? -1 : 0);
	} else {
		ETK_DEBUG("[INTERFACE]: %s --- Unable to set pulse rate.", __PRETTY_FUNCTION__);
	}
}


bigtime_t
BWindow::PulseRate() const
{
	return fPulseRate;
}


const char*
BWindow::Title() const
{
	return fWindowTitle;
}


void
BWindow::SetTitle(const char *title)
{
	BString str(title);
	if (str != fWindowTitle) {
		if (fWindowTitle) delete[] fWindowTitle;
		fWindowTitle = EStrdup(str.String());
		if (fWindow) fWindow->SetTitle(fWindowTitle);
	}
}

