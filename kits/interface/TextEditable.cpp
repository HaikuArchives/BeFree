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
 * File: TextEditable.cpp
 * Description: BTextEditable --- a single-line editable field
 *
 * --------------------------------------------------------------------------*/

#include <support/String.h>
#include <app/Application.h>

#include "Window.h"
#include "TextEditable.h"


BTextEditable::BTextEditable(BRect frame,
                             const char *name,
                             const char *initial_text,
                             BMessage *message,
                             uint32 resizeMode,
                             uint32 flags)
		: BControl(frame, name, NULL, message, resizeMode, flags),
		fText(NULL), fEditable(true), fSelectable(true), fAlignment(B_ALIGN_LEFT), fPosition(0), fSelectStart(-1), fSelectEnd(-1),
		fCharWidths(NULL), fCount(0), locationOffset(0), fSelectTracking(-1), fMaxChars(B_MAXINT32), fTypingHidden(0)
{
	fMargins = BRect(0, 0, 0, 0);
	if (initial_text) {
		fText = EStrdup(initial_text);
		if (fText) {
			BFont font;
			GetFont(&font);
			fCharWidths = _CharWidths(font, fText, &fCount);
		}
	}
}


BTextEditable::~BTextEditable()
{
	if (fText) delete[] fText;
	if (fCharWidths) delete[] fCharWidths;
}


void
BTextEditable::MakeEditable(bool editable)
{
	if (fEditable != editable) {
		fEditable = editable;
		if (!fEditable && !fSelectable) {
			fPosition = 0;
			locationOffset = 0;
		}
		Invalidate();
	}
}


bool
BTextEditable::IsEditable() const
{
	return fEditable;
}


void
BTextEditable::MakeSelectable(bool selectable)
{
	if (fSelectable != selectable) {
		fSelectable = selectable;
		fSelectTracking = -1;
		if (!fSelectable && !fEditable) {
			fPosition = 0;
			locationOffset = 0;
		}
		Invalidate();
	}
}


bool
BTextEditable::IsSelectable() const
{
	return fSelectable;
}


void
BTextEditable::SetTextAlignment(e_alignment alignment)
{
	if (fAlignment != alignment) {
		fAlignment = alignment;
		locationOffset = 0;
		Invalidate();
	}
}


e_alignment
BTextEditable::TextAlignment() const
{
	return fAlignment;
}


void
BTextEditable::SetPosition(int32 pos)
{
	if (pos < 0 || pos > fCount) pos = fCount;

	fSelectStart = fSelectEnd = -1;
	fSelectTracking = -1;

	// call virtual function
	Select(fSelectStart, fSelectEnd);

	if (fPosition != pos) {
		fPosition = pos;
		Invalidate();
	}
}


int32
BTextEditable::Position() const
{
	return fPosition;
}


void
BTextEditable::SetText(const char *str)
{
	if (fText) delete[] fText;

	const char *end = e_utf8_at(str, min_c(e_utf8_strlen(str), fMaxChars), NULL);
	fText = (str ? EStrdup(str, end == NULL ? -1 : (end - str)) : NULL);

	if (fCharWidths) delete[] fCharWidths;
	fCount = 0;
	fCharWidths = NULL;
	if (fText) {
		BFont font;
		GetFont(&font);
		fCharWidths = _CharWidths(font, fText, &fCount);
	}

	if (fCount <= 0 || fText == NULL) locationOffset = 0;
	if (fPosition > fCount) fPosition = fCount;

	fSelectStart = fSelectEnd = -1;
	fSelectTracking = -1;

	// call virtual function
	Select(fSelectStart, fSelectEnd);

	Invalidate();
}


void
BTextEditable::SetText(const BString &text)
{
	SetText(text.String());
}


const char*
BTextEditable::Text() const
{
	return fText;
}


char*
BTextEditable::DuplicateText(int32 startPos, int32 endPos)
{
	if (!fText || fCount <= 0 || startPos < 0) return NULL;
	if (endPos < 0 || endPos >= fCount) endPos = fCount - 1;
	if (endPos < startPos) return NULL;

	const char* start = e_utf8_at(fText, startPos, NULL);
	uint8 endLen = 0;
	const char* end = e_utf8_at(fText, endPos, &endLen);

	if (start == NULL || (end == NULL || endLen == 0)) return NULL;

	return b_strndup(start, end - start + (int32)endLen);
}



void
BTextEditable::InsertText(const char *text, int32 nChars, int32 position)
{
	if (text == NULL || *text == 0 || nChars == 0) return;
	if (position < 0) position = fCount;

	int32 length = 0;
	uint8 chLen = 0;
	const char* str = NULL;
	if (!(nChars < 0 || (str = e_utf8_at(text, nChars - 1, &chLen)) == NULL || chLen == 0)) length = (int32)chLen + (str - text);
	else length = (int32)strlen(text);

	if (length <= 0) return;

	if (fText == NULL) {
		BString astr(text, length);
		SetText(astr);
	} else {
		int32 pos = -1;
		if (position < fCount) {
			uint8 len = 0;
			str = e_utf8_at(fText, position, &len);
			if (!(str == NULL || len == 0)) pos = (str - fText);
		}

		BString astr(fText);
		if (pos < 0 || pos >= (int32)strlen(fText))
			astr.Append(text, length);
		else
			astr.Insert(text, length, pos);
		SetText(astr);
	}
}


void
BTextEditable::RemoveText(int32 startPos, int32 endPos)
{
	if (!fText || fCount <= 0 || startPos < 0) return;
	if (endPos < 0 || endPos >= fCount) endPos = fCount - 1;
	if (endPos < startPos) return;

	const char* start = e_utf8_at(fText, startPos, NULL);
	uint8 endLen = 0;
	const char* end = e_utf8_at(fText, endPos, &endLen);

	if (start == NULL || (end == NULL || endLen == 0)) return;

	BString astr(fText);
	astr.Remove(start - fText, end - start + (int32)endLen);
	SetText(astr);
}


void
BTextEditable::Select(int32 startPos, int32 endPos)
{
	if ((startPos == fSelectStart && endPos == fSelectEnd) || fText == NULL || fCount <= 0) return;
	if (endPos < 0 || endPos >= fCount) endPos = fCount - 1;
	if (endPos < startPos) return;

	if (startPos < 0) {
		if (fSelectStart >= 0 || fSelectEnd >= 0) {
			fSelectStart = fSelectEnd = -1;
			Invalidate();
		}
	} else if (fSelectStart != startPos || fSelectEnd != endPos) {
		fSelectStart = startPos;
		fSelectEnd = endPos;
		Invalidate();
	}
}


bool
BTextEditable::GetSelection(int32 *startPos, int32 *endPos) const
{
	if (fSelectStart < 0 || fSelectEnd < 0 || fSelectEnd < fSelectStart || fSelectEnd >= fCount) return false;
	if (!startPos && !endPos) return true;

	if (startPos) *startPos = fSelectStart;
	if (endPos) *endPos = fSelectEnd;

	return true;
}


void
BTextEditable::SetMargins(float left, float top, float right, float bottom)
{
	if (left < 0) left = 0;
	if (top < 0) top = 0;
	if (right < 0) right = 0;
	if (bottom < 0) bottom = 0;

	BRect r(left, top, right, bottom);
	if (r != fMargins) {
		fMargins = r;
		Invalidate();
	}
}


void
BTextEditable::GetMargins(float *left, float *top, float *right, float *bottom) const
{
	if (left) *left = fMargins.left;
	if (top) *top = fMargins.top;
	if (right) *right = fMargins.right;
	if (bottom) *bottom = fMargins.bottom;
}


void
BTextEditable::SetFont(const BFont *font, uint8 mask)
{
	BFont fontPrev;
	BFont fontCurr;
	GetFont(&fontPrev);
	BControl::SetFont(font, mask);
	GetFont(&fontCurr);

	if (fontPrev != fontCurr) {
		if (fCharWidths) delete[] fCharWidths;
		fCount = 0;
		fCharWidths = NULL;
		if (fText) fCharWidths = _CharWidths(fontCurr, fText, &fCount);
		Invalidate();
	}
}


void
BTextEditable::FrameResized(float new_width, float new_height)
{
	locationOffset = 0;
	Invalidate();
}


void
BTextEditable::Draw(BRect updateRect)
{
	if (!IsVisible()) return;

	BRect rect = Frame().OffsetToSelf(B_ORIGIN);
	rect.left += fMargins.left;
	rect.top += fMargins.top;
	rect.right -= fMargins.right;
	rect.bottom -= fMargins.bottom;

	if (!rect.IsValid()) return;

	BRegion clipping;
	GetClippingRegion(&clipping);
	if (clipping.CountRects() > 0) clipping &= (rect & updateRect);
	else clipping = (rect & updateRect);
	if (clipping.CountRects() <= 0) return;

	rgb_color bkColor = ui_color(B_DOCUMENT_BACKGROUND_COLOR);
	rgb_color fgColor = ui_color(B_DOCUMENT_TEXT_COLOR);

	if (!IsEnabled()) {
		bkColor.disable(ViewColor());
		fgColor.disable(ViewColor());
	}

	if (!IsFocusChanging()) {
		PushState();
		ConstrainClippingRegion(&clipping);
		SetDrawingMode(B_OP_COPY);
		SetPenSize(0);
		SetHighColor(bkColor);
		FillRect(rect & updateRect, B_SOLID_HIGH);
		PopState();
	}

	BFont font;
	font_height fontHeight;
	GetFont(&font);
	font.GetHeight(&fontHeight);

	if (fCount > 0 && !IsFocusChanging()) {
		PushState();

		ConstrainClippingRegion(&clipping);

		float x = 0, y = 0;
		if (GetCharLocation(0, &x, &y, &font)) {
			SetDrawingMode(B_OP_COPY);
			SetPenSize(0);
			SetHighColor(fgColor);
			SetLowColor(bkColor);
			_DrawString(fText, BPoint(x, y));

			if (IsEnabled() && IsSelected()) {
				char *selectedText = DuplicateText(fSelectStart, fSelectEnd);
				if (selectedText != NULL) {
					x = 0;
					y = 0;
					if (GetCharLocation(fSelectStart, &x, &y, &font)) {
						DrawSelectedBackground(updateRect);
						SetLowColor(ui_color(B_DOCUMENT_HIGHLIGHT_COLOR));
						_DrawString(selectedText, BPoint(x, y));
					}
					free(selectedText);
				}
			}
		}

		PopState();
	}

	if (IsEnabled() && IsEditable() && (IsFocus() || IsFocusChanging())) {
		PushState();
		ConstrainClippingRegion(&clipping);
		DrawCursor();
		PopState();
	}

	if ((IsFocus() || IsFocusChanging()) && Window()->IsActivate() && IsEnabled() && (Flags() &B_NAVIGABLE)) {
		rgb_color color = ui_color(B_NAVIGATION_BASE_COLOR);
		if (IsFocusChanging() && !IsFocus()) color = ui_color(B_DOCUMENT_BACKGROUND_COLOR);

		PushState();
		ConstrainClippingRegion(&clipping);
		SetDrawingMode(B_OP_COPY);
		SetPenSize(0);
		SetHighColor(color);
		StrokeRect(rect, B_SOLID_HIGH);
		PopState();
	}
}


void
BTextEditable::DrawSelectedBackground(BRect updateRect)
{
	if (fCount <= 0 || !IsEnabled()) return;
	if (fSelectStart < 0 || fSelectEnd < 0 || fSelectEnd < fSelectStart || fSelectEnd >= fCount || fCharWidths == NULL) return;

	BRect rect = Frame().OffsetToSelf(B_ORIGIN);
	rect.left += fMargins.left;
	rect.top += fMargins.top;
	rect.right -= fMargins.right;
	rect.bottom -= fMargins.bottom;

	if (!rect.IsValid()) return;

	BFont font;
	font_height fontHeight;
	GetFont(&font);
	font.GetHeight(&fontHeight);
	float sHeight = fontHeight.ascent + fontHeight.descent;

	BRect hlRect;
	if (!GetCharLocation(0, &(hlRect.left), NULL, &font)) return;
	hlRect.top = rect.Center().y - sHeight / 2.f - 1;
	hlRect.bottom = rect.Center().y + sHeight / 2.f + 1;

	for (int32 i = 0; i < fSelectStart; i++) {
		hlRect.left += (float)ceil((double)fCharWidths[i]);
		hlRect.left += (float)ceil((double)(font.Spacing() * font.Size()));
	}

	hlRect.right = hlRect.left;

	for (int32 i = fSelectStart; i <= fSelectEnd; i++) {
		hlRect.right += (float)ceil((double)fCharWidths[i]);
		if (i != fSelectEnd) hlRect.right += (float)ceil((double)(font.Spacing() * font.Size()));
	}

	hlRect &= updateRect;
	if (!hlRect.IsValid()) return;

	rgb_color hlColor = ui_color(B_DOCUMENT_HIGHLIGHT_COLOR);

	PushState();

	SetDrawingMode(B_OP_COPY);
	SetPenSize(0);
	SetHighColor(hlColor);
	FillRect(hlRect, B_SOLID_HIGH);

	PopState();
}


void
BTextEditable::DrawCursor()
{
	if (!IsEnabled() || !IsEditable() || fPosition < 0 || fPosition > fCount || (fCount > 0 && fCharWidths == NULL)) return;
	if (Window() == NULL || Window()->IsActivate() == false) return;
	if (!(IsFocus() || IsFocusChanging())) return;

	BRect rect = Frame().OffsetToSelf(B_ORIGIN);
	rect.left += fMargins.left;
	rect.top += fMargins.top;
	rect.right -= fMargins.right;
	rect.bottom -= fMargins.bottom;

	if (!rect.IsValid()) return;

	BFont font;
	font_height fontHeight;
	GetFont(&font);
	font.GetHeight(&fontHeight);
	float sHeight = fontHeight.ascent + fontHeight.descent;

	BPoint pt1;
	if (!GetCharLocation(fPosition, &(pt1.x), NULL, &font)) return;
	pt1.x -= 1;
	pt1.y = rect.Center().y - sHeight / 2.f;

	BPoint pt2 = pt1;
	pt2.y += sHeight;

	rgb_color crColor = ui_color(B_DOCUMENT_CURSOR_COLOR);

	if (IsFocusChanging() && !IsFocus()) {
		if (fPosition > fSelectStart && fPosition <= fSelectEnd && fSelectEnd > fSelectStart) {
			crColor = ui_color(B_DOCUMENT_HIGHLIGHT_COLOR);
		} else {
			crColor = ui_color(B_DOCUMENT_BACKGROUND_COLOR);
		}
	}

	PushState();

	SetDrawingMode(B_OP_COPY);
	SetPenSize(0);
	SetHighColor(crColor);
	StrokeLine(pt1, pt2, B_SOLID_HIGH);

	PopState();
}


void
BTextEditable::MouseDown(BPoint where)
{
	BRect rect = Frame().OffsetToSelf(B_ORIGIN);
	rect.left += fMargins.left;
	rect.top += fMargins.top;
	rect.right -= fMargins.right;
	rect.bottom -= fMargins.bottom;

	if (!IsEnabled() || !rect.Contains(where) || !QueryCurrentMouse(true, B_PRIMARY_MOUSE_BUTTON)) return;
	if (!(IsEditable() || IsSelectable())) return;
	if (!IsFocus()) MakeFocus(true);

	BFont font;
	GetFont(&font);

	float x = 0;
	if (!GetCharLocation(0, &x, NULL, &font)) return;

	int32 pos = 0;

	if (where.x > x) {
		for (int32 i = 0; i <= fCount; i++) {
			if (i == fCount) {
				pos = fCount;
			} else {
				x += (float)ceil((double)fCharWidths[i]);
				if (where.x < x) {
					pos = i;
					break;
				}
				x += (float)ceil((double)(font.Spacing() * font.Size()));
				if (where.x < x) {
					pos = i + 1;
					break;
				}
			}
		}
	}

	bool redraw = IsSelected();

	if (IsFocus() && fSelectTracking < 0) {
		if (!(!IsSelectable() || SetPrivateEventMask(B_POINTER_EVENTS, B_LOCK_WINDOW_FOCUS) != B_OK)) {
			fSelectStart = fSelectEnd = -1;
			fSelectTracking = pos;
		} else {
			fSelectStart = fSelectEnd = -1;
			fSelectTracking = -1;
		}

		// call virtual function
		Select(fSelectStart, fSelectEnd);
	}

	if (fPosition != pos) {
		fPosition = pos;
		redraw = true;
	}

	if (redraw) Invalidate();
}


void
BTextEditable::MouseUp(BPoint where)
{
	fSelectTracking = -1;

	BRect rect = Frame().OffsetToSelf(B_ORIGIN);
	rect.left += fMargins.left;
	rect.top += fMargins.top;
	rect.right -= fMargins.right;
	rect.bottom -= fMargins.bottom;

	if (rect.Contains(where)) app->ObscureCursor();
}


void
BTextEditable::MouseMoved(BPoint where, uint32 code, const BMessage *a_message)
{
	BRect rect = Frame().OffsetToSelf(B_ORIGIN);
	rect.left += fMargins.left;
	rect.top += fMargins.top;
	rect.right -= fMargins.right;
	rect.bottom -= fMargins.bottom;

	if (rect.Contains(where) == false || code == B_EXITED_VIEW) {
		app->SetCursor(B_CURSOR_SYSTEM_DEFAULT, false);
		return;
	}

	app->SetCursor(B_CURSOR_I_BEAM, false);

	if (!IsEnabled() || !IsSelectable() || fSelectTracking < 0) return;

	BWindow *win = Window();
	if (!win) return;

	if (!VisibleBounds().Contains(where)) return;
	if (!(IsEditable() || IsSelectable())) return;

	BFont font;
	GetFont(&font);

	float x = 0;
	if (!GetCharLocation(0, &x, NULL, &font)) return;

	int32 pos = 0;

	if (where.x > x) {
		for (int32 i = 0; i <= fCount; i++) {
			if (i == fCount) {
				pos = fCount;
			} else {
				x += (float)ceil((double)fCharWidths[i]);
				if (where.x < x) {
					pos = i;
					break;
				}
				x += (float)ceil((double)(font.Spacing() * font.Size()));
				if (where.x < x) {
					pos = i + 1;
					break;
				}
			}
		}
	}

	bool redraw = false;

	int32 oldStart = fSelectStart;
	int32 oldEnd = fSelectEnd;
	if (pos == fSelectTracking) {
		if (IsSelected()) redraw = true;
		fSelectStart = fSelectEnd = -1;
	} else if (pos > fSelectTracking) {
		fSelectStart = fSelectTracking;
		fSelectEnd = pos - 1;
	} else { // pos < fSelectTracking
		fSelectStart = pos;
		fSelectEnd = fSelectTracking - 1;
	}

	if (oldStart != fSelectStart || oldEnd != fSelectEnd) {
		// call virtual function
		Select(fSelectStart, fSelectEnd);
		redraw = true;
	}

	if (fPosition != pos) {
		fPosition = pos;
		redraw = true;
	}

	if (redraw) Invalidate();
}


void
BTextEditable::WindowActivated(bool state)
{
	fSelectTracking = -1;
	Invalidate();
}


void
BTextEditable::KeyDown(const char *bytes, int32 numBytes)
{
	if (!IsEnabled() || !(IsEditable() || IsSelectable()) || !IsFocus() || numBytes < 1) return;
	if (bytes[0] == B_ENTER) return;

	BWindow *win = Window();
	if (!win) return;

	BMessage *msg = win->CurrentMessage();
	if (!msg || !(msg->what == B_KEY_DOWN || msg->what == B_UNMAPPED_KEY_DOWN)) return;

	int32 modifiers = 0;
	msg->FindInt32("modifiers", &modifiers);
	if ((modifiers &B_CONTROL_KEY) || (modifiers &B_COMMAND_KEY) ||
	        (modifiers &B_MENU_KEY) || (modifiers &B_OPTION_KEY)) return;

	bool shift_only = false;
	if (IsSelectable()) {
		modifiers &= ~(B_CAPS_LOCK |B_SCROLL_LOCK |B_NUM_LOCK |B_LEFT_SHIFT_KEY |B_RIGHT_SHIFT_KEY);
		if (modifiers == B_SHIFT_KEY) shift_only = true;
	}

	if (numBytes == 1) {
		switch (bytes[0]) {
			case B_ESCAPE:
				if (IsSelectable() && (fSelectTracking > 0 || IsSelected())) {
					fSelectTracking = -1;
					fSelectStart = fSelectEnd = -1;

					// call virtual function
					Select(fSelectStart, fSelectEnd);

					Invalidate();
				}
				break;

			case B_UP_ARROW:
			case B_DOWN_ARROW:
				break;

			case B_LEFT_ARROW: {
				bool redraw = false;

				int32 oldStart = fSelectStart;
				int32 oldEnd = fSelectEnd;
				if (IsSelectable() && shift_only) {
					if (fSelectTracking < 0) {
						if (IsSelected() && fSelectStart == fPosition) {
							fSelectTracking = fSelectEnd + 1;
							if (fPosition > 0) fSelectStart = fPosition - 1;
						} else {
							fSelectTracking = fPosition;
							fSelectStart = fSelectEnd = fPosition - 1;
						}
					} else if (fPosition > 0) {
						if (fPosition <= fSelectTracking) {
							fSelectStart = fPosition - 1;
							fSelectEnd = fSelectTracking - 1;
						} else {
							fSelectStart = fSelectTracking;
							fSelectEnd = fPosition - 2;
						}
					}
				} else if (IsSelectable()) {
					fSelectStart = fSelectEnd = -1;
				}

				if (oldStart != fSelectStart || oldEnd != fSelectEnd) {
					// call virtual function
					Select(fSelectStart, fSelectEnd);
					redraw = true;
				}

				if (fPosition > 0) {
					fPosition = fPosition - 1;
					redraw = true;
				}

				if (redraw) Invalidate();
			}
			break;

			case B_RIGHT_ARROW: {
				bool redraw = false;

				int32 oldStart = fSelectStart;
				int32 oldEnd = fSelectEnd;
				if (IsSelectable() && shift_only) {
					if (fSelectTracking < 0) {
						if (IsSelected() && fSelectEnd == fPosition - 1) {
							fSelectTracking = fSelectStart;
							if (fPosition < fCount) fSelectEnd = fPosition;
						} else {
							fSelectTracking = fPosition;
							fSelectStart = fSelectEnd = fPosition;
						}
					} else if (fPosition < fCount) {
						if (fPosition >= fSelectTracking) {
							fSelectStart = fSelectTracking;
							fSelectEnd = fPosition;
						} else {
							fSelectStart = fPosition + 1;
							fSelectEnd = fSelectTracking - 1;
						}
					}
				} else if (IsSelectable()) {
					fSelectStart = fSelectEnd = -1;
				}

				if (oldStart != fSelectStart || oldEnd != fSelectEnd) {
					// call virtual function
					Select(fSelectStart, fSelectEnd);
					redraw = true;
				}

				if (fPosition < fCount) {
					fPosition = fPosition + 1;
					redraw = true;
				}

				if (redraw) Invalidate();
			}
			break;

			case B_DELETE:
				if (IsSelectable() && IsEditable() && IsSelected()) {
					int32 oldPos = fSelectStart;
					RemoveText(fSelectStart, fSelectEnd);
					SetPosition(oldPos);
				} else if (fPosition < fCount && fPosition >= 0 && IsEditable()) {
					RemoveText(fPosition, fPosition);
				}
				break;

			case B_BACKSPACE:
				if (IsSelectable() && IsEditable() && IsSelected()) {
					int32 oldPos = fSelectStart;
					RemoveText(fSelectStart, fSelectEnd);
					SetPosition(oldPos);
				} else if (fPosition > 0 && fPosition <= fCount && IsEditable()) {
					int32 oldCount = fCount;
					int32 oldPos = fPosition;
					RemoveText(fPosition - 1, fPosition - 1);
					if (fCount < oldCount && oldPos == fPosition) SetPosition(oldPos - 1);
				}
				break;

			case B_HOME: {
				bool redraw = false;

				int32 oldStart = fSelectStart;
				int32 oldEnd = fSelectEnd;
				if (IsSelectable() && shift_only) {
					if (fSelectTracking < 0) {
						if (IsSelected() && fSelectStart == fPosition)
							fSelectTracking = fSelectEnd + 1;
						else
							fSelectTracking = fPosition;
					}
					fSelectStart = 0;
					fSelectEnd = fSelectTracking - 1;
				} else if (IsSelectable()) {
					fSelectStart = fSelectEnd = -1;
				}

				if (oldStart != fSelectStart || oldEnd != fSelectEnd) {
					// call virtual function
					Select(fSelectStart, fSelectEnd);
					redraw = true;
				}

				if (fPosition != 0) {
					fPosition = 0;
					redraw = true;
				}

				if (redraw) Invalidate();
			}
			break;

			case B_END: {
				bool redraw = false;

				int32 oldStart = fSelectStart;
				int32 oldEnd = fSelectEnd;
				if (IsSelectable() && shift_only) {
					if (fSelectTracking < 0) {
						if (IsSelected() && fSelectEnd == fPosition - 1)
							fSelectTracking = fSelectStart;
						else
							fSelectTracking = fPosition;
					}
					fSelectStart = fSelectTracking;
					fSelectEnd = fCount - 1;
				} else if (IsSelectable()) {
					fSelectStart = fSelectEnd = -1;
				}

				if (oldStart != fSelectStart || oldEnd != fSelectEnd) {
					// call virtual function
					Select(fSelectStart, fSelectEnd);
					redraw = true;
				}

				if (fPosition != fCount) {
					fPosition = fCount;
					redraw = true;
				}

				if (redraw) Invalidate();
			}
			break;

			default:
				if (bytes[0] >= 0x20 && bytes[0] <= 0x7e && IsEditable()) { // printable
					if (IsSelectable() && IsSelected()) {
						int32 oldPos = fSelectStart;
						RemoveText(fSelectStart, fSelectEnd);
						InsertText(bytes, 1, oldPos);
						SetPosition(oldPos + 1);
					} else {
						int32 oldCount = fCount;
						int32 oldPos = fPosition;
						InsertText(bytes, 1, fPosition);
						if (fCount > oldCount && oldPos == fPosition) SetPosition(oldPos + 1);
					}
				}
				break;
		}
	} else {
		if (IsEditable()) {
			int32 len = e_utf8_strlen(bytes);
			if (len > 0) {
				if (IsSelectable() && IsSelected()) {
					int32 oldPos = fSelectStart;
					RemoveText(fSelectStart, fSelectEnd);
					InsertText(bytes, len, oldPos);
					SetPosition(oldPos + len);
				} else {
					int32 oldCount = fCount;
					int32 oldPos = fPosition;
					InsertText(bytes, len, fPosition);
					if (fCount > oldCount && oldPos == fPosition) SetPosition(oldPos + len);
				}
			}
		}

		// TODO: input method
	}
}


void
BTextEditable::KeyUp(const char *bytes, int32 numBytes)
{
	if (!IsEnabled() || !IsEditable() || !IsFocus() || numBytes != 1 || bytes[0] != B_ENTER) return;

	if (Message() != NULL && fCount >= 0 && fText != NULL) {
		BMessage msg(*Message());
		msg.AddString("etk:texteditable-content", fText);
		Invoke(&msg);
	} else {
		Invoke();
	}
}


void
BTextEditable::MessageReceived(BMessage *msg)
{
	if (msg->what == B_MODIFIERS_CHANGED) {
		int32 modifiers = 0, old_modifiers = 0;
		msg->FindInt32("modifiers", &modifiers);
		msg->FindInt32("etk:old_modifiers", &old_modifiers);
		if ((old_modifiers &B_SHIFT_KEY) && !(modifiers &B_SHIFT_KEY)) fSelectTracking = -1;
	}
	BControl::MessageReceived(msg);
}


void
BTextEditable::GetPreferredSize(float *width, float *height)
{
	if (!width && !height) return;

	BFont font;
	GetFont(&font);

	if (width) {
		*width = fText ? (float)ceil((double)_StringWidth(font, fText)) : 0;
		*width += 6;
	}

	if (height) {
		font_height fontHeight;
		font.GetHeight(&fontHeight);
		*height = fText ? (float)ceil((double)(fontHeight.ascent + fontHeight.descent)) : 0;
		*height += 4;
	}
}


bool
BTextEditable::GetCharLocation(int32 pos, float *x, float *y, BFont *tFont)
{
	if (!x) return false;

	BRect rect = Frame().OffsetToSelf(B_ORIGIN);
	rect.left += fMargins.left;
	rect.top += fMargins.top;
	rect.right -= fMargins.right;
	rect.bottom -= fMargins.bottom;

	rect.InsetBy(2, 2);

	if (!rect.IsValid()) return false;

	BFont font;
	font_height fontHeight;

	if (tFont) font = *tFont;
	else GetFont(&font);
	font.GetHeight(&fontHeight);
	float sHeight = fontHeight.ascent + fontHeight.descent;
	float strWidth = (fCount <= 0 ? 0.f : max_c(0.f, _StringWidth(font, fText)));
	float fontSpacing = (float)ceil((double)font.Spacing() * font.Size());

	if (fAlignment == B_ALIGN_RIGHT) *x = rect.right - strWidth;
	else if (fAlignment == B_ALIGN_CENTER) *x = rect.Center().x - strWidth / 2.f;
	else *x = rect.left; /*B_ALIGN_LEFT */
	if (y) *y = (rect.Center().y - sHeight/ 2.f + fontHeight.ascent + 1);

	if (strWidth <= rect.Width() || !IsEnabled() ||
	        !(IsEditable() || (IsSelectable() && IsSelected())) ||
	        fPosition < 0 || fPosition > fCount) {
		locationOffset = 0;
	} else {
		float xx = *x + locationOffset;

		if (fPosition > 0 && fPosition < fCount) {
			const char *p = e_utf8_at((const char*)fText, fPosition, NULL);
			if (p != NULL) {
				BString str;
				str.Append(fText, (int32)(p - (const char*)fText));
				xx += _StringWidth(font, str.String()) + fontSpacing;
			}
		} else if (fPosition == fCount) {
			xx += strWidth + fontSpacing;
		}

		if (xx < rect.left)
			locationOffset += (rect.left - xx);
		else if (xx > rect.right)
			locationOffset += (rect.right - xx);
	}

	*x += locationOffset;

	if (pos > 0 && pos < fCount) {
		const char *p = e_utf8_at((const char*)fText, pos, NULL);
		if (p != NULL) {
			BString str;
			str.Append(fText, (int32)(p - (const char*)fText));
			*x += _StringWidth(font, str.String()) + fontSpacing;
		}
	} else if (pos < 0 || pos >= fCount) {
		*x += strWidth + fontSpacing;
	}

	return true;
}


void
BTextEditable::MakeFocus(bool focusState)
{
	if (!focusState) fSelectTracking = -1;
	BControl::MakeFocus(focusState);
}


void
BTextEditable::SetMaxChars(int32 max)
{
	if (max < 0) max = B_MAXINT32;
	if (fMaxChars != max) {
		fMaxChars = max;
		if (fCount > fMaxChars) {
			RemoveText(fMaxChars, -1);
			Invalidate();
		}
	}
}


int32
BTextEditable::MaxChars() const
{
	return fMaxChars;
}


void
BTextEditable::HideTyping(uint8 flag)
{
	if ((flag != 0x00 && flag < 0x20) || flag > 0x7e) flag = 0x01;

	if (fTypingHidden != flag) {
		fTypingHidden = flag;

		BString aStr(fText);
		BTextEditable::SetText(aStr.String());

		Invalidate();
	}
}


uint8
BTextEditable::IsTypingHidden() const
{
	return fTypingHidden;
}


float
BTextEditable::_StringWidth(const BFont &font, const char *str) const
{
	if (fTypingHidden == 0x01 || str == NULL || *str == 0) return 0;
	if (fTypingHidden == 0x00) return font.StringWidth(str);

	BString aStr;
	aStr.Append(*((char*)&fTypingHidden), e_utf8_strlen(str));
	return font.StringWidth(aStr);
}


float*
BTextEditable::_CharWidths(const BFont &font, const char *str, int32 *count) const
{
	if (fTypingHidden == 0x01 || str == NULL || *str == 0) return NULL;
	if (fTypingHidden == 0x00) return font.CharWidths(str, count);

	BString aStr;
	aStr.Append(*((char*)&fTypingHidden), e_utf8_strlen(str));
	return font.CharWidths(aStr.String(), count);
}


void
BTextEditable::_DrawString(const char *str, BPoint location)
{
	if (fTypingHidden == 0x01 || str == NULL || *str == 0) return;

	if (fTypingHidden == 0x00) {
		DrawString(str, location);
	} else {
		BString aStr;
		aStr.Append(*((char*)&fTypingHidden), e_utf8_strlen(str));
		DrawString(aStr.String(), location);
	}
}

