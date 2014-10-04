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
 * File: VolumeRoster.cpp
 *
 * --------------------------------------------------------------------------*/

#include "VolumeRoster.h"


BVolumeRoster::BVolumeRoster()
		: fPos(0)
{
}


BVolumeRoster::~BVolumeRoster()
{
}


status_t
BVolumeRoster::GetNextVolume(BVolume *vol)
{
	if (vol == NULL) return B_BAD_VALUE;

	BVolume aVol;
	while (true) {
		fPos++;

		status_t status = aVol.SetTo((e_dev_t)fPos);
		if (status == B_ENTRY_NOT_FOUND) return B_BAD_VALUE;
		if (status == B_BAD_VALUE) continue;
		if (status != B_OK) return status;

		status = vol->SetTo(aVol.Device());
		return status;
	}

	return B_NO_ERROR;
}


void
BVolumeRoster::Rewind()
{
	fPos = 0;
}


status_t
BVolumeRoster::GetBootVolume(BVolume *vol)
{
	if (vol == NULL) return B_BAD_VALUE;

#ifdef _WIN32
	if (vol->SetTo(3) == B_OK) return B_NO_ERROR;
	return B_ENTRY_NOT_FOUND;
#else
	e_dev_t dev = 0;
	BVolume aVol;
	while (true) {
		dev++;

		status_t status = aVol.SetTo(dev);
		if (status == B_ENTRY_NOT_FOUND) return B_BAD_VALUE;
		if (status == B_BAD_VALUE) continue;
		if (status != B_OK) return status;

		BDirectory dir;
		if (aVol.GetRootDirectory(&dir) != B_OK) continue;

		BEntry entry;
		if (dir.GetEntry(&entry) != B_OK) continue;

		BPath path;
		if (entry.GetPath(&path) != B_OK) continue;

#ifndef __BEOS__
		if (path != "/") continue;
#else
	if (path != "/boot") continue;
#endif

		if (vol->SetTo(dev) == B_OK) return B_NO_ERROR;
		break;
	}

	return B_ENTRY_NOT_FOUND;
#endif
}

