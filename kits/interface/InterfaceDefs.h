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
 * File: InterfaceDefs.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_INTERFACE_DEFS_H__
#define __ETK_INTERFACE_DEFS_H__

#include <interface/GraphicsDefs.h>

enum {
	B_FOLLOW_NONE			= 0,
	B_FOLLOW_LEFT			= 1,
	B_FOLLOW_RIGHT			= 1 << 1,
	B_FOLLOW_TOP			= 1 << 2,
	B_FOLLOW_BOTTOM			= 1 << 3,
	B_FOLLOW_H_CENTER		= 1 << 4,
	B_FOLLOW_V_CENTER		= 1 << 5,
	B_FOLLOW_ALL			= 0xffff
};

#define B_FOLLOW_LEFT_RIGHT	(B_FOLLOW_LEFT |B_FOLLOW_RIGHT)
#define B_FOLLOW_TOP_BOTTOM	(B_FOLLOW_TOP |B_FOLLOW_BOTTOM)
#define B_FOLLOW_ALL_SIDES	B_FOLLOW_ALL

enum {
	B_BACKSPACE	= 0x08,
	B_RETURN	= 0x0a,
	B_ENTER		= 0x0a,
	B_SPACE		= 0x20,
	B_TAB		= 0x09,
	B_ESCAPE	= 0x1b,

	B_LEFT_ARROW	= 0x1c,
	B_RIGHT_ARROW	= 0x1d,
	B_UP_ARROW	= 0x1e,
	B_DOWN_ARROW	= 0x1f,

	B_INSERT	= 0x05,
	B_DELETE	= 0x7f,
	B_HOME		= 0x01,
	B_END		= 0x04,
	B_PAGE_UP	= 0x0b,
	B_PAGE_DOWN	= 0x0c,

	B_FUNCTION_KEY	= 0x10
};

enum {
	B_F1_KEY	= 0x02,
	B_F2_KEY	= 0x03,
	B_F3_KEY	= 0x04,
	B_F4_KEY	= 0x05,
	B_F5_KEY	= 0x06,
	B_F6_KEY	= 0x07,
	B_F7_KEY	= 0x08,
	B_F8_KEY	= 0x09,
	B_F9_KEY	= 0x0a,
	B_F10_KEY	= 0x0b,
	B_F11_KEY	= 0x0c,
	B_F12_KEY	= 0x0d,
	B_PRINT_KEY	= 0x0e,
	B_SCROLL_KEY	= 0x0f,
	B_PAUSB_KEY	= 0x10
};

enum border_style {
	B_PLAIN_BORDER,
	B_FANCY_BORDER,
	B_NO_BORDER
};

enum orientation {
	B_HORIZONTAL,
	B_VERTICAL
};

enum e_join_mode {
	B_ROUND_JOIN = 0,
	B_MITER_JOIN,
	B_BEVEL_JOIN,
	B_BUTT_JOIN,
	B_SQUARB_JOIN
};

enum e_cap_mode {
	B_ROUND_CAP = B_ROUND_JOIN,
	B_BUTT_CAP = B_BUTT_JOIN,
	B_SQUARB_CAP = B_SQUARB_JOIN
};

enum e_alignment {
	B_ALIGN_LEFT,
	B_ALIGN_RIGHT,
	B_ALIGN_CENTER
};

enum e_vertical_alignment {
	B_ALIGN_TOP,
	B_ALIGN_BOTTOM,
	B_ALIGN_MIDDLE
};

enum {
	B_SHIFT_KEY		= 1 << 1,
	B_COMMAND_KEY		= 1 << 2,
	B_CONTROL_KEY		= 1 << 3,
	B_CAPS_LOCK		= 1 << 4,
	B_SCROLL_LOCK		= 1 << 5,
	B_NUM_LOCK		= 1 << 6,
	B_OPTION_KEY		= 1 << 7,
	B_MENU_KEY		= 1 << 8,
	B_LEFT_SHIFT_KEY	= 1 << 9,
	B_RIGHT_SHIFT_KEY	= 1 << 10,
	B_LEFT_COMMAND_KEY	= 1 << 11,
	B_RIGHT_COMMAND_KEY	= 1 << 12,
	B_LEFT_CONTROL_KEY	= 1 << 13,
	B_RIGHT_CONTROL_KEY	= 1 << 14,
	B_LEFT_OPTION_KEY	= 1 << 15,
	B_RIGHT_OPTION_KEY	= 1 << 16,
	B_FUNCTIONS_KEY		= 1 << 17
};

enum {
	B_PRIMARY_MOUSE_BUTTON = 1,
	B_SECONDARY_MOUSB_BUTTON = 2,
	B_TERTIARY_MOUSB_BUTTON = 3
};

typedef enum e_color_which {
	B_DESKTOP_COLOR = 0,

	B_PANEL_BACKGROUND_COLOR = 1,
	B_PANEL_TEXT_COLOR = 2,

	B_DOCUMENT_BACKGROUND_COLOR = 3,
	B_DOCUMENT_TEXT_COLOR = 4,
	B_DOCUMENT_HIGHLIGHT_COLOR = 5,
	B_DOCUMENT_CURSOR_COLOR = 6,

	B_BUTTON_BACKGROUND_COLOR = 7,
	B_BUTTON_TEXT_COLOR = 8,
	B_BUTTON_BORDER_COLOR = 9,

	B_NAVIGATION_BASE_COLOR = 10,
	B_NAVIGATION_PULSE_COLOR = 11,

	B_MENU_BACKGROUND_COLOR = 12,
	B_MENU_BORDER_COLOR = 13,
	B_MENU_SELECTED_BACKGROUND_COLOR = 14,
	B_MENU_ITEM_TEXT_COLOR = 15,
	B_MENU_SELECTED_ITEM_TEXT_COLOR = 16,
	B_MENU_SELECTED_BORDER_COLOR = 17,

	B_TOOLTIP_BACKGROUND_COLOR = 18,
	B_TOOLTIP_TEXT_COLOR = 19,

	B_SHINE_COLOR = 20,
	B_SHADOW_COLOR = 21,

	B_STATUSBAR_COLOR = 22,

} e_color_which;


enum e_button_width {
	B_WIDTH_AS_USUAL,
	B_WIDTH_FROM_LABEL
};


#ifdef __cplusplus /* Just for C++ */
extern "C"
{
#endif

	rgb_color ui_color(e_color_which which);
	float e_ui_get_scrollbar_horizontal_height();
	float e_ui_get_scrollbar_vertical_width();

#ifdef __cplusplus /* Just for C++ */
} /* extern "C" */
#endif

#endif /* __ETK_INTERFACE_DEFS_H__ */

