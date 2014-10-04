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
 * File: FindDirectory.cpp
 *
 * --------------------------------------------------------------------------*/

#include <stdlib.h>

#include <support/String.h>

#include "FindDirectory.h"

#include <befree-fsh.h>

status_t e_find_directory(e_directory_which which, BPath *path)
{
	if (path == NULL) return B_ERROR;

	status_t retVal = B_ERROR;

	switch (which) {
		case B_BOOT_DIRECTORY:
			if (path->SetTo("/") == B_OK) retVal = B_OK;
			break;

		case B_APPS_DIRECTORY:
			if (path->SetTo(DATA_DIR, "apps") == B_OK) retVal = B_OK;
			break;

		case B_BIN_DIRECTORY:
			if (path->SetTo(BIN_DIR) == B_OK) retVal = B_OK;
			break;

		case B_LIB_DIRECTORY:
			if (path->SetTo(LIB_DIR) == B_OK) retVal = B_OK;
			break;

		case B_ETC_DIRECTORY:
			if (path->SetTo(ETC_DIR) == B_OK) retVal = B_OK;
			break;

		case B_ADDONS_DIRECTORY:
			if (path->SetTo(LIB_DIR, "add-ons") == B_OK) retVal = B_OK;
			break;

		case B_TEMP_DIRECTORY:
			if (path->SetTo("/tmp") == B_OK) retVal = B_OK;
			break;

		case B_USER_DIRECTORY:
			if (path->SetTo(getenv("HOME")) == B_OK) retVal = B_OK;
			break;

		case B_USER_CONFIG_DIRECTORY:
			if (path->SetTo(getenv("HOME"), ".config") == B_OK) retVal = B_OK;
			break;

		case B_USER_BIN_DIRECTORY:
			if (path->SetTo(getenv("HOME"), ".config/bin") == B_OK) retVal = B_OK;
			break;

		case B_USER_LIB_DIRECTORY:
			if (path->SetTo(getenv("HOME"), ".config/lib") == B_OK) retVal = B_OK;
			break;

		case B_USER_ETC_DIRECTORY:
			if (path->SetTo(getenv("HOME"), ".config/etc") == B_OK) retVal = B_OK;
			break;

		case B_USER_ADDONS_DIRECTORY:
			if (path->SetTo(getenv("HOME"), ".config/add-ons") == B_OK) retVal = B_OK;
			break;

		default:
			break;
	}

	return retVal;
}

