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
 * File: NetDebug.cpp
 *
 * --------------------------------------------------------------------------*/

#include <stdarg.h>
#include <stdio.h>

#include <support/Autolock.h>
#include <support/SimpleLocker.h>
#include <support/String.h>

#include "NetDebug.h"

static BSimpleLocker _e_net_locker(true);
static bool _e_net_enabled = false;


void
BNetDebug::Enable(bool state)
{
	BAutolock <BSimpleLocker> autolock(_e_net_locker);
	if (autolock.IsLocked()) _e_net_enabled = state;
}


bool
BNetDebug::IsEnabled()
{
	return _e_net_enabled;
}


void
BNetDebug::Debug(const char *format, ...)
{
	if (!format) return;

	BAutolock <BSimpleLocker> autolock(_e_net_locker);
	if (!autolock.IsLocked() || !_e_net_enabled) return;

	va_list args;

	char *buffer = NULL;

	va_start(args, format);
	buffer = b_strdup_vprintf(format, args);
	va_end(args);

	if (buffer == NULL) return;

	fputs(buffer, stderr);
	free(buffer);
}


void
BNetDebug::Print(const char *string)
{
	if (string == NULL || *string == 0) return;

	BAutolock <BSimpleLocker> autolock(_e_net_locker);
	if (!autolock.IsLocked() || !_e_net_enabled) return;

	fputs(string, stderr);
}


void
BNetDebug::Dump(const char *data, size_t len, const char *title)
{
	if (!data || len == 0) return;

	BAutolock <BSimpleLocker> autolock(_e_net_locker);
	if (!autolock.IsLocked() || !_e_net_enabled) return;

	fprintf(stderr, "[NET]: -------- %s (START) --------\n", title ? title : "No title");

	while (len > 0) {
		for (int i = 0; i < 16 && len > 0; i++, len--, data++) {
			fprintf(stderr, "%02x", *data);
			if (!(i == 15 || len == 1)) fputc(' ', stderr);
		}
		fputs("\n", stderr);
	}

	fprintf(stderr, "[NET]: --------- %s (END) ---------\n", title ? title : "No title");
}


