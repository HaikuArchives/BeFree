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
 * File: DataIO.cpp
 *
 * --------------------------------------------------------------------------*/

#include <support/SupportDefs.h>

#include "DataIO.h"


BDataIO::BDataIO()
{
}


BDataIO::~BDataIO()
{
}


BPositionIO::BPositionIO()
		: BDataIO()
{
}


BPositionIO::~BPositionIO()
{
}


ssize_t
BPositionIO::Read(void *buffer, size_t size)
{
	return ReadAt(0, buffer, size);
}


ssize_t
BPositionIO::Write(const void *buffer, size_t size)
{
	return WriteAt(0, buffer, size);
}


BMallocIO::BMallocIO()
		: BPositionIO(), fData(NULL), fBlockSize(256), fMallocSize(0), fLength(0), fPosition(0)
{
}


BMallocIO::~BMallocIO()
{
	if (fData != NULL) free(fData);
}


ssize_t
BMallocIO::ReadAt(int64 pos, void *buffer, size_t size)
{
	if (buffer == NULL) return B_BAD_VALUE;

	if (size == 0 || BMallocIO::Seek(pos, B_SEEK_CUR) < 0) return 0;
	if (fPosition >= fLength) return 0;

	size = min_c(size, fLength - fPosition);
	if (memcpy(buffer, fData + fPosition, size) == NULL) return B_ERROR;

	fPosition += size;
	return size;
}


ssize_t
BMallocIO::WriteAt(int64 pos, const void *buffer, size_t size)
{
	if (buffer == NULL) return B_BAD_VALUE;

	if (size == 0 || BMallocIO::Seek(pos, B_SEEK_CUR) < 0) return 0;

	if (fMallocSize >= fPosition + size ||
	        BMallocIO::SetSize((int64)(fPosition + size)) == B_OK ||
	        (fMallocSize >= fPosition ? (size = min_c(size, fMallocSize - fPosition)) > 0 : false)) {
		if (memcpy(fData + fPosition, buffer, size) == NULL) return B_ERROR;
		fPosition += size;
		if (fLength < fPosition) fLength = fPosition;
		return size;
	}
	return B_NO_MEMORY;
}


int64
BMallocIO::Seek(int64 position, uint32 seek_mode)
{
	int64 retVal = B_INT64_CONSTANT(-1);

	switch (seek_mode) {
		case B_SEEK_SET:
			if (!(position < 0 || position > (int64)~((size_t)0)))
				fPosition = (size_t)(retVal = position);
			break;

		case B_SEEK_CUR:
			if (position < 0 ? (int64)fPosition >= -position : position <= (int64)(~((size_t)0) - fPosition)) {
				if (position < 0) fPosition -= (size_t)(-position);
				else fPosition += (size_t)position;
				retVal = (int64)fPosition;
			}
			break;

		case B_SEEK_END:
			if (position < 0 ? (int64)fLength >= -position : position <= (int64)(~((size_t)0) - fLength)) {
				if (position < 0) fPosition = fLength - (size_t)(-position);
				else fPosition = fLength + (size_t)position;
				retVal = (int64)fPosition;
			}
			break;

		default:
			break;
	}

	return retVal;
}


int64
BMallocIO::Position() const
{
	return (int64)fPosition;
}


status_t
BMallocIO::SetSize(int64 size)
{
	if (size < 0) return B_BAD_VALUE;
	if (size == 0) {
		if (fData) free(fData);
		fData = NULL;
		fPosition = fLength = fMallocSize = 0;
		return B_OK;
	}

	int64 alloc_size = size >= B_MAXINT64 - fBlockSize ?
	                   B_MAXINT64 : ((size + (int64)fBlockSize - 1) & ~((int64)fBlockSize - 1));

	if (alloc_size > (int64)~((size_t)0)) alloc_size = (int64)~((size_t)0);
	if (alloc_size != (int64)fMallocSize) {
		char *data = (char*)realloc(fData, (size_t)alloc_size);
		if (data == NULL) {
			if (size > (int64)fMallocSize) return B_NO_MEMORY;
		} else {
			fData = data;
			fMallocSize = (size_t)alloc_size;
		}
	}

	fLength = (size_t)size;

	return B_OK;
}


void
BMallocIO::SetBlockSize(size_t blocksize)
{
	fBlockSize = blocksize;
}


const void*
BMallocIO::Buffer() const
{
	return((const void*)fData);
}


size_t
BMallocIO::BufferLength()
{
	return fLength;
}


BMemoryIO::BMemoryIO(void *ptr, size_t length)
		: BPositionIO(), fReadOnly(false), fBuffer((char*)ptr), fLen(length), fRealLen(length), fPosition(0)
{
	if (fBuffer == NULL) fRealLen = fLen = 0;
}


BMemoryIO::BMemoryIO(const void *ptr, size_t length)
		: BPositionIO(), fReadOnly(true), fBuffer((char*)ptr), fLen(length), fRealLen(length), fPosition(0)
{
	if (fBuffer == NULL) fRealLen = fLen = 0;
}


BMemoryIO::~BMemoryIO()
{
}


ssize_t
BMemoryIO::ReadAt(int64 pos, void *buffer, size_t size)
{
	if (buffer == NULL) return B_BAD_VALUE;

	if (size == 0 || BMemoryIO::Seek(pos, B_SEEK_CUR) < 0) return 0;
	if (fPosition >= fLen) return 0;

	size = min_c(size, fLen - fPosition);
	if (memcpy(buffer, fBuffer + fPosition, size) == NULL) return B_ERROR;

	fPosition += size;
	return size;
}


ssize_t
BMemoryIO::WriteAt(int64 pos, const void *buffer, size_t size)
{
	if (fReadOnly) return B_NOT_ALLOWED;
	if (buffer == NULL) return B_BAD_VALUE;

	if (size == 0 || BMemoryIO::Seek(pos, B_SEEK_CUR) < 0) return 0;

	if (fLen > fPosition) {
		size = min_c(size, fLen - fPosition);
		if (memcpy(fBuffer + fPosition, buffer, size) == NULL) return B_ERROR;
		fPosition += size;
		return size;
	}

	return 0;
}


int64
BMemoryIO::Seek(int64 position, uint32 seek_mode)
{
	int64 retVal = B_INT64_CONSTANT(-1);

	switch (seek_mode) {
		case B_SEEK_SET:
			if (!(position < 0 || position > (int64)fRealLen))
				fPosition = (size_t)(retVal = position);
			break;

		case B_SEEK_CUR:
			if (position < 0 ? (int64)fPosition >= -position : position <= (int64)(fRealLen - fPosition)) {
				if (position < 0) fPosition -= (size_t)(-position);
				else fPosition += (size_t)position;
				retVal = (int64)fPosition;
			}
			break;

		case B_SEEK_END:
			if (position < 0 ? (int64)fLen >= -position : position <= (int64)(fRealLen - fLen)) {
				if (position < 0) fPosition = fLen - (size_t)(-position);
				else fPosition = fLen + (size_t)position;
				retVal = (int64)fPosition;
			}
			break;

		default:
			break;
	}

	return retVal;
}


int64
BMemoryIO::Position() const
{
	return (int64)fPosition;
}


status_t
BMemoryIO::SetSize(int64 size)
{
	if (size > (int64)fRealLen) return B_NO_MEMORY;
	fLen = (size_t)size;
	return B_OK;
}

