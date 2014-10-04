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
 * File: Cursor.cpp
 * Description: mouse cursor for application
 *
 * --------------------------------------------------------------------------*/


#include "Cursor.h"


static uint32 get_cursor_data_bits_length(const uint8 *data)
{
	if (data == NULL || data[0] == 0 || data[1] > 32) return 0;
	uint32 rowBytes = (((uint32)data[0] * (uint32)data[1] + 0x00000007) & 0xfffffff8) >> 3;
	return((uint32)data[0] * (uint32)rowBytes);
}


static uint32 get_cursor_data_length(const uint8 *data)
{
	uint32 bits_length = get_cursor_data_bits_length(data);
	return(bits_length > 0 ? (4 + 2 * bits_length) : 0);
}


static void* duplicate_cursor_data(const uint8 *data)
{
	size_t len = (size_t)get_cursor_data_length(data);

	void *cursor_data = (len > 0 ? malloc(len) : NULL);
	if (cursor_data) memcpy(cursor_data, data, len);

	return cursor_data;
}


BCursor::BCursor(const void *cursorData)
		: BArchivable()
{
	fData = duplicate_cursor_data((const uint8*)cursorData);
}


BCursor::BCursor(const BCursor &cursor)
		: BArchivable()
{
	fData = duplicate_cursor_data((const uint8*)cursor.fData);
}


BCursor::~BCursor()
{
	if (fData) free(fData);
}


BCursor&
BCursor::operator=(const BCursor &from)
{
	if (fData) free(fData);
	fData = duplicate_cursor_data((const uint8*)from.fData);
	return *this;
}


bool
BCursor::operator==(const BCursor &other) const
{
	if (DataLength() == 0 && other.DataLength() == 0) return true;
	if (DataLength() == 0 || other.DataLength() == 0 || DataLength() != other.DataLength()) return false;
	return(memcmp(Data(), other.Data(), (size_t)DataLength()) == 0);
}


bool
BCursor::operator!=(const BCursor &other) const
{
	if (DataLength() == 0 && other.DataLength() == 0) return false;
	if (DataLength() == 0 || other.DataLength() == 0 || DataLength() != other.DataLength()) return true;
	return(memcmp(Data(), other.Data(), (size_t)DataLength()) != 0);
}


const void*
BCursor::Data() const
{
	return fData;
}


uint32
BCursor::DataLength() const
{
	return(get_cursor_data_length((const uint8*)fData));
}


uint8
BCursor::Width() const
{
	return(fData ? *((const uint8*)fData) : 0);
}


uint8
BCursor::Height() const
{
	return(fData ? *((const uint8*)fData) : 0);
}


uint8
BCursor::ColorDepth() const
{
	return(fData ? *((const uint8*)fData + 1) : 0);
}


uint16
BCursor::Spot() const
{
	if (fData == NULL) return 0;

#ifdef ETK_LITTLE_ENDIAN
	return(*((uint16*)fData + 1));
#else
	const uint8 *tmp = (const uint8*)fData + 2;
	return((uint16)tmp[0] | ((uint16)tmp[1] << 8));
#endif
}


const void*
BCursor::Bits() const
{
	if (fData == NULL) return NULL;
	return((const void*)((const uint32*)fData + 1));
}


const void*
BCursor::Mask() const
{
	if (fData == NULL) return NULL;
	return((const void*)((const uint8*)fData + 4 + get_cursor_data_bits_length((const uint8*)fData)));
}


static uint8 cursor_hand[] = {
	/* size, depth */
	16, 1,

	/* Y, X */
	1, 3,

	/* bits */
	0x70, 0x00,	// 0111000000000000
	0x48, 0x00,	// 0100100000000000
	0x48, 0x00,	// 0100100000000000
	0x27, 0xc0,	// 0010011111000000
	0x24, 0xb8,	// 0010010010111000
	0x12, 0x54,	// 0001001001010100
	0x10, 0x02,	// 0001000000000010
	0x78, 0x02,	// 0111100000000010
	0x98, 0x02,	// 1001100000000010
	0x80, 0x02,	// 1000000000000010
	0x60, 0x02,	// 0110000000000010
	0x18, 0x02,	// 0001100000000010
	0x04, 0x00,	// 0000010000000000
	0x02, 0x00,	// 0000001000000000
	0x01, 0x00,	// 0000000100000000
	0x00, 0x00,	// 0000000000000000

	/* mask */
	0x70, 0x00,	// 0111000000000000
	0x78, 0x00,	// 0111100000000000
	0x78, 0x00,	// 0111100000000000
	0x3f, 0xc0,	// 0011111111000000
	0x3f, 0xf8,	// 0011111111111000
	0x1f, 0xfc,	// 0001111111111100
	0x1f, 0xfe,	// 0001111111111110
	0x7f, 0xfe,	// 0111111111111110
	0xff, 0xfe,	// 1111111111111110
	0xff, 0xfe,	// 1111111111111110
	0x7f, 0xfe,	// 0111111111111110
	0x1f, 0xfe,	// 0001111111111110
	0x07, 0xfc,	// 0000011111111100
	0x03, 0xf8,	// 0000001111111000
	0x01, 0xf0,	// 0000000111110000
	0x00, 0x00,	// 0000000000000000
};


static uint8 cursor_hand_move[] = {
	/* size, depth */
	16, 1,

	/* Y, X */
	1, 3,

	/* bits */
	0x70, 0x00,	// 0111000000000000
	0x48, 0x00,	// 0100100000000000
	0x48, 0x00,	// 0100100000000000
	0x27, 0xc0,	// 0010011111000000
	0x24, 0xb8,	// 0010010010111000
	0x12, 0x54,	// 0001001001010100
	0x10, 0x02,	// 0001000000000010
	0x78, 0x42,	// 0111100001000010
	0x98, 0xe2,	// 1001100011100010
	0x81, 0x52,	// 1000000101010010
	0x63, 0xfa,	// 0110001111111010
	0x19, 0x52,	// 0001100101010010
	0x04, 0xe0,	// 0000010011100000
	0x02, 0x40,	// 0000001001000000
	0x01, 0x00,	// 0000000100000000
	0x00, 0x00,	// 0000000000000000

	/* mask */
	0x70, 0x00,	// 0111000000000000
	0x78, 0x00,	// 0111100000000000
	0x78, 0x00,	// 0111100000000000
	0x3f, 0xc0,	// 0011111111000000
	0x3f, 0xf8,	// 0011111111111000
	0x1f, 0xfc,	// 0001111111111100
	0x1f, 0xfe,	// 0001111111111110
	0x7f, 0xfe,	// 0111111111111110
	0xff, 0xfe,	// 1111111111111110
	0xff, 0xfe,	// 1111111111111110
	0x7f, 0xfe,	// 0111111111111110
	0x1f, 0xfe,	// 0001111111111110
	0x07, 0xfc,	// 0000011111111100
	0x03, 0xf8,	// 0000001111111000
	0x01, 0xf0,	// 0000000111110000
	0x00, 0x00,	// 0000000000000000
};


static uint8 cursor_i_beam[] = {
	/* size, depth */
	16, 1,

	/* Y, X */
	8, 8,

	/* bits */
	0x00, 0x00,	// 0000000000000000
	0x00, 0x00,	// 0000000000000000
	0x0f, 0xf0,	// 0000111111110000
	0x01, 0x00,	// 0000000100000000
	0x01, 0x00,	// 0000000100000000
	0x01, 0x00,	// 0000000100000000
	0x01, 0x00,	// 0000000100000000
	0x01, 0x00,	// 0000000100000000
	0x01, 0x00,	// 0000000100000000
	0x01, 0x00,	// 0000000100000000
	0x01, 0x00,	// 0000000100000000
	0x01, 0x00,	// 0000000100000000
	0x01, 0x00,	// 0000000100000000
	0x0f, 0xf0,	// 0000111111110000
	0x00, 0x00,	// 0000000000000000
	0x00, 0x00,	// 0000000000000000

	/* mask */
	0x00, 0x00,	// 0000000000000000
	0x1f, 0xf8,	// 0001111111111000
	0x1f, 0xf8,	// 0001111111111000
	0x1f, 0xf8,	// 0001111111111000
	0x03, 0x80,	// 0000001110000000
	0x03, 0x80,	// 0000001110000000
	0x03, 0x80,	// 0000001110000000
	0x03, 0x80,	// 0000001110000000
	0x03, 0x80,	// 0000001110000000
	0x03, 0x80,	// 0000001110000000
	0x03, 0x80,	// 0000001110000000
	0x03, 0x80,	// 0000001110000000
	0x1f, 0xf8,	// 0001111111111000
	0x1f, 0xf8,	// 0001111111111000
	0x1f, 0xf8,	// 0001111111111000
	0x00, 0x00,	// 0000000000000000
};


_LOCAL BCursor _B_CURSOR_HAND(cursor_hand);
_LOCAL BCursor _B_CURSOR_HAND_MOVE(cursor_hand_move);
_LOCAL BCursor _B_CURSOR_I_BEAM(cursor_i_beam);

const BCursor *B_CURSOR_HAND = &_B_CURSOR_HAND;
const BCursor *B_CURSOR_HAND_MOVE = &_B_CURSOR_HAND_MOVE;
const BCursor *B_CURSOR_I_BEAM = &_B_CURSOR_I_BEAM;

