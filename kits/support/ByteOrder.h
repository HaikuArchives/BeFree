/* --------------------------------------------------------------------------
 *
 * ETK++ --- The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2007, Anthony Lee, All Rights Reserved
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
 * File: ByteOrder.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_BYTE_ORDER_H__
#define __ETK_BYTE_ORDER_H__

#include <support/SupportDefs.h>

#ifdef __cplusplus
extern "C"
{
#endif

	typedef enum e_swap_action {
		B_SWAP_HOST_TO_LENDIAN,
		B_SWAP_HOST_TO_BENDIAN,
		B_SWAP_LENDIAN_TO_HOST,
		B_SWAP_BENDIAN_TO_HOST,
		B_SWAP_ALWAYS
	} e_swap_action;

	status_t			e_swap_data(type_code type, void *data, size_t len, e_swap_action action);
	bool			e_is_type_swapped(type_code type);
	float			e_swap_float(float value);
	double			e_swap_double(double value);

#define B_SWAP_INT16(v)			((((v) & 0xff) << 8) | (((v) >> 8) & 0xff))
#define B_SWAP_INT32(v)			((B_SWAP_INT16((v) & 0xffff) << 16) |B_SWAP_INT16(((v) >> 16) & 0xffff))
#define B_SWAP_INT64(v)			((B_SWAP_INT32((v) & 0xffffffff) << 32) |B_SWAP_INT32(((v) >> 32) & 0xffffffff))
#define B_SWAP_FLOAT(v)			e_swap_float(v)
#define B_SWAP_DOUBLE(v)		e_swap_double(v)

#ifdef ETK_LITTLE_ENDIAN

#define B_HOST_IS_LENDIAN		1
#define B_HOST_IS_BENDIAN		0

#define B_HOST_TO_LENDIAN_INT16(v)	((v) & 0xffff)
#define B_HOST_TO_LENDIAN_INT32(v)	((v) & 0xffffffff)
#define B_HOST_TO_LENDIAN_INT64(v)	(v)
#define B_HOST_TO_LENDIAN_FLOAT(v)	(v)
#define B_HOST_TO_LENDIAN_DOUBLE(v)	(v)

#define B_HOST_TO_BENDIAN_INT16(v)	B_SWAP_INT16(v)
#define B_HOST_TO_BENDIAN_INT32(v)	B_SWAP_INT32(v)
#define B_HOST_TO_BENDIAN_INT64(v)	B_SWAP_INT64(v)
#define B_HOST_TO_BENDIAN_FLOAT(v)	B_SWAP_FLOAT(v)
#define B_HOST_TO_BENDIAN_DOUBLE(v)	B_SWAP_DOUBLE(v)

#define B_LENDIAN_TO_HOST_INT16(v)	((v) & 0xffff)
#define B_LENDIAN_TO_HOST_INT32(v)	((v) & 0xffffffff)
#define B_LENDIAN_TO_HOST_INT64(v)	(v)
#define B_LENDIAN_TO_HOST_FLOAT(v)	(v)
#define B_LENDIAN_TO_HOST_DOUBLE(v)	(v)

#define B_BENDIAN_TO_HOST_INT16(v)	B_SWAP_INT16(v)
#define B_BENDIAN_TO_HOST_INT32(v)	B_SWAP_INT32(v)
#define B_BENDIAN_TO_HOST_INT64(v)	B_SWAP_INT64(v)
#define B_BENDIAN_TO_HOST_FLOAT(v)	B_SWAP_FLOAT(v)
#define B_BENDIAN_TO_HOST_DOUBLE(v)	B_SWAP_DOUBLE(v)

#else

#define B_HOST_IS_LENDIAN		0
#define B_HOST_IS_BENDIAN		1

#define B_HOST_TO_LENDIAN_INT16(v)	B_SWAP_INT16(v)
#define B_HOST_TO_LENDIAN_INT32(v)	B_SWAP_INT32(v)
#define B_HOST_TO_LENDIAN_INT64(v)	B_SWAP_INT64(v)
#define B_HOST_TO_LENDIAN_FLOAT(v)	B_SWAP_FLOAT(v)
#define B_HOST_TO_LENDIAN_DOUBLE(v)	B_SWAP_DOUBLE(v)

#define B_HOST_TO_BENDIAN_INT16(v)	((v) & 0xffff)
#define B_HOST_TO_BENDIAN_INT32(v)	((v) & 0xffffffff)
#define B_HOST_TO_BENDIAN_INT64(v)	(v)
#define B_HOST_TO_BENDIAN_FLOAT(v)	(v)
#define B_HOST_TO_BENDIAN_DOUBLE(v)	(v)

#define B_LENDIAN_TO_HOST_INT16(v)	B_SWAP_INT16(v)
#define B_LENDIAN_TO_HOST_INT32(v)	B_SWAP_INT32(v)
#define B_LENDIAN_TO_HOST_INT64(v)	B_SWAP_INT64(v)
#define B_LENDIAN_TO_HOST_FLOAT(v)	B_SWAP_FLOAT(v)
#define B_LENDIAN_TO_HOST_DOUBLE(v)	B_SWAP_DOUBLE(v)

#define B_BENDIAN_TO_HOST_INT16(v)	((v) & 0xffff)
#define B_BENDIAN_TO_HOST_INT32(v)	((v) & 0xffffffff)
#define B_BENDIAN_TO_HOST_INT64(v)	(v)
#define B_BENDIAN_TO_HOST_FLOAT(v)	(v)
#define B_BENDIAN_TO_HOST_DOUBLE(v)	(v)

#endif /* ETK_LITTLE_ENDIAN */


#ifdef __cplusplus
} // extern "C"
#endif

#endif /* __ETK_BYTE_ORDER_H__ */

