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
 * File: Theme.cpp
 *
 * --------------------------------------------------------------------------*/

#include "InterfaceDefs.h"


rgb_color ui_color(e_color_which which)
{
	rgb_color color;

	switch (which) {
		case B_DESKTOP_COLOR:
			color.set_to(118, 132, 143);
			break;

		case B_PANEL_BACKGROUND_COLOR:
			color.set_to(240, 240, 235);
			break;

		case B_TOOLTIP_TEXT_COLOR:
		case B_BUTTON_TEXT_COLOR:
		case B_DOCUMENT_TEXT_COLOR:
		case B_PANEL_TEXT_COLOR:
			color.set_to(0, 0, 0);
			break;

		case B_DOCUMENT_BACKGROUND_COLOR:
			color.set_to(250, 250, 250);
			break;

		case B_DOCUMENT_HIGHLIGHT_COLOR:
			color.set_to(170, 210, 240);
			break;

		case B_DOCUMENT_CURSOR_COLOR:
			color.set_to(0, 0, 0);
			break;

		case B_BUTTON_BACKGROUND_COLOR:
			color.set_to(245, 245, 245);
			break;

		case B_BUTTON_BORDER_COLOR:
//			color.set_to(200, 150, 150);
			color.set_to(50, 50, 50);
			break;

		case B_NAVIGATION_BASE_COLOR:
//			color.set_to(225, 140, 190);
			color.set_to(170, 210, 240);
			break;

		case B_NAVIGATION_PULSE_COLOR:
//			color.set_to(190, 120, 160);
			color.set_to(90, 100, 120);
			break;

		case B_MENU_BACKGROUND_COLOR:
			color.set_to(245, 245, 245);
			break;

		case B_MENU_BORDER_COLOR:
//			color.set_to(200, 150, 150);
			color.set_to(50, 50, 50);
			break;

		case B_MENU_SELECTED_BACKGROUND_COLOR:
//			color.set_to(225, 170, 170);
			color.set_to(170, 210, 240);
			break;

		case B_MENU_ITEM_TEXT_COLOR:
			color.set_to(80, 80, 80);
			break;

		case B_MENU_SELECTED_ITEM_TEXT_COLOR:
			color.set_to(0, 0, 0);
			break;

		case B_MENU_SELECTED_BORDER_COLOR:
//			color.set_to(225, 140, 190);
			color.set_to(100, 100, 100);
			break;

		case B_TOOLTIP_BACKGROUND_COLOR:
			color.set_to(235, 220, 30);
			break;

		case B_SHINE_COLOR:
			color.set_to(250, 250, 250);
			break;

		case B_SHADOW_COLOR:
			color.set_to(50, 50, 50);
			break;

		case B_STATUSBAR_COLOR:
			color.set_to(235, 220, 30);
			break;

		default:
			color.set_to(0, 0, 0);
	}

	return color;
}


float e_ui_get_scrollbar_vertical_width()
{
	return 16;
}


float e_ui_get_scrollbar_horizontal_height()
{
	return 16;
}


