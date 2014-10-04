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
 * File: Debug.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_DEBUG_H__
#define __ETK_DEBUG_H__

#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>

#ifdef __cplusplus
#include <new>
#endif /* __cplusplus */

#include <support/SupportDefs.h>

#if defined(__MWERKS__) || defined(_MSC_VER)
/* for Metrowerks, etc. */
#ifndef __FUNCTION__
#define __FUNCTION__	"Unknown"
#endif
#endif

#ifndef __PRETTY_FUNCTION__
//#ifndef __GNUC__
#define __PRETTY_FUNCTION__	__FUNCTION__
//#endif /* __GNUC__ */
#endif /* __PRETTY_FUNCTION__ */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

	typedef enum debug_level {
		DEBUG_NORMAL,
		DEBUG_OUTPUT,
		DEBUG_WARNING,
		DEBUG_ERROR
	} debug_level;

	void debug_log(debug_level level, const char *format, va_list ap);

#ifndef ETK_DEBUG
#if defined(ETK_DISABLE_MORE_CHECKS)
#	if __GNUC__ >= 3 || defined(__INTEL_COMPILER)
#		define ETK_DEBUG(...)				(void)0
#	else
#		ifdef __GNUC__
#			define ETK_DEBUG(a...)			(void)0
#		else
	static void ETK_DEBUG(const char *format, ...) {
	}
#		endif
#	endif
#else
	void ETK_DEBUG(const char *format, ...);
#endif
#endif

	void ETK_OUTPUT(const char *format, ...);

#ifndef ETK_WARNING
#if defined(ETK_DISABLE_CHECKS)
#	if __GNUC__ >= 3 || defined(__INTEL_COMPILER)
#		define ETK_WARNING(...)				(void)0
#	else
#		ifdef __GNUC__
#			define ETK_WARNING(a...)		(void)0
#		else
	static void ETK_WARNING(const char *format, ...) {
	}
#		endif
#	endif
#else
	void ETK_WARNING(const char *format, ...);
#endif
#endif

	void ETK_ERROR(const char *format, ...);

#ifdef ETK_BUILD_WITH_MEMORY_TRACING
	void* calloc(size_t nmemb, size_t size, const char *file, int line, const char *method);
	void* malloc(size_t size, const char *file, int line, const char *method);
	void* realloc(void *ptr, size_t size, const char *file, int line, const char *method);
	void free(void *ptr, const char *file, int line, const char *method);
#define calloc(a, b)	calloc(a, b, __FILE__, __LINE__, "calloc")
#define malloc(a)	malloc(a, __FILE__, __LINE__, "malloc")
#define realloc(a, b)	realloc(a, b, __FILE__, __LINE__, "realloc")
#define free(a)		free(a, __FILE__, __LINE__, "free")
#endif /* ETK_BUILD_WITH_MEMORY_TRACING */

#ifdef __cplusplus
} /* extern "C" */

#ifdef ETK_BUILD_WITH_MEMORY_TRACING
struct memory_flag_t {};
void* operator new(size_t size, const char *file, int line, const char *method, struct memory_flag_t *flag);
void* operator new[](size_t size, const char *file, int line, const char *method, struct memory_flag_t *flag);
void operator delete(void *ptr, const char *file, int line, const char *method, struct memory_flag_t *flag);
void operator delete[](void *ptr, const char *file, int line, const char *method, struct memory_flag_t *flag);
#define new new(__FILE__, __LINE__, NULL, (struct memory_flag_t*)0)
#endif /* ETK_BUILD_WITH_MEMORY_TRACING */

#endif /* __cplusplus */

#endif /* __ETK_DEBUG_H__ */

