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
 * File: Pixmap.cpp
 *
 * --------------------------------------------------------------------------*/

#include <string.h>
#include <stdlib.h>

#include <support/StringArray.h>

#include "Pixmap.h"


BPixmap::BPixmap()
		: BRender(), fPtr(NULL), fColorSpace((color_space)0), fRows(0), fColumns(0), fRowBytes(0)
{
}


BPixmap::BPixmap(uint32 width, uint32 height, color_space space)
		: BRender(), fPtr(NULL), fColorSpace((color_space)0), fRows(0), fColumns(0), fRowBytes(0)
{
	ResizeTo(width, height, space);
}


BPixmap::BPixmap(BRect bounds, color_space space)
		: BRender(), fPtr(NULL), fColorSpace((color_space)0), fRows(0), fColumns(0), fRowBytes(0)
{
	ResizeTo((uint32)(max_c(bounds.IntegerWidth() + 1, 0)), (uint32)(max_c(bounds.IntegerHeight() + 1, 0)), space);
}


BPixmap::~BPixmap()
{
	if (fPtr) FreeData(fPtr);
}


void
BPixmap::MakeEmpty()
{
	if (fPtr) FreeData(fPtr);
	fPtr = NULL;
}


status_t
BPixmap::InitCheck() const
{
	return(fPtr != NULL ?B_OK :B_ERROR);
}


void*
BPixmap::Bits() const
{
	return((void*)fPtr);
}


uint32
BPixmap::BitsLength() const
{
	if (!fPtr) return 0;
	return(fRowBytes * fRows);
}


uint32
BPixmap::BytesPerRow() const
{
	if (!fPtr) return 0;
	return fRowBytes;
}


color_space
BPixmap::ColorSpace() const
{
	return fColorSpace;
}


BRect
BPixmap::Bounds() const
{
	if (!fPtr) return BRect();
	return BRect(0, 0, (float)(fColumns - 1), (float)(fRows - 1));
}


bool
BPixmap::ResizeTo(uint32 width, uint32 height, color_space space)
{
	if (width == 0 || height == 0) {
		MakeEmpty();
		return true;
	}

	size_t allocSize;

	switch (space) {
		case B_CMAP8:
			allocSize = (size_t)(width * height);
			break;

		case B_RGB24:
		case B_RGB24_BIG:
			allocSize = 3 * (size_t)(width * height);
			break;

		case B_RGB32:
		case B_RGBA32:
			allocSize = 4 * (size_t)(width * height);
			break;

		default:
			allocSize = 0;
	}

	if (allocSize == 0) {
		ETK_WARNING("[RENDER]: %s --- color space(%d) not supported!", __PRETTY_FUNCTION__, space);
		return false;
	}

	void *newPtr = AllocData(allocSize);
	if (!newPtr) return false;

	if (fPtr) FreeData(fPtr);
	fPtr = newPtr;
	bzero(fPtr, allocSize);

	fRows = height;
	fColumns = width;
	fColorSpace = space;
	fRowBytes = (uint32)allocSize / fRows;

	return true;
}


bool
BPixmap::ResizeTo(BRect bounds, color_space space)
{
	return ResizeTo((uint32)(max_c(bounds.IntegerWidth() + 1, 0)), (uint32)(max_c(bounds.IntegerHeight() + 1, 0)), space);
}


void*
BPixmap::AllocData(size_t size)
{
	return(size > 0 && size <= 16000000 ? malloc(size) : NULL);
}


void
BPixmap::FreeData(void *data)
{
	if (data) free(data);
}


void
BPixmap::GetFrame(int32 *originX, int32 *originY, uint32 *width, uint32 *height) const
{
	if (originX) *originX = 0;
	if (originY) *originY = 0;
	if (width) *width = fColumns;
	if (height) *height = fRows;
}


void
BPixmap::GetPixel(int32 x, int32 y, rgb_color &color) const
{
	if (fPtr == NULL) return;

	if (x < 0 || y < 0 || (uint32)x >= fColumns || (uint32)y >= fRows) return;

	if (fColorSpace == B_RGB32 || fColorSpace == B_RGBA32) {
		const uint32 *bits = (const uint32*)fPtr;
		bits += (size_t)(fColumns * (uint32)y + (uint32)x);
#ifndef ETK_BIG_ENDIAN
		/* A-R-G-B */
		color.set_to((*bits >> 16) & 0xff, (*bits >> 8) & 0xff, *bits & 0xff,
		             fColorSpace == B_RGBA32 ? (*bits >> 24) : 0xff);
#else
		/* B-G-R-A */
		color.set_to((*bits >> 8) & 0xff, (*bits >> 16) & 0xff, *bits >> 24,
		             fColorSpace == B_RGBA32 ? (*bits & 0xff) : 0xff);
#endif
	} else if (fColorSpace == B_RGB24) {
		const uint8 *bits = (const uint8*)fPtr;
		bits += 3 * (size_t)(fColumns * (uint32)y + (uint32)x);
		color.blue = *bits++;
		color.green = *bits++;
		color.red = *bits;
		color.alpha = 0xff;
	} else if (fColorSpace == B_RGB24_BIG) {
		const uint8 *bits = (const uint8*)fPtr;
		bits += 3 * (size_t)(fColumns * (uint32)y + (uint32)x);
		color.red = *bits++;
		color.green = *bits++;
		color.blue = *bits;
		color.alpha = 0xff;
	} else if (fColorSpace == B_CMAP8) {
		const uint8 *bits = (const uint8*)fPtr;
		bits += (size_t)(fColumns * (uint32)y + (uint32)x);
		color = find_color_for_index(*bits);
	}
}


void
BPixmap::PutPixel(int32 x, int32 y, rgb_color color)
{
	if (fPtr == NULL) return;

	if (x < 0 || y < 0 || (uint32)x >= fColumns || (uint32)y >= fRows) return;

	if (fColorSpace == B_RGB32 || fColorSpace == B_RGBA32) {
		uint32 *bits = (uint32*)fPtr;
		bits += (size_t)(fColumns * (uint32)y + (uint32)x);
#ifndef ETK_BIG_ENDIAN
		/* A-R-G-B */
		*bits = ((fColorSpace == B_RGBA32 ? (uint32)color.alpha : 0x000000ff) << 24) |
		        ((uint32)color.red << 16) | ((uint32)color.green << 8) | (uint32)color.blue;
#else
		/* B-G-R-A */
		*bits = (fColorSpace == B_RGBA32 ? (uint32)color.alpha : 0x000000ff) |
		        ((uint32)color.red << 8) | ((uint32)color.green << 16) | ((uint32)color.blue << 24);
#endif
	} else if (fColorSpace == B_RGB24) {
		uint8 *bits = (uint8*)fPtr;
		bits += 3 * (size_t)(fColumns * (uint32)y + (uint32)x);

		*bits++ = color.blue;
		*bits++ = color.green;
		*bits = color.red;
	} else if (fColorSpace == B_RGB24_BIG) {
		uint8 *bits = (uint8*)fPtr;
		bits += 3 * (size_t)(fColumns * (uint32)y + (uint32)x);

		*bits++ = color.red;
		*bits++ = color.green;
		*bits = color.blue;
	} else if (fColorSpace == B_CMAP8) {
		uint8 *bits = (uint8*)fPtr;
		bits += (size_t)(fColumns * (uint32)y + (uint32)x);

		*bits = find_index_for_color(color.red, color.green, color.blue);
	}
}


void
BPixmap::PutRect(int32 x, int32 y, uint32 width, uint32 height, rgb_color color)
{
	if (fPtr == NULL) return;

	if (x < 0 || y < 0 || (uint32)x >= fColumns || (uint32)y >= fRows) return;

	width = min_c((uint32)(fColumns - x), width);
	height = min_c((uint32)(fRows - y), height);

	if (width == 0 || height == 0) return;

	if ((fColorSpace == B_RGB32 || fColorSpace == B_RGBA32) && DrawingMode() == B_OP_COPY) {
		uint32 val;
#ifndef ETK_BIG_ENDIAN
		/* A-R-G-B */
		val = ((fColorSpace == B_RGBA32 ? (uint32)color.alpha : 0x000000ff) << 24) |
		      ((uint32)color.red << 16) | ((uint32)color.green << 8) | (uint32)color.blue;
#else
		/* B-G-R-A */
		val = (fColorSpace == B_RGBA32 ? (uint32)color.alpha : 0x000000ff) |
		      ((uint32)color.red << 8) | ((uint32)color.green << 16) | ((uint32)color.blue << 24);
#endif
		for (uint32 k = 0; k < height; k++) {
			uint32 *bits = (uint32*)fPtr + (size_t)(fColumns * (uint32)(y + k) + (uint32)x);
			for (uint32 i = 0; i < width; i++) memcpy(bits++, &val, sizeof(val));
		}
	} else if (fColorSpace == B_CMAP8 && DrawingMode() == B_OP_COPY) {
		uint8 val = find_index_for_color(color.red, color.green, color.blue);
		for (uint32 k = 0; k < height; k++) {
			uint8 *bits = (uint8*)fPtr + (size_t)(fColumns * (uint32)(y + k) + (uint32)x);
			for (uint32 i = 0; i < width; i++) memcpy(bits++, &val, sizeof(val));
		}
	} else {
		for (uint32 k = 0; k < height; k++)
			for (uint32 i = 0; i < width; i++) PutPixel(x + i, y + k, color);
	}
}


void
BPixmap::SetBits(const void *data, int32 length, int32 offset, color_space space)
{
	if (data == NULL || length <= 0 || offset < 0 || !IsValid()) return;
	if (BitsLength() - (uint32)length < (uint32)offset) length = BitsLength() - (uint32)offset;
	if (length <= 0) return;

	if (fColorSpace == space) {
		memcpy((uint8*)fPtr + offset, data, (size_t)length);
	} else {
		// TODO
		ETK_WARNING("[RENDER]: %s --- color space must same as the pixmap.", __PRETTY_FUNCTION__);
	}
}


void
BPixmap::SetPixel(int32 x, int32 y, rgb_color color)
{
	PutPixel(x, y, color);
}


rgb_color
BPixmap::GetPixel(int32 x, int32 y) const
{
	rgb_color color = e_makrgb_color(0, 0, 0, 0);
	GetPixel(x, y, color);
	return color;
}


void
BPixmap::DrawXPM(const char **xpm_data, int32 destX, int32 destY, int32 srcX, int32 srcY, int32 srcW, int32 srcH, uint8 alpha)
{
	if (xpm_data == NULL || *xpm_data == NULL ||
	        destX >= (int32)fColumns || destY >= (int32)fRows || srcX < 0 || srcY < 0 || srcW == 0 || srcH == 0) return;

	BString str, tmp;
	BStringArray colors;
	int32 xpmWidth = 0, xpmHeight = 0, numColor = 0, bytesColor = 0, offset;

	str.SetTo(*xpm_data++);

	offset = 0;
	for (int32 i = 0; i < 4; i++) {
		int32 oldOffset = offset;
		offset = (i == 3 ? str.Length() : str.FindFirst(" ", offset));
		if (offset < 0) break;
		tmp.SetTo(str.String() + oldOffset, (offset++) - oldOffset);

		if (i == 0)
			tmp.GetInteger(&xpmWidth);
		else if (i == 1)
			tmp.GetInteger(&xpmHeight);
		else if (i == 2)
			tmp.GetInteger(&numColor);
		else if (i == 3)
			tmp.GetInteger(&bytesColor);
	}

//	ETK_DEBUG("[RENDER]: %s --- xpmWidth: %d, xpmHeight: %d, numColor: %d, bytesColor: %d", __PRETTY_FUNCTION__,
//		  xpmWidth, xpmHeight, numColor, bytesColor);

	if (xpmWidth <= 0 || xpmHeight <= 0 || numColor <= 0 || bytesColor <= 0) return;
	if (srcX >= xpmWidth || srcY >= xpmHeight) return;
	if (srcW < 0) srcW = xpmWidth;
	if (srcH < 0) srcH = xpmHeight;

	uint32 *colors_array = (uint32*)malloc(sizeof(uint32) * (size_t)numColor);
	if (colors_array == NULL) return;

	for (int32 i = 0; i < numColor && *xpm_data != NULL; i++, xpm_data++) {
		str.SetTo(*xpm_data);

		if ((offset = str.FindLast(" ")) < 0) offset = str.FindLast("\t");
		if (offset < bytesColor + 2 || offset == str.Length() - 1) break;

		tmp.SetTo(str.String() + offset + 1, -1);
//		ETK_DEBUG("[RENDER]: %s --- color(%d) %s", __PRETTY_FUNCTION__, i, tmp.String());

		// TODO: other colors
		if (tmp.ICompare("None") == 0) {
			*(colors_array + i) = 0xE9E9E9;
		} else {
			if (tmp.ByteAt(0) != '#' || tmp.Length() != 7) break;
			tmp.ReplaceFirst("#", "0x");
			if (tmp.GetInteger(colors_array + i) == false) break;
			if (*(colors_array + i) == 0xE9E9E9) *(colors_array + i) = 0xEAEAEA;
		}

		tmp.SetTo(str, bytesColor);
		if (colors.AddItem(tmp, colors_array + i) == false) break;
	}

	if (colors.CountItems() != numColor) {
		free(colors_array);
		return;
	}

	for (int32 j = 0; j < xpmHeight && *xpm_data != NULL; j++, xpm_data++) {
		int32 Y = destY + (j - srcY);

		if (Y < 0 || j < srcY) continue;
		if (Y >= (int32)fRows || j >= srcY + (int32)srcH) break;

		offset = 0;
		str.SetTo(*xpm_data);

		for (int32 i = 0; i < xpmWidth && offset <= str.Length() - bytesColor; i++, offset += bytesColor) {
			int32 X = destX + (i - srcX);

			if (X < 0 || i < srcX) continue;
			if (X >= (int32)fColumns || i >= srcX + (int32)srcW) break;

			tmp.SetTo(str.String() + offset, bytesColor);
			int32 *color = NULL;
			int32 found = colors.FindString(tmp);
			if (found < 0) break;
			if (colors.ItemAt(found, (void**)&color) == NULL) break;
			if (color == NULL || *color == 0xE9E9E9) continue;

			rgb_color c;
			if (alpha == 255) {
				c.set_to((*color >> 16) & 0xff, (*color >> 8) & 0xff, *color & 0xff);
			} else {
				c = GetPixel(X, Y);
				c.mix((*color >> 16) & 0xff, (*color >> 8) & 0xff, *color & 0xff, alpha);
			}

			SetPixel(X, Y, c);
		}
	}

	free(colors_array);
}

