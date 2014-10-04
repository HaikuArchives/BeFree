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
 * File: ByteOrder.c
 *
 * --------------------------------------------------------------------------*/

#include <support/Errors.h>

#include "ByteOrder.h"

// FIXME: remove it
#define SIZEOF_FLOAT 4
#define SIZEOF_DOUBLE 8

status_t e_swap_data(type_code type, void *_data, size_t len, e_swap_action action)
{
	status_t retVal = B_BAD_VALUE;

	if(_data == NULL || len == 0) return B_BAD_VALUE;

	switch(action)
	{
#ifdef ETK_LITTLE_ENDIAN
		case B_SWAP_HOST_TO_LENDIAN:
		case B_SWAP_LENDIAN_TO_HOST:
#else
		case B_SWAP_HOST_TO_BENDIAN:
		case B_SWAP_BENDIAN_TO_HOST:
#endif
			return B_OK;

		default:
			break;
	}

	switch(type)
	{
		case B_BOOL_TYPE:
		case B_INT8_TYPE:
		case B_UINT8_TYPE:
		case B_CHAR_TYPE:
		case B_STRING_TYPE:
		case B_MIME_TYPE:
			retVal = B_OK;
			break;

		case B_INT16_TYPE:
		case B_UINT16_TYPE:
			if(len % 2 == 0)
			{
				uint16 *data = (uint16*)_data;
				for(len = len / 2; len > 0; len--, data++) *data = B_SWAP_INT16(*data);
				retVal = B_OK;
			}
			break;

		case B_INT32_TYPE:
		case B_UINT32_TYPE:
			if(len % 4 == 0)
			{
				uint32 *data = (uint32*)_data;
				for(len = len / 4; len > 0; len--, data++) *data = B_SWAP_INT32(*data);
				retVal = B_OK;
			}
			break;

		case B_INT64_TYPE:
		case B_UINT64_TYPE:
			if(len % 8 == 0)
			{
				uint64 *data = (uint64*)_data;
				for(len = len / 8; len > 0; len--, data++) *data = B_SWAP_INT64(*data);
				retVal = B_OK;
			}
			break;

#if SIZEOF_FLOAT == 4
		case B_FLOAT_TYPE:
		case B_RECT_TYPE:
		case B_POINT_TYPE:
			if(len % 4 == 0)
			{
				float *data = (float*)_data;
				for(len = len / 4; len > 0; len--, data++) *data = B_SWAP_FLOAT(*data);
				retVal = B_OK;
			}
			break;
#endif

#if SIZEOF_DOUBLE == 8
		case B_DOUBLE_TYPE:
			if(len % 8 == 0)
			{
				double *data = (double*)_data;
				for(len = len / 8; len > 0; len--, data++) *data = B_SWAP_DOUBLE(*data);
				retVal = B_OK;
			}
			break;
#endif

		default:
			/* TODO: other types */
			break;
	}

	return retVal;
}


bool e_is_type_swapped(type_code type)
{
	switch(type)
	{
		case B_ANY_TYPE:
		case B_BOOL_TYPE:
		case B_CHAR_TYPE:
		case B_DOUBLE_TYPE:
		case B_FLOAT_TYPE:
		case B_INT64_TYPE:
		case B_INT32_TYPE:
		case B_INT16_TYPE:
		case B_INT8_TYPE:
		case B_MESSAGE_TYPE:
		case B_MESSENGER_TYPE:
		case B_POINTER_TYPE:
		case B_SIZE_T_TYPE:
		case B_SSIZE_T_TYPE:
		case B_STRING_TYPE:
		case B_UINT64_TYPE:
		case B_UINT32_TYPE:
		case B_UINT16_TYPE:
		case B_UINT8_TYPE:
		case B_POINT_TYPE:
		case B_RECT_TYPE:
		case B_MIME_TYPE:
			return true;

		default:
			return false;
	}
}


float e_swap_float(float value)
{
#if SIZEOF_FLOAT == 4
	int32 v;
	memcpy(&v, &value, 4);
	v = B_SWAP_INT32(v);
	memcpy(&value, &v, 4);
	return value;
#else
	#error "Unknown"
#endif
}


double e_swap_double(double value)
{
#if SIZEOF_DOUBLE == 8
	int64 v;
	memcpy(&v, &value, 8);
	v = B_SWAP_INT64(v);
	memcpy(&value, &v, 8);
	return value;
#else
	#error "Unknown"
#endif
}

