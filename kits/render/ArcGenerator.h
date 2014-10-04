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
 * File: ArcGenerator.h
 * Description: BArcGenerator --- Pixel generator for zero-width-arc-drawing
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_ARC_GENERATOR_H__
#define __ETK_ARC_GENERATOR_H__

#include <interface/Rect.h>

#ifdef __cplusplus /* Just for C++ */

class BArcGenerator
{
	public:
		BArcGenerator(BPoint center, float xRadius, float yRadius, BPoint start, BPoint end);

		bool Start(int32 &x, int32 &y, int32 &step, int32 &pixels, bool &both, bool isLoopX = true, float pixel_size = 1);
		bool Next(int32 &next, int32 &pixels, bool &both);

	private:
		BPoint fCenter;
		BPoint fRadius;
		BPoint fStart;
		BPoint fEnd;

		int32 fStep;
		bool fIsLoopX;

		BPoint fRadius2;
		BPoint _fRadius;
		BPoint _fStart;
		BPoint _fEnd;
		float fDeltaNext;

		int32 _fCenterX;
		int32 _fCenterY;
		int32 _fRadiusX;
		int32 _fStartX;
		int32 _fStartY;
		int32 _fEndX;
		int32 _fEndY;
		int32 _X;
		int32 _Y;
};

#endif /* __cplusplus */

#endif /* __ETK_ARC_GENERATOR_H__ */

