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
 * File: Font.cpp
 *
 * --------------------------------------------------------------------------*/

#include <string.h>

#include <add-ons/font/FontEngine.h>
#include <support/Autolock.h>

#include "Font.h"


extern const BFont* be_plain_font = NULL;
extern const BFont* be_bold_font = NULL;
extern const BFont* be_fixed_font = NULL;

static BLocker font_info_locker;

typedef struct font_info {
	BFontEngine *engine;
	e_font_detach_callback *detach_callback;
	float size;
	float spacing;
	float shear;
	bool bold;
	int32 family_index;
	int32 style_index;

	font_info() {
		engine = NULL;
		detach_callback = NULL;
		size = 10.f;
		spacing = 0.1f;
		shear = 90.f;
		bold = false;
		family_index = -1;
		style_index = -1;
	}

	static void _detach_callback_(font_info *info) {
		if (!info) return;
		font_info_locker.Lock();
		info->engine = NULL;
		info->detach_callback = NULL;
		font_info_locker.Unlock();
	}

	font_info& operator=(const font_info &info) {
		font_info_locker.Lock();

		BFontEngine *_engine_ = engine;
		if (_engine_) {
			_engine_->Lock();
			if (_engine_->Detach(detach_callback) == false) {
				_engine_->Unlock();
				font_info_locker.Unlock();
				return *this;
			}
			_engine_->Unlock();
		}
		engine = NULL;
		detach_callback = NULL;

		if (info.engine) {
			info.engine->Lock();
			if ((detach_callback = info.engine->Attach((void(*)(void*))_detach_callback_, this)) == NULL) {
				info.engine->Unlock();
				font_info_locker.Unlock();
				return *this;
			}
			info.engine->Unlock();
		}

		engine = info.engine;
		family_index = info.family_index;
		style_index = info.style_index;
		size = info.size;
		spacing = info.spacing;
		shear = info.shear;
		bold = info.bold;

		font_info_locker.Unlock();

		return *this;
	}

	bool operator==(const font_info &info) {
		BAutolock <BLocker> autolock(&font_info_locker);
		return(engine == info.engine && size == info.size && spacing == info.spacing && shear == info.shear && bold == info.bold ? true : false);
	}

	bool operator!=(const font_info &info) {
		BAutolock <BLocker> autolock(&font_info_locker);
		return(engine == info.engine && size == info.size && spacing == info.spacing && shear == info.shear && bold == info.bold ? false : true);
	}

	bool SetEngine(BFontEngine *fengine) {
		font_info_locker.Lock();

		BFontEngine *_engine_ = engine;
		if (_engine_) {
			_engine_->Lock();
			if (_engine_->Detach(detach_callback) == false) {
				_engine_->Unlock();
				font_info_locker.Unlock();
				return false;
			}
			_engine_->Unlock();
		}
		engine = NULL;
		detach_callback = NULL;

		if (fengine) {
			fengine->Lock();
			if ((detach_callback = fengine->Attach((void(*)(void*))_detach_callback_, this)) == NULL) {
				fengine->Unlock();
				font_info_locker.Unlock();
				return false;
			}
			fengine->Unlock();
		}

		engine = fengine;
		family_index = engine ? get_font_family_index(engine->Family()) : -1;
		style_index = engine ? get_font_style_index(engine->Family(), engine->Style()) : -1;

		font_info_locker.Unlock();

		return true;
	}

	uint32 FamilyAndStyle() const {
		if (family_index < 0 || style_index < 0) return B_MAXUINT32;
		uint32 fIndex = (uint32)family_index;
		uint32 sIndex = (uint32)style_index;
		return((fIndex << 16) | sIndex);
	}

	BFontEngine* Engine() const {
		BAutolock <BLocker> autolock(&font_info_locker);
		return engine;
	}

	void SetSize(float fsize) {
		BAutolock <BLocker> autolock(&font_info_locker);
		size = fsize;
	}

	float Size() const {
		BAutolock <BLocker> autolock(&font_info_locker);
		return size;
	}

	void SetSpacing(float fspacing) {
		BAutolock <BLocker> autolock(&font_info_locker);
		spacing = fspacing;
	}

	float Spacing() const {
		BAutolock <BLocker> autolock(&font_info_locker);
		return spacing;
	}

	void SetShear(float fshear) {
		BAutolock <BLocker> autolock(&font_info_locker);
		shear = fshear;
	}

	float Shear() const {
		BAutolock <BLocker> autolock(&font_info_locker);
		return shear;
	}

	void SetBoldStyle(bool fbold) {
		BAutolock <BLocker> autolock(&font_info_locker);
		bold = fbold;
	}

	bool IsBoldStyle() const {
		BAutolock <BLocker> autolock(&font_info_locker);
		return bold;
	}

	float StringWidth(const char *string, int32 length, float tabWidth) const {
		if (string == NULL || *string == 0 || length == 0) return 0;

		font_info_locker.Lock();

		float width = 0;
		if (engine) {
			BString aStr(string, length);

			engine->Lock();

			if (tabWidth == 0) {
				width = engine->StringWidth(aStr.String(), size, spacing, shear, bold, aStr.Length());
			} else {
				float spacing_width = spacing * size;
				if (tabWidth < 0) {
					if (tabWidth <B_FONT_MIN_TAB_WIDTH) tabWidth = -B_FONT_MIN_TAB_WIDTH;
					else tabWidth = (float)(ceil((double)(-tabWidth)));
					tabWidth = (tabWidth * engine->StringWidth(" ", size, spacing, shear, bold, 1)) +
					           (tabWidth - 1.f) * spacing_width;
				}

				for (int32 aOffset = 0; aOffset < aStr.Length(); aOffset++) {
					int32 oldOffset = aOffset, len;
					aOffset = aStr.FindFirst("\t", aOffset);

					len = (aOffset < 0 ? aStr.Length() : aOffset) - oldOffset;
					if (len > 0)
						width += engine->StringWidth(aStr.String() + oldOffset,
						                             size, spacing, shear, bold, len);

					if (aOffset < 0) break;
					width += (aOffset > 0 ? spacing_width : 0) + tabWidth;
				}
			}

			engine->Unlock();
		}

		font_info_locker.Unlock();

		return width;
	}

	float* CharWidths(const char *string, int32 length, int32 *nChars, float tabWidth) const {
		if (string == NULL || *string == 0 || length == 0 || nChars == NULL) return NULL;

		int32 strLen = (int32)strlen(string);
		if (length < 0 || length > strLen) length = strLen;

		float *widths = new float[length];
		if (widths == NULL) return NULL;

		font_info_locker.Lock();

		uint8 len = 0;
		const char *ch = e_utf8_at(string, 0, &len);
		int32 count = 0;

		if (engine) {
			engine->Lock();
			if (tabWidth < 0) {
				float spacing_width = spacing * size;
				if (tabWidth <B_FONT_MIN_TAB_WIDTH) tabWidth = -B_FONT_MIN_TAB_WIDTH;
				else tabWidth = (float)(ceil((double)(-tabWidth)));
				tabWidth = (tabWidth * engine->StringWidth(" ", size, spacing, shear, bold, 1)) +
				           (tabWidth - 1.f) * spacing_width;
			}
		}

		while (!(ch == NULL || len <= 0 || (ch - string > length - len))) {
			if (engine == NULL) {
				widths[count] = 0;
			} else if (tabWidth == 0 || *ch != '\t') {
				widths[count] = engine->StringWidth(ch, size, spacing, shear, bold, (int32)len);
			} else {
				widths[count] = tabWidth;
			}

			count++;
			ch = e_utf8_next(ch, &len);
		}
		if (engine) engine->Unlock();

		font_info_locker.Unlock();

		*nChars = count;

		return widths;
	}

	void GetHeight(font_height *height) const {
		if (!height) return;

		font_info_locker.Lock();

		if (engine) {
			engine->Lock();
			engine->GetHeight(height, size, shear, bold);
			engine->Unlock();
		}

		font_info_locker.Unlock();
	}

	~font_info() {
		font_info_locker.Lock();
		BFontEngine *_engine_ = engine;
		if (_engine_) {
			_engine_->Lock();
			_engine_->Detach(detach_callback);
			_engine_->Unlock();
		}
		if (detach_callback) detach_callback->data = NULL;
		engine = NULL;
		detach_callback = NULL;
		font_info_locker.Unlock();
	}
} font_info;


BFont::BFont()
		: fInfo(NULL)
{
	font_info *fontInfo = new font_info;
	if (!fontInfo) return;

	fInfo = (void*)fontInfo;
}


BFont::BFont(const BFont &font)
		: fInfo(NULL)
{
	font_info *fontInfo = new font_info;
	if (!fontInfo) return;

	if (font.fInfo)
		*fontInfo = *((font_info*)(font.fInfo));

	fInfo = (void*)fontInfo;
}


BFont::BFont(const BFont *font)
		: fInfo(NULL)
{
	font_info *fontInfo = new font_info;
	if (!fontInfo) return;

	if (font) {
		if (font->fInfo) *fontInfo = *((font_info*)(font->fInfo));
	}

	fInfo = (void*)fontInfo;
}


BFont::BFont(const e_font_desc &fontDesc)
		: fInfo(NULL)
{
	font_info *fontInfo = new font_info;
	if (!fontInfo) return;

	fInfo = (void*)fontInfo;

	*this = fontDesc;
}


BFont::~BFont()
{
	if (fInfo) delete (font_info*)fInfo;
}


BFont&
BFont::operator=(const BFont &font)
{
	if (font.fInfo == NULL) {
		if (fInfo) delete (font_info*)fInfo;
		fInfo = NULL;
		return *this;
	}

	if (fInfo == NULL) {
		font_info *fontInfo = new font_info;
		if (!fontInfo) return *this;
		*fontInfo = *((font_info*)(font.fInfo));
		fInfo = (void*)fontInfo;
	} else {
		font_info *fontInfo = (font_info*)fInfo;
		*fontInfo = *((font_info*)(font.fInfo));
	}

	return *this;
}


BFont&
BFont::operator=(const e_font_desc &fontDesc)
{
	SetFamilyAndStyle(fontDesc.family, fontDesc.style);
	SetSize(fontDesc.size);
	SetSpacing(fontDesc.spacing);
	SetShear(fontDesc.shear);
	SetBoldStyle(fontDesc.bold);
	return *this;
}


bool
BFont::operator==(const BFont &font)
{
	if (fInfo == NULL && font.fInfo == NULL) return true;
	if (fInfo == NULL || font.fInfo == NULL) return false;
	font_info* fInfoA = (font_info*)fInfo;
	font_info* fInfoB = (font_info*)font.fInfo;
	return(*fInfoA == *fInfoB);
}


bool
BFont::operator!=(const BFont &font)
{
	if (fInfo == NULL && font.fInfo == NULL) return false;
	if (fInfo == NULL || font.fInfo == NULL) return true;
	font_info* fInfoA = (font_info*)fInfo;
	font_info* fInfoB = (font_info*)font.fInfo;
	return(*fInfoA != *fInfoB);
}


void
BFont::SetSize(float size)
{
	if (size <= 0) return;

	font_info *fontInfo = (font_info*)fInfo;
	if (fontInfo) fontInfo->SetSize(size);
}


float
BFont::Size() const
{
	font_info *fontInfo = (font_info*)fInfo;
	return(fontInfo ? fontInfo->Size() : -1.f);
}


void
BFont::SetSpacing(float spacing)
{
	font_info *fontInfo = (font_info*)fInfo;
	if (fontInfo) fontInfo->SetSpacing(spacing);
}


float
BFont::Spacing() const
{
	font_info *fontInfo = (font_info*)fInfo;
	return(fontInfo ? fontInfo->Spacing() : 0.f);
}


void
BFont::SetShear(float shear)
{
	if (shear < 45.f) shear = 45.f;
	else if (shear > 135.f) shear = 135.f;

	font_info *fontInfo = (font_info*)fInfo;
	if (fontInfo) fontInfo->SetShear(shear);
}


float
BFont::Shear() const
{
	font_info *fontInfo = (font_info*)fInfo;
	return(fontInfo ? fontInfo->Shear() : -1.f);
}


void
BFont::SetBoldStyle(bool bold)
{
	font_info *fontInfo = (font_info*)fInfo;
	if (fontInfo) fontInfo->SetBoldStyle(bold);
}


bool
BFont::IsBoldStyle() const
{
	font_info *fontInfo = (font_info*)fInfo;
	return(fontInfo ? fontInfo->IsBoldStyle() : false);
}


BFontEngine*
BFont::Engine() const
{
	font_info *fontInfo = (font_info*)fInfo;
	return(fontInfo ? fontInfo->Engine() : NULL);
}


bool
BFont::IsScalable() const
{
	BFontEngine *engine = Engine();
	return(engine ? engine->IsScalable() : false);
}


bool
BFont::HasFixedSize(int32 *count) const
{
	BFontEngine *engine = Engine();
	return(engine ? engine->HasFixedSize(count) : false);
}


bool
BFont::GetFixedSize(float *size, int32 index) const
{
	BFontEngine *engine = Engine();
	return(engine ? engine->GetFixedSize(size, index) : false);
}


status_t
BFont::SetFamilyAndStyle(const font_family family, const font_style style)
{
	font_info *fontInfo = (font_info*)fInfo;
	if (!fontInfo) return B_ERROR;

	BFontEngine *engine = get_font_engine(family, style);
	if (!engine) return B_ERROR;

	return(fontInfo->SetEngine(engine) ?B_OK :B_ERROR);
}


status_t
BFont::SetFamilyAndStyle(uint32 code)
{
	if (code == B_MAXUINT32) return B_BAD_VALUE;

	font_info *fontInfo = (font_info*)fInfo;
	if (!fontInfo) return B_ERROR;

	uint32 familyIndex = code >> 16;
	uint32 styleIndex = code & 0xffff;

	BFontEngine *engine = get_font_engine((int32)familyIndex, (int32)styleIndex);
	if (!engine) return B_ERROR;

	return(fontInfo->SetEngine(engine) ?B_OK :B_ERROR);
}


status_t
BFont::GetFamilyAndStyle(font_family *family, font_style *style) const
{
	if (!family || !style) return B_BAD_VALUE;

	font_info *fontInfo = (font_info*)fInfo;
	if (!fontInfo) return B_ERROR;

	uint32 code = fontInfo->FamilyAndStyle();
	if (code == B_MAXUINT32) return B_ERROR;

	uint32 familyIndex = code >> 16;
	uint32 styleIndex = code & 0xffff;

	const char *fFamily = NULL;
	const char *fStyle = NULL;
	get_font_family((int32)familyIndex, &fFamily);
	get_font_style(fFamily, (int32)styleIndex, &fStyle);

	if (!fFamily || !fStyle) return B_ERROR;

	strncpy(*family, fFamily, sizeof(font_family));
	strncpy(*style, fStyle, sizeof(font_style));

	return B_OK;
}


uint32
BFont::FamilyAndStyle() const
{
	font_info *fontInfo = (font_info*)fInfo;
	if (!fontInfo) return B_MAXUINT32;

	return fontInfo->FamilyAndStyle();
}


float
BFont::StringWidth(const char *string, int32 length, float tabWidth) const
{
	if (string == NULL || *string == 0 || length == 0) return 0;

	font_info *fontInfo = (font_info*)fInfo;
	if (!fontInfo) return 0;

	return fontInfo->StringWidth(string, length, tabWidth);
}


float
BFont::StringWidth(const BString &str, int32 length, float tabWidth) const
{
	return StringWidth(str.String(), length, tabWidth);
}


void
BFont::GetHeight(font_height *height) const
{
	if (!height) return;

	font_info *fontInfo = (font_info*)fInfo;
	if (!fontInfo) return;

	fontInfo->GetHeight(height);
}


float*
BFont::CharWidths(const char *string, int32 *nChars, float tabWidth) const
{
	if (string == NULL || *string == 0 || nChars == NULL) return NULL;

	font_info *fontInfo = (font_info*)fInfo;
	if (!fontInfo) return NULL;

	return fontInfo->CharWidths(string, -1, nChars, tabWidth);
}


float*
BFont::CharWidths(const BString &str, int32 *nChars, float tabWidth) const
{
	return CharWidths(str.String(), nChars, tabWidth);
}


float*
BFont::CharWidths(const char *string, int32 length, int32 *nChars, float tabWidth) const
{
	if (string == NULL || *string == 0 || length == 0 || nChars == NULL) return NULL;

	font_info *fontInfo = (font_info*)fInfo;
	if (!fontInfo) return NULL;

	return fontInfo->CharWidths(string, length, nChars, tabWidth);
}


float*
BFont::CharWidths(const BString &str, int32 length, int32 *nChars, float tabWidth) const
{
	return CharWidths(str.String(), length, nChars, tabWidth);
}


void
BFont::PrintToStream() const
{
	font_family family;
	font_style style;
	bzero(family, sizeof(font_family));
	bzero(style, sizeof(font_style));
	GetFamilyAndStyle(&family, &style);
	uint32 code = FamilyAndStyle();
	uint32 familyIndex = code >> 16;
	uint32 styleIndex = code & 0xffff;
	float size = Size();
	float spacing = Spacing();
	float shear = Shear();
	bool scalable = IsScalable();

	ETK_OUTPUT("\n");
	ETK_OUTPUT("Family: %s\t\tStyle: %s\t\t(%s)\n", family, style, scalable ? "Scalable" : "Not Scalable");
	ETK_OUTPUT("code = %I32u(%I32u,%I32u)\n", code, familyIndex, styleIndex);
	ETK_OUTPUT("size = %g\tspacing = %g\tshear = %g\n", size, spacing, shear);

	int32 count;
	if (HasFixedSize(&count)) {
		ETK_OUTPUT("fixed size [%I32i] --- ", count);
		for (int32 i = 0; i < count; i++) {
			float fixedSize;
			if (GetFixedSize(&fixedSize, i)) ETK_OUTPUT("%g ", fixedSize);
		}
	}

	ETK_OUTPUT("\n");
}


