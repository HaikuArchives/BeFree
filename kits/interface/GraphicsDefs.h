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
 * File: GraphicsDefs.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_GRAPHICS_DEFS_H__
#define __ETK_GRAPHICS_DEFS_H__

#include <support/SupportDefs.h>


typedef struct pattern {
	uint8		data[8];
#ifdef __cplusplus // just for C++
	inline bool operator==(const pattern& o) const {
		return (*((const uint64*)this)) == (*((const uint64*)&o));
	}

	inline bool operator!=(const pattern& o) const {
		return (*((const uint64*)this)) != (*((const uint64*)&o));
	}
#endif /* __cplusplus */
} pattern;


#ifdef __cplusplus // just for C++
inline pattern e_makpattern(uint8 d1, uint8 d2, uint8 d3, uint8 d4, uint8 d5, uint8 d6, uint8 d7, uint8 d8)
{
	pattern p;
	p.data[0] = d1;
	p.data[1] = d2;
	p.data[2] = d3;
	p.data[3] = d4;
	p.data[4] = d5;
	p.data[5] = d6;
	p.data[6] = d7;
	p.data[7] = d8;
	return p;
}
#endif /* __cplusplus */


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

	extern const pattern B_SOLID_HIGH;
	extern const pattern B_MIXED_COLORS;
	extern const pattern B_SOLID_LOW;

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */


typedef struct rgb_color {
	uint8		red;
	uint8		green;
	uint8		blue;
	uint8		alpha;

#ifdef __cplusplus // just for C++
	inline rgb_color& set_to(uint8 r, uint8 g, uint8 b, uint8 a = 0xff) {
		red = r;
		green = g;
		blue = b;
		alpha = a;
		return *this;
	}

	inline rgb_color& set_to(const rgb_color& o) {
		return set_to(o.red, o.green, o.blue, o.alpha);
	}

	inline bool operator==(const rgb_color& o) const {
		return(*((const uint32*)this)) == (*((const uint32*)&o));
	}

	inline bool operator!=(const rgb_color& o) const {
		return(*((const uint32*)this)) != (*((const uint32*)&o));
	}

	rgb_color& mix(uint8 r, uint8 g, uint8 b, uint8 a);
	rgb_color& mix(const rgb_color &o);
	rgb_color& mix_copy(uint8 r, uint8 g, uint8 b, uint8 a) const;
	rgb_color& mix_copy(const rgb_color &o) const;

	rgb_color& disable(uint8 r, uint8 g, uint8 b, uint8 a);
	rgb_color& disable(const rgb_color &background);
	rgb_color& disable_copy(uint8 r, uint8 g, uint8 b, uint8 a) const;
	rgb_color& disable_copy(const rgb_color &background) const;
#endif /* __cplusplus */
} rgb_color;


#ifdef __cplusplus // just for C++
inline rgb_color e_makrgb_color(uint8 r, uint8 g, uint8 b, uint8 a = 0xff)
{
	rgb_color c;
	c.set_to(r, g, b, a);
	return c;
}
#endif /* __cplusplus */


typedef enum e_drawing_mode {
	B_OP_COPY,
	B_OP_XOR,

	B_OP_OVER,
	B_OP_ERASE,
	B_OP_INVERT,
	B_OP_ADD,
	B_OP_SUBTRACT,
	B_OP_BLEND,
	B_OP_MIN,
	B_OP_MAX,
	B_OP_SELECT,
	B_OP_ALPHA,
} e_drawing_mode;


typedef enum color_space {
	B_CMAP8 = 0,		/* D(8) */
	B_RGB32 = 1,		/* BGRx(8:8:8:8) */
	B_RGBA32 = 2,		/* BGRA(8:8:8:8) */
	B_RGB24 = 3,		/* BGR(8:8:8) */
	B_RGB24_BIG = 4,	/* RGB(8:8:8) */
} color_space;


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

	extern const rgb_color	B_TRANSPARENT_COLOR;
	extern const uint32	E_TRANSPARENT_MAGIC_RGBA32;

	uint8 find_index_for_color(uint8 r, uint8 g, uint8 b);
	rgb_color find_color_for_index(uint8 index);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */


#endif /* __ETK_GRAPHICS_DEFS_H__ */

