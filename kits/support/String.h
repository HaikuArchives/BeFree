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
 * File: String.h
 * Description: BString --- string allocation and manipulation
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_STRING_H__
#define __ETK_STRING_H__

#include <stdarg.h>
#include <support/SupportDefs.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

	/* the result must be free by "free" */
	char*		b_strdup(const char *src);
	char*		b_strndup(const char *src, int32 length);
	char*		b_strdup_vprintf(const char *format, va_list ap);
	char*		b_strdup_printf(const char *format, ...);

	unichar*		e_utf8_convert_to_unicode(const char *str, int32 length);
	unichar32*		e_utf8_convert_to_utf32(const char *str, int32 length);
	char*		e_unicode_convert_to_utf8(const unichar *str, int32 ulength);
	unichar32*		e_unicode_convert_to_utf32(const unichar *str, int32 ulength);
	char*		e_utf32_convert_to_utf8(const unichar32 *str, int32 ulength);
	unichar*		e_utf32_convert_to_unicode(const unichar32 *str, int32 ulength);

	bool		e_utf8_is_token(const char *str);
	int32		e_utf8_strlen(const char *str);
	int32		e_utf8_strlen_etc(const char *str, int32 nbytes);
	int32		e_utf8_strlen_fast(const char *str, int32 nbytes); /* none checking */
	const char*		e_utf8_at(const char *str, int32 index, uint8 *nbytes);
	const char*		e_utf8_next(const char *str, uint8 *length);

	int32		e_unicodb_strlen(const unichar *ustr);
	int32		e_unicodb_strlen_etc(const unichar *ustr, int32 nchars, bool utf16_style);
	const unichar*	e_unicode_at(const unichar* ustr, int32 index, bool *utf16);
	const unichar*	e_unicode_next(const unichar* ustr, bool *utf16);

	int32		e_utf32_strlen(const unichar32 *ustr);
	int32		e_utf32_strlen_etc(const unichar32 *ustr, int32 nchars);
	const unichar32*	e_utf32_at(const unichar32* ustr, int32 index);
	const unichar32*	e_utf32_next(const unichar32* ustr);

#ifdef __cplusplus /* Just for C++ */
} // extern "C"

// EStrdup(): like b_strdup(), but the result must be free by "delete[]"
char *EStrdup(const char* src, int32 length = -1);


class BStringArray;


class BString
{
	public:
		BString();
		BString(const char *str);
		BString(const BString &str);
		BString(const char *str, int32 maxLength);
		~BString();

		const char	*String() const;

		int32		Length() const; // ASCII
		int32		CountChars() const; // UTF-8

		char		operator[](int32 index) const; // ASCII
		char		ByteAt(int32 index) const; // ASCII
		const char*	CharAt(int32 index, uint8 *length = NULL) const; // UTF-8

		BString 	&operator=(const BString &str);
		BString 	&operator=(const char *str);
		BString 	&operator=(char c);

		BString		&SetTo(const BString &str);
		BString		&SetTo(const BString &str, int32 length);
		BString		&SetTo(const char *str);
		BString		&SetTo(const char *str, int32 length);
		BString		&SetTo(char c, int32 count);

		BString		&Adopt(BString &from);
		BString		&Adopt(BString &from, int32 length);

		BString		&CopyInto(BString &into, int32 fromOffset, int32 length) const;
		void		CopyInto(char *into, size_t into_size, int32 fromOffset, int32 length) const;

		BString		&MoveInto(BString &into, int32 from, int32 length);
		void		MoveInto(char *into, size_t into_size, int32 from, int32 length);

		void		MakeEmpty();

		BString		&operator+=(const BString &str);
		BString		&operator+=(const char *str);
		BString		&operator+=(char c);

		BString		&Append(const BString &str);
		BString		&Append(const BString &str, int32 length);
		BString		&Append(const char *str);
		BString		&Append(const char *str, int32 length);
		BString		&Append(char c, int32 count);
		BString		&AppendFormat(const char *format, ...);

		BString		&Prepend(const BString &str);
		BString		&Prepend(const BString &str, int32 length);
		BString		&Prepend(const char *str);
		BString		&Prepend(const char *str, int32 length);
		BString		&Prepend(char c, int32 count);
		BString		&PrependFormat(const char *format, ...);

		BString		&Insert(const BString &str, int32 pos);
		BString		&Insert(const BString &str, int32 length, int32 pos);
		BString		&Insert(const BString &str, int32 fromOffset, int32 length, int32 pos);
		BString		&Insert(const char *str, int32 pos);
		BString		&Insert(const char *str, int32 length, int32 pos);
		BString		&Insert(const char *str, int32 fromOffset, int32 length, int32 pos);
		BString		&Insert(char c, int32 count, int32 pos);

		BString		&Truncate(int32 newLength);

		BString		&Remove(int32 from, int32 length);

		BString		&RemoveFirst(const BString &str);
		BString		&RemoveLast(const BString &str);
		BString		&RemoveAll(const BString &str);
		BString		&RemoveFirst(const char *str);
		BString		&RemoveLast(const char *str);
		BString		&RemoveAll(const char *str);
		BString		&RemoveSet(const char *setOfCharsToRemove);

		BString		&IRemoveFirst(const BString &str);
		BString		&IRemoveLast(const BString &str);
		BString		&IRemoveAll(const BString &str);
		BString		&IRemoveFirst(const char *str);
		BString		&IRemoveLast(const char *str);
		BString		&IRemoveAll(const char *str);
		BString		&IRemoveSet(const char *setOfCharsToRemove);

		bool		operator<(const BString &str) const;
		bool		operator<=(const BString &str) const;
		bool		operator==(const BString &str) const;
		bool		operator>=(const BString &str) const;
		bool		operator>(const BString &str) const;
		bool		operator!=(const BString &str) const;

		bool		operator<(const char *str) const;
		bool		operator<=(const char *str) const;
		bool		operator==(const char *str) const;
		bool		operator>=(const char *str) const;
		bool		operator>(const char *str) const;
		bool		operator!=(const char *str) const;

		int		Compare(const BString &str) const;
		int		Compare(const char *str) const;
		int		Compare(const BString &str, int32 n) const;
		int		Compare(const char *str, int32 n) const;
		int		ICompare(const BString &str) const;
		int		ICompare(const char *str) const;
		int		ICompare(const BString &str, int32 n) const;
		int		ICompare(const char *str, int32 n) const;

		int32 		FindFirst(const BString &string) const;
		int32 		FindFirst(const char *string) const;
		int32 		FindFirst(const BString &string, int32 fromOffset) const;
		int32 		FindFirst(const char *string, int32 fromOffset) const;
		int32		FindFirst(char c) const;
		int32		FindFirst(char c, int32 fromOffset) const;

		int32 		FindLast(const BString &string) const;
		int32 		FindLast(const char *string) const;
		int32 		FindLast(const BString &string, int32 beforeOffset) const;
		int32 		FindLast(const char *string, int32 beforeOffset) const;
		int32		FindLast(char c) const;
		int32		FindLast(char c, int32 beforeOffset) const;

		int32 		IFindFirst(const BString &string) const;
		int32 		IFindFirst(const char *string) const;
		int32 		IFindFirst(const BString &string, int32 fromOffset) const;
		int32 		IFindFirst(const char *string, int32 fromOffset) const;
		int32		IFindFirst(char c) const;
		int32		IFindFirst(char c, int32 fromOffset) const;

		int32 		IFindLast(const BString &string) const;
		int32 		IFindLast(const char *string) const;
		int32 		IFindLast(const BString &string, int32 beforeOffset) const;
		int32 		IFindLast(const char *string, int32 beforeOffset) const;
		int32		IFindLast(char c) const;
		int32		IFindLast(char c, int32 beforeOffset) const;

		BString		&ReplaceFirst(char replaceThis, char withThis);
		BString		&ReplaceLast(char replaceThis, char withThis);
		BString		&ReplaceAll(char replaceThis, char withThis, int32 fromOffset = 0);
		BString		&Replace(char replaceThis, char withThis, int32 maxReplaceCount, int32 fromOffset = 0);
		BString 	&ReplaceFirst(const char *replaceThis, const char *withThis);
		BString		&ReplaceLast(const char *replaceThis, const char *withThis);
		BString		&ReplaceAll(const char *replaceThis, const char *withThis, int32 fromOffset = 0);
		BString		&Replace(const char *replaceThis, const char *withThis, int32 maxReplaceCount, int32 fromOffset = 0);
		BString		&ReplaceSet(const char *setOfChars, char with);
		BString		&ReplaceSet(const char *setOfChars, const char *with);

		BString		&IReplaceFirst(char replaceThis, char withThis);
		BString		&IReplaceLast(char replaceThis, char withThis);
		BString		&IReplaceAll(char replaceThis, char withThis, int32 fromOffset = 0);
		BString		&IReplace(char replaceThis, char withThis, int32 maxReplaceCount, int32 fromOffset = 0);
		BString 	&IReplaceFirst(const char *replaceThis, const char *withThis);
		BString		&IReplaceLast(const char *replaceThis, const char *withThis);
		BString		&IReplaceAll(const char *replaceThis, const char *withThis, int32 fromOffset = 0);
		BString		&IReplace(const char *replaceThis, const char *withThis, int32 maxReplaceCount, int32 fromOffset = 0);
		BString		&IReplaceSet(const char *setOfChars, char with);
		BString		&IReplaceSet(const char *setOfChars, const char *with);

		BString		&ToLower();
		BString		&ToUpper();

		// Converts first character to upper-case, rest to lower-case
		BString		&Capitalize();

		// Converts first character in each white-space-separated word to upper-case, rest to lower-case
		BString		&CapitalizeEachWord();

		// copies original into <this>, escaping characters specified in <setOfCharsToEscape> by prepending them with <escapeWith>
		BString		&CharacterEscape(const char *original, const char *setOfCharsToEscape, char escapeWith);

		// escapes characters specified in <setOfCharsToEscape> by prepending them with <escapeWith>
		BString		&CharacterEscape(const char *setOfCharsToEscape, char escapeWith);

		// copy <original> into the string removing the escaping characters <escapeChar>
		BString		&CharacterDeescape(const char *original, char escapeChar);

		// remove the escaping characters <escapeChar> from the string
		BString		&CharacterDeescape(char escapeChar);

		// IsNumber: whether it is decimalism number like +1.23,-12,12.21,-.12,+.98,3.14e+04,0xffff
		bool		IsNumber() const;

		// IsInteger: whether it is integer in decimalism like +1.000,-2.0,1,2,+1,-2,0xffff don't support x.xxxe+xx style
		bool		IsInteger() const;

		// IsDecimal: whether it is decimal in decimalism like +1.2343,-0.23,12.43,-.23,+.23,3.14e+02
		bool		IsDecimal() const;

		// if IsNumber() is "true", it convert the string to double then return "true", else do nothing and return "false"
		bool		GetDecimal(float *value) const;
		bool		GetDecimal(double *value) const;
		bool		GetInteger(int8 *value) const;
		bool		GetInteger(uint8 *value) const;
		bool		GetInteger(int16 *value) const;
		bool		GetInteger(uint16 *value) const;
		bool		GetInteger(int32 *value) const;
		bool		GetInteger(uint32 *value) const;
		bool		GetInteger(int64 *value) const;
		bool		GetInteger(uint64 *value) const;

		BString 	&operator<<(const char *str);
		BString 	&operator<<(const BString &str);
		BString 	&operator<<(char c);
		BString 	&operator<<(int8 value);
		BString 	&operator<<(uint8 value);
		BString 	&operator<<(int16 value);
		BString 	&operator<<(uint16 value);
		BString 	&operator<<(int32 value);
		BString 	&operator<<(uint32 value);
		BString 	&operator<<(int64 value);
		BString 	&operator<<(uint64 value);
		BString 	&operator<<(float value);
		BString 	&operator<<(double value);

		// Split: splits a string into a maximum of max_tokens pieces, using the given delimiter.
		//        If max_tokens is reached, the remainder of string is appended to the last token
		// Returns : a newly-allocated array of strings
		BStringArray	*Split(const char *delimiter, uint32 max_tokens = B_MAXUINT32 - 1) const;
		BStringArray	*Split(const char delimiter, uint32 max_tokens = B_MAXUINT32 - 1) const;

		// SetMinimumBufferSize: It's NOT to be absolute minimum buffer size even it return "true",
		//                       just for speed up sometimes. The "length" include the null character
		bool		SetMinimumBufferSize(int32 length);
		int32		MinimumBufferSize() const;

	private:
		int32 fLen;
		int32 fLenReal;
		int32 fMinBufferSize;
		char *fBuffer;

		bool _Resize(int32 length);
};


#endif /* __cplusplus */

#endif /* __ETK_STRING_H__ */

