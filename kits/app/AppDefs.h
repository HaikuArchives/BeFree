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
 * File: AppDefs.h
 * Description: Definition of Application Kit
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_APPDEFS_H__
#define __ETK_APPDEFS_H__

#include <support/SupportDefs.h>

/* System Message Codes */
enum {
	E_ABOUT_REQUESTED			= '_ABR',
	B_WINDOW_ACTIVATED			= '_ACT',
	B_APP_ACTIVATED				= '_ACT', /* Same asB_WINDOW_ACTIVATED */
	B_APP_CURSOR_REQUESTED			= '_CUR',
	E_ARGV_RECEIVED 			= '_ARG',
	B_QUIT_REQUESTED 			= '_QRQ',
	B_CANCEL				= '_CNC',
	B_KEY_DOWN 				= '_KYD',
	B_KEY_UP 				= '_KYU',
	B_UNMAPPED_KEY_DOWN 			= '_UKD',
	B_UNMAPPED_KEY_UP 			= '_UKU',
	B_MODIFIERS_CHANGED			= '_MCH',
	B_MINIMIZE				= '_WMN',
	B_MINIMIZED				= '_WMD',
	B_MOUSE_DOWN 				= '_MDN',
	B_MOUSE_MOVED 				= '_MMV',
	B_MOUSE_ENTER_EXIT			= '_MEX',
	B_MOUSE_UP 				= '_MUP',
	B_MOUSE_WHEEL_CHANGED			= '_MWC',
	B_OPEN_IN_WORKSPACE			= '_OWS',
	E_PRINTER_CHANGED			= '_PCH',
	B_PULSE 				= '_PUL',
	B_READY_TO_RUN 				= '_RTR',
	E_REFS_RECEIVED 			= '_RRC',
	E_SCREEN_CHANGED 			= '_SCH',
	E_VALUE_CHANGED 			= '_VCH',
	B_VIEW_MOVED 				= '_VMV',
	B_VIEW_RESIZED 				= '_VRS',
	B_WINDOW_MOVED 				= '_WMV',
	B_WINDOW_RESIZED 			= '_WRS',
	B_WORKSPACES_CHANGED			= '_WCG',
	B_WORKSPACE_ACTIVATED			= '_WAC',
	E_ZOOM					= '_WZM',
	_QUIT_					= '_QIT',
	_EVENTS_PENDING_			= '_EVP',
	_UPDATE_				= '_UPD',
	_UPDATE_IF_NEEDED_			= '_UPN',
	_MENU_EVENT_				= '_MEV'
};


/* Other Command */
enum {
	B_UPDATE_STATUS_BAR			= 'SBUP',
	B_RESET_STATUS_BAR			= 'SBRS',
	B_OBSERVER_NOTICE_CHANGE		= 'NTCH',
	B_CONTROL_INVOKED			= 'CIVK',
	B_CONTROL_MODIFIED			= 'CMOD',
	B_CLIPBOARD_CHANGED			= 'CLCH',
	E_SAVE_REQUESTED			= 'SAVE',
	B_NO_REPLY				= 'NONE',
	E_REPLY					= 'RPLY'
};

#endif /* __ETK_APPDEFS_H__ */


