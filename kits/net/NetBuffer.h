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
 * File: NetBuffer.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_NET_BUFFER_H__
#define __ETK_NET_BUFFER_H__

#include <support/Archivable.h>

#ifdef __cplusplus /* Just for C++ */

class BNetBuffer : public BArchivable
{
	public:
		BNetBuffer(size_t size = 0);
		BNetBuffer(const BNetBuffer &from);
		virtual ~BNetBuffer();

		// Archiving
		BNetBuffer(const BMessage *from);
		virtual status_t Archive(BMessage *into, bool deep = true) const;
		static BArchivable *Instantiate(const BMessage *from);

		status_t	InitCheck() const;

		BNetBuffer	&operator=(const BNetBuffer &buf);

		status_t	AppendData(const void *data, size_t len);
		status_t	AppendInt8(int8 value);
		status_t	AppendUint8(uint8 value);
		status_t	AppendInt16(int16 value);
		status_t	AppendUint16(uint16 value);
		status_t	AppendInt32(int32 value);
		status_t	AppendUint32(uint32 value);
		status_t	AppendInt64(int64 value);
		status_t	AppendUint64(uint64 value);
		status_t	AppendFloat(float value);
		status_t	AppendDouble(double value);
		status_t	AppendString(const char *string, int32 len = -1);
		status_t	AppendMessage(const BMessage &msg);

		status_t	RemoveData(void *data, size_t len);
		status_t	RemoveInt8(int8 &value);
		status_t	RemoveUint8(uint8 &value);
		status_t	RemoveInt16(int16 &value);
		status_t	RemoveUint16(uint16 &value);
		status_t	RemoveInt32(int32 &value);
		status_t	RemoveUint32(uint32 &value);
		status_t	RemoveInt64(int64 &value);
		status_t	RemoveUint64(uint64 &value);
		status_t	RemoveFloat(float &value);
		status_t	RemoveDouble(double &value);
		status_t	RemoveString(char *string, size_t len);
		status_t	RemoveMessage(BMessage &msg);

		unsigned char	*Data() const;
		size_t		Size() const;
		size_t		BytesRemaining() const;

	private:
		unsigned char *fData;
		size_t fSize;
		size_t fPos;
};

#endif /* __cplusplus */

#endif /* __ETK_NET_BUFFER_H__ */

