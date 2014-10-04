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
 * File: LineGenerator.cpp
 * Description: BLineGenerator --- Pixel generator for zero-width-line-drawing
 *
 * --------------------------------------------------------------------------*/

/* --------------------------------------------------------------------------
<document lang="zh_CN.UTF-8">
<documentinfo>
	<title>像素方式直线生成器</title>
</documentinfo>
<xref linkend="ELINEGENERATOR_DESCRIPTION" /><para></para>
<xref linkend="ELINEGENERATOR_FUNCTIONS" />
</document>
-----------------------------------------------------------------------------*/

/* --------------------------------------------------------------------------
<document lang="zh_CN.UTF-8">
<section id="ELINEGENERATOR_DESCRIPTION">
	<title>BLineGenerator类描述</title>
	<para>声明所在：<emphasis>&lt;render/LineGenerator.h&gt;</emphasis></para>
	<para>链 接 库：<emphasis>libetkxx</emphasis></para>
	<para>派生关系：<emphasis>BLineGenerator</emphasis></para>
	<para>BLineGenerator是一个用来生成无宽度直线的逐个像素点坐标的类(采用Bresenham法)。</para>
</section>

<section id="ELINEGENERATOR_FUNCTIONS">
	<title>BLineGenerator类成员函数</title>
	<xref linkend="ELINEGENERATOR_FUNCTION_CONSTRUCT" /><para></para>
	<xref linkend="ELINEGENERATOR_FUNCTION_START" /><para></para>
	<xref linkend="ELINEGENERATOR_FUNCTION_NEXT" />
</section>
</document>
-----------------------------------------------------------------------------*/

#include <math.h>
#include <string.h>

#include "LineGenerator.h"

/* --------------------------------------------------------------------------
<document lang="zh_CN.UTF-8">
<section id="ELINEGENERATOR_FUNCTION_CONSTRUCT">
	<title>构造函数</title>
	<programlisting>
BLineGenerator::BLineGenerator(BPoint <emphasis>start</emphasis>, BPoint <emphasis>end</emphasis>)
	</programlisting>
	<itemizedlist>
		<listitem><para><emphasis>start</emphasis>是开始点坐标。
		</para></listitem>
		<listitem><para><emphasis>end</emphasis>是结束点坐标。
		</para></listitem>
	</itemizedlist>
</section>
</document>
-----------------------------------------------------------------------------*/
BLineGenerator::BLineGenerator(BPoint start, BPoint end)
		: fStep(0)
{
	fStart = start;
	fEnd = end;

	InitDirection();
}


// 初始化直线方向
// fDirection: 0 - 7 (0为水平递增方向,相应逆时针方向递增)
void
BLineGenerator::InitDirection()
{
	if (fEnd.y == fStart.y)
		fDirection = (fEnd.x >= fStart.x ? 0 : 4);
	else if (fEnd.x == fStart.x)
		fDirection = (fEnd.y > fStart.y ? 6 : 2);
	else if (fEnd.x > fStart.x)
		fDirection = (fEnd.y > fStart.y ? 7 : 1);
	else // fEnd.x < fStart.x
		fDirection = (fEnd.y > fStart.y ? 5 : 3);
}


/* --------------------------------------------------------------------------
<document lang="zh_CN.UTF-8">
<section id="ELINEGENERATOR_FUNCTION_START">
	<title>开始像素点生成</title>
	<programlisting>
bool BLineGenerator::Start(int32 &<emphasis>x</emphasis>,
                           int32 &<emphasis>y</emphasis>,
                           int32 &<emphasis>step</emphasis>,
                           int32 &<emphasis>pixels</emphasis>,
                           bool <emphasis>isLoopX = true</emphasis>,
                           float <emphasis>pixelSize = 1</emphasis>)
	</programlisting>
	<itemizedlist>
		<listitem><para><emphasis>x</emphasis>是开始像素点 x 坐标。
		</para></listitem>
		<listitem><para><emphasis>y</emphasis>是开始像素点 y 坐标。
		</para></listitem>
		<listitem><para><emphasis>step</emphasis>是需要循环的次数(不包括开始像素点)。
			<footnote><para>正值为 x(y) 递增, 负值为 x(y) 递减。</para></footnote>
		</para></listitem>
		<listitem><para><emphasis>pixels</emphasis>是开始像素点处另外需要绘制的像素点数(不包括开始像素点)。
			<footnote><para>正值为 y(x) 递增, 负值为 y(x) 递减。</para></footnote>
		</para></listitem>
		<listitem><para><emphasis>isLooperX</emphasis>是否沿 X 轴循环生成点，当为false时表示沿 Y 轴循环生成点。
		</para></listitem>
		<listitem><para><emphasis>pixelSize</emphasis>是每个像素相对应的坐标大小。
		</para></listitem>
	</itemizedlist>
	<para>BLineGenerator::Start()声明开始画直线, 返回true时表示需要画点, 反之表示不需要画点。</para>
</section>
</document>
-----------------------------------------------------------------------------*/
bool
BLineGenerator::Start(int32 &x, int32 &y, int32 &step, int32 &pixels, bool isLoopX, float pixel_size)
{
	if (pixel_size <= 0) return false;

	float minf = B_MINFLOAT * pixel_size;
	float maxf = B_MAXFLOAT * pixel_size;

	// 避免浮点溢出
	if ((fStart.x > 0 && fStart.x > maxf) || (fStart.x < 0 && fStart.x < minf)) return false;
	if ((fStart.y > 0 && fStart.y > maxf) || (fStart.y < 0 && fStart.y < minf)) return false;
	if ((fEnd.x > 0 && fEnd.x > maxf) || (fEnd.x < 0 && fEnd.x < minf)) return false;
	if ((fEnd.y > 0 && fEnd.y > maxf) || (fEnd.y < 0 && fEnd.y < minf)) return false;

	BPoint ptStart = fStart;
	BPoint ptEnd = fEnd;

	if (pixel_size != 1) {
		ptStart.x /= pixel_size;
		ptStart.y /= pixel_size;
		ptEnd.x /= pixel_size;
		ptEnd.y /= pixel_size;
	}

	fIsLoopX = isLoopX;
	fStep = 0;

	BPoint ptStartR = ptStart.FloorCopy();
	BPoint ptEndR = ptEnd.FloorCopy();

	if (ptStart.x == ptStartR.x || ptStart.y == ptStartR.y) {
		if (ptStart.y == ptStartR.y) if (fDirection > 0 && fDirection < 4) ptStartR.y -= 1;
		if (ptStart.x == ptStartR.x) if (fDirection > 2 && fDirection < 6) ptStartR.x -= 1;
	}

	if (ptEnd.x == ptEndR.x || ptEnd.y == ptEndR.y) {
		if (ptEnd.y == ptEndR.y) if (fDirection > 4 && fDirection <= 7) ptEndR.y -= 1;
		if (ptEnd.x == ptEndR.x) if (fDirection == 7 || fDirection < 2) ptEndR.x -= 1;
	}

	// 数值范围检查
	if (ptStartR.x > (float)B_MAXINT32 || ptStartR.x < (float)B_MININT32) return false;
	if (ptStartR.y > (float)B_MAXINT32 || ptStartR.y < (float)B_MININT32) return false;
	if (ptEndR.x > (float)B_MAXINT32 || ptEndR.x < (float)B_MININT32) return false;
	if (ptEndR.y > (float)B_MAXINT32 || ptEndR.y < (float)B_MININT32) return false;

	x = (int32)ptStartR.x;
	y = (int32)ptStartR.y;

	int32 x2 = (int32)ptEndR.x;
	int32 y2 = (int32)ptEndR.y;

	if ((step = (fIsLoopX ? x2 - x : y2 - y)) == 0) {
		pixels = (fIsLoopX ? y2 - y : x2 - x);

		if (pixels != 0) { /* 避免y(x)在调用过程中的加减 */
			if (fIsLoopX)
				y += pixels;
			else
				x += pixels;
			pixels = -pixels;
		}

		return true;
	}

	fStepOther = (fIsLoopX ? y2 - y : x2 - x);
	fStep = step;

	pixels = 0;

	if (fIsLoopX) {
		fDeltaElta = (ptEnd.y - ptStart.y) / (ptEnd.x - ptStart.x);
		if (fDeltaElta < 0) fDeltaElta = -fDeltaElta;
		if (fDirection <= 4) fDeltaElta = -fDeltaElta;

		float deltaX = ptStart.x - ((float)x + 0.5f);
		if (step < 0)
			deltaX = 1 + deltaX;
		else
			deltaX = 1 - deltaX;
		fEltaNextAbs = fEltaNext = ptStart.y - ((float)y + 0.5f) + deltaX * fDeltaElta;

		if (fEltaNextAbs < 0) fEltaNextAbs = -fEltaNextAbs;

		if (fEltaNextAbs > 1.5 || (fEltaNextAbs == 1.5 && fEltaNext >= 0)) { // 斜率过大
			float eltaEdge, eltaEdgeAbs;
			eltaEdgeAbs = eltaEdge = fEltaNext - fDeltaElta * 0.5f;
			if (eltaEdgeAbs < 0) eltaEdgeAbs = -eltaEdgeAbs;

			while ((eltaEdgeAbs > 1 || (eltaEdgeAbs == 1 && fDirection > 2 && fDirection < 6)) && fStepOther != 0) {
				pixels += (eltaEdge > 0 ? 1 : -1);
				fStepOther += (eltaEdge > 0 ? -1 : 1);
				eltaEdgeAbs--;
			}

			if (pixels != 0) {
				fEltaNext -= (float)pixels;
				fEltaNextAbs = (fEltaNext < 0 ? -fEltaNext : fEltaNext);
			}
		}
	} else {
		fDeltaElta = (ptEnd.x - ptStart.x) / (ptEnd.y - ptStart.y);
		if (fDeltaElta < 0) fDeltaElta = -fDeltaElta;
		if (fDirection > 2 && fDirection < 6) fDeltaElta = -fDeltaElta;

		float deltaY = ptStart.y - ((float)y + 0.5f);
		if (step < 0)
			deltaY = 1 + deltaY;
		else
			deltaY = 1 - deltaY;
		fEltaNextAbs = fEltaNext = ptStart.x - ((float)x + 0.5f) + deltaY * fDeltaElta;

		if (fEltaNextAbs < 0) fEltaNextAbs = -fEltaNextAbs;

		if (fEltaNextAbs > 1.5 || (fEltaNextAbs == 1.5 && fEltaNext >= 0)) { // 斜率过大
			float eltaEdge, eltaEdgeAbs;
			eltaEdgeAbs = eltaEdge = fEltaNext - fDeltaElta * 0.5f;
			if (eltaEdgeAbs < 0) eltaEdgeAbs = -eltaEdgeAbs;

			while ((eltaEdgeAbs > 1 || (eltaEdgeAbs == 1 && fDirection > 0 && fDirection < 4)) && fStepOther != 0) {
				pixels += (eltaEdge > 0 ? 1 : -1);
				fStepOther += (eltaEdge > 0 ? -1 : 1);
				eltaEdgeAbs--;
			}

			if (pixels != 0) {
				fEltaNext -= (float)pixels;
				fEltaNextAbs = (fEltaNext < 0 ? -fEltaNext : fEltaNext);
			}
		}
	}

	if (pixels != 0) { /* 避免y(x)在调用过程中的加减 */
		if (fIsLoopX)
			y += pixels;
		else
			x += pixels;
		pixels = -pixels;
	}

	return true;
}


/* --------------------------------------------------------------------------
<document lang="zh_CN.UTF-8">
<section id="ELINEGENERATOR_FUNCTION_NEXT">
	<title>下一个像素点生成</title>
	<programlisting>
bool BLineGenerator::Next(int32 &<emphasis>next</emphasis>, int32 &<emphasis>pixels</emphasis>)
	</programlisting>
	<itemizedlist>
		<listitem><para><emphasis>next</emphasis>是下一像素点 y(x) 坐标。
			<footnote><para>注意此处自动利用原来像素点坐标进行加减运算。</para></footnote>
		</para></listitem>
		<listitem><para><emphasis>pixels</emphasis>是下一像素点处另外需要绘制的像素点数(不包括该像素点)。
			<footnote><para>正值为 y(x) 递增, 负值为 y(x) 递减。</para></footnote>
		</para></listitem>
	</itemizedlist>
	<para>BLineGenerator::Next()用于取得下一像素点 x(y) 坐标对应 y(x) 坐标, 返回true时表示需要画点, 反之表示不需要画点(或许未使用 Start(...) 函数)；注意使用Next()时x(y)应在此前根据调用Start()返回的step进行相应的加减运算。</para>
</section>
</document>
-----------------------------------------------------------------------------*/
bool
BLineGenerator::Next(int32 &next, int32 &pixels)
{
	if (fStep == 0) return false;

	if (fIsLoopX) {
		if (fEltaNextAbs < 0.5f || (fEltaNextAbs == 0.5f && fEltaNext < 0)) { // y 不变
			fEltaNext += fDeltaElta;
		} else { // fEltaNext > 0 则 y+1, 否则 y-1
			next += (fEltaNext > 0 ? 1 : -1);
			fStepOther += (fEltaNext > 0 ? -1 : 1);
			fEltaNext += (fEltaNext > 0 ? -1 : 1);
			fEltaNext += fDeltaElta;
		}

		pixels = 0;
		fEltaNextAbs = (fEltaNext < 0 ? -fEltaNext : fEltaNext);

		if (fEltaNextAbs > 1.5 || (fEltaNextAbs == 1.5 && fEltaNext >= 0)) { // 斜率过大
			float eltaEdge, eltaEdgeAbs;
			eltaEdgeAbs = eltaEdge = fEltaNext - fDeltaElta * 0.5f;
			if (eltaEdgeAbs < 0) eltaEdgeAbs = -eltaEdgeAbs;

			while ((eltaEdgeAbs > 1 || (eltaEdgeAbs == 1 && fDirection > 2 && fDirection < 6)) && fStepOther != 0) {
				pixels += (eltaEdge > 0 ? 1 : -1);
				fStepOther += (eltaEdge > 0 ? -1 : 1);
				eltaEdgeAbs--;
			}

			if (pixels != 0) {
				fEltaNext -= (float)pixels;
				fEltaNextAbs = (fEltaNext < 0 ? -fEltaNext : fEltaNext);
			}
		}
	} else {
		if (fEltaNextAbs < 0.5f || (fEltaNextAbs == 0.5f && fEltaNext < 0)) { // x 不变
			fEltaNext += fDeltaElta;
		} else { // fEltaNext > 0 则 x+1, 否则 x-1
			next += (fEltaNext > 0 ? 1 : -1);
			fStepOther += (fEltaNext > 0 ? -1 : 1);
			fEltaNext += (fEltaNext > 0 ? -1 : 1);
			fEltaNext += fDeltaElta;
		}

		pixels = 0;
		fEltaNextAbs = (fEltaNext < 0 ? -fEltaNext : fEltaNext);

		if (fEltaNextAbs > 1.5 || (fEltaNextAbs == 1.5 && fEltaNext >= 0)) { // 斜率过大
			float eltaEdge, eltaEdgeAbs;
			eltaEdgeAbs = eltaEdge = fEltaNext - fDeltaElta * 0.5f;
			if (eltaEdgeAbs < 0) eltaEdgeAbs = -eltaEdgeAbs;

			while ((eltaEdgeAbs > 1 || (eltaEdgeAbs == 1 && fDirection > 0 && fDirection < 4)) && fStepOther != 0) {
				pixels += (eltaEdge > 0 ? 1 : -1);
				fStepOther += (eltaEdge > 0 ? -1 : 1);
				eltaEdgeAbs--;
			}

			if (pixels != 0) {
				fEltaNext -= (float)pixels;
				fEltaNextAbs = (fEltaNext < 0 ? -fEltaNext : fEltaNext);
			}
		}
	}

	fStep += (fStep > 0 ? -1 : 1);

	if (pixels != 0) { /* 避免next在调用过程中的加减 */
		next += pixels;
		pixels = -pixels;
	}

	return true;
}

