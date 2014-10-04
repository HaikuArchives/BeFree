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
 * File: Pixmap.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_PIXMAP_H__
#define __ETK_PIXMAP_H__

#include <interface/Region.h>
#include <render/Render.h>

#ifdef __cplusplus /* Just for C++ */

class BPixmap : public BRender
{
	public:
		BPixmap();
		BPixmap(uint32 width, uint32 height, color_space space);
		BPixmap(BRect bounds, color_space space);
		virtual ~BPixmap();

		void*		Bits() const;
		uint32		BitsLength() const;
		uint32		BytesPerRow() const;
		color_space	ColorSpace() const;
		BRect		Bounds() const;
		void		MakeEmpty();

		bool		ResizeTo(uint32 width, uint32 height, color_space space);
		bool		ResizeTo(BRect bounds, color_space space);

		void		SetBits(const void *data, int32 length, int32 offset, color_space space);

		void		SetPixel(int32 x, int32 y, rgb_color color);
		rgb_color	GetPixel(int32 x, int32 y) const;

		void		DrawXPM(const char **xpm_data,
		              int32 destX, int32 destY,
		              int32 srcX, int32 srcY,
		              int32 srcW = -1, int32 srcH = -1,
		              uint8 alpha = 255);

	private:
		void* fPtr;
		color_space fColorSpace;
		uint32 fRows;
		uint32 fColumns;
		uint32 fRowBytes;

		virtual void *AllocData(size_t size);
		virtual void FreeData(void *data);

		virtual status_t InitCheck() const;
		virtual void GetFrame(int32 *originX, int32 *originY, uint32 *width, uint32 *height) const;
		virtual void GetPixel(int32 x, int32 y, rgb_color &color) const;
		virtual void PutPixel(int32 x, int32 y, rgb_color color);
		virtual void PutRect(int32 x, int32 y, uint32 width, uint32 height, rgb_color color);
};

#endif /* __cplusplus */

#endif /* __ETK_PIXMAP_H__ */

