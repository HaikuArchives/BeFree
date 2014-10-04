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
 * File: Alert.cpp
 * Description: BAlert --- Display a modal window that notifies something
 *
 * --------------------------------------------------------------------------*/

#include <support/ClassInfo.h>
#include <kernel/Kernel.h>
#include <app/Invoker.h>
#include <app/Application.h>
#include <add-ons/graphics/GraphicsEngine.h>
#include <render/Pixmap.h>

#include "Button.h"
#include "TextView.h"
#include "Alert.h"
#include "Bitmap.h"

#define ICON_WIDTH	36
#define ICON_HEIGHT	36

#include "icons/info.xpm"
#include "icons/idea.xpm"
#include "icons/warning.xpm"
#include "icons/stop.xpm"


class BAlertTypeView : public BView
{
	public:
		BAlertTypeView(BRect frame, e_alert_type type);
		virtual ~BAlertTypeView();

		virtual void SetViewColor(rgb_color c);
		virtual void Draw(BRect updateRect);

		virtual void GetPreferredSize(float *width, float *height);
		void InitBitmap();

		e_alert_type fAlertType;
		BBitmap *fBitmap;
		uint8 fState;
		BInvoker *fInvoker;
		void *fSem;
};


class BAlertButton : public BButton
{
	public:
		BAlertButton(BRect frame, const char *name, const char *label, BMessage *message,
		             uint32 resizeMode, uint32 flags, uint8 index);
		virtual status_t Invoke(const BMessage *msg);

	private:
		uint8 fIndex;
};


BAlertTypeView::BAlertTypeView(BRect frame, e_alert_type type)
		: BView(frame, "alert_type_view", B_FOLLOW_LEFT |B_FOLLOW_TOP_BOTTOM, B_WILL_DRAW),
		fAlertType(type), fBitmap(NULL), fState(0), fInvoker(NULL), fSem(NULL)
{
	InitBitmap();
}


BAlertTypeView::~BAlertTypeView()
{
	if (fBitmap) delete fBitmap;
}


void
BAlertTypeView::InitBitmap()
{
	const char **xpm_data = NULL;

	switch (fAlertType) {
		case B_INFO_ALERT:
			xpm_data = (const char**)info_xpm;
			break;
		case B_IDEA_ALERT:
			xpm_data = (const char**)idea_xpm;
			break;
		case B_WARNING_ALERT:
			xpm_data = (const char**)warning_xpm;
			break;
		case B_STOP_ALERT:
			xpm_data = (const char**)stop_xpm;
			break;
		default:
			break;
	}

	if (xpm_data != NULL) {
#if defined(ETK_BIG_ENDIAN)
		BPixmap *pix = new BPixmap(ICON_WIDTH, ICON_HEIGHT, B_RGB24_BIG);
#else
		BPixmap *pix = new BPixmap(ICON_WIDTH, ICON_HEIGHT, B_RGB24);
#endif
		pix->SetDrawingMode(B_OP_COPY);
		pix->SetHighColor(ViewColor());
		pix->FillRect(0, 0, ICON_WIDTH, ICON_HEIGHT);
		pix->SetHighColor(200, 200, 200);
		pix->FillRect(0, 0, ICON_WIDTH / 2, ICON_HEIGHT);
		pix->DrawXPM(xpm_data, 0, 0, 0, 0);

		if (fBitmap != NULL) delete fBitmap;
		fBitmap = new BBitmap(pix);
		delete pix;
	}
}


void
BAlertTypeView::SetViewColor(rgb_color c)
{
	if (ViewColor() != c) {
		BView::SetViewColor(c);
		InitBitmap();
		Invalidate();
	}
}


void
BAlertTypeView::Draw(BRect updateRect)
{
	BRect rect = Bounds();
	rect.right = rect.left + rect.Width() / 2;
	SetHighColor(200, 200, 200);
	FillRect(rect);

	if (fBitmap) DrawBitmap(fBitmap, Bounds().Center() - BPoint(ICON_WIDTH / 2.f - 1.f, Bounds().Height() / 2.f - 10.f));
}


void
BAlertTypeView::GetPreferredSize(float *width, float *height)
{
	if (width) *width = 100;
	if (height) *height = 150;
}


BAlertButton::BAlertButton(BRect frame, const char *name, const char *label, BMessage *message,
                           uint32 resizeMode, uint32 flags, uint8 index)
		: BButton(frame, name, label, message, resizeMode, flags)
{
	fIndex = index;
}


status_t
BAlertButton::Invoke(const BMessage *msg)
{
	BAlert *alert = cast_as(Window(), BAlert);
	BAlertTypeView *alert_view = cast_as(Window()->FindView("alert_type_view"), BAlertTypeView);

	if (alert == NULL || alert_view == NULL) {
		return BButton::Invoke(msg);
	}

	if (fIndex > 7) return B_ERROR;

	if (alert_view->fState & 0x80) { // async
		if (!(alert_view->fInvoker == NULL || alert_view->fInvoker->Message() == NULL)) {
			BMessage aMsg = *(alert_view->fInvoker->Message());
			aMsg.AddInt32("which", (int32)fIndex);
			alert_view->fInvoker->Invoke(&aMsg);
		}
	} else {
		alert_view->fState = (0x01 << fIndex);
		if (alert_view->fSem) release_sem_etc(alert_view->fSem, (int64)(fIndex + 1), 0);
	}

	alert->PostMessage(B_QUIT_REQUESTED);

	return B_OK;
}


BAlert::BAlert(const char *title,
               const char *text,
               const char *button1_label,
               const char *button2_label,
               const char *button3_label,
               e_button_width btnWidth,
               e_alert_type type)
		: BWindow(BRect(-100, -100, -10, -10), title, B_MODAL_WINDOW, 0)
{
	BRect tmpR(0, 0, 1, 1);

	BView *alert_view = new BAlertTypeView(tmpR, type);
	BView *info_view = new BView(tmpR, NULL, B_FOLLOW_ALL, 0);
	BView *btns_view = new BView(tmpR, NULL, B_FOLLOW_LEFT_RIGHT |B_FOLLOW_BOTTOM, 0);

	fButtons[0] = new BAlertButton(tmpR, NULL, button1_label ? button1_label : "OK", NULL, B_FOLLOW_RIGHT, B_WILL_DRAW, 0);

	if (button2_label) fButtons[1] = new BAlertButton(tmpR, NULL, button2_label, NULL, B_FOLLOW_RIGHT, B_WILL_DRAW, 1);
	else fButtons[1] = NULL;

	if (button3_label) fButtons[2] = new BAlertButton(tmpR, NULL, button3_label, NULL, B_FOLLOW_RIGHT, B_WILL_DRAW, 2);
	else fButtons[2] = NULL;

	float max_w = 0, max_h = 20, all_w = 0;

	for (int8 i = 0; i < 3; i++) {
		if (fButtons[i] == NULL) continue;

		fButtons[i]->ResizeToPreferred();
		if (fButtons[i]->Frame().Width() > max_w) max_w = fButtons[i]->Frame().Width();
		if (fButtons[i]->Frame().Height() > max_h) max_h = fButtons[i]->Frame().Height();
		all_w += fButtons[i]->Frame().Width() + 10;
	}

	btns_view->ResizeTo(max_c((btnWidth == B_WIDTH_AS_USUAL ? (3 * max_w + 20) : all_w), 200), max_h);
	BRect btnR = btns_view->Bounds();

	for (int8 i = 2; i >= 0; i--) {
		if (fButtons[i] == NULL) continue;
		btnR.left = btnR.right - (btnWidth == B_WIDTH_AS_USUAL ? max_w : fButtons[i]->Frame().Width());
		fButtons[i]->ResizeTo(btnR.Width(), btnR.Height());
		fButtons[i]->MoveTo(btnR.LeftTop());
		btns_view->AddChild(fButtons[i]);

		btnR.right -= btnR.Width() + 10;
	}

	fTextView = new BTextView(tmpR, NULL, tmpR, B_FOLLOW_NONE);
	fTextView->SetText(text);
	fTextView->MakeEditable(false);
	fTextView->MakeSelectable(false);
	fTextView->ResizeToPreferred();
	fTextView->MoveTo(5, 5);
	btns_view->MoveTo(fTextView->Frame().LeftBottom() + BPoint(0, 10));

	alert_view->ResizeToPreferred();

	info_view->ResizeTo(max_c(fTextView->Frame().Width(), btns_view->Frame().Width()) + 10,
	                    fTextView->Frame().Height() + btns_view->Frame().Height() + 20);
	info_view->MoveTo(alert_view->Frame().Width() + 1, 0);
	info_view->AddChild(fTextView);
	info_view->AddChild(btns_view);
	btns_view->ResizeBy(info_view->Frame().Width() - (btns_view->Frame().Width() + 10), 0);

	alert_view->ResizeBy(0, info_view->Frame().Height() - alert_view->Frame().Height());
	alert_view->MoveTo(0, 0);

	ResizeTo(alert_view->Frame().Width() + info_view->Frame().Width(), info_view->Frame().Height());
	AddChild(alert_view);
	AddChild(info_view);

	fTextView->SetTextBackground(btns_view->ViewColor());

	MoveToCenter();
}


BAlert::~BAlert()
{
}


bool
BAlert::QuitRequested()
{
	return true;
}


int32
BAlert::Go(bool could_proxy)
{
	if (IsRunning() || Proxy() != this || IsLockedByCurrentThread()) {
		ETK_WARNING("[INTERFACE]: %s --- IsRunning() || Proxy() != this || IsLockedByCurrentThread()", __PRETTY_FUNCTION__);
		return -1;
	}

	Lock();

	BAlertTypeView *alert_view = cast_as(FindView("alert_type_view"), BAlertTypeView);
	alert_view->fState = 0x40;
	alert_view->fInvoker = NULL;

	if (could_proxy) {
		BLooper *looper = BLooper::LooperForThread(get_current_thread_id());
		if (looper) {
			looper->Lock();
			ProxyBy(looper);
			looper->Unlock();
		}
	}

	Show();
	SendBehind(NULL);
	Activate();

	int32 retVal = -1;

	if (Proxy() != this) {
		while (true) {
			BMessage *aMsg = NextLooperMessage(B_INFINITE_TIMEOUT);
			DispatchLooperMessage(aMsg);
			if (aMsg == NULL) break;
		}

		if (!(alert_view->fState & 0x40)) {
			if (alert_view->fState & 0x01) retVal = 0;
			else if (alert_view->fState & 0x02) retVal = 1;
			else if (alert_view->fState & 0x04) retVal = 2;
		}

		Quit();
	} else {
		void *trackingSem = create_sem(0, NULL);
		alert_view->fSem = trackingSem;

		Unlock();

		int64 count = 0;
		if (!(acquire_sem(trackingSem) != B_OK ||
		        get_sem_count(trackingSem, &count) != B_OK ||
		        count < 0 || count > 2)) {
			retVal = (int32)count;
		}
		delete_sem(trackingSem);
	}

	return retVal;
}


status_t
BAlert::Go(BInvoker *invoker)
{
	if (IsRunning() || Proxy() != this || IsLockedByCurrentThread()) {
		ETK_WARNING("[INTERFACE]: %s --- IsRunning() || Proxy() != this || IsLockedByCurrentThread()", __PRETTY_FUNCTION__);
		return B_ERROR;
	}

	Lock();

	BAlertTypeView *alert_view = cast_as(FindView("alert_type_view"), BAlertTypeView);
	alert_view->fState = 0x80;
	alert_view->fInvoker = invoker;

	Show();
	SendBehind(NULL);

	Unlock();

	return B_OK;
}


BButton*
BAlert::ButtonAt(int32 index) const
{
	if (index < 0 || index > 2) return NULL;
	return fButtons[index];
}


BTextView*
BAlert::TextView() const
{
	return fTextView;
}

