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
 * File: NetBuffer.cpp
 *
 * --------------------------------------------------------------------------*/

#include <support/ByteOrder.h>

#include "NetBuffer.h"


BNetBuffer::BNetBuffer(size_t size)
		: BArchivable(), fData(NULL), fSize(size), fPos(0)
{
	if (size > 0) fData = (unsigned char*)malloc(size);
}


BNetBuffer::BNetBuffer(const BNetBuffer &from)
		: BArchivable(), fData(NULL), fSize(0), fPos(0)
{
	BNetBuffer::operator=(from);
}


BNetBuffer::~BNetBuffer()
{
	if (fData) free(fData);
}


BNetBuffer::BNetBuffer(const BMessage *from)
		: BArchivable(from), fData(NULL), fSize(0), fPos(0)
{
	// TODO
}


status_t
BNetBuffer::Archive(BMessage *into, bool deep) const
{
	if (!into) return B_ERROR;

	BArchivable::Archive(into, deep);
	into->AddString("class", "BNetBuffer");

	// TODO

	return B_OK;
}


BArchivable*
BNetBuffer::Instantiate(const BMessage *from)
{
	if (e_validate_instantiation(from, "BNetBuffer"))
		return new BNetBuffer(from);
	return NULL;
}


status_t
BNetBuffer::InitCheck() const
{
	return((fData != NULL && fSize != 0 && fPos < fSize) ?B_OK :B_ERROR);
}


BNetBuffer&
BNetBuffer::operator=(const BNetBuffer &buf)
{
	if (fData) free(fData);

	if (!(buf.InitCheck() != B_OK ||
	        (fData = (unsigned char*)malloc(buf.fSize)) == NULL)) {
		memcpy(fData, buf.fData, fSize);
		fSize = buf.fSize;
		fPos = buf.fPos;
	} else {
		fData = NULL;
		fSize = 0;
		fPos = 0;
	}

	return *this;
}


status_t
BNetBuffer::AppendData(const void *data, size_t len)
{
	if (data == NULL || len == 0 ||
	        fData == NULL || fPos + len > fSize) return B_ERROR;

	memcpy(fData + fPos, data, len);
	fPos += len;

	return B_OK;
}


status_t
BNetBuffer::AppendInt8(int8 value)
{
	return AppendData(&value, 1);
}


status_t
BNetBuffer::AppendUint8(uint8 value)
{
	return AppendData(&value, 1);
}


status_t
BNetBuffer::AppendInt16(int16 value)
{
	int16 v = B_HOST_TO_BENDIAN_INT16(value);
	return AppendData(&v, 2);
}


status_t
BNetBuffer::AppendUint16(uint16 value)
{
	int16 v = B_HOST_TO_BENDIAN_INT16(value);
	return AppendData(&v, 2);
}


status_t
BNetBuffer::AppendInt32(int32 value)
{
	int32 v = B_HOST_TO_BENDIAN_INT32(value);
	return AppendData(&v, 4);
}


status_t
BNetBuffer::AppendUint32(uint32 value)
{
	int32 v = B_HOST_TO_BENDIAN_INT32(value);
	return AppendData(&v, 4);
}


status_t
BNetBuffer::AppendInt64(int64 value)
{
	int64 v = B_HOST_TO_BENDIAN_INT64(value);
	return AppendData(&v, 8);
}


status_t
BNetBuffer::AppendUint64(uint64 value)
{
	int64 v = B_HOST_TO_BENDIAN_INT64(value);
	return AppendData(&v, 8);
}


status_t
BNetBuffer::AppendFloat(float value)
{
	return AppendData(&value, sizeof(float));
}


status_t
BNetBuffer::AppendDouble(double value)
{
	return AppendData(&value, sizeof(double));
}


status_t
BNetBuffer::AppendString(const char *string, int32 len)
{
	size_t strLen = 0;

	if (fData == NULL) return B_ERROR;

	if (!(string == NULL || *string == 0)) strLen = strlen(string);
	if (!(len < 0 || (size_t)len >= strLen)) strLen = (size_t)len;

	if (fPos + strLen + 1 > fSize) return B_ERROR;

	if (strLen > 0) {
		memcpy(fData + fPos, string, strLen);
		fPos += strLen;
	}

	*(fData + (fPos++)) = 0;

	return B_OK;
}


status_t
BNetBuffer::AppendMessage(const BMessage &msg)
{
	if (fData == NULL) return B_ERROR;

	size_t msgSize = msg.FlattenedSize();
	if (msgSize == 0 || fPos + msgSize + 8 > fSize) return B_ERROR;

	char *buf = (char*)malloc(msgSize);
	if (buf == NULL) return B_NO_MEMORY;

	if (msg.Flatten(buf, msgSize) == false) {
		free(buf);
		return B_ERROR;
	}

	// data
	memcpy(fData + fPos, buf, msgSize);
	fPos += msgSize;
	free(buf);

	// size
	uint32 tmp = (uint32)msgSize;
#ifdef ETK_LITTLE_LENDIAN
	tmp = B_SWAP_INT32(tmp);
#endif
	memcpy(fData + fPos, &tmp, 4);
	fPos += 4;

	// magic
	*(fData + (fPos++)) = 0;
	*(fData + (fPos++)) = 'm';
	*(fData + (fPos++)) = 's';
	*(fData + (fPos++)) = 'g';

	return B_OK;
}


status_t
BNetBuffer::RemoveData(void *data, size_t len)
{
	if (data == NULL || len == 0 || fData == NULL || fPos < len) return B_ERROR;

	fPos -= len;
	memcpy(data, fData + fPos, len);

	return B_OK;
}


status_t
BNetBuffer::RemoveInt8(int8 &value)
{
	return RemoveData(&value, 1);
}


status_t
BNetBuffer::RemoveUint8(uint8 &value)
{
	return RemoveData(&value, 1);
}


status_t
BNetBuffer::RemoveInt16(int16 &value)
{
	if (RemoveData(&value, 2) != B_OK) return B_ERROR;

	value = B_BENDIAN_TO_HOST_INT16(value);
	return B_OK;
}


status_t
BNetBuffer::RemoveUint16(uint16 &value)
{
	if (RemoveData(&value, 2) != B_OK) return B_ERROR;

	value = B_BENDIAN_TO_HOST_INT16(value);
	return B_OK;
}


status_t
BNetBuffer::RemoveInt32(int32 &value)
{
	if (RemoveData(&value, 4) != B_OK) return B_ERROR;

	value = B_BENDIAN_TO_HOST_INT32(value);
	return B_OK;
}


status_t
BNetBuffer::RemoveUint32(uint32 &value)
{
	if (RemoveData(&value, 4) != B_OK) return B_ERROR;

	value = B_BENDIAN_TO_HOST_INT32(value);
	return B_OK;
}


status_t
BNetBuffer::RemoveInt64(int64 &value)
{
	if (RemoveData(&value, 8) != B_OK) return B_ERROR;

	value = B_BENDIAN_TO_HOST_INT64(value);
	return B_OK;
}


status_t
BNetBuffer::RemoveUint64(uint64 &value)
{
	if (RemoveData(&value, 8) != B_OK) return B_ERROR;

	value = B_BENDIAN_TO_HOST_INT64(value);
	return B_OK;
}


status_t
BNetBuffer::RemoveFloat(float &value)
{
	return RemoveData(&value, sizeof(float));
}


status_t
BNetBuffer::RemoveDouble(double &value)
{
	return RemoveData(&value, sizeof(double));
}


status_t
BNetBuffer::RemoveString(char *string, size_t len)
{
	if (string == NULL || len == 0 || fData == NULL || fPos == 0) return B_ERROR;

	if (*(fData + fPos - 1) != 0) return B_ERROR;
	if (len > fPos) len = fPos;

	fPos -= len;
	memcpy(string, fData + fPos, len);

	return B_OK;
}


status_t
BNetBuffer::RemoveMessage(BMessage &msg)
{
	if (fData == NULL || fPos <= 8) return B_ERROR;

	const unsigned char *magic = fData + fPos - 4;
	if (magic[0] != 0 || magic[1] != 'm' || magic[2] != 's' || magic[3] != 'g') return B_ERROR;

	uint32 msgSize = 0;
	memcpy(&msgSize, fData + fPos - 8, 4);
#ifdef ETK_LITTLE_LENDIAN
	msgSize = B_SWAP_INT32(msgSize);
#endif
	if (msgSize == 0 || (size_t)msgSize > fPos - 8) return B_ERROR;

	if (msg.Unflatten((const char*)(fData + fPos - (size_t)msgSize - 8),
	                  (size_t)msgSize) == false) return B_ERROR;

	fPos -= (size_t)msgSize + 8;
	return B_OK;
}


unsigned char*
BNetBuffer::Data() const
{
	return fData;
}


size_t
BNetBuffer::Size() const
{
	return fSize;
}


size_t
BNetBuffer::BytesRemaining() const
{
	return(InitCheck() == B_OK ? fSize - fPos : 0);
}

