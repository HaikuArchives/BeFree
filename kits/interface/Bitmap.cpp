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
 * File: Bitmap.cpp
 * Description: BBitmap --- a rectangular image for drawing
 * Warning: Unfinished.
 *
 * --------------------------------------------------------------------------*/


#include <add-ons/graphics/GraphicsEngine.h>
#include <app/Application.h>
#include <render/Pixmap.h>

#include "Bitmap.h"

class _LOCAL BBitmapWindow : public BWindow
{
	public:
		BBitmapWindow(BRect frame);
		virtual ~BBitmapWindow();

	private:
		virtual bool IsDependsOnOthersWhenQuitRequested() const;
};


BBitmapWindow::BBitmapWindow(BRect frame)
		: BWindow(frame, NULL, B_NO_BORDER_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL, 0)
{
}


BBitmapWindow::~BBitmapWindow()
{
}


bool
BBitmapWindow::IsDependsOnOthersWhenQuitRequested() const
{
	return true;
}


void
BBitmap::InitSelf(BRect bounds, bool acceptsViews)
{
	fRows = 0;
	fColumns = 0;
	fPixmap = NULL;
	fWindow = NULL;

	if (bounds.IsValid() == false) return;

	if (app == NULL || app->fGraphicsEngine == NULL) {
		ETK_WARNING("[INTERFACE]: %s --- You should create BBitmap within graphics engine.", __PRETTY_FUNCTION__);
		return;
	}

	fColumns = (uint32)bounds.IntegerWidth() + 1;
	fRows = (uint32)bounds.IntegerHeight() + 1;

	if (acceptsViews == false) {
		fPixmap = app->fGraphicsEngine->CreatePixmap(fColumns - 1, fRows - 1);
	} else {
		fWindow = new BBitmapWindow(bounds);
		fWindow->Lock();
		delete fWindow->fWindow;
		fWindow->fWindow = NULL;
		fPixmap = fWindow->fPixmap;
		fWindow->Show();
		fWindow->Unlock();
	}
}


BBitmap::BBitmap(BRect bounds, bool acceptsViews)
		: BArchivable()
{
	InitSelf(bounds, acceptsViews);
}


BBitmap::BBitmap(const BBitmap *bitmap, bool acceptsViews)
		: BArchivable()
{
	InitSelf(bitmap == NULL ? BRect() : bitmap->Bounds(), acceptsViews);
	if (bitmap->fPixmap != NULL) {
		BGraphicsContext *dc = app->fGraphicsEngine->CreateContext();
		if (dc) {
			bitmap->fPixmap->CopyTo(dc, fPixmap, 0, 0, fColumns - 1, fRows - 1, 0, 0, fColumns - 1, fRows - 1);
			delete dc;
		}
	}
}


BBitmap::BBitmap(const BPixmap *pixmap, bool acceptsViews)
		: BArchivable()
{
	InitSelf((pixmap == NULL || pixmap->IsValid() == false) ? BRect() : pixmap->Bounds(), acceptsViews);
	if (fPixmap != NULL) {
		BGraphicsContext *dc = app->fGraphicsEngine->CreateContext();
		if (dc) {
			dc->SetDrawingMode(B_OP_COPY);
			dc->SetHighColor(0, 0, 0, 255);
			dc->SetClipping(BRegion(pixmap->Bounds()));
			fPixmap->DrawPixmap(dc, pixmap, 0, 0, fColumns - 1, fRows - 1, 0, 0, fColumns - 1, fRows - 1);
			delete dc;
		}
	}
}


BBitmap::~BBitmap()
{
	if (fWindow) {
		fWindow->Lock();
		fWindow->Quit();
	} else if (fPixmap) {
		delete fPixmap;
	}
}


status_t
BBitmap::InitCheck() const
{
	return(fPixmap != NULL ?B_OK :B_ERROR);
}


bool
BBitmap::IsValid() const
{
	return(fPixmap != NULL);
}


BRect
BBitmap::Bounds() const
{
	if (fPixmap == NULL) return BRect();
	return BRect(0, 0, (float)(fColumns - 1), (float)(fRows - 1));
}


void
BBitmap::AddChild(BView *view)
{
	if (fWindow != NULL) fWindow->AddChild(view);
}


bool
BBitmap::RemoveChild(BView *view)
{
	return(fWindow != NULL ? fWindow->RemoveChild(view) : false);
}


int32
BBitmap::CountChildren() const
{
	return(fWindow != NULL ? fWindow->CountChildren() : 0);
}


BView*
BBitmap::ChildAt(int32 index) const
{
	return(fWindow != NULL ? fWindow->ChildAt(index) : NULL);
}


BView*
BBitmap::FindView(const char *name) const
{
	return(fWindow != NULL ? fWindow->FindView(name) : NULL);
}


BView*
BBitmap::FindView(BPoint where) const
{
	return(fWindow != NULL ? fWindow->FindView(where) : NULL);
}


bool
BBitmap::Lock()
{
	return(fWindow != NULL ? fWindow->Lock() : false);
}


void
BBitmap::Unlock()
{
	if (fWindow != NULL) fWindow->Unlock();
}

