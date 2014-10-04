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
 * File: Screen.cpp
 *
 * --------------------------------------------------------------------------*/

#include <add-ons/graphics/GraphicsEngine.h>
#include <app/Application.h>

#include "Screen.h"
#include "Window.h"

BScreen::BScreen(uint32 id)
		: fID(id)
{
	// TODO
}


BScreen::BScreen(BWindow *win)
		: fID(B_MAXUINT32)
{
	// TODO
	if (win) fID = 0;
}


BScreen::~BScreen()
{
	// TODO
}


bool
BScreen::IsValid() const
{
	// TODO
	if (fID != 0) return false;
	return(!(app == NULL || app->fGraphicsEngine == NULL));
}


status_t
BScreen::SetToNext()
{
	// TODO
	return B_ERROR;
}


BRect
BScreen::Frame() const
{
	BRect r;
	if (fID == 0 && !(app == NULL || app->fGraphicsEngine == NULL)) {
		uint32 scrW = 0, scrH = 0;
		app->fGraphicsEngine->GetDesktopBounds(&scrW, &scrH);
		if (scrW > 0 && scrH > 0) r.Set(0, 0, (float)(scrW - 1), (float)(scrH - 1));
	}
	return r;
}


uint32
BScreen::ID() const
{
	return fID;
}

