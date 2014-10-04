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
 * File: FontEngine.cpp
 *
 * --------------------------------------------------------------------------*/

#include <string.h>

#include "FontEngine.h"

#include <support/Locker.h>
#include <support/Autolock.h>
#include <app/Application.h>
#include <add-ons/graphics/GraphicsEngine.h>

#define BEFREE_PLAIN_FONT_FAMILY "DejaVu Sans"
#define BEFREE_PLAIN_FONT_STYLE "Book"
#define BEFREE_PLAIN_FONT_SIZE 10

#define BEFREE_BOLD_FONT_FAMILY "DejaVu Sans"
#define BEFREE_BOLD_FONT_STYLE "Bold"
#define BEFREE_BOLD_FONT_SIZE 10

#define BEFREE_FIXED_FONT_FAMILY "DejaVu Sans Mono"
#define BEFREE_FIXED_FONT_STYLE "Book"
#define BEFREE_FIXED_FONT_SIZE 10

static BFont* _be_plain_font = NULL;
static BFont* _be_bold_font = NULL;
static BFont* _be_fixed_font = NULL;

static BLocker font_locker;
static bool _font_initialized_ = false;
static bool _font_canceling_ = false;
static BStringArray font_families;

extern bool font_freetype2_init(void);
extern bool font_freetype2_is_valid(void);
extern void font_freetype2_cancel(void);
extern void font_freetype2_quit(void);
extern bool update_freetype2_font_families(bool check_only);


bool font_add(const char *family, const char *style, BFontEngine *engine)
{
	if (family == NULL || *family == 0 || style == NULL || *style == 0 || engine == NULL) return false;

	BAutolock <BLocker> autolock(&font_locker);
	if (!_font_initialized_ || engine->fServing != NULL) return false;

	BStringArray *styles = NULL;
	font_families.ItemAt(font_families.FindString(family), (void**)&styles);
	if (styles ? styles->FindString(style) >= 0 : false) {
//		ETK_DEBUG("[FONT]: %s --- style[%s] of family[%s] already exists.", __PRETTY_FUNCTION__, style, family);
		return false;
	}

	if (!styles) {
		styles = new BStringArray;
		if (!styles || !styles->AddItem(style, (void*)engine) ||
		        !font_families.AddItem(family, (void*)styles)) {
			ETK_DEBUG("[FONT]: %s --- Add style[%s] of family[%s] failed.", __PRETTY_FUNCTION__, style, family);
			if (styles) delete styles;
			return false;
		}
	} else {
		if (!styles->AddItem(style, (void*)engine)) {
			ETK_DEBUG("[FONT]: %s --- Add style[%s] of family[%s] failed.", __PRETTY_FUNCTION__, style, family);
			return false;
		}
	}

	engine->fServing = styles;

	return true;
}


BFontEngine::BFontEngine()
		: fFamily(NULL), fStyle(NULL), fFixedSize(NULL), nFixedSize(0), fRenderMode(B_FONT_RENDER_UNKNOWN), fServing(NULL)
{
}


BFontEngine::BFontEngine(const char *family, const char *style)
		: fFamily(NULL), fStyle(NULL), fFixedSize(NULL), nFixedSize(0), fRenderMode(B_FONT_RENDER_UNKNOWN), fServing(NULL)
{
	SetFamily(family);
	SetStyle(style);
}


bool
BFontEngine::InServing() const
{
	BAutolock <BLocker> autolock(&font_locker);
	return(fServing != NULL);
}


void
BFontEngine::OutOfServing()
{
	BAutolock <BLocker> autolock(&font_locker);
	if (_font_canceling_ || fServing == NULL) return;

	BFontEngine *engine = NULL;
	for (int32 i = 0; i < fServing->CountItems(); i++) {
		fServing->ItemAt(i, (void**)&engine);
		if (engine != this) continue;
		if (fServing->RemoveItem(i)) {
			if (fServing->CountItems() <= 0) {
				BStringArray *styles = NULL;
				for (int32 j = 0; j < font_families.CountItems(); j++) {
					font_families.ItemAt(j, (void**)&styles);
					if (styles != fServing) continue;
					if (font_families.RemoveItem(j)) delete fServing;
					break;
				}
			}
			fServing = NULL;
		}
		break;
	}
}


BFontEngine::~BFontEngine()
{
	if (InServing()) OutOfServing();

	for (int32 i = 0; i < fAttached.CountItems(); i++) {
		e_font_detach_callback *eCallback = (e_font_detach_callback*)fAttached.ItemAt(i);
		if (!eCallback) continue;
		if (eCallback->callback != NULL) eCallback->callback(eCallback->data);
		delete eCallback;
	}
	fAttached.MakeEmpty();

	if (fFamily) delete[] fFamily;
	if (fStyle) delete[] fStyle;
	if (fFixedSize) delete[] fFixedSize;
}


e_font_detach_callback*
BFontEngine::Attach(void (*callback)(void*), void *data)
{
	e_font_detach_callback *eCallback = new e_font_detach_callback;
	if (!eCallback) return NULL;

	if (!fAttached.AddItem(eCallback)) {
		delete eCallback;
		return NULL;
	}

	eCallback->callback = callback;
	eCallback->data = data;

	return eCallback;
}


bool
BFontEngine::IsAttached() const
{
	return(fAttached.CountItems() > 0);
}


bool
BFontEngine::Detach(e_font_detach_callback *callback)
{
	if (!callback) return false;
	for (int32 i = fAttached.CountItems() - 1; i >= 0; i--) {
		e_font_detach_callback *eCallback = (e_font_detach_callback*)fAttached.ItemAt(i);
		if (eCallback != callback) continue;
		if (fAttached.RemoveItem(i)) {
			if (eCallback->callback != NULL) eCallback->callback(eCallback->data);
			delete eCallback;
			return true;
		}
		break;
	}
	return false;
}


bool
BFontEngine::Lock()
{
	return fLocker.Lock();
}


void
BFontEngine::Unlock()
{
	fLocker.Unlock();
}


const char*
BFontEngine::Family() const
{
	return fFamily;
}


const char*
BFontEngine::Style() const
{
	return fStyle;
}


e_font_render_mode
BFontEngine::RenderMode() const
{
	return fRenderMode;
}


void
BFontEngine::SetRenderMode(e_font_render_mode rmode)
{
	fRenderMode = rmode;
}


bool
BFontEngine::HasFixedSize(int32 *count) const
{
	if (nFixedSize > 0 && fFixedSize != NULL) {
		if (count) *count = nFixedSize;
		return true;
	}

	return false;
}


bool
BFontEngine::GetFixedSize(float *size, int32 index) const
{
	if (size == NULL || index < 0) return false;

	if (nFixedSize > index && fFixedSize != NULL) {
		*size = fFixedSize[index];
		return true;
	}

	return false;
}


void
BFontEngine::SetFamily(const char *family)
{
	if (fFamily) delete[] fFamily;
	fFamily = NULL;
	if (family) fFamily = EStrdup(family);
}


void
BFontEngine::SetStyle(const char *style)
{
	if (fStyle) delete[] fStyle;
	fStyle = NULL;
	if (style) fStyle = EStrdup(style);
}


void
BFontEngine::SetFixedSize(float *sizes, int32 count)
{
	if (fFixedSize) {
		delete[] fFixedSize;
		fFixedSize = NULL;
	}

	nFixedSize = 0;

	if (sizes && count > 0) {
		fFixedSize = new float[count];
		memcpy(fFixedSize, sizes, sizeof(float) * (size_t)count);
		nFixedSize = count;
	}
}


bool
BFontEngine::IsValid() const
{
	return false;
}


bool
BFontEngine::IsScalable() const
{
	return false;
}


void
BFontEngine::ForceFontAliasing(bool enable)
{
}


float
BFontEngine::StringWidth(const char *string, float size, float spacing, float shear, bool bold, int32 length) const
{
	return 0;
}


void
BFontEngine::GetHeight(font_height *height, float size, float shear, bool bold) const
{
	if (height) bzero(height, sizeof(font_height));
}


float
BFontEngine::StringWidth(const BString &str, float size, float spacing, float shear, bool bold, int32 length) const
{
	return StringWidth(str.String(), size, spacing, shear, bold, length);
}


BRect
BFontEngine::RenderString(BHandler *view, const char *string,
                          float size, float spacing,
                          float shear, bool bold, int32 length)
{
	return BRect();
}


BRect
BFontEngine::RenderString(BHandler *view, const BString &str,
                          float size, float spacing,
                          float shear, bool bold, int32 length)
{
	return RenderString(view, str.String(), size, spacing, shear, bold, length);
}


uint8*
BFontEngine::RenderString(const char *string, int32 *width, int32 *height, bool *is_mono,
                          float size, float spacing,
                          float shear, bool bold, int32 length)
{
	return NULL;
}


uint8*
BFontEngine::RenderString(const BString &str, int32 *width, int32 *height, bool *is_mono,
                          float size, float spacing,
                          float shear, bool bold, int32 length)
{
	return RenderString(str.String(), width, height, is_mono, size, spacing, shear, bold, length);
}


int32 count_font_families(void)
{
	BAutolock <BLocker> autolock(&font_locker);

	return font_families.CountItems();
}


status_t get_font_family(int32 index, const char **name)
{
	if (!name) return B_BAD_VALUE;

	BAutolock <BLocker> autolock(&font_locker);

	const BString *str = font_families.ItemAt(index);
	if (!str) return B_ERROR;

	*name = str->String();
	return B_OK;
}


int32 get_font_family_index(const char *name)
{
	if (!name) return -1;

	BAutolock <BLocker> autolock(&font_locker);

	int32 fIndex = font_families.FindString(name);
	return fIndex;
}


int32 get_font_style_index(const char *family, const char *name)
{
	if (!family || !name) return -1;

	BAutolock <BLocker> autolock(&font_locker);

	int32 index = font_families.FindString(family);
	if (index < 0) return -1;

	BStringArray *styles = NULL;
	font_families.ItemAt(index, (void**)&styles);
	if (!styles) return -1;

	index = styles->FindString(name);
	return index;
}


int32 count_font_styles(const char *name)
{
	BAutolock <BLocker> autolock(&font_locker);

	return count_font_styles(font_families.FindString(name));
}


int32 count_font_styles(int32 index)
{
	if (index < 0) return -1;

	BAutolock <BLocker> autolock(&font_locker);

	BStringArray *styles = NULL;
	font_families.ItemAt(index, (void**)&styles);

	return(styles ? styles->CountItems() : -1);
}


status_t get_font_style(const char *family, int32 index, const char **name)
{
	if (!family || !name) return B_BAD_VALUE;

	BAutolock <BLocker> autolock(&font_locker);

	int32 fIndex = font_families.FindString(family);
	if (fIndex < 0) return B_ERROR;

	BStringArray *styles = NULL;
	font_families.ItemAt(fIndex, (void**)&styles);
	if (!styles) return B_ERROR;

	const BString *str = styles->ItemAt(index);
	if (!str) return B_ERROR;

	*name = str->String();
	return B_OK;
}


BFontEngine* get_font_engine(const char *family, const char *style)
{
	if (!family || !style) return NULL;

	BAutolock <BLocker> autolock(&font_locker);

	BStringArray *styles = NULL;
	font_families.ItemAt(font_families.FindString(family), (void**)&styles);
	if (!styles) return NULL;

	BFontEngine *engine = NULL;
	styles->ItemAt(styles->FindString(style), (void**)&engine);

	return engine;
}


BFontEngine* get_font_engine(int32 familyIndex, int32 styleIndex)
{
	BAutolock <BLocker> autolock(&font_locker);

	BStringArray *styles = NULL;
	font_families.ItemAt(familyIndex, (void**)&styles);
	if (!styles) return NULL;

	BFontEngine *engine = NULL;
	styles->ItemAt(styleIndex, (void**)&engine);

	return engine;
}


static bool font_other_init()
{
	// TODO
	return true;
}


static void font_other_cancel()
{
	// TODO
}


static bool update_other_font_families(bool check_only)
{
	// TODO
	return(check_only ? false : true);
}


bool update_font_families(bool check_only)
{
	BAutolock <BLocker> autolock(&font_locker);

	if (!_font_initialized_) return false;

	if (!check_only) {
		BStringArray *styles;
		for (int32 i = 0; i < font_families.CountItems(); i++) {
			styles = NULL;
			font_families.ItemAt(i, (void**)&styles);
			if (styles) {
				BFontEngine *engine;
				for (int32 j = 0; j < styles->CountItems(); j++) {
					engine = NULL;
					styles->ItemAt(j, (void**)&engine);
					if (engine) delete engine;
				}
				styles->MakeEmpty();
				delete styles;
			}
		}
		font_families.MakeEmpty();
	}

	bool updateFailed = false;

	if (!(app == NULL || app->fGraphicsEngine == NULL)) app->fGraphicsEngine->UpdateFonts(check_only);

	// TODO: fix the return value style
	if (font_freetype2_is_valid()) {
		if (update_freetype2_font_families(check_only)) {
			if (check_only) return true;
		} else if (!updateFailed && !check_only) {
			updateFailed = true;
		}
	}

	if (update_other_font_families(check_only)) {
		if (check_only) return true;
	} else if (!updateFailed && !check_only) {
		updateFailed = true;
	}

	return(check_only ? false : !updateFailed);
}


_LOCAL bool font_init(void)
{
	BAutolock <BLocker> autolock(&font_locker);

	if (!_font_initialized_) {
		ETK_DEBUG("[FONT]: Initalizing fonts ...");

		font_freetype2_init();
		font_other_init();
		_font_initialized_ = true;
		update_font_families(false);

		// TODO: take from config
		_be_plain_font = new BFont();
		if (_be_plain_font) {
				if (_be_plain_font->SetFamilyAndStyle(BEFREE_PLAIN_FONT_FAMILY, BEFREE_PLAIN_FONT_STYLE) != B_OK)
					_be_plain_font->SetFamilyAndStyle(0);
				float fsize = BEFREE_PLAIN_FONT_SIZE;

				if (_be_plain_font->IsScalable() == false) {
					_be_plain_font->GetFixedSize(&fsize);
					_be_plain_font->SetSize(fsize);
				} else {
					_be_plain_font->SetSize(fsize);
				}
			_be_plain_font->SetSpacing(0.05f);
			_be_plain_font->SetShear(90);
		}
		_be_bold_font = new BFont();
		if (_be_bold_font) {
				if (_be_bold_font->SetFamilyAndStyle(BEFREE_BOLD_FONT_FAMILY, BEFREE_BOLD_FONT_STYLE) != B_OK)
					_be_bold_font->SetFamilyAndStyle(0);

				float fsize = BEFREE_BOLD_FONT_SIZE;

				if (_be_bold_font->IsScalable() == false) {
					_be_bold_font->GetFixedSize(&fsize);
					_be_bold_font->SetSize(fsize);
				} else {
					_be_bold_font->SetSize(fsize);
				}
			_be_bold_font->SetSpacing(0.05f);
			_be_bold_font->SetShear(90);
		}
		_be_fixed_font = new BFont();
		if (_be_fixed_font) {
				if (_be_fixed_font->SetFamilyAndStyle(BEFREE_FIXED_FONT_FAMILY, BEFREE_FIXED_FONT_STYLE) != B_OK)
					_be_fixed_font->SetFamilyAndStyle(0);

				float fsize = BEFREE_FIXED_FONT_SIZE;

				if (_be_fixed_font->IsScalable() == false) {
					_be_fixed_font->GetFixedSize(&fsize);
					_be_fixed_font->SetSize(fsize);
				} else {
					_be_fixed_font->SetSize(fsize);
				}
			_be_fixed_font->SetSpacing(0.05f);
			_be_fixed_font->SetShear(70);
		}

		be_plain_font = _be_plain_font;
		be_bold_font = _be_bold_font;
		be_fixed_font = _be_fixed_font;

		ETK_DEBUG("[FONT]: Fonts initalized.");
	}

	return(font_families.CountItems() > 0);
}


_LOCAL void font_cancel(void)
{
	BAutolock <BLocker> autolock(&font_locker);

	if (_font_initialized_) {
		_font_canceling_ = true;

		if (_be_plain_font) delete _be_plain_font;
		if (_be_bold_font) delete _be_bold_font;
		if (_be_fixed_font) delete _be_fixed_font;
		be_plain_font = _be_plain_font = NULL;
		be_bold_font = _be_bold_font = NULL;
		be_fixed_font = _be_fixed_font = NULL;

		font_freetype2_cancel();
		font_other_cancel();

		BStringArray *styles;
		for (int32 i = 0; i < font_families.CountItems(); i++) {
			styles = NULL;
			font_families.ItemAt(i, (void**)&styles);
			if (styles) {
				BFontEngine *engine;
				for (int32 j = 0; j < styles->CountItems(); j++) {
					engine = NULL;
					styles->ItemAt(j, (void**)&engine);
					if (engine) delete engine;
				}
				styles->MakeEmpty();
				delete styles;
			}
		}
		font_families.MakeEmpty();

		_font_canceling_ = false;
		_font_initialized_ = false;
	}
}


_LOCAL bool font_lock(void)
{
	return font_locker.Lock();
}


_LOCAL void font_unlock(void)
{
	font_locker.Unlock();
}

