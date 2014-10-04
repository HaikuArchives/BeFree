/* --------------------------------------------------------------------------
 *
 * DirectFB Graphics Add-on for ETK++
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
 * File: etk-wm.cpp
 *
 * --------------------------------------------------------------------------*/

#include "etk-dfb.h"

#define DFB_WINDOW_SMALL_TITLEBAR_HEIGHT	16
#define DFB_WINDOW_NORMAL_TITLEBAR_HEIGHT	20
#define DFB_WINDOW_THIN_BORDER_WIDTH		2
#define DFB_WINDOW_NORMAL_BORDER_WIDTH		3
#define DFB_WINDOW_THICK_BORDER_WIDTH		5
#define DFB_WINDOW_RESIZE_BOX_HEIGHT		6

#define WINDOW_ACTIVED_COLOR		255, 203, 0, 128
#define WINDOW_ACTIVED_BORDER_COLOR	156, 154, 156, 255
#define WINDOW_ACTIVED_SHINER_COLOR	32, 32, 32, 0
#define WINDOW_ACTIVED_MIDDLB_COLOR	126, 126, 126, 255
#define WINDOW_ACTIVED_DARKER_COLOR	63, 73, 90, 0
#define WINDOW_TITLB_TEXT_COLOR		0, 0, 0, 255


void
BDFBGraphicsWindow::AdjustFrameByDecoration()
{
	fWidth -= (uint32)(fMargins.left + fMargins.right);
	fHeight -= (uint32)(fMargins.top + fMargins.bottom);
	fOriginX += (int32)fMargins.left;
	fOriginY += (int32)fMargins.top;

	switch (fLook) {
		case B_BORDERED_WINDOW_LOOK:
			fMargins = BRect(1, 1, 1, 1);
			break;

		case B_TITLED_WINDOW_LOOK:
		case B_DOCUMENT_WINDOW_LOOK:
			fMargins = BRect(DFB_WINDOW_NORMAL_BORDER_WIDTH, DFB_WINDOW_NORMAL_TITLEBAR_HEIGHT,
			                 DFB_WINDOW_NORMAL_BORDER_WIDTH, DFB_WINDOW_NORMAL_BORDER_WIDTH);
			break;

		case B_MODAL_WINDOW_LOOK:
			fMargins = BRect(DFB_WINDOW_THICK_BORDER_WIDTH, DFB_WINDOW_THICK_BORDER_WIDTH,
			                 DFB_WINDOW_THICK_BORDER_WIDTH, DFB_WINDOW_THICK_BORDER_WIDTH);
			break;

		case B_FLOATING_WINDOW_LOOK:
			fMargins = BRect(DFB_WINDOW_THIN_BORDER_WIDTH, DFB_WINDOW_SMALL_TITLEBAR_HEIGHT,
			                 DFB_WINDOW_THIN_BORDER_WIDTH, DFB_WINDOW_THIN_BORDER_WIDTH);
			break;

		default:
			fMargins = BRect(0, 0, 0, 0);
			break;
	}

	fWidth += (uint32)fMargins.left + (uint32)fMargins.right;
	fHeight += (uint32)fMargins.top + (uint32)fMargins.bottom;
	fOriginX -= (int32)fMargins.left;
	fOriginY -= (int32)fMargins.top;
}


void
BDFBGraphicsWindow::RenderDecoration()
{
	if (dfbSurface == NULL) return;

	dfbSurface->SetDrawingFlags(dfbSurface, DSDRAW_NOFX);

	switch (fLook) {
		case B_BORDERED_WINDOW_LOOK: {
			dfbSurface->SetColor(dfbSurface, 0, 0, 0, 255);
			dfbSurface->DrawRectangle(dfbSurface, 0, 0, fWidth, fHeight);
		}
		break;

		case B_FLOATING_WINDOW_LOOK:
		case B_TITLED_WINDOW_LOOK:
		case B_DOCUMENT_WINDOW_LOOK: {
			int titlebar_height = (fLook == B_FLOATING_WINDOW_LOOK ?
			                       DFB_WINDOW_SMALL_TITLEBAR_HEIGHT : DFB_WINDOW_NORMAL_TITLEBAR_HEIGHT);
			int btn_space = 3;
			int btn_h = titlebar_height - 5;
			int btn_w = btn_h;

			// draw border
			dfbSurface->SetColor(dfbSurface, WINDOW_ACTIVED_BORDER_COLOR);
			dfbSurface->DrawRectangle(dfbSurface, 0, 0, fWidth, fHeight);
			dfbSurface->SetColor(dfbSurface, WINDOW_ACTIVED_COLOR);
			dfbSurface->DrawRectangle(dfbSurface, 1, 1, fWidth - 2, fHeight - 2);
			if (fLook != B_FLOATING_WINDOW_LOOK) {
				dfbSurface->SetColor(dfbSurface, WINDOW_ACTIVED_SHINER_COLOR);
				dfbSurface->DrawLine(dfbSurface, 2, titlebar_height, 2, fHeight - 4);
				dfbSurface->SetColor(dfbSurface, WINDOW_ACTIVED_DARKER_COLOR);
				dfbSurface->DrawLine(dfbSurface, 2, fHeight - 3, fWidth - 3, fHeight - 3);
				dfbSurface->DrawLine(dfbSurface, fWidth - 3, titlebar_height, fWidth - 3, fHeight - 4);
			}
			dfbSurface->SetColor(dfbSurface, 0, 0, 0, 0);
			dfbSurface->FillRectangle(dfbSurface, 0, fHeight - 2, 2, 2);
			dfbSurface->FillRectangle(dfbSurface, fWidth - 2, fHeight - 2, 2, 2);
			dfbSurface->SetColor(dfbSurface, WINDOW_ACTIVED_BORDER_COLOR);
			dfbSurface->FillRectangle(dfbSurface, 1, fHeight - 2, 1, 1);
			dfbSurface->FillRectangle(dfbSurface, fWidth - 2, fHeight - 2, 1, 1);

			// draw titlebar
			dfbSurface->SetColor(dfbSurface, WINDOW_ACTIVED_COLOR);
			dfbSurface->FillRectangle(dfbSurface, 1, 1, fWidth - 2, titlebar_height - 1);

			dfbSurface->SetColor(dfbSurface, WINDOW_ACTIVED_SHINER_COLOR);
			dfbSurface->DrawLine(dfbSurface, 2, titlebar_height - 1, fWidth - 4, titlebar_height - 1);

			dfbSurface->SetColor(dfbSurface, WINDOW_ACTIVED_BORDER_COLOR);
			for (int32 i = 0; i < 8; i += 2) dfbSurface->DrawLine(dfbSurface, 2, 3 + i, 10 - i, 3 + i);

//				RenderTitle();

			dfbSurface->SetColor(dfbSurface, 0, 0, 0, 0);
			dfbSurface->FillRectangle(dfbSurface, 0, 0, 2, 2);
			dfbSurface->FillRectangle(dfbSurface, fWidth - 2, 0, 2, 2);
			dfbSurface->SetColor(dfbSurface, WINDOW_ACTIVED_BORDER_COLOR);
			dfbSurface->FillRectangle(dfbSurface, 1, 1, 1, 1);
			dfbSurface->FillRectangle(dfbSurface, fWidth - 2, 1, 1, 1);

			int btn_x = fWidth - 1 - 4 - btn_w;
			int btn_y = 3;

			// draw close button
			if (!(fFlags & B_NOT_CLOSABLE)) {
				dfbSurface->SetColor(dfbSurface, WINDOW_ACTIVED_SHINER_COLOR);
				dfbSurface->DrawRectangle(dfbSurface, btn_x, btn_y, btn_w, btn_h);
				// TODO: gradient
				dfbSurface->SetColor(dfbSurface, WINDOW_ACTIVED_COLOR);
				dfbSurface->FillRectangle(dfbSurface, btn_x, btn_y, 1, 1);
				dfbSurface->FillRectangle(dfbSurface, btn_x, btn_y + btn_h - 1, 1, 1);
				dfbSurface->FillRectangle(dfbSurface, btn_x + btn_w - 1, btn_y, 1, 1);
				dfbSurface->FillRectangle(dfbSurface, btn_x + btn_w - 1, btn_y + btn_h - 1, 1, 1);
				btn_x -= btn_w + btn_space;
			}

#if 0
			// draw zoom button
			if (!(fFlags & B_NOT_ZOOMABLE)) {
				dfbSurface->SetColor(dfbSurface, WINDOW_ACTIVED_SHINER_COLOR);
				dfbSurface->DrawRectangle(dfbSurface, btn_x, btn_y, btn_w, btn_h);
				dfbSurface->SetColor(dfbSurface, WINDOW_ACTIVED_COLOR);
				dfbSurface->FillRectangle(dfbSurface, btn_x, btn_y, 1, 1);
				dfbSurface->FillRectangle(dfbSurface, btn_x, btn_y + btn_h - 1, 1, 1);
				dfbSurface->FillRectangle(dfbSurface, btn_x + btn_w - 1, btn_y, 1, 1);
				dfbSurface->FillRectangle(dfbSurface, btn_x + btn_w - 1, btn_y + btn_h - 1, 1, 1);

				dfbSurface->SetColor(dfbSurface, WINDOW_ACTIVED_BORDER_COLOR);
				dfbSurface->DrawRectangle(dfbSurface, btn_x + 1, btn_y + 1, btn_w - 2, btn_h - 2);
				dfbSurface->FillRectangle(dfbSurface, btn_x + 3, btn_y + btn_h - 5, 2, 2);
				dfbSurface->DrawLine(dfbSurface, btn_x + 3, btn_y + btn_h - 7, btn_x + 3, btn_y + btn_h - 6);
				dfbSurface->DrawLine(dfbSurface, btn_x + 5, btn_y + btn_h - 4, btn_x + 6, btn_y + btn_h - 4);
				dfbSurface->FillRectangle(dfbSurface, btn_x + btn_w - 5, btn_y + 3, 2, 2);
				dfbSurface->DrawLine(dfbSurface, btn_x + btn_w - 7, btn_y + 3, btn_x + btn_w - 6, btn_y + 3);
				dfbSurface->DrawLine(dfbSurface, btn_x + btn_w - 4, btn_y + 5, btn_x + btn_w - 4, btn_y + 6);

				btn_x -= btn_w + btn_space;
			}

			// draw minimize button
			if (!(fFlags & B_NOT_MINIMIZABLE)) {
				dfbSurface->SetColor(dfbSurface, WINDOW_ACTIVED_SHINER_COLOR);
				dfbSurface->DrawRectangle(dfbSurface, btn_x, btn_y, btn_w, btn_h);
				dfbSurface->SetColor(dfbSurface, WINDOW_ACTIVED_COLOR);
				dfbSurface->FillRectangle(dfbSurface, btn_x, btn_y, 1, 1);
				dfbSurface->FillRectangle(dfbSurface, btn_x, btn_y + btn_h - 1, 1, 1);
				dfbSurface->FillRectangle(dfbSurface, btn_x + btn_w - 1, btn_y, 1, 1);
				dfbSurface->FillRectangle(dfbSurface, btn_x + btn_w - 1, btn_y + btn_h - 1, 1, 1);

				dfbSurface->SetColor(dfbSurface, WINDOW_ACTIVED_BORDER_COLOR);
				dfbSurface->DrawRectangle(dfbSurface, btn_x + 1, btn_y + 1, btn_w - 2, btn_h - 2);
				dfbSurface->DrawLine(dfbSurface, btn_x + 3, btn_y + btn_h / 2, btn_x + btn_w - 4, btn_y + btn_h / 2);

				btn_x -= btn_w + btn_space;
			}
#endif
		}
		break;

		case B_MODAL_WINDOW_LOOK: {
			dfbSurface->SetColor(dfbSurface, WINDOW_ACTIVED_BORDER_COLOR);
			dfbSurface->DrawRectangle(dfbSurface, 0, 0, fWidth, fHeight);
			dfbSurface->SetColor(dfbSurface, WINDOW_ACTIVED_MIDDLB_COLOR);
			dfbSurface->DrawRectangle(dfbSurface, 1, 1, fWidth - 2, fHeight - 2);
			dfbSurface->SetColor(dfbSurface, WINDOW_ACTIVED_COLOR);
			dfbSurface->DrawRectangle(dfbSurface, 2, 2, fWidth - 4, fHeight - 4);
			dfbSurface->SetColor(dfbSurface, WINDOW_ACTIVED_SHINER_COLOR);
			dfbSurface->DrawRectangle(dfbSurface, 3, 3, fWidth - 6, fHeight - 6);
			dfbSurface->SetColor(dfbSurface, WINDOW_ACTIVED_DARKER_COLOR);
			dfbSurface->DrawRectangle(dfbSurface, 4, 4, fWidth - 8, fHeight - 8);

			dfbSurface->SetColor(dfbSurface, 0, 0, 0, 0);
			dfbSurface->FillRectangle(dfbSurface, 0, 0, 1, 1);
			dfbSurface->FillRectangle(dfbSurface, fWidth - 1, 0, 1, 1);
			dfbSurface->FillRectangle(dfbSurface, 0, fHeight - 1, 1, 1);
			dfbSurface->FillRectangle(dfbSurface, fWidth - 1, fHeight - 1, 1, 1);
		}
		break;

		default:
			break;
	}

	dfbSurface->Flip(dfbSurface, NULL, DSFLIP_WAITFORSYNC);
}


bool
BDFBGraphicsWindow::HandleMouseEvent(DFBWindowEvent *event)
{
	if (dfbWindow == NULL || fHidden) return false;
	if (!(event->type == DWET_BUTTONDOWN || event->type == DWET_BUTTONUP || event->type == DWET_MOTION)) return false;

	int winLeft, winTop;
	int winWidth, winHeight;
	dfbWindow->GetPosition(dfbWindow, &winLeft, &winTop);
	dfbWindow->GetSize(dfbWindow, &winWidth, &winHeight);

	BPoint where((float)(event->cx - winLeft), (float)(event->cy - winTop));
	BRect allBounds(0, 0, (float)fWidth - 1, (float)fHeight - 1);
	BRect winBounds = allBounds;
	winBounds.left += fMargins.left;
	winBounds.top += fMargins.top;
	winBounds.right -= fMargins.right;
	winBounds.bottom -= fMargins.bottom;

	int32 button = 0;
	if (event->type != DWET_MOTION) {
		if (event->button == DIBI_LEFT) button = 1;
		else if (event->button == DIBI_MIDDLE) button = 2;
		else if (event->button == DIBI_RIGHT) button = 3;
	} else {
		DFBInputDeviceButtonMask state = event->buttons;
		if (state & DIBM_LEFT) button += 1;
		if (state & DIBM_MIDDLE) button += 2;
		if (state & DIBM_RIGHT) button += 3;
	}

	if (button == 0 && !(fHandlingMove || fHandlingResize))
		return((allBounds.Contains(where) && !winBounds.Contains(where)) ? true : false);

	switch (fLook) {
		case B_MODAL_WINDOW_LOOK:
		case B_FLOATING_WINDOW_LOOK:
		case B_TITLED_WINDOW_LOOK:
		case B_DOCUMENT_WINDOW_LOOK: {
			if (fEngine->dfbCurPointerGrabbed != B_MAXUINT) break;
			if (!(allBounds.Contains(where) && !winBounds.Contains(where))) break;

			int titlebar_height = (fLook == B_FLOATING_WINDOW_LOOK ?
			                       DFB_WINDOW_SMALL_TITLEBAR_HEIGHT : DFB_WINDOW_NORMAL_TITLEBAR_HEIGHT);

			int btn_h = titlebar_height - 5;
			int button_space = 3;
			int btn_w = btn_h;
			int btn_x = fWidth - 1 - 4 - btn_w;
			int btn_y = 3;

			// close button
			if (!(fFlags & B_NOT_CLOSABLE) && fLook != B_MODAL_WINDOW_LOOK) {
				BRect buttonBounds((float)btn_x, (float)btn_y, (float)(btn_x + btn_w), (float)(btn_y + btn_h));
				if (buttonBounds.Contains(where)) {
					if (button == 1 && event->type == DWET_BUTTONDOWN) {
						fMsgr.SendMessage(B_QUIT_REQUESTED);
						return true;
					}
				}
				btn_x -= btn_w + button_space;
			}

			if (button == 1 && event->type == DWET_BUTTONDOWN) {
				if (dfbWindow->GrabPointer(dfbWindow) != DFB_OK) break;
				fEngine->dfbCurPointerGrabbed = dfbWindowID;
				wmPointerOffsetX = event->cx - winLeft;
				wmPointerOffsetY = event->cy - winTop;
				if (where.x < 10 && where.y < 10) fHandlingResize = true;
				else fHandlingMove = true;
				if (!(fFlags & B_AVOID_FOCUS)) dfbWindow->RaiseToTop(dfbWindow);
				return true;
			}
		}
		break;

		default:
			break;
	}

	if (fEngine->dfbCurPointerGrabbed == dfbWindowID && (fHandlingMove || fHandlingResize)) {
		DFBEvent evt;
		while (fEngine->dfbEventBuffer->PeekEvent(fEngine->dfbEventBuffer, &evt) == DFB_OK) {
			if (evt.clazz != DFEC_WINDOW) break;
			if (evt.window.window_id != dfbWindowID) break;
			if (!(evt.window.type == DWET_MOTION ||
			        evt.window.type == DWET_GOTFOCUS ||
			        evt.window.type == DWET_LOSTFOCUS)) break;
			if (fEngine->dfbEventBuffer->GetEvent(fEngine->dfbEventBuffer, &evt) != DFB_OK) break;
			if (evt.window.type == DWET_MOTION) {
				event->cx = evt.window.cx;
				event->cy = evt.window.cy;
			}
		}

		if (event->type == DWET_BUTTONUP) {
			dfbWindow->UngrabPointer(dfbWindow);
			fEngine->dfbCurPointerGrabbed = B_MAXUINT;
			fHandlingMove = false;
			fHandlingResize = false;
		} else if (event->type == DWET_MOTION &&
		           (wmPointerOffsetX != event->cx - winLeft || wmPointerOffsetY != event->cy - winTop)) {
			int dx = event->cx - winLeft - wmPointerOffsetX;
			int dy = event->cy - winTop - wmPointerOffsetY;

			if (fHandlingMove || fHandlingResize)
				dfbWindow->MoveTo(dfbWindow, winLeft + dx, winTop + dy);

			if (fHandlingResize) {
				int newWidth = max_c(winWidth - dx, (int)fMargins.left + (int)fMargins.right);
				int newHeight = max_c(winHeight - dy, (int)fMargins.top + (int)fMargins.bottom);
				if (newWidth != winWidth || newHeight != winHeight) dfbWindow->Resize(dfbWindow, newWidth, newHeight);
			}
		}

		return true;
	}

	return(fEngine->dfbCurPointerGrabbed == dfbWindowID ? false : !winBounds.Contains(where));
}

