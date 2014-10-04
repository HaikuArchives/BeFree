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
 * File: SupportDefs.h
 * Description: Definition for macro and type
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_SUPPORT_DEFS_H__
#define __ETK_SUPPORT_DEFS_H__

#include <string.h> /* for bzero */
#include <sys/types.h>
#include <limits.h>
#include <unistd.h>
#include <float.h>

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 8)
#define __GNUC_EXTENSION __extension__
#else
#define __GNUC_EXTENSION
#endif

#define B_MINFLOAT	FLT_MIN
#define B_MAXFLOAT	FLT_MAX
#define B_MINDOUBLE	DBL_MIN
#define B_MAXDOUBLE	DBL_MAX
#define B_MINSHORT	SHRT_MIN
#define B_MAXSHORT	SHRT_MAX
#define B_MAXUSHORT	USHRT_MAX
#define B_MININT	INT_MIN
#define B_MAXINT	INT_MAX
#define B_MAXUINT	UINT_MAX
#define B_MINLONG	LONG_MIN
#define B_MAXLONG	LONG_MAX
#define B_MAXULONG	ULONG_MAX

typedef signed char int8;
typedef unsigned char uint8;
#define B_MININT8	((int8)0x80)
#define B_MAXINT8	((int8)0x7f)
#define B_MAXUINT8	((uint8)0xff)

typedef signed short int16;
typedef unsigned short uint16;
#define unichar unsigned short
#define B_MININT16	((int16)0x8000)
#define B_MAXINT16	((int16)0x7fff)
#define B_MAXUINT16	((uint16)0xffff)

typedef signed int int32;
typedef unsigned int uint32;
typedef unsigned int unichar32;
#define B_MININT32	((int32)0x80000000)
#define B_MAXINT32	((int32)0x7fffffff)
#define B_MAXUINT32	((uint32)0xffffffff)


__GNUC_EXTENSION typedef signed long long int64;
__GNUC_EXTENSION typedef unsigned long long uint64;
#define B_INT64_CONSTANT(val)	(__GNUC_EXTENSION (val##LL))
#define B_MININT64	((int64)B_INT64_CONSTANT(0x8000000000000000))
#define B_MAXINT64	((int64)B_INT64_CONSTANT(0x7fffffffffffffff))
#define B_MAXUINT64	((uint64)B_INT64_CONSTANT(0xffffffffffffffff))

#if __WORDSIZE == 32
typedef uint32 address_t;
#elif __WORDSIZE == 64
typedef uint64 address_t;
#else
#error "Unsupported __WORDSIZE!"
#endif

#if !defined(B_MAXPATH) && defined(PATH_MAX)
	#define B_MAXPATH	PATH_MAX
#endif

typedef int32	status_t;
typedef int64	bigtime_t;
typedef int64	e_thread_id;
typedef uint32	type_code;

enum {
	B_ANY_TYPE 				= 'ANYT',
	B_BOOL_TYPE 				= 'BOOL',
	B_CHAR_TYPE 				= 'CHAR',
	B_DOUBLE_TYPE 				= 'DBLE',
	B_FLOAT_TYPE 				= 'FLOT',
	B_INT64_TYPE 				= 'LLNG',
	B_INT32_TYPE 				= 'LONG',
	B_INT16_TYPE 				= 'SHRT',
	B_INT8_TYPE 				= 'BYTE',
	B_MESSAGE_TYPE				= 'MSGG',
	B_MESSENGER_TYPE			= 'MSNG',
	B_POINTER_TYPE				= 'PNTR',
	B_SIZE_T_TYPE	 			= 'SIZT',
	B_SSIZE_T_TYPE	 			= 'SSZT',
	B_STRING_TYPE 				= 'CSTR',
	B_UINT64_TYPE				= 'ULLG',
	B_UINT32_TYPE				= 'ULNG',
	B_UINT16_TYPE 				= 'USHT',
	B_UINT8_TYPE 				= 'UBYT',
	B_POINT_TYPE				= 'SPNT',
	B_RECT_TYPE				= 'RECT',
	B_MIME_TYPE				= 'MIME'
};


#ifndef HAVE_BZERO
#define bzero(ptr, len) memset(ptr, 0, len)
#endif /* HAVE_BZERO */

#ifndef __cplusplus

typedef	int8	bool;

#ifndef false
#define false (0)
#endif

#ifndef true
#define true (!false)
#endif

#endif /* !__cplusplus */

#ifndef FALSE
#define FALSE (0)
#endif

#ifndef TRUE
#define TRUE (!FALSE)
#endif

#ifndef min_c
#define min_c(a, b)  ((a) > (b) ? (b) : (a))
#endif

#ifndef max_c
#define max_c(a, b)  ((a) > (b) ? (a) : (b))
#endif

#ifndef __cplusplus
#ifndef min
#define min(a, b) ((a) > (b) ? (b) : (a))
#endif

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif
#endif /* !__cplusplus */


#ifndef NULL
#  ifdef __cplusplus
#    define NULL        (0L)
#  else /* !__cplusplus */
#    define NULL        ((void*) 0)
#  endif /* !__cplusplus */
#endif

#ifndef _LOCAL
#  if (__GNUC__ > 3 || __GNUC__ == 3 && __GNUC_MINOR__ >= 3 && __GNUC_PATCHLEVEL__ > 3) && !defined(__MINGW32__)
#    define _LOCAL __attribute__((visibility("hidden")))
#  else
#    define _LOCAL
#  endif
#endif /* _LOCAL */

/* seek_mode */
enum {
	B_SEEK_SET = 0,
	B_SEEK_CUR,
	B_SEEK_END,
};

#ifndef __ETK_DEBUG_H__
#include <kernel/Debug.h>
#endif

#include <support/Errors.h>

#endif /* __ETK_SUPPORT_DEFS_H__ */


