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
 * File: String.cpp
 * Description: BString --- string allocation and manipulation
 *
 * --------------------------------------------------------------------------*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <stdarg.h>

#include "String.h"
#include "StringArray.h"

static char* strrstr(const char *haystack, const char *needle)
{
	if (!haystack || !needle) return NULL;

	size_t needle_len = strlen(needle);
	size_t haystack_len = strlen(haystack);

	if (needle_len == 0) return (char*)haystack;
	if (haystack_len < needle_len) return NULL;

	const char *tmp = haystack + haystack_len - needle_len;

	while (*tmp && tmp >= haystack) {
		if (strncmp(tmp, needle, needle_len) == 0) return (char*)tmp;
		tmp--;
	}

	return NULL;
}

static char* strrcasestr(const char *haystack, const char *needle)
{
	if (!haystack || !needle) return NULL;

	size_t needle_len = strlen(needle);
	size_t haystack_len = strlen(haystack);

	if (needle_len == 0) return (char*)haystack;
	if (haystack_len < needle_len) return NULL;

	const char *tmp = haystack + haystack_len - needle_len;

	while (*tmp && tmp >= haystack) {
		if (strncasecmp(tmp, needle, needle_len) == 0) return (char*)tmp;
		tmp--;
	}

	return NULL;
}

static char* strcasechr(const char *s, int c)
{
	if (s == NULL || *s == 0 || c == 0) return NULL;

	const char *tmp = strchr(s, tolower(c));
	if (tmp == NULL) tmp = strchr(s, toupper(c));

	return (char*)tmp;
}

static char* strrcasechr(const char *s, int c)
{
	if (s == NULL || *s == 0 || c == 0) return NULL;

	const char *tmp = strrchr(s, tolower(c));
	if (tmp == NULL) tmp = strrchr(s, toupper(c));

	return (char*)tmp;
}

char* EStrdup(const char* src, int32 length)
{
	if (src == NULL || *src == 0 || length == 0) return NULL;

	char* dest;

	int32 len = 0;

	if (length < 0)
		len = (int32)strlen(src);
	else
		len = min_c(length, (int32)strlen(src));

	dest = new char[len + 1];

	if (!dest) return NULL;

	if (len != 0) memcpy(dest, src, len);
	dest[len] = 0;

	return dest;
}

#define MAX_STRING_LENGTH	(B_MAXINT32 - 1)

bool
BString::SetMinimumBufferSize(int32 length)
{
	if (length > MAX_STRING_LENGTH + 1) return false;

	if (length <= 0 || length <= fLenReal) {
		fMinBufferSize = length;
		return true;
	}

	char *newData = (char*)realloc(fBuffer, length);
	if (!newData) return false;

	fMinBufferSize = length;
	fBuffer = newData;
	fBuffer[max_c(fLen, 0)] = 0;
	fLenReal = length;

	return true;
}


int32
BString::MinimumBufferSize() const
{
	return fMinBufferSize;
}


bool
BString::_Resize(int32 length)
{
	if (length <= 0) {
		if (fMinBufferSize > 0) {
			if (fLenReal != fMinBufferSize) {
				char *newData = (char*)realloc(fBuffer, fMinBufferSize);
				if (newData) {
					fBuffer = newData;
					fLenReal = fMinBufferSize;
				}
			}

			fLen = 0;
			if (fBuffer) fBuffer[0] = 0;
			else fLenReal = 0;
		} else {
			if (fBuffer) free(fBuffer);
			fBuffer = NULL;
			fLen = 0;
			fLenReal = 0;
		}
		return true;
	} else if (length == fLenReal - 1) {
		fLen = length;
		return true;
	} else if (length > MAX_STRING_LENGTH) {
		return false;
	} else if (length < fMinBufferSize && fMinBufferSize == fLenReal) {
		fLen = length;
		fBuffer[fLen] = 0;
		return true;
	}

	int32 length_to_alloc = max_c(length + 1, fMinBufferSize);
	char *newData = (char*)realloc(fBuffer, length_to_alloc);
	if (!newData && length + 1 < fMinBufferSize) {
		length_to_alloc = length + 1;
		newData = (char*)realloc(fBuffer, length_to_alloc);
	}

	if (!newData) {
		if (length < fLenReal) {
			fLen = length;
			fBuffer[fLen] = 0;
			return true;
		} else {
			return false;
		}
	}

	newData[length] = 0;
	fBuffer = newData;
	fLen = length;
	fLenReal = length_to_alloc;

	return true;
}


BString::BString()
		: fLen(0), fLenReal(0), fMinBufferSize(0), fBuffer(NULL)
{
}


BString::BString(const char *str)
		: fLen(0), fLenReal(0), fMinBufferSize(0), fBuffer(NULL)
{
	Append(str);
}


BString::BString(const BString &str)
		: fLen(0), fLenReal(0), fMinBufferSize(0), fBuffer(NULL)
{
	Append(str);
}


BString::BString(const char *str, int32 maxLength)
		: fLen(0), fLenReal(0), fMinBufferSize(0), fBuffer(NULL)
{
	Append(str, maxLength);
}


BString::~BString()
{
	if (fBuffer) free(fBuffer);
}


const char*
BString::String() const
{
	return fBuffer;
}


int32
BString::Length() const
{
	return fLen;
}


BString&
BString::operator=(const BString &str)
{
	this->SetTo(str);
	return *this;
}


BString&
BString::operator=(const char *str)
{
	this->SetTo(str);
	return *this;
}


BString&
BString::operator=(char c)
{
	this->SetTo(c, 1);
	return *this;
}


BString&
BString::SetTo(const BString &str)
{
	MakeEmpty();
	return Append(str);
}


BString&
BString::SetTo(const BString &str, int32 length)
{
	MakeEmpty();
	return Append(str, length);
}


BString&
BString::SetTo(const char *str)
{
	MakeEmpty();
	return Append(str);
}


BString&
BString::SetTo(const char *str, int32 length)
{
	MakeEmpty();
	return Append(str, length);
}


BString&
BString::SetTo(char c, int32 count)
{
	MakeEmpty();
	return Append(c, count);
}


BString&
BString::Adopt(BString &from)
{
	Append(from);
	from.MakeEmpty();

	return *this;
}


BString&
BString::Adopt(BString &from, int32 length)
{
	Append(from, length);
	from.MakeEmpty();

	return *this;
}


BString&
BString::CopyInto(BString &into, int32 fromOffset, int32 length) const
{
	if (this == &into) {
		BString str = *this;
		if (str == *this) str.CopyInto(into, fromOffset, length);
	} else if (fromOffset >= 0 && fromOffset < fLen) {
		into.MakeEmpty();
		into.Append(fBuffer + fromOffset, length);
	}

	return into;
}


void
BString::CopyInto(char *into, size_t into_size, int32 fromOffset, int32 length) const
{
	if (!into || into_size <= 0) return;
	if (fromOffset < 0 || fromOffset >= fLen) return;
	if (length < 0 || length > fLen - fromOffset) length = fLen - fromOffset;

	if (fBuffer && fromOffset >= 0 && fromOffset < fLen && length > 0)
		strncpy(into, fBuffer + fromOffset, min_c(into_size, (size_t)length));
	else
		into[0] = '\0';
}


BString&
BString::MoveInto(BString &into, int32 from, int32 length)
{
	if (this == &into) {
		BString str = *this;
		if (str == *this) str.MoveInto(into, from, length);
	} else if (from >= 0 && from < fLen) {
		into.MakeEmpty();
		into.Append(fBuffer + from, length);

		Remove(from, length);
	}

	return into;
}


void
BString::MoveInto(char *into, size_t into_size, int32 from, int32 length)
{
	if (!into || into_size <= 0) return;
	if (from < 0 || from >= fLen) return;
	if (length < 0 || length > fLen - from) length = fLen - from;

	if (fBuffer && from >= 0 && from < fLen) {
		strncpy(into, fBuffer + from, min_c(into_size, (size_t)length));
		Remove(from, length);
	} else {
		into[0] = '\0';
	}
}


void
BString::MakeEmpty()
{
	Remove(0, -1);
}


BString&
BString::operator+=(const BString &str)
{
	return Append(str);
}


BString&
BString::operator+=(const char *str)
{
	return Append(str);
}


BString&
BString::operator+=(char c)
{
	return Append(c, 1);
}


BString&
BString::Append(const BString &str)
{
	return Append(str, str.Length());
}


BString&
BString::Append(const BString &str, int32 length)
{
	if (str.Length() < 1 || length == 0) return *this;
	if (length < 0) length = str.Length();

	return Append(str.String(), length);
}


BString&
BString::Append(const char *str)
{
	if (str) Append(str, -1);

	return *this;
}


BString&
BString::Append(const char *str, int32 length)
{
	if (str == NULL || *str == 0 || length == 0) return *this;

	int32 strLen = (int32)strlen(str);

	if (length < 0 || length > strLen) length = strLen;
	if (MAX_STRING_LENGTH - fLen < length) return *this;

	if (_Resize(fLen + length)) {
		if (memcpy(fBuffer + fLen - length, str, length) == NULL) {
			fLen -= length;
		}
	}

	return *this;
}


BString&
BString::Append(char c, int32 count)
{
	if (c == 0 || MAX_STRING_LENGTH - fLen < count || count <= 0) return *this;

	if (_Resize(fLen + count)) {
		if (memset(fBuffer + fLen - count, (int)c, count) == NULL) {
			fLen -= count;
		}
	}

	return *this;
}


BString&
BString::Prepend(const BString &str)
{
	return Prepend(str, str.Length());
}


BString&
BString::Prepend(const BString &str, int32 length)
{
	if (str.Length() < 1 || length == 0) return *this;
	if (length < 0) length = str.Length();

	return Prepend(str.String(), length);
}


BString&
BString::Prepend(const char *str)
{
	if (str) Prepend(str, -1);

	return *this;
}


BString&
BString::Prepend(const char *str, int32 length)
{
	return Insert(str, length, 0);
}


BString&
BString::Prepend(char c, int32 count)
{
	return Insert(c, count, 0);
}


BString&
BString::Insert(const BString &str, int32 pos)
{
	return Insert(str, str.Length(), pos);
}


BString&
BString::Insert(const BString &str, int32 length, int32 pos)
{
	return Insert(str, 0, length, pos);
}


BString&
BString::Insert(const BString &str, int32 fromOffset, int32 length, int32 pos)
{
	return Insert(str.String(), fromOffset, length, pos);
}


BString&
BString::Insert(const char *str, int32 pos)
{
	if (str) Insert(str, -1, pos);
	return *this;
}


BString&
BString::Insert(const char *str, int32 length, int32 pos)
{
	return Insert(str, 0, length, pos);
}


BString&
BString::Insert(const char *str, int32 fromOffset, int32 length, int32 pos)
{
	if (str == NULL || *str == 0 || length == 0 || fromOffset < 0) return *this;

	int32 strLen = (int32)strlen(str);
	if (fromOffset >= strLen) return *this;

	if ((pos == 0 && fLen <= 0) || (pos > 0 && pos == fLen)) return Append(str + fromOffset, length);
	if (pos < 0 || pos >= fLen) return *this;

	if (length < 0 || length > strLen - fromOffset) length = strLen - fromOffset;

	if (MAX_STRING_LENGTH - fLen < length) return *this;

	if (!_Resize(fLen + length)) return *this;

	if (memmove(fBuffer + pos + length, fBuffer + pos, fLen - length - pos) == NULL) {
		fLen -= length;
		return *this;
	} else if (memcpy(fBuffer + pos, str + fromOffset, length) == NULL) {
		fLen -= length;
	}

	return *this;
}


BString&
BString::Insert(char c, int32 count, int32 pos)
{
	if ((pos == 0 && fLen <= 0) || (pos > 0 && pos == fLen)) return Append(c, count);

	if (pos < 0 || pos >= fLen) return *this;
	if (c == 0 || MAX_STRING_LENGTH - fLen < count || count <= 0) return *this;

	if (!_Resize(fLen + count)) return *this;

	if (memmove(fBuffer + pos + count, fBuffer + pos, fLen - count - pos) == NULL) {
		fLen -= count;
		return *this;
	} else if (memset(fBuffer + pos, (int)c, count) == NULL) {
		fLen -= count;
	}

	return *this;
}


BString&
BString::Truncate(int32 newLength)
{
	if (!(newLength < 0 || newLength >= fLen)) _Resize(newLength);

	return *this;
}


BString&
BString::Remove(int32 from, int32 length)
{
	if (from < 0 || from >= fLen) return *this;

	if (length == 0) return *this;
	else if (length < 0) length = fLen - from;
	else length = min_c(fLen - from, length);

	if (from < (fLen - 1) && length != (fLen - from)) {
		if (memmove(fBuffer + from, fBuffer + from + length, fLen - from - length) == NULL) return *this;
	}

	_Resize(fLen - length);

	return *this;
}


BString&
BString::RemoveFirst(const BString &str)
{
	return RemoveFirst(str.String());
}


BString&
BString::RemoveLast(const BString &str)
{
	return RemoveLast(str.String());
}


BString&
BString::RemoveAll(const BString &str)
{
	return RemoveAll(str.String());
}


BString&
BString::RemoveFirst(const char *str)
{
	if (str == NULL || *str == 0) return *this;

	const char *s = String();
	if (s) {
		const char *tmp = strstr(s, str);
		if (tmp) {
			int32 start = (int32)(tmp - s);
			Remove(start, (int32)strlen(str));
		}
	}

	return *this;
}


BString&
BString::RemoveLast(const char *str)
{
	if (str == NULL || *str == 0) return *this;

	const char *s = String();
	if (s) {
		const char *tmp = strrstr(s, str);
		if (tmp) {
			int32 start = (int32)(tmp - s);
			Remove(start, (int32)strlen(str));
		}
	}

	return *this;
}


BString&
BString::RemoveAll(const char *str)
{
	if (str == NULL || *str == 0) return *this;

	const char *s = String();
	if (s) {
		const char *tmp;
		while ((tmp = strstr(s, str)) != NULL) {
			int32 start = (int32)(tmp - s);
			Remove(start, (int32)strlen(str));
			s = String();

			if (!s) break;
		}
	}

	return *this;
}


BString&
BString::RemoveSet(const char *setOfCharsToRemove)
{
	if (setOfCharsToRemove == NULL || *setOfCharsToRemove == 0) return *this;

	int32 len = (int32)strlen(setOfCharsToRemove);

	char str[2];
	str[1] = '\0';

	for (int32 i = 0; i < len; i++) {
		str[0] = setOfCharsToRemove[i];
		RemoveAll(str);

		if (Length() == 0) break;
	}

	return *this;
}


BString&
BString::IRemoveFirst(const BString &str)
{
	return IRemoveFirst(str.String());
}


BString&
BString::IRemoveLast(const BString &str)
{
	return IRemoveLast(str.String());
}


BString&
BString::IRemoveAll(const BString &str)
{
	return IRemoveAll(str.String());
}


BString&
BString::IRemoveFirst(const char *str)
{
	if (str == NULL || *str == 0) return *this;

	const char *s = String();
	if (s) {
		const char *tmp = strcasestr(s, str);
		if (tmp) {
			int32 start = (int32)(tmp - s);
			Remove(start, (int32)strlen(str));
		}
	}

	return *this;
}


BString&
BString::IRemoveLast(const char *str)
{
	if (str == NULL || *str == 0) return *this;

	const char *s = String();
	if (s) {
		const char *tmp = strrcasestr(s, str);
		if (tmp) {
			int32 start = (int32)(tmp - s);
			Remove(start, (int32)strlen(str));
		}
	}

	return *this;
}


BString&
BString::IRemoveAll(const char *str)
{
	if (str == NULL || *str == 0) return *this;

	const char *s = String();
	if (s) {
		const char *tmp;
		while ((tmp = strcasestr(s, str)) != NULL) {
			int32 start = (int32)(tmp - s);
			Remove(start, (int32)strlen(str));
			s = String();

			if (!s) break;
		}
	}

	return *this;
}


BString&
BString::IRemoveSet(const char *setOfCharsToRemove)
{
	if (setOfCharsToRemove == NULL || *setOfCharsToRemove == 0) return *this;

	int32 len = (int32)strlen(setOfCharsToRemove);

	char str[2];
	str[1] = 0;

	for (int32 i = 0; i < len; i++) {
		str[0] = setOfCharsToRemove[i];
		IRemoveAll(str);

		if (Length() == 0) break;
	}

	return *this;
}


bool
BString::operator<(const BString &str) const
{
	return(Compare(str) < 0);
}


bool
BString::operator<=(const BString &str) const
{
	return(Compare(str) <= 0);
}


bool
BString::operator==(const BString &str) const
{
	const char *a = String();
	const char *b = str.String();

	if (a == NULL && b == NULL) return true;
	if (a == NULL || b == NULL) return false;

	return(!(strlen(a) != strlen(b) || strcmp(a, b) != 0));
}


bool
BString::operator>=(const BString &str) const
{
	return(Compare(str) >= 0);
}


bool
BString::operator>(const BString &str) const
{
	return(Compare(str) > 0);
}


bool
BString::operator!=(const BString &str) const
{
	return !operator==(str);
}


bool
BString::operator<(const char *str) const
{
	return(Compare(str) < 0);
}


bool
BString::operator<=(const char *str) const
{
	return(Compare(str) <= 0);
}


bool
BString::operator==(const char *str) const
{
	const char *a = String();
	const char *b = str;

	if (a == NULL && b == NULL) return true;
	if (a == NULL || b == NULL) return false;

	return(!(strlen(a) != strlen(b) || strcmp(a, b) != 0));
}


bool
BString::operator>=(const char *str) const
{
	return(Compare(str) >= 0);
}


bool
BString::operator>(const char *str) const
{
	return(Compare(str) > 0);
}


bool
BString::operator!=(const char *str) const
{
	return !operator==(str);
}


int
BString::Compare(const BString &str) const
{
	const char *a = "";
	const char *b = "";

	if (String()) a = String();
	if (str.String()) b = str.String();

	return strcmp(a, b);
}


int
BString::Compare(const char *str) const
{
	const char *a = "";
	const char *b = "";

	if (String()) a = String();
	if (str) b = str;

	return strcmp(a, b);
}


int
BString::Compare(const BString &str, int32 n) const
{
	const char *a = "";
	const char *b = "";

	if (String()) a = String();
	if (str.String()) b = str.String();

	return strncmp(a, b, (size_t)n);
}


int
BString::Compare(const char *str, int32 n) const
{
	const char *a = "";
	const char *b = "";

	if (String()) a = String();
	if (str) b = str;

	return strncmp(a, b, (size_t)n);
}


int
BString::ICompare(const BString &str) const
{
	const char *a = "";
	const char *b = "";

	if (String()) a = String();
	if (str.String()) b = str.String();

	return strcasecmp(a, b);
}


int
BString::ICompare(const char *str) const
{
	const char *a = "";
	const char *b = "";

	if (String()) a = String();
	if (str) b = str;

	return strcasecmp(a, b);
}


int
BString::ICompare(const BString &str, int32 n) const
{
	const char *a = "";
	const char *b = "";

	if (String()) a = String();
	if (str.String()) b = str.String();

	return strncasecmp(a, b, (size_t)n);
}


int
BString::ICompare(const char *str, int32 n) const
{
	const char *a = "";
	const char *b = "";

	if (String()) a = String();
	if (str) b = str;

	return strncasecmp(a, b, (size_t)n);
}


int32
BString::FindFirst(const BString &string) const
{
	if (String() == NULL || string.String() == NULL) return -1;

	const char *tmp = strstr(String(), string.String());

	if (tmp == NULL) return -1;

	return((int32)(tmp - String()));
}


int32
BString::FindFirst(const char *string) const
{
	if (String() == NULL || string == NULL || *string == 0) return -1;

	const char *tmp = strstr(String(), string);

	if (tmp == NULL) return -1;

	return((int32)(tmp - String()));
}


int32
BString::FindFirst(const BString &string, int32 fromOffset) const
{
	if (String() == NULL || string.String() == NULL || fromOffset < 0 || fromOffset >= Length()) return -1;

	const char *string_find = String() + fromOffset;
	const char *tmp = strstr(string_find, string.String());

	if (tmp == NULL) return -1;

	return((int32)(tmp - String()));
}


int32
BString::FindFirst(const char *string, int32 fromOffset) const
{
	if (String() == NULL || string == NULL || *string == 0 || fromOffset < 0 || fromOffset >= Length()) return -1;

	const char *string_find = String() + fromOffset;
	const char *tmp = strstr(string_find, string);

	if (tmp == NULL) return -1;

	return((int32)(tmp - String()));
}


int32
BString::FindFirst(char c) const
{
	if (String() == NULL || c == 0) return -1;

	const char *tmp = strchr(String(), c);

	if (tmp == NULL) return -1;

	return((int32)(tmp - String()));
}


int32
BString::FindFirst(char c, int32 fromOffset) const
{
	if (String() == NULL || c == 0 || fromOffset < 0 || fromOffset >= Length()) return -1;

	const char *string_find = String() + fromOffset;
	const char *tmp = strchr(string_find, c);

	if (tmp == NULL) return -1;

	return((int32)(tmp - String()));
}


int32
BString::FindLast(const BString &string) const
{
	if (String() == NULL || string.String() == NULL) return -1;

	const char *tmp = strrstr(String(), string.String());

	if (tmp == NULL) return -1;

	return((int32)(tmp - String()));
}


int32
BString::FindLast(const char *string) const
{
	if (String() == NULL || string == NULL || *string == 0) return -1;

	const char *tmp = strrstr(String(), string);

	if (tmp == NULL) return -1;

	return((int32)(tmp - String()));
}


int32
BString::FindLast(const BString &string, int32 beforeOffset) const
{
	if (String() == NULL || string.String() == NULL || beforeOffset < 0 || beforeOffset >= Length()) return -1;

	char *string_find = b_strndup(String(), beforeOffset + 1);
	if (string_find == NULL) return -1;

	const char *tmp = strrstr(string_find, string.String());

	int32 ret = -1;
	if (tmp != NULL) ret = (int32)(tmp - string_find);

	free(string_find);

	return ret;
}


int32
BString::FindLast(const char *string, int32 beforeOffset) const
{
	if (String() == NULL || string == NULL || *string == 0 || beforeOffset < 0 || beforeOffset >= Length()) return -1;

	char *string_find = b_strndup(String(), beforeOffset + 1);
	if (string_find == NULL) return -1;

	const char *tmp = strrstr(string_find, string);

	int32 ret = -1;
	if (tmp != NULL) ret = (int32)(tmp - string_find);

	free(string_find);

	return ret;
}


int32
BString::FindLast(char c) const
{
	if (String() == 0 || c == 0) return -1;

	const char *tmp = strrchr(String(), c);

	if (tmp == NULL) return -1;

	return((int32)(tmp - String()));
}


int32
BString::FindLast(char c, int32 beforeOffset) const
{
	if (String() == NULL || c == 0 || beforeOffset < 0 || beforeOffset >= Length()) return -1;

	char *string_find = b_strndup(String(), beforeOffset + 1);
	if (string_find == NULL) return -1;

	const char *tmp = strrchr(string_find, c);

	int32 ret = -1;
	if (tmp != NULL) ret = (int32)(tmp - string_find);

	free(string_find);

	return ret;
}


int32
BString::IFindFirst(const BString &string) const
{
	if (String() == NULL || string.String() == NULL) return -1;

	const char *tmp = strcasestr(String(), string.String());

	if (tmp == NULL) return -1;

	return((int32)(tmp - String()));
}


int32
BString::IFindFirst(const char *string) const
{
	if (String() == NULL || string == NULL || *string == 0) return -1;

	const char *tmp = strcasestr(String(), string);

	if (tmp == NULL) return -1;

	return((int32)(tmp - String()));
}


int32
BString::IFindFirst(const BString &string, int32 fromOffset) const
{
	if (String() == NULL || string.String() == NULL || fromOffset < 0 || fromOffset >= Length()) return -1;

	const char *string_find = String() + fromOffset;
	const char *tmp = strcasestr(string_find, string.String());

	if (tmp == NULL) return -1;

	return((int32)(tmp - String()));
}


int32
BString::IFindFirst(const char *string, int32 fromOffset) const
{
	if (String() == NULL || string == NULL || *string == 0 || fromOffset < 0 || fromOffset >= Length()) return -1;

	const char *string_find = String() + fromOffset;
	const char *tmp = strcasestr(string_find, string);

	if (tmp == NULL) return -1;

	return((int32)(tmp - String()));
}


int32
BString::IFindFirst(char c) const
{
	if (String() == NULL || c == 0) return -1;

	const char *tmp = strcasechr(String(), c);

	if (tmp == NULL) return -1;

	return((int32)(tmp - String()));
}


int32
BString::IFindFirst(char c, int32 fromOffset) const
{
	if (String() == NULL || c == 0 || fromOffset < 0 || fromOffset >= Length()) return -1;

	const char *string_find = String() + fromOffset;
	const char *tmp = strcasechr(string_find, c);

	if (tmp == NULL) return -1;

	return((int32)(tmp - String()));
}


int32
BString::IFindLast(const BString &string) const
{
	if (String() == NULL || string.String() == NULL) return -1;

	const char *tmp = strrcasestr(String(), string.String());

	if (tmp == NULL) return -1;

	return((int32)(tmp - String()));
}


int32
BString::IFindLast(const char *string) const
{
	if (String() == NULL || string == NULL || *string == 0) return -1;

	const char *tmp = strrcasestr(String(), string);

	if (tmp == NULL) return -1;

	return((int32)(tmp - String()));
}


int32
BString::IFindLast(const BString &string, int32 beforeOffset) const
{
	if (String() == NULL || string.String() == NULL || beforeOffset < 0 || beforeOffset >= Length()) return -1;

	char *string_find = b_strndup(String(), beforeOffset + 1);
	if (string_find == NULL) return -1;

	const char *tmp = strrcasestr(string_find, string.String());

	int32 ret = -1;
	if (tmp != NULL) ret = (int32)(tmp - string_find);

	free(string_find);

	return ret;
}


int32
BString::IFindLast(const char *string, int32 beforeOffset) const
{
	if (String() == NULL || string == NULL || *string == 0 || beforeOffset < 0 || beforeOffset >= Length()) return -1;

	char *string_find = b_strndup(String(), beforeOffset + 1);
	if (string_find == NULL) return -1;

	const char *tmp = strrcasestr(string_find, string);

	int32 ret = -1;
	if (tmp != NULL) ret = (int32)(tmp - string_find);

	free(string_find);

	return ret;
}


int32
BString::IFindLast(char c) const
{
	if (String() == NULL || c == 0) return -1;

	const char *tmp = strrcasechr(String(), c);

	if (tmp == NULL) return -1;

	return((int32)(tmp - String()));
}


int32
BString::IFindLast(char c, int32 beforeOffset) const
{
	if (String() == NULL || c == 0 || beforeOffset < 0 || beforeOffset >= Length()) return -1;

	char *string_find = b_strndup(String(), beforeOffset + 1);
	if (string_find == NULL) return -1;

	const char *tmp = strrcasechr(string_find, c);

	int32 ret = -1;
	if (tmp != NULL) ret = (int32)(tmp - string_find);

	free(string_find);

	return ret;
}


BString&
BString::ReplaceFirst(char replaceThis, char withThis)
{
	int32 index = FindFirst(replaceThis);

	if (index >= 0) fBuffer[index] = withThis;

	return *this;
}


BString&
BString::ReplaceLast(char replaceThis, char withThis)
{
	int32 index = FindLast(replaceThis);

	if (index >= 0) fBuffer[index] = withThis;

	return *this;
}


BString&
BString::ReplaceAll(char replaceThis, char withThis, int32 fromOffset)
{
	if (fromOffset < 0 || fromOffset >= fLen) return *this;

	while (fromOffset < fLen) {
		int32 index = FindFirst(replaceThis, fromOffset);

		if (index >= 0) {
			fBuffer[index] = withThis;
			fromOffset = index + 1;
		} else break;
	}

	return *this;
}


BString&
BString::ReplaceFirst(const char *replaceThis, const char *withThis)
{
	if (replaceThis == NULL || *replaceThis == 0 || withThis == NULL || *withThis == 0) return *this;

	int32 index = FindFirst(replaceThis);

	if (index >= 0) {
		Remove(index, (int32)strlen(replaceThis));
		Insert(withThis, index);
	}

	return *this;
}


BString&
BString::ReplaceLast(const char *replaceThis, const char *withThis)
{
	if (replaceThis == NULL || *replaceThis == 0 || withThis == NULL || *withThis == 0) return *this;

	int32 index = FindLast(replaceThis);

	if (index >= 0) {
		Remove(index, (int32)strlen(replaceThis));
		Insert(withThis, index);
	}

	return *this;
}


BString&
BString::ReplaceAll(const char *replaceThis, const char *withThis, int32 fromOffset)
{
	if (fromOffset < 0 || fromOffset >= fLen) return *this;
	if (replaceThis == NULL || *replaceThis == 0 || withThis == NULL || *withThis == 0) return *this;

	int32 strLenReplace = (int32)strlen(replaceThis);
	int32 strLenWith = (int32)strlen(withThis);

	while (fromOffset < fLen) {
		int32 index = FindFirst(replaceThis, fromOffset);

		if (index >= 0) {
			Remove(index, strLenReplace);
			Insert(withThis, index);
			fromOffset = index + strLenWith;
		} else break;
	}

	return *this;
}


BString&
BString::Replace(const char *replaceThis, const char *withThis, int32 maxReplaceCount, int32 fromOffset)
{
	if (fromOffset < 0 || fromOffset >= fLen) return *this;
	if (replaceThis == NULL || *replaceThis == 0 || withThis == NULL || *withThis == 0) return *this;

	if (maxReplaceCount == 0) return *this;
	if (maxReplaceCount < 0) return ReplaceAll(replaceThis, withThis, fromOffset);

	int count = 0;
	int32 strLenReplace = (int32)strlen(replaceThis);
	int32 strLenWith = (int32)strlen(withThis);

	while (true) {
		int32 index = FindFirst(replaceThis, fromOffset);

		if (index >= 0) {
			Remove(index, strLenReplace);
			Insert(withThis, index);
			fromOffset = index + strLenWith;
			count++;
		} else break;

		if (count >= maxReplaceCount) break;
	}

	return *this;
}


BString&
BString::IReplaceFirst(char replaceThis, char withThis)
{
	int32 index = IFindFirst(replaceThis);

	if (index >= 0) fBuffer[index] = withThis;

	return *this;
}


BString&
BString::IReplaceLast(char replaceThis, char withThis)
{
	int32 index = IFindLast(replaceThis);

	if (index >= 0) fBuffer[index] = withThis;

	return *this;
}


BString&
BString::IReplaceAll(char replaceThis, char withThis, int32 fromOffset)
{
	if (fromOffset < 0 || fromOffset >= fLen) return *this;

	while (fromOffset < fLen) {
		int32 index = IFindFirst(replaceThis, fromOffset);

		if (index >= 0) {
			fBuffer[index] = withThis;
			fromOffset = index + 1;
		} else break;
	}

	return *this;
}


BString&
BString::IReplaceFirst(const char *replaceThis, const char *withThis)
{
	if (replaceThis == NULL || *replaceThis == 0 || withThis == NULL || *withThis == 0) return *this;

	int32 index = IFindFirst(replaceThis);

	if (index >= 0) {
		Remove(index, (int32)strlen(replaceThis));
		Insert(withThis, index);
	}

	return *this;
}


BString&
BString::IReplaceLast(const char *replaceThis, const char *withThis)
{
	if (replaceThis == NULL || *replaceThis == 0 || withThis == NULL || *withThis == 0) return *this;

	int32 index = IFindLast(replaceThis);

	if (index >= 0) {
		Remove(index, (int32)strlen(replaceThis));
		Insert(withThis, index);
	}

	return *this;
}


BString&
BString::IReplaceAll(const char *replaceThis, const char *withThis, int32 fromOffset)
{
	if (fromOffset < 0 || fromOffset >= fLen) return *this;
	if (replaceThis == NULL || *replaceThis == 0 || withThis == NULL || *withThis == 0) return *this;

	int32 strLenReplace = (int32)strlen(replaceThis);
	int32 strLenWith = (int32)strlen(withThis);

	while (fromOffset < fLen) {
		int32 index = IFindFirst(replaceThis, fromOffset);

		if (index >= 0) {
			Remove(index, strLenReplace);
			Insert(withThis, index);
			fromOffset = index + strLenWith;
		} else break;
	}

	return *this;
}


BString&
BString::IReplace(const char *replaceThis, const char *withThis, int32 maxReplaceCount, int32 fromOffset)
{
	if (fromOffset < 0 || fromOffset >= fLen) return *this;
	if (replaceThis == NULL || *replaceThis == 0 || withThis == NULL || *withThis == 0) return *this;

	if (maxReplaceCount == 0) return *this;
	if (maxReplaceCount < 0) return IReplaceAll(replaceThis, withThis, fromOffset);

	int count = 0;
	int32 strLenReplace = (int32)strlen(replaceThis);
	int32 strLenWith = (int32)strlen(withThis);

	while (true) {
		int32 index = IFindFirst(replaceThis, fromOffset);

		if (index >= 0) {
			Remove(index, strLenReplace);
			Insert(withThis, index);
			fromOffset = index + strLenWith;
			count++;
		} else break;

		if (count >= maxReplaceCount) break;
	}

	return *this;
}


BString&
BString::ReplaceSet(const char *set_to_replace, char with)
{
	if (set_to_replace == NULL || *set_to_replace == 0 || with == 0) return *this;

	int32 length = (int32)strlen(set_to_replace);
	int32 offset = 0;

	while (true) {
		int32 index = -1;
		int32 i;

		for (i = 0; i < length; i++) {
			index = FindFirst(set_to_replace[i], offset);
			if (index >= 0) break;
		}

		if (index >= 0) {
			Remove(index, 1);
			Insert(with, 1, index);
			offset = index + 1;
		} else break;
	}

	return *this;
}


BString&
BString::ReplaceSet(const char *set_to_replace, const char *with)
{
	if (set_to_replace == NULL || *set_to_replace == 0 || with == NULL || *with == 0) return *this;

	int32 length = (int32)strlen(set_to_replace);
	int32 strLenWith = (int32)strlen(with);
	int32 offset = 0;

	while (true) {
		int32 index = -1;
		int32 i;

		for (i = 0; i < length; i++) {
			index = FindFirst(set_to_replace[i], offset);
			if (index >= 0) break;
		}

		if (index >= 0) {
			Remove(index, 1);
			Insert(with, index);
			offset = index + strLenWith;
		} else break;
	}

	return *this;
}



BString&
BString::IReplaceSet(const char *set_to_replace, char with)
{
	if (set_to_replace == NULL || *set_to_replace == 0 || with == 0) return *this;

	int32 length = (int32)strlen(set_to_replace);
	int32 offset = 0;

	while (true) {
		int32 index = -1;
		int32 i;

		for (i = 0; i < length; i++) {
			index = IFindFirst(set_to_replace[i], offset);
			if (index >= 0) break;
		}

		if (index >= 0) {
			Remove(index, 1);
			Insert(with, 1, index);
			offset = index + 1;
		} else break;
	}

	return *this;
}


BString&
BString::IReplaceSet(const char *set_to_replace, const char *with)
{
	if (set_to_replace == NULL || *set_to_replace == 0 || with == NULL || *with == 0) return *this;

	int32 length = (int32)strlen(set_to_replace);
	int32 strLenWith = (int32)strlen(with);
	int32 offset = 0;

	while (true) {
		int32 index = -1;
		int32 i;

		for (i = 0; i < length; i++) {
			index = IFindFirst(set_to_replace[i], offset);
			if (index >= 0) break;
		}

		if (index >= 0) {
			Remove(index, 1);
			Insert(with, index);
			offset = index + strLenWith;
		} else break;
	}

	return *this;
}


char
BString::operator[](int32 index) const
{
	return fBuffer[index];
}


char
BString::ByteAt(int32 index) const
{
	if (!fBuffer || index < 0 || index >= Length()) return 0;
	return fBuffer[index];
}


BString&
BString::ToLower()
{
	for (int32 i = 0; i < fLen; i++) fBuffer[i] = tolower(fBuffer[i]);

	return *this;
}


BString&
BString::ToUpper()
{
	for (int32 i = 0; i < fLen; i++) fBuffer[i] = toupper(fBuffer[i]);

	return *this;
}


BString&
BString::CharacterEscape(const char *original, const char *setOfCharsToEscape, char escapeWith)
{
	SetTo(original);
	return CharacterEscape(setOfCharsToEscape, escapeWith);
}


BString&
BString::CharacterEscape(const char *setOfCharsToEscape, char escapeWith)
{
	if (Length() <= 0 || setOfCharsToEscape == NULL || *setOfCharsToEscape == 0 || escapeWith == 0) return *this;

	int32 length = (int32)strlen(setOfCharsToEscape);
	int32 offset = 0;

	while (true) {
		int32 index = -1;
		int32 i;

		for (i = 0; i < length; i++) {
			index = FindFirst(setOfCharsToEscape[i], offset);
			if (index >= 0) break;
		}

		if (index >= 0) {
			Insert(escapeWith, 1, index);
			offset = index + 2;
		} else break;
	}

	return *this;
}


BString&
BString::CharacterDeescape(const char *original, char escapeChar)
{
	SetTo(original);
	return CharacterDeescape(escapeChar);
}


BString&
BString::CharacterDeescape(char escapeChar)
{
	if (Length() <= 0 || escapeChar == 0) return *this;

	int32 offset = 0;

	while (true) {
		int32 index = -1;

		index = FindFirst(escapeChar, offset);

		if (index >= 0) {
			Remove(index, 1);
			offset = index + 1;
		} else break;
	}

	return *this;
}


BString&
BString::Capitalize()
{
	ToLower();

	if (Length() > 0) fBuffer[0] = toupper(fBuffer[0]);

	return *this;
}


BString&
BString::CapitalizeEachWord()
{
	ToLower();

	int32 length = Length();
	if (length > 0) {
		if (fBuffer[0] != ' ') fBuffer[0] = toupper(fBuffer[0]);

		char *src = fBuffer;

		while (true) {
			char *tmp = strchr(src, ' ');

			if (!tmp) break;
			if (tmp - fBuffer >= length - 1) break;

			tmp++;
			if (*tmp != ' ') {
				*tmp = toupper(*tmp);
				src = (tmp + 1);
			} else {
				src = (tmp + 2);
			}
		}
	}

	return *this;
}


bool
BString::IsNumber() const
{
	const char *s = String();
	int32 length = Length();

	if (s == NULL) return false;
	if (length < 1) return false;

	if (!(length < 2 || s[0] != '0' || !(s[1] == 'x' || s[1] == 'X'))) {
		int32 i;
		for (i = 2; i < length; i++) {
			if (s[i] >= '0' && s[i] <= '9') continue;
			if (s[i] >= 'a' && s[i] <= 'f') continue;
			if (s[i] >= 'A' && s[i] <= 'F') continue;
			break;
		}

		return(i != 2 && i >= length);
	}

	if (s[0] == '-' || s[0] == '+') {
		if (length < 2) return false;
		if (strchr(&s[1], '-') != NULL || strchr(&s[1], '+') != NULL) return false; // double '-' or '+'
	}

	const char *tmp = strchr(s, '.');
	if (tmp != NULL) {
		tmp++;
		if (strchr(tmp, '.') != NULL) return false; // double '.'
	}

	for (int32 i = 0; i < length; i++) {
		if (!(isdigit(s[i]) || s[i] == '-' || s[i] == '.' || s[i] == '+')) {
			if (s[i] != 'e' || s[i] != 'E' || i != length - 4) return false;
			if (s[i + 1] != '+' || s[i + 1] != '-') return false;
			if (!isdigit(s[i + 2]) || !isdigit(s[i + 3])) return false;
			break;
		}
	}

	return true;
}


bool
BString::IsInteger() const
{
	if (!IsNumber()) return false;

	const char *s = strchr(String(), '.');

	if (s == NULL) return true;

	if (strchr(s, '.') != NULL) {
		s++;
		int32 length = (int32)strlen(s);
		if (length > 0) {
			for (int32 i = 0; i < length; i++) {
				if (s[i] != '0') return false;
			}
		}
	}

	return true;
}


bool
BString::IsDecimal() const
{
	if (!IsNumber()) return false;

	return(!IsInteger());
}


template<class TYPB_INT>
bool e_get_hex(const BString &str, TYPB_INT *value)
{
	if (value == NULL || str.Length() < 2 || str[0] != '0' || !(str[1] == 'x' || str[1] == 'X')) return false;

	*value = (TYPB_INT)0;

	int32 maxLen = 2 + (int32)sizeof(TYPB_INT) * 2;
	uint32 bitOffset = 0;

	for (int32 i = min_c(str.Length(), maxLen) - 1; i >= 2; i--, bitOffset += 4) {
		int v = 0;
		if (str[i] >= '0' && str[i] <= '9') v = str[i] - '0';
		else if (str[i] >= 'a' && str[i] <= 'f') v = str[i] - 'a' + 10;
		else v = str[i] - 'A' + 10;
		*value |= (((TYPB_INT)v) << bitOffset);
	}

	return true;
}


bool
BString::GetDecimal(float *value) const
{
#if !(defined(_MSC_VER) && _MSC_VER <= 0x4b0)

	if (!value || !IsNumber()) return false;

	uint64 tmp;
	if (e_get_hex(*this, &tmp)) {
		*value = (float)tmp;
		return true;
	}

#ifdef HAVE_STRTOF
	*value = strtof(String(), NULL);
#else
	*value = (float)strtod(String(), NULL);
#endif

	return true;

#else
	// TODO
	return false;
#endif
}


bool
BString::GetDecimal(double *value) const
{
#if !(defined(_MSC_VER) && _MSC_VER <= 0x4b0)

	if (!value || !IsNumber()) return false;

	uint64 tmp;
	if (e_get_hex(*this, &tmp)) {
		*value = (double)tmp;
		return true;
	}

	*value = strtod(String(), NULL);

	return true;

#else
	// TODO
	return false;
#endif
}


#ifdef ETK_SUPPORT_LONG_DOUBLE
bool
BString::GetDecimal(long double *value) const
{
	if (!value || !IsNumber()) return false;

	uint64 tmp;
	if (e_get_hex(*this, &tmp)) {
		*value = (long double)tmp;
		return true;
	}

	*value = strtold(String(), NULL);

	return true;
}
#endif // ETK_SUPPORT_LONG_DOUBLE


bool
BString::GetInteger(int8 *value) const
{
	if (!value || !IsNumber()) return false;

	*value = (int8)strtol(String(), NULL, 10);

	return true;
}


bool
BString::GetInteger(uint8 *value) const
{
	if (!value || !IsNumber()) return false;

	if (e_get_hex(*this, value)) return true;

	*value = (uint8)strtoul(String(), NULL, 10);

	return true;
}


bool
BString::GetInteger(int16 *value) const
{
	if (!value || !IsNumber()) return false;

	*value = (int16)strtol(String(), NULL, 10);

	return true;
}


bool
BString::GetInteger(uint16 *value) const
{
	if (!value || !IsNumber()) return false;

	if (e_get_hex(*this, value)) return true;

	*value = (uint16)strtoul(String(), NULL, 10);

	return true;
}


bool
BString::GetInteger(int32 *value) const
{
	if (!value || !IsNumber()) return false;

	if (e_get_hex(*this, value)) return true;

	*value = (int32)strtol(String(), NULL, 10);

	return true;
}


bool
BString::GetInteger(uint32 *value) const
{
	if (!value || !IsNumber()) return false;

	if (e_get_hex(*this, value)) return true;

	*value = (uint32)strtoul(String(), NULL, 10);

	return true;
}


bool
BString::GetInteger(int64 *value) const
{
	if (!value || !IsNumber()) return false;

	if (e_get_hex(*this, value)) return true;

#if SIZEOF_INT == 8 || SIZEOF_LONG == 8
	*value = (int64)strtol(String(), NULL, 10);
	return true;
#else
#if SIZEOF_LONG_LONG == 8 && !defined(_MSC_VER)
	*value = (int64)strtoll(String(), NULL, 10);
	return true;
#else
#if SIZEOF___INT64 == 8
#if _MSC_VER <= 0x4b0
	*value = (int64)_atoi64(String());
#else
	*value = (int64)_strtoi64(String(), NULL, 10);
#endif
	return true;
#else
	return false;
#endif
#endif
#endif
}


bool
BString::GetInteger(uint64 *value) const
{
#if !(defined(_MSC_VER) && _MSC_VER <= 0x4b0)
	if (!value || !IsNumber()) return false;

	if (e_get_hex(*this, value)) return true;

#if SIZEOF_INT == 8 || SIZEOF_LONG == 8
	*value = (uint64)strtoul(String(), NULL, 10);
	return true;
#else
#if SIZEOF_LONG_LONG == 8 && !defined(_MSC_VER)
	*value = (uint64)strtoull(String(), NULL, 10);
	return true;
#else
#if SIZEOF___INT64 == 8
	*value = (uint64)_strtoui64(String(), NULL, 10);
	return true;
#else
	return false;
#endif
#endif
#endif

#else
	// TODO
	return false;
#endif
}


BString&
BString::operator<<(const char *str)
{
	return Append(str);
}


BString&
BString::operator<<(const BString &str)
{
	return Append(str);
}


BString&
BString::operator<<(char c)
{
	return Append(c, 1);
}


BString&
BString::operator<<(int8 value)
{
	return AppendFormat("%I8i", value);
}


BString&
BString::operator<<(uint8 value)
{
	return AppendFormat("%I8u", value);
}


BString&
BString::operator<<(int16 value)
{
	return AppendFormat("%I16i", value);
}


BString&
BString::operator<<(uint16 value)
{
	return AppendFormat("%I16u", value);
}


BString&
BString::operator<<(int32 value)
{
	return AppendFormat("%I32i", value);
}


BString&
BString::operator<<(uint32 value)
{
	return AppendFormat("%I32u", value);
}


BString&
BString::operator<<(int64 value)
{
	return AppendFormat("%I64i", value);
}


BString&
BString::operator<<(uint64 value)
{
	return AppendFormat("%I64u", value);
}


BString&
BString::operator<<(float value)
{
	return AppendFormat("%g", value);
}


BString&
BString::operator<<(double value)
{
	return AppendFormat("%g", value);
}


#ifdef ETK_SUPPORT_LONG_DOUBLE
BString&
BString::operator<<(long double value)
{
	return AppendFormat("%Lg", value);
}
#endif // ETK_SUPPORT_LONG_DOUBLE


BString&
BString::AppendFormat(const char *format, ...)
{
	if (!format) return *this;

	va_list args;

	char *buffer = NULL;

	va_start(args, format);

	buffer = b_strdup_vprintf(format, args);
	if (buffer) {
		Append(buffer);
		free(buffer);
	}

	va_end(args);

	return *this;
}


BString&
BString::PrependFormat(const char *format, ...)
{
	if (!format) return *this;

	va_list args;

	char *buffer = NULL;

	va_start(args, format);

	buffer = b_strdup_vprintf(format, args);
	if (buffer) {
		Prepend(buffer);
		free(buffer);
	}

	va_end(args);

	return *this;
}


BStringArray*
BString::Split(const char *delimiter, uint32 max_tokens) const
{
	if (delimiter == NULL || *delimiter == 0) return NULL;
	if (max_tokens == B_MAXUINT32) return NULL;

	int32 delimiter_len = (int32)strlen(delimiter);

	BStringArray *array = new BStringArray();
	if (!array) return NULL;

	int32 offset = 0;
	int32 found = 0;
	uint32 i = 0;

	while (true) {
		if (offset >= Length() || i >= max_tokens) break;

		if ((found = FindFirst(delimiter, offset)) < 0) found = Length();
		i++;

		BString line;

		int32 length = found - offset;

		if (length > 0) line.SetTo(String() + offset, length);

		array->AddItem(line);

		offset = found + delimiter_len;
	}

	if (array->CountItems() <= 0) {
		delete array;
		array = NULL;
	}

	return array;
}


BStringArray*
BString::Split(const char delimiter, uint32 max_tokens) const
{
	char string[2];
	bzero(string, 2);
	string[0] = delimiter;
	return Split(string, max_tokens);
}


int32
BString::CountChars() const
{
	if (fLen <= 0) return fLen;
	return e_utf8_strlen(fBuffer);
}


const char*
BString::CharAt(int32 index, uint8 *length) const
{
	if (length) *length = 0;
	if (fLen <= index) return NULL;
	return e_utf8_at(fBuffer, index, length);
}


template<class TYPB_INT>
void e_printf_int(BString &str, TYPB_INT value, uint8 _base, int precision_width, bool upper_style)
{
	if (str.Length() > 0 || !(_base == 8 || _base == 10 || _base == 16)) return;

	if (value == (TYPB_INT)0) {
		if (precision_width != 0)
			str.Append('0', max_c(precision_width, 1));
		return;
	}

	TYPB_INT base = (TYPB_INT)_base;

	while (value != (TYPB_INT)0) {
		int nIndex = (int)(value % base);
		if (nIndex < 0) nIndex = -nIndex;
		if (nIndex >= (int)_base) break;
		str.Prepend((char)(nIndex <= 9 ? '0' + nIndex : (upper_style ? 'A' : 'a') + nIndex - 10), 1);
		value /= base;
	}

	if (!(str.Length() < 0 || precision_width < 0 || str.Length() >= precision_width)) str.Prepend('0', precision_width - str.Length());
}


int32 e_printf_double(BString &str, double value, int precision_width, int32 *exponent, bool g_style, bool upper_style)
{
	if (exponent) *exponent = -1;
	if (str.Length() > 0 || (!g_style && exponent == NULL)) return -1;

#if defined(HAVE_ISFINITE) && defined(HAVE_ISNAN)
	if (isfinite(value) == 0) {
		if (upper_style)
			str = (isnan(value) ? "NAN" : "INF");
		else
			str = (isnan(value) ? "nan" : "inf");
		return -1;
	}
#endif

	if (precision_width < 0 && !g_style) precision_width = 6;
	else if (precision_width <= 0 && g_style) precision_width = (precision_width < 0 ? 6 : 1);

	int32 digits = 0;
	double iValue = 0;
	double fValue = modf(value, &iValue);
	bool iValueZero = (iValue == 0);

	if (iValueZero) {
		str.Append("0");
		digits++;
	} else while (iValue != 0 && precision_width > 0) {
			double n = fmod(iValue, 10);
			int nIndex = (int)n;
			if (nIndex < 0) nIndex = -nIndex;
			if (nIndex >= 10) {
				str = (upper_style ? "NAN" : "nan");
				return -1;
			}
			str.Prepend((char)('0' + nIndex), 1);
			modf(iValue / 10, &iValue);
			digits++;
		}

	int32 exp = -1;
#if 0
	if (exponent != NULL) // 'g' or 'G' or 'e' or 'E' style
#else
	if (!g_style && exponent != NULL) // 'e' or 'E' style
#endif
	{
		exp = digits - 1;
		if (exp >= precision_width) {
			*exponent = precision_width + (g_style ? 0 : 1);
			str.Remove(*exponent, -1);

			str.Append(upper_style ? "E+" : "e+");
			BString nStr;
			e_printf_int(nStr, exp, 10, 2, false);
			str.Append(nStr);

			return 1;
		}

		precision_width -= (g_style ? digits : exp);
		if (!g_style) digits = 1;
	}

	while (fValue != 0 && precision_width > 0) {
		fValue *= 10;
		double n = 0;
		fValue = modf(fValue, &n);
		int nIndex = (int)n;
		if (nIndex < 0) nIndex = -nIndex;
		if (nIndex >= 10) {
			str = (upper_style ? "NAN" : "nan");
			return -1;
		} else if (g_style && iValueZero && exp <= 0) {
			if (nIndex == 0) {
				exp--;
				continue;
			} else {
				exp--;
				if (exp < -4) {
					str = (char)('0' + nIndex);
					iValueZero = false;
					continue;
				} else {
					if (exp < -1) {
						str.Append('0', min_c(-exp - 1, precision_width));
						precision_width -= (-exp - 1);
						if (precision_width <= 0) break;
					}
					iValueZero = false;
				}
			}
		}
		str.Append((char)('0' + nIndex), 1);
		precision_width--;
	}

	if ((g_style && exp < -4) || (!g_style && exponent != NULL)) {
		*exponent = str.Length();

		str.Append(upper_style ? "E" : "e");
		str.Append(exp > 0 ? "+" : "-");
		BString nStr;
		e_printf_int(nStr, exp, 10, 2, false);
		str.Append(nStr);
	}

	return digits;
}


#ifdef ETK_SUPPORT_LONG_DOUBLE
int32 e_printf_double(BString &str, long double value, int precision_width, int32 *exponent, bool g_style, bool upper_style)
{
	if (exponent) *exponent = -1;
	if (str.Length() > 0 || (!g_style && exponent == NULL)) return -1;

#if defined(HAVE_ISFINITE) && defined(HAVE_ISNAN)
	if (isfinite(value) == 0) {
		if (upper_style)
			str = (isnan(value) ? "NAN" : "INF");
		else
			str = (isnan(value) ? "nan" : "inf");
		return -1;
	}
#endif

	if (precision_width < 0 && !g_style) precision_width = 6;
	else if (precision_width <= 0 && g_style) precision_width = (precision_width < 0 ? 6 : 1);

	int32 digits = 0;
	long double iValue = 0;
	long double fValue = modfl(value, &iValue);
	bool iValueZero = (iValue == 0);

	if (iValueZero) {
		str.Append("0");
		digits++;
	} else while (iValue != 0 && precision_width > 0) {
			long double n = fmodl(iValue, 10);
			int nIndex = (int)n;
			if (nIndex < 0) nIndex = -nIndex;
			if (nIndex >= 10) {
				str = (upper_style ? "NAN" : "nan");
				return -1;
			}
			str.Prepend((char)('0' + nIndex), 1);
			modfl(iValue / 10, &iValue);
			digits++;
		}

	int32 exp = -1;
#if 0
	if (exponent != NULL) // 'g' or 'G' or 'e' or 'E' style
#else
	if (!g_style && exponent != NULL) // 'e' or 'E' style
#endif
	{
		exp = digits - 1;
		if (exp >= precision_width) {
			*exponent = precision_width + (g_style ? 0 : 1);
			str.Remove(*exponent, -1);

			str.Append(upper_style ? "E+" : "e+");
			BString nStr;
			e_printf_int(nStr, exp, 10, 2, false);
			str.Append(nStr);

			return 1;
		}

		precision_width -= (g_style ? digits : exp);
		if (!g_style) digits = 1;
	}

	while (fValue != 0 && precision_width > 0) {
		fValue *= 10;
		long double n = 0;
		fValue = modfl(fValue, &n);
		int nIndex = (int)n;
		if (nIndex < 0) nIndex = -nIndex;
		if (nIndex >= 10) {
			str = (upper_style ? "NAN" : "nan");
			return -1;
		} else if (g_style && iValueZero && exp <= 0) {
			if (nIndex == 0) {
				exp--;
				continue;
			} else {
				exp--;
				if (exp < -4) {
					str = (char)('0' + nIndex);
					iValueZero = false;
					continue;
				} else {
					if (exp < -1) {
						str.Append('0', min_c(-exp - 1, precision_width));
						precision_width -= (-exp - 1);
						if (precision_width <= 0) break;
					}
					iValueZero = false;
				}
			}
		}
		str.Append((char)('0' + nIndex), 1);
		precision_width--;
	}

	if ((g_style && exp < -4) || (!g_style && exponent != NULL)) {
		*exponent = str.Length();

		str.Append(upper_style ? "E" : "e");
		str.Append(exp > 0 ? "+" : "-");
		BString nStr;
		e_printf_int(nStr, exp, 10, 2, false);
		str.Append(nStr);
	}

	return digits;
}
#endif // ETK_SUPPORT_LONG_DOUBLE


extern "C"
{

	char* b_strndup(const char* src, int32 length) {
		char *dest;
		int32 len = 0;

		if (src == NULL || *src == 0 || length == 0) return NULL;

		if (length < 0)
			len = (int32)strlen(src);
		else
			len = min_c(length, (int32)strlen(src));

		dest = (char*)malloc((size_t)(len + 1));

		if (!dest) return NULL;

		if (len != 0) memcpy(dest, src, (size_t)len);
		dest[len] = 0;

		return dest;
	}


	char* b_strdup(const char* src) {
		return b_strndup(src, -1);
	}


	char* b_strdup_vprintf(const char *format, va_list ap) {
		va_list args;
		BString str(format);

		if (str.Length() <= 0) return NULL;
		str.SetMinimumBufferSize(1024);

		va_copy(args, ap);

		int32 offset = 0;
		while (!(offset < 0 || offset >= str.Length() || (offset = str.FindFirst("%", offset)) < 0)) {
			int32 curOffset = offset + 1;

			// flags
			bool do_alternate_form = false;
			bool left_align = false;
			bool show_positive = false;
			bool zero_padded = false;

			// field width
			bool has_precision_width = false;
			int field_width = 0;
			int precision_width = 0;

			// length modifier
			BString length_modifier;

			while (true) {
				bool doBreak = false;

				if (curOffset < 0 || curOffset >= str.Length()) {
					str.Remove(offset, -1);
					break;
				}

				switch (str[curOffset]) {
					case '0':
						if (!left_align) zero_padded = true;
						break;

					case ' ':
						break;

					case '+':
						show_positive = true;
						break;

					case '#':
						do_alternate_form = true;
						break;

					case '.':
						if (curOffset >= (str.Length() - 2)) break;
						has_precision_width = true;
						if (str[curOffset + 1] == '0') {
							curOffset++;
						} else if (str[curOffset + 1] == '*') {
							precision_width = va_arg(args, int);
							if (precision_width < 0) precision_width = 0;
							curOffset++;
						} else if (str[curOffset + 1] == '-' && str[curOffset + 2] >= '1' && str[curOffset + 2] <= '9') {
							precision_width = 0;
							curOffset++;
							while (true) {
								curOffset++;
								if (curOffset < 0 || curOffset >= str.Length()) break;
								if (str[curOffset] < '0' || str[curOffset] > '9') {
									curOffset--;
									break;
								}
							}
						} else if (str[curOffset + 1] >= '1' && str[curOffset + 1] <= '9') {
							curOffset++;

							BString strWidth;
							strWidth.Append(str[curOffset], 1);
							while (true) {
								curOffset++;
								if (curOffset < 0 || curOffset >= str.Length()) break;
								if (str[curOffset] < '0' || str[curOffset] > '9') {
									curOffset--;
									break;
								}

								strWidth.Append(str[curOffset], 1);
							}
							strWidth.GetInteger(&precision_width);
						}
						break;

					case '-':
						if (curOffset >= (str.Length() - 1) || str[curOffset + 1] < '1' || str[curOffset + 1] > '9') {
							left_align = true;
							zero_padded = false;
							break;
						}
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9': {
						BString strWidth;
						strWidth.Append(str[curOffset], 1);
						while (true) {
							curOffset++;
							if (curOffset < 0 || curOffset >= str.Length()) break;
							if (str[curOffset] < '0' || str[curOffset] > '9') {
								curOffset--;
								break;
							}

							strWidth.Append(str[curOffset], 1);
						}
						strWidth.GetInteger(&field_width);
					}
					break;

					case '*':
						field_width = va_arg(args, int);
						break;

					case 'h':
						if (!(curOffset >= (str.Length() - 1) || str[curOffset + 1] != 'h')) {
							length_modifier = "hh";
							curOffset++;
						} else {
							length_modifier = "h";
						}
						break;

					case 'l':
						if (!(curOffset >= (str.Length() - 1) || str[curOffset + 1] != 'l')) {
							length_modifier = "ll";
							curOffset++;
						} else {
							length_modifier = "l";
						}
						break;

					case 'I':
						if (!(curOffset >= (str.Length() - 2) || str[curOffset + 1] != '6' || str[curOffset + 2] != '4')) {
							length_modifier = "I64";
							curOffset += 2;
						} else if (!(curOffset >= (str.Length() - 2) || str[curOffset + 1] != '3' || str[curOffset + 2] != '2')) {
							length_modifier = "I32";
							curOffset += 2;
						} else if (!(curOffset >= (str.Length() - 2) || str[curOffset + 1] != '1' || str[curOffset + 2] != '6')) {
							length_modifier = "I16";
							curOffset += 2;
						} else if (!(curOffset >= (str.Length() - 1) || str[curOffset + 1] != '8')) {
							length_modifier = "I8";
							curOffset += 1;
						}
						break;

					case 'q':
						length_modifier = "ll";
						break;

					case 'L':
						length_modifier = "L";
						break;

					case 'p':
#if SIZEOF_VOID_P == SIZEOF_SHORT
						length_modifier = "h";
						do_alternate_form = true;
#else
#if SIZEOF_VOID_P == SIZEOF_INT
						length_modifier.MakeEmpty();
						do_alternate_form = true;
#else
#if SIZEOF_VOID_P == SIZEOF_LONG
						length_modifier = "l";
						do_alternate_form = true;
#else
#if SIZEOF_VOID_P == SIZEOF_LONG_LONG
						length_modifier = "ll";
						do_alternate_form = true;
#else
#if SIZEOF_VOID_P == SIZEOF___INT64
						length_modifier = "I64";
						do_alternate_form = true;
#endif // SIZEOF_VOID_P == SIZEOF___INT64
#endif // SIZEOF_VOID_P == SIZEOF_LONG_LONG
#endif // SIZEOF_VOID_P == SIZEOF_LONG
#endif // SIZEOF_VOID_P == SIZEOF_INT
#endif // SIZEOF_VOID_P == SIZEOF_SHORT

					case 'd':
					case 'i':
					case 'o':
					case 'u':
					case 'x':
					case 'X': {
						bool negative = false;
						bool has_signed;
						uint8 base;

						if (str[curOffset] == 'o') {
							has_signed = false;
							base = 8;
						} else if (str[curOffset] == 'u') {
							has_signed = false;
							base = 10;
						} else if (str[curOffset] == 'x' || str[curOffset] == 'X' || str[curOffset] == 'p') {
							has_signed = false;
							base = 16;
						} else {
							has_signed = true;
							base = 10;
						}

						BString aStr;
						if (length_modifier == "hh") {
							int value = va_arg(args, int);
							if (has_signed) {
								negative = (value < 0);
								e_printf_int(aStr, (char)value, base,
								             (has_precision_width ? max_c(precision_width, 0) : -1),
								             str[curOffset] == 'X');
							} else {
								e_printf_int(aStr, (unsigned char)value, base,
								             (has_precision_width ? max_c(precision_width, 0) : -1),
								             str[curOffset] == 'X');
							}
						} else if (length_modifier == "h") {
							int value = va_arg(args, int);
							if (has_signed) {
								negative = (value < 0);
								e_printf_int(aStr, (short int)value, base,
								             (has_precision_width ? max_c(precision_width, 0) : -1),
								             str[curOffset] == 'X');
							} else {
								e_printf_int(aStr, (unsigned short int)value, base,
								             (has_precision_width ? max_c(precision_width, 0) : -1),
								             str[curOffset] == 'X');
							}
						}
#ifdef HAVE_LONG_LONG
						else if (length_modifier == "ll") {
							long long int value = va_arg(args, long long int);
							if (has_signed) {
								negative = (value < (long long int)0);
								e_printf_int(aStr, value, base,
								             (has_precision_width ? max_c(precision_width, 0) : -1),
								             str[curOffset] == 'X');
							} else {
								e_printf_int(aStr, (unsigned long long int)value, base,
								             (has_precision_width ? max_c(precision_width, 0) : -1),
								             str[curOffset] == 'X');
							}
						}
#endif // HAVE_LONG_LONG
						else if (length_modifier == "I64") {
							int64 value = va_arg(args, int64);
							if (has_signed) {
								negative = (value <B_INT64_CONSTANT(0));
								e_printf_int(aStr, value, base,
								             (has_precision_width ? max_c(precision_width, 0) : -1),
								             str[curOffset] == 'X');
							} else {
								e_printf_int(aStr, (uint64)value, base,
								             (has_precision_width ? max_c(precision_width, 0) : -1),
								             str[curOffset] == 'X');
							}
						} else if (length_modifier == "I32") {
							int32 value = va_arg(args, int32);
							if (has_signed) {
								negative = (value < 0);
								e_printf_int(aStr, value, base,
								             (has_precision_width ? max_c(precision_width, 0) : -1),
								             str[curOffset] == 'X');
							} else {
								e_printf_int(aStr, (uint32)value, base,
								             (has_precision_width ? max_c(precision_width, 0) : -1),
								             str[curOffset] == 'X');
							}
						} else if (length_modifier == "I16") {
							int value = va_arg(args, int);
							if (has_signed) {
								negative = (value < 0);
								e_printf_int(aStr, (int16)value, base,
								             (has_precision_width ? max_c(precision_width, 0) : -1),
								             str[curOffset] == 'X');
							} else {
								e_printf_int(aStr, (uint16)value, base,
								             (has_precision_width ? max_c(precision_width, 0) : -1),
								             str[curOffset] == 'X');
							}
						} else if (length_modifier == "I8") {
							int value = va_arg(args, int);
							if (has_signed) {
								negative = (value < 0);
								e_printf_int(aStr, (int8)value, base,
								             (has_precision_width ? max_c(precision_width, 0) : -1),
								             str[curOffset] == 'X');
							} else {
								e_printf_int(aStr, (uint8)value, base,
								             (has_precision_width ? max_c(precision_width, 0) : -1),
								             str[curOffset] == 'X');
							}
						} else if (length_modifier == "l") {
							long int value = va_arg(args, long int);
							if (has_signed) {
								negative = (value < (long int)0);
								e_printf_int(aStr, value, base,
								             (has_precision_width ? max_c(precision_width, 0) : -1),
								             str[curOffset] == 'X');
							} else {
								e_printf_int(aStr, (unsigned long int)value, base,
								             (has_precision_width ? max_c(precision_width, 0) : -1),
								             str[curOffset] == 'X');
							}
						} else {
							int value = va_arg(args, int);
							if (has_signed) {
								negative = (value < 0);
								e_printf_int(aStr, value, base,
								             (has_precision_width ? max_c(precision_width, 0) : -1),
								             str[curOffset] == 'X');
							} else {
								e_printf_int(aStr, (unsigned int)value, base,
								             (has_precision_width ? max_c(precision_width, 0) : -1),
								             str[curOffset] == 'X');
							}
						}

						if (do_alternate_form) {
							if (str[curOffset] == 'o' && aStr.Length() > 0) {
								if (aStr[0] != 0) aStr.Prepend('0', 1);
							} else if ((str[curOffset] == 'x' || str[curOffset] == 'X' || str[curOffset] == 'p') && aStr.Length() > 0) {
								aStr.Prepend(str[curOffset] == 'X' ? "0X" : "0x");
							}
						}

						if (show_positive) aStr.Prepend(negative ? '-' : '+', 1);
						else if (negative) aStr.Prepend('-', 1);

						if (aStr.Length() < field_width) {
							if (!left_align) {
								char cStr = zero_padded && !has_precision_width ? '0' : ' ';
								int32 cOffset = (cStr == '0' && (show_positive || negative) ? 1 : 0);
								int32 cCount = field_width - aStr.Length();
								if (cOffset != 0) cCount--;
								aStr.Insert(cStr, cCount, cOffset);
							} else {
								aStr.Append(' ', field_width - aStr.Length());
							}
						}
						str.Remove(offset, curOffset - offset + 1);
						str.Insert(aStr, offset);
						curOffset = offset + max_c(aStr.Length(), 0);
						doBreak = true;
					}
					break;

					case 'f':
					case 'F':
					case 'e':
					case 'E':
					case 'g':
					case 'G': {
						bool negative = false;;
						int32 exponent = -1;
						bool g_style = (str[curOffset] == 'g' || str[curOffset] == 'G');
						int32 digits = -1;

						BString aStr;

						if (length_modifier != "L") {
							double value = va_arg(args, double);
							negative = (value < 0);
							digits = e_printf_double(aStr, value, (has_precision_width ? precision_width : -1),
							                         &exponent, g_style, isupper(str[curOffset]) != 0);
						} else {
#ifdef ETK_SUPPORT_LONG_DOUBLE
							long double value = va_arg(args, long double);
							negative = (value < 0);
							digits = e_printf_double(aStr, value, (has_precision_width ? precision_width : -1),
							                         &exponent, g_style, isupper(str[curOffset]) != 0);
#else
							ETK_DEBUG("[SUPPORT]: ETK++ don't support \"long double\".");
							(void)va_arg(args, long double);
							str.Remove(offset, curOffset - offset + 1);
							doBreak = true;
							break;
#endif
						}

						if (digits > 0 && str.Length() > 0) {
							if (exponent < 0 && digits < aStr.Length() && g_style) {
								for (int32 i = aStr.Length() - 1; i > digits - 1; i--) {
									if (aStr[i] != '0') {
										aStr.Remove(i + 1, -1);
										break;
									}
								}
							}

							if (digits < (exponent < 0 ? aStr.Length() : exponent) || do_alternate_form)
								aStr.Insert('.', 1, digits);
						}

						if (show_positive) aStr.Prepend(negative ? '-' : '+', 1);
						else if (negative) aStr.Prepend('-', 1);

						if (aStr.Length() < field_width) {
							if (!left_align) {
								char cStr = zero_padded && !has_precision_width ? '0' : ' ';
								int32 cOffset = (cStr == '0' && (show_positive || negative) ? 1 : 0);
								int32 cCount = field_width - aStr.Length();
								if (cOffset != 0) cCount--;
								aStr.Insert(cStr, cCount, cOffset);
							} else {
								aStr.Append(' ', field_width - aStr.Length());
							}
						}
						str.Remove(offset, curOffset - offset + 1);
						str.Insert(aStr, offset);
						curOffset = offset + max_c(aStr.Length(), 0);
						doBreak = true;
					}
					break;

					case 'c':
					case 's': {
						BString aStr;
						if (str[curOffset] == 's') {
							char *string = va_arg(args, char*);
							aStr = (string ? string : "(null)");
						} else {
							aStr = (char)va_arg(args, int);
						}

						if (has_precision_width && aStr.Length() > max_c(precision_width, 0)) {
							aStr.Remove(precision_width, -1);
						}
						if (aStr.Length() < field_width) {
							if (!left_align)
								aStr.Prepend(' ', field_width - aStr.Length());
							else
								aStr.Append(' ', field_width - aStr.Length());
						}
						str.Remove(offset, curOffset - offset + 1);
						str.Insert(aStr, offset);
						curOffset = offset + max_c(aStr.Length(), 0);
						doBreak = true;
					}
					break;

					case '%':
						if (curOffset == offset + 1) {
							str.Remove(curOffset, 1);
							doBreak = true;
							break;
						}

					default:
						str.Remove(offset, curOffset - offset);
						curOffset = offset;
						doBreak = true;
						break;
				}

				if (doBreak) break;
				else if (curOffset >= 0) curOffset++;
			}

			offset = curOffset;
		}

		va_end(args);

		return (str.Length() > 0 ? b_strdup(str.String()) : NULL);
	}


	char* b_strdup_printf(const char *format, ...) {
		va_list args;
		char *buffer = NULL;

		if (!format) return NULL;

		va_start(args, format);

		buffer = b_strdup_vprintf(format, args);

		va_end(args);

		return buffer;
	}


	bool e_utf8_is_token(const char *str) {
		if (str == NULL) return true;

		const unsigned char *p = (const unsigned char*)str;
		return(*p < 0x80 || !(*p < 0xc0 || *p >= 0xfe));
	}



	int32 e_utf8_strlen(const char *str) {
		return e_utf8_strlen_etc(str, -1);
	}


	int32 e_utf8_strlen_etc(const char *str, int32 nbytes) {
		if (str == NULL || *str == 0 || nbytes == 0) return 0;

		int32 strLen = (int32)strlen(str);
		if (nbytes < 0 || nbytes > strLen) nbytes = strLen;

		const unsigned char *p = (const unsigned char*)str;
		int32 uLen = 0;

		while (*p) {
			int8 len = 0;

			if (*p < 0x80) len = 1; // 0xxxxxxx : ASCII
			else if (*p < 0xc0 || *p >= 0xfe) len = 0; // 10xxxxxx or 1111111x : invalid UTF8
			else if (*p < 0xe0) len = 2; // 110xxxxx : 2 bytes
			else if (*p < 0xf0) len = 3; // 1110xxxx : 3 bytes
			else if (*p < 0xf8) len = 4; // 11110xxx : 4 bytes
			else if (*p < 0xfc) len = 5; // 111110xx : 5 bytes, it's invalid UTF8 util this wrote
			else if (*p < 0xfe) len = 6; // 1111110x : 6 bytes, it's invalid UTF8 util this wrote

			if (len == 4) { // check UTF-16
				unichar high;
				high = ((unichar)(*p++) & 0x07) << 2;
				high |= ((unichar)(*p--) & 0x30) >> 4;
				if (high == 0) len = 0;
			}

			for (int8 i = len; i >= 0; i--) {
				p++;
				if (i <= 1) break;
				if (*p < 0x80 || *p >= 0xc0) i = len = 0; // 0xxxxxxx or 11xxxxxx : invalid UTF8
			}

			if ((int32)(p - (const unsigned char*)str) > nbytes) break;

			if (len > 0 && len <= 4) uLen++;
		}

		return uLen;
	}


// none checking
	int32 e_utf8_strlen_fast(const char *str, int32 nbytes) {
		if (str == NULL || *str == 0 || nbytes == 0) return 0;

		if (nbytes < 0) nbytes = (int32)strlen(str);

		const unsigned char *p = (const unsigned char*)str;
		int32 uLen = 0;

		while (*p) {
			int8 len = 0;

			if (*p < 0x80) len = 1; // 0xxxxxxx : ASCII
			else if (*p < 0xc0 || *p >= 0xfe) len = 0; // 10xxxxxx or 1111111x : invalid UTF8
			else if (*p < 0xe0) len = 2; // 110xxxxx : 2 bytes
			else if (*p < 0xf0) len = 3; // 1110xxxx : 3 bytes
			else if (*p < 0xf8) len = 4; // 11110xxx : 4 bytes
			else if (*p < 0xfc) len = 5; // 111110xx : 5 bytes, it's invalid UTF8 util this wrote
			else if (*p < 0xfe) len = 6; // 1111110x : 6 bytes, it's invalid UTF8 util this wrote

			p += max_c(len, 1);

			if ((int32)(p - (const unsigned char*)str) > nbytes) break;

			if (len > 0 && len <= 4) uLen++;
		}

		return uLen;
	}


	const char* e_utf8_at(const char *str, int32 index, uint8 *length) {
		if (length) *length = 0;
		if (index < 0 || str == NULL || strlen(str) <= (size_t)index) return NULL;

		const unsigned char *p = (const unsigned char*)str;
		int32 uLen = 0;
		int8 len = 0;

		while (*p && index >= uLen) {
			if (*p < 0x80) len = 1; // 0xxxxxxx : ASCII
			else if (*p < 0xc0 || *p >= 0xfe) len = 0; // 10xxxxxx or 1111111x : invalid UTF8
			else if (*p < 0xe0) len = 2; // 110xxxxx : 2 bytes
			else if (*p < 0xf0) len = 3; // 1110xxxx : 3 bytes
			else if (*p < 0xf8) len = 4; // 11110xxx : 4 bytes
			else if (*p < 0xfc) len = 5; // 111110xx : 5 bytes, it's invalid UTF8 util this wrote
			else if (*p < 0xfe) len = 6; // 1111110x : 6 bytes, it's invalid UTF8 util this wrote

			if (len == 4) { // check UTF-16
				unichar high;
				high = ((unichar)(*p++) & 0x07) << 2;
				high |= ((unichar)(*p--) & 0x30) >> 4;
				if (high == 0) len = 0;
			}

			for (int8 i = len; i >= 0; i--) {
				p++;
				if (i <= 1) break;
				if (*p < 0x80 || *p >= 0xc0) i = len = -(len - i + 1); // 0xxxxxxx or 11xxxxxx : invalid UTF8
			}

			if (len > 0 && len <= 4) uLen++;
		}

		if (index == uLen - 1) {
			if (len < 0) {
				p += len;
				len = 0;
			} else p -= len;
			if (length) *length = (uint8)len;
			return (const char*)p;
		}

		return NULL;
	}


	const char* e_utf8_next(const char *str, uint8 *length) {
		if (length) *length = 0;
		if (str == NULL || *str == 0) return NULL;

		const unsigned char *p = (const unsigned char*)str;
		bool first = true;

		while (*p) {
			int8 len = 0;

			if (*p < 0x80) len = 1; // 0xxxxxxx : ASCII
			else if (*p < 0xc0 || *p >= 0xfe) len = 0; // 10xxxxxx or 1111111x : invalid UTF8
			else if (*p < 0xe0) len = 2; // 110xxxxx : 2 bytes
			else if (*p < 0xf0) len = 3; // 1110xxxx : 3 bytes
			else if (*p < 0xf8) len = 4; // 11110xxx : 4 bytes
			else if (*p < 0xfc) len = 5; // 111110xx : 5 bytes, it's invalid UTF8 util this wrote
			else if (*p < 0xfe) len = 6; // 1111110x : 6 bytes, it's invalid UTF8 util this wrote

			if (len == 4) { // check UTF-16
				unichar high;
				high = ((unichar)(*p++) & 0x07) << 2;
				high |= ((unichar)(*p--) & 0x30) >> 4;
				if (high == 0) len = 0;
			}

			for (int8 i = len; i >= 0; i--) {
				p++;
				if (i <= 1) break;
				if (*p < 0x80 || *p >= 0xc0) i = len = -(len - i + 1); // 0xxxxxxx or 11xxxxxx : invalid UTF8
			}

			if (!first && len > 0 && len <= 4) {
				if (length) *length = len;
				p -= len;
				return (const char*)p;
			} else {
				first = false;
			}
		}

		return NULL;
	}


	unichar* e_utf8_convert_to_unicode(const char *str, int32 length) {
		if (str == NULL || *str == 0 || length == 0) return NULL;

		int32 strLen = (int32)strlen(str);
		if (length < 0 || length > strLen) length = strLen;

		unichar* unicode = (unichar*)malloc(sizeof(unichar) * (size_t)(length + 1));
		if (!unicode) return NULL;

		const unsigned char *p = (const unsigned char*)str;
		unichar *tmp = unicode;

		while (*p) {
			int8 len = 0;

			if (*p < 0x80) len = 1; // 0xxxxxxx : ASCII
			else if (*p < 0xc0 || *p >= 0xfe) len = 0; // 10xxxxxx or 1111111x : invalid UTF8
			else if (*p < 0xe0) len = 2; // 110xxxxx : 2 bytes
			else if (*p < 0xf0) len = 3; // 1110xxxx : 3 bytes
			else if (*p < 0xf8) len = 4; // 11110xxx : 4 bytes
			else if (*p < 0xfc) len = 5; // 111110xx : 5 bytes, it's invalid UTF8 util this wrote
			else if (*p < 0xfe) len = 6; // 1111110x : 6 bytes, it's invalid UTF8 util this wrote

			for (int8 i = len; i >= 0; i--) {
				p++;
				if (i <= 1) break;
				if (*p < 0x80 || *p >= 0xc0) i = len = 0; // 0xxxxxxx or 11xxxxxx : invalid UTF8
			}

			if ((int32)((const char*)p - str) > length) break;

			if (len > 0) {
				p -= len;
				if (len == 1) {
					*tmp = (unichar)(*p++);
				} else if (len == 2) {
					*tmp = ((unichar)(*p++) & 0x1f) << 6;
					*tmp |= ((unichar)(*p++) & 0x3f);
				} else if (len == 3) {
					*tmp = ((unichar)(*p++) & 0x0f) << 12;
					*tmp |= ((unichar)(*p++) & 0x3f) << 6;
					*tmp |= ((unichar)(*p++) & 0x3f);
				} else if (len == 4) { // convert to UTF-16
					unichar high, low;
					high = ((unichar)(*p++) & 0x07) << 2;
					high |= ((unichar)(*p) & 0x30) >> 4;
					low = ((unichar)(*p++) & 0x0f) << 12;
					low |= ((unichar)(*p++) & 0x3f) << 6;
					low |= ((unichar)(*p++) & 0x3f);
					if (high == 0) continue; // invalid UTF-16
					high -= 1;
					*tmp = (0xd800 | (high << 6));
					*tmp |= low >> 10;
					tmp++;
					*tmp = (0xdc00 | (low & 0x03ff));
				} else {
					p += len;    // don't support the 5 or 6 bytes UTF-8
					continue;
				}

				tmp++;
			}
		}
		*tmp = 0;

		return unicode;
	}


	unichar32* e_utf8_convert_to_utf32(const char *str, int32 length) {
		if (str == NULL || *str == 0 || length == 0) return NULL;

		int32 strLen = (int32)strlen(str);
		if (length < 0 || length > strLen) length = strLen;

		unichar32* unicode = (unichar32*)malloc(sizeof(unichar32) * (size_t)(length + 1));
		if (unicode == NULL) return NULL;

		const unsigned char *p = (const unsigned char*)str;
		unichar32 *tmp = unicode;

		while (*p) {
			int8 len = 0;

			if (*p < 0x80) len = 1; // 0xxxxxxx : ASCII
			else if (*p < 0xc0 || *p >= 0xfe) len = 0; // 10xxxxxx or 1111111x : invalid UTF8
			else if (*p < 0xe0) len = 2; // 110xxxxx : 2 bytes
			else if (*p < 0xf0) len = 3; // 1110xxxx : 3 bytes
			else if (*p < 0xf8) len = 4; // 11110xxx : 4 bytes
			else if (*p < 0xfc) len = 5; // 111110xx : 5 bytes, it's invalid UTF8 util this wrote
			else if (*p < 0xfe) len = 6; // 1111110x : 6 bytes, it's invalid UTF8 util this wrote

			for (int8 i = len; i >= 0; i--) {
				p++;
				if (i <= 1) break;
				if (*p < 0x80 || *p >= 0xc0) i = len = 0; // 0xxxxxxx or 11xxxxxx : invalid UTF8
			}

			if ((int32)((const char*)p - str) > length) break;

			if (len > 0) {
				p -= len;
				if (len == 1) {
					*tmp = (unichar32)(*p++);
				} else if (len == 2) {
					*tmp = ((unichar32)(*p++) & 0x1f) << 6;
					*tmp |= ((unichar32)(*p++) & 0x3f);
				} else if (len == 3) {
					*tmp = ((unichar32)(*p++) & 0x0f) << 12;
					*tmp |= ((unichar32)(*p++) & 0x3f) << 6;
					*tmp |= ((unichar32)(*p++) & 0x3f);
				} else if (len == 4) { // convert to UTF-16
					unichar high, low;
					high = ((unichar)(*p++) & 0x07) << 2;
					high |= ((unichar)(*p) & 0x30) >> 4;
					low = ((unichar)(*p++) & 0x0f) << 12;
					low |= ((unichar)(*p++) & 0x3f) << 6;
					low |= ((unichar)(*p++) & 0x3f);
					if (high == 0) continue; // invalid UTF-16
					*tmp = (((unichar32)high) << 16) | (unichar32)low;
				} else {
					p += len;    // don't support the 5 or 6 bytes UTF-8
					continue;
				}

				tmp++;
			}
		}
		*tmp = 0;

		return unicode;
	}


	int32 e_unicodb_strlen(const unichar *ustr) {
		return e_unicodb_strlen_etc(ustr, -1, true);
	}


	int32 e_unicodb_strlen_etc(const unichar *ustr, int32 nchars, bool utf16_style) {
		if (ustr == NULL || nchars == 0) return 0;

		int32 len = 0;
		const unichar *p = ustr;

		while (*p) {
			if (utf16_style ? (*p >= 0xd800 && *p <= 0xdfff) : false) {
				p++;
				if (*p >= 0xdc00 && *p <= 0xdfff) {
					if (nchars > 0) if ((int32)(p - ustr) > nchars - 1) break;
					p++;
					len++;
				}
			} else {
				if (nchars > 0) if ((int32)(p - ustr) > nchars - 1) break;
				p++;
				len++;
			}
		}

		return len;
	}


	const unichar* e_unicode_at(const unichar* ustr, int32 index, bool *utf16) {
		if (utf16) *utf16 = false;
		if (ustr == NULL || index < 0) return NULL;

		int32 len = 0;
		const unichar *p = ustr;

		while (*p) {
			bool _utf16_ = false;

			if (*p >= 0xd800 && *p <= 0xdfff) {
				p++;
				if (*p >= 0xdc00 && *p <= 0xdfff) {
					p++;
					len++;
					_utf16_ = true;
				}
			} else {
				p++;
				len++;
			}

			if (index == len - 1) {
				if (utf16) *utf16 = _utf16_;
				p--;
				if (_utf16_) p--;
				return p;
			}
		}

		return NULL;
	}


	const unichar* e_unicode_next(const unichar* ustr, bool *utf16) {
		if (utf16) *utf16 = false;
		if (ustr == NULL) return NULL;

		const unichar *p = ustr;
		bool first = true;

		while (*p) {
			if (*p >= 0xd800 && *p <= 0xdfff) {
				p++;
				if (*p >= 0xdc00 && *p <= 0xdfff) {
					if (!first) {
						if (utf16) *utf16 = true;
						p--;
						return p;
					} else {
						first = false;
						p++;
					}
				}
			} else {
				if (!first) {
					if (utf16) *utf16 = false;
					return p;
				} else {
					first = false;
					p++;
				}
			}
		}

		return NULL;
	}


	char* e_unicode_convert_to_utf8(const unichar *str, int32 ulength) {
		if (str == NULL || *str == 0 || ulength == 0) return NULL;

		BString utf8;

		int32 ulen = 0;
		const unichar *p = str;

		while (*p != 0 && (ulength > 0 ? ulength > ulen : true)) {
			if (*p >= 0xd800 && *p <= 0xdfff) {
				p++;
				if (*p >= 0xdc00 && *p <= 0xdfff) {
					p--;

					// convert UTF-16 to UCS4
					uint32 tmp = (uint32)(*p++ & 0x03ff) << 10;
					tmp |= (uint32)(*p++ & 0x03ff);

					// convert UCS4 to UTF-8
					uint8 ustr[5];
					bzero(ustr, sizeof(ustr));
					ustr[3] = 0x80 | (uint8)(tmp & 0x3f);
					ustr[2] = 0x80 | (uint8)((tmp >> 6) & 0x3f);
					ustr[1] = 0x80 | (uint8)((tmp >> 12) & 0x3f);
					ustr[0] = 0xf0 | (uint8)((tmp >> 18) & 0x07);
					utf8.Append((const char*)ustr);

					ulen++;
				}
			} else {
				uint8 len;
				if (*p < 0x80) len = 1;
				else if (*p < 0x0800) len = 2;
				else len = 3;

				uint16 tmp = *p++;

				uint8 ustr[4];
				bzero(ustr, sizeof(ustr));

				if (len == 1) {
					ustr[0] = (uint8)tmp;
				} else if (len == 2) {
					ustr[1] = 0x80 | (uint8)(tmp & 0x3f);
					ustr[0] = 0xc0 | (uint8)((tmp >> 6) & 0x1f);
				} else { // len == 3
					ustr[2] = 0x80 | (uint8)(tmp & 0x3f);
					ustr[1] = 0x80 | (uint8)((tmp >> 6) & 0x3f);
					ustr[0] = 0xe0 | (uint8)((tmp >> 12) & 0x0f);
				}

				utf8.Append((const char*)ustr);

				ulen++;
			}
		}

		return b_strndup(utf8.String(), -1);
	}


	unichar32* e_unicode_convert_to_utf32(const unichar *str, int32 ulength) {
		if (str == NULL || *str == 0 || ulength == 0) return NULL;

		int32 unicode_length = (ulength < 0 ? e_unicodb_strlen(str) : ulength);
		if (unicode_length <= 0) return NULL;

		unichar32* utf32 = (unichar32*)malloc(sizeof(unichar32) * (size_t)(unicode_length + 1));
		if (!utf32) return NULL;

		int32 ulen = 0;
		const unichar *p = str;

		while (*p != 0 && (ulength > 0 ? ulength > ulen : true)) {
			if (*p >= 0xd800 && *p <= 0xdfff) {
				p++;
				if (*p >= 0xdc00 && *p <= 0xdfff) {
					p--;

					// convert UTF-16 to UCS4
					uint32 tmp = (uint32)(*p++ & 0x03ff) << 10;
					tmp |= (uint32)(*p++ & 0x03ff);

					utf32[ulen++] = tmp;

					ulen++;
				}
			} else {
				utf32[ulen++] = (unichar32)(*p++);
			}
		}

		utf32[ulen] = 0;

		return utf32;
	}


	int32 e_utf32_strlen(const unichar32 *ustr) {
		return e_utf32_strlen_etc(ustr, -1);
	}


	int32 e_utf32_strlen_etc(const unichar32 *ustr, int32 nchars) {
		if (ustr == NULL || nchars == 0) return 0;

		int32 len = 0;
		const unichar32 *p = ustr;

		while (*p) {
			if (nchars > 0) if ((int32)(p - ustr) > nchars - 1) break;
			if (*p < 0xf8ffff) len++;
			p++;
		}

		return len;
	}


	const unichar32* e_utf32_at(const unichar32* ustr, int32 index) {
		if (ustr == NULL || index < 0) return NULL;

		int32 len = 0;
		const unichar32 *p = ustr;

		while (*p) {
			if (*p < 0xf8ffff) {
				if (index == len) return p;
				else len++;
			}
			p++;
		}

		return NULL;
	}


	const unichar32* e_utf32_next(const unichar32* ustr) {
		if (ustr == NULL) return NULL;

		const unichar32 *p = ustr;

		while (*p) {
			p++;
			if (*p > 0 && *p < 0xf8ffff) return p;
		}

		return NULL;
	}


	char* e_utf32_convert_to_utf8(const unichar32 *str, int32 ulength) {
		if (str == NULL || *str == 0 || ulength == 0) return NULL;

		BString utf8;

		int32 ulen = 0;
		const unichar32 *p = str;

		while (*p != 0 && (ulength > 0 ? ulength > ulen : true)) {
			// refuse to convert to UTF-8 more than 4 bytes
			if (*p > 0xffff && *p < 0xf8ffff) {
				uint32 tmp = *p;

				// convert UCS4 to UTF-8
				uint8 ustr[5];
				bzero(ustr, sizeof(ustr));
				ustr[3] = 0x80 | (uint8)(tmp & 0x3f);
				ustr[2] = 0x80 | (uint8)((tmp >> 6) & 0x3f);
				ustr[1] = 0x80 | (uint8)((tmp >> 12) & 0x3f);
				ustr[0] = 0xf0 | (uint8)((tmp >> 18) & 0x07);
				utf8.Append((const char*)ustr);

				ulen++;
			} else if (*p <= 0xffff) {
				uint8 len;
				if (*p < 0x80) len = 1;
				else if (*p < 0x0800) len = 2;
				else len = 3;

				uint32 tmp = *p;

				uint8 ustr[4];
				bzero(ustr, sizeof(ustr));

				if (len == 1) {
					ustr[0] = (uint8)tmp;
				} else if (len == 2) {
					ustr[1] = 0x80 | (uint8)(tmp & 0x3f);
					ustr[0] = 0xc0 | (uint8)((tmp >> 6) & 0x1f);
				} else { // len == 3
					ustr[2] = 0x80 | (uint8)(tmp & 0x3f);
					ustr[1] = 0x80 | (uint8)((tmp >> 6) & 0x3f);
					ustr[0] = 0xe0 | (uint8)((tmp >> 12) & 0x0f);
				}

				utf8.Append((const char*)ustr);

				ulen++;
			}

			p++;
		}

		return b_strndup(utf8.String(), -1);
	}


	unichar* e_utf32_convert_to_unicode(const unichar32 *str, int32 ulength) {
		if (str == NULL || *str == 0 || ulength == 0) return NULL;

		int32 utf32_length = (ulength < 0 ? e_utf32_strlen(str) : ulength);
		if (utf32_length <= 0) return NULL;

		unichar* unicode = (unichar*)malloc(sizeof(unichar) * (size_t)(2 * utf32_length + 1));
		if (!unicode) return NULL;

		int32 ulen = 0;
		const unichar32 *p = str;
		unichar *tmp = unicode;

		while (*p != 0 && (ulength > 0 ? ulength > ulen : true)) {
			if (*p > 0xffff && *p < 0xf8ffff) {
				unichar high = (unichar)(*p >> 16);
				unichar low = (unichar)(*p & 0xffff);

				high -= 1;
				*tmp = (0xd800 | (high << 6));
				*tmp |= low >> 10;
				tmp++;
				*tmp++ = (0xdc00 | (low & 0x03ff));

				ulen++;
			} else if (*p <= 0xffff) {
				*tmp++ = (unichar)(*p);

				ulen++;
			}

			p++;
		}

		*tmp = 0;

		return unicode;
	}

} // extern "C"

