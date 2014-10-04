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
 * File: Region.cpp
 * Description: BRegion --- Combination of rectangles to describe region
 *
 * --------------------------------------------------------------------------*/

/* --------------------------------------------------------------------------
<document lang="zh_CN.UTF-8">
<documentinfo>
	<title>区域范围描述</title>
</documentinfo>
<xref linkend="EREGION_DESCRIPTION" /><para></para>
<xref linkend="EREGION_FUNCTIONS" />
</document>
-----------------------------------------------------------------------------*/

/* --------------------------------------------------------------------------
<document lang="zh_CN.UTF-8">
<section id="EREGION_DESCRIPTION">
	<title>BRegion类描述</title>
	<para>声明所在：<emphasis>&lt;interface/Region.h&gt;</emphasis></para>
	<para>链 接 库：<emphasis>libetkxx</emphasis></para>
	<para>派生关系：<emphasis>BRegion</emphasis></para>
	<para>BRegion是一个由多个矩形组合的区域范围描述类，主要用于描述不规则区域。</para>
</section>

<section id="EREGION_FUNCTIONS">
	<title>BRegion类成员函数</title>
	<xref linkend="EREGION_FUNCTION_CONSTRUCT" /><para></para>
	<xref linkend="EREGION_FUNCTION_OPERATOR" /><para></para>
	<xref linkend="EREGION_FUNCTION_FRAME" /><para></para>
	<xref linkend="EREGION_FUNCTION_COUNT_RECTS" /><para></para>
	<xref linkend="EREGION_FUNCTION_RECT_AT" /><para></para>
	<xref linkend="EREGION_FUNCTION_MODIFICATION" /><para></para>
	<xref linkend="EREGION_FUNCTION_QUERY" />
</section>
</document>
-----------------------------------------------------------------------------*/

#include "Region.h"

/* --------------------------------------------------------------------------
<document lang="zh_CN.UTF-8">
<section id="EREGION_FUNCTION_CONSTRUCT">
	<title>构造函数</title>
	<programlisting>
BRegion::BRegion()
BRegion::BRegion(const BRegion &<emphasis>region</emphasis>)
BRegion::BRegion(const BRect &<emphasis>rect</emphasis>)
virtual BRegion::~BRegion()
	</programlisting>
	<itemizedlist>
		<listitem><para><emphasis>region</emphasis>是已创建的区域范围描述。
		</para></listitem>
		<listitem><para><emphasis>rect</emphasis>是初始的一个矩形区域范围。
		</para></listitem>
	</itemizedlist>
</section>
</document>
-----------------------------------------------------------------------------*/
BRegion::BRegion()
{
}


BRegion::BRegion(const BRegion &region)
{
	if (region.fRects.CountItems() > 0) *this = region;
}


BRegion::BRegion(const BRect &rect)
{
	Set(rect);
}


BRegion::~BRegion()
{
	MakeEmpty();
}


/* --------------------------------------------------------------------------
<document lang="zh_CN.UTF-8">
<section id="EREGION_FUNCTION_OPERATOR">
	<title>区域范围操作符</title>
	<programlisting>
BRegion& BRegion::operator=(const BRegion &<emphasis>region</emphasis>)

BRegion BRegion::operator&(BRect <emphasis>rect</emphasis>) const
BRegion BRegion::operator|(BRect <emphasis>rect</emphasis>) const

BRegion& BRegion::operator&=(BRect <emphasis>rect</emphasis>)
BRegion& BRegion::operator|=(BRect <emphasis>rect</emphasis>)

BRegion BRegion::operator&(const BRegion &<emphasis>region</emphasis>) const
BRegion BRegion::operator|(const BRegion &<emphasis>region</emphasis>) const

BRegion& BRegion::operator&=(const BRegion &<emphasis>region</emphasis>)
BRegion& BRegion::operator|=(const BRegion &<emphasis>region</emphasis>)
	</programlisting>
	<itemizedlist>
		<listitem><para><emphasis>region</emphasis>与其操作的区域范围描述。
			<footnote><para>如果该区域范围描述为无效区域时，进行赋值(operator=)
					或位与(operator&=)操作时会导致当前区域范围描述无效。</para></footnote>
		</para></listitem>
		<listitem><para><emphasis>rect</emphasis>与其操作的矩形区域。
			<footnote><para>如果该矩形为无效矩形时，进行赋值(operator=)
					或位与(operator&=)操作时会导致当前区域范围描述无效。</para></footnote>
		</para></listitem>
	</itemizedlist>
	<para>BRegion::operator=()用于赋值给当前区域，返回值为赋值后的当前区域本身引用。</para>
	<para>BRegion::operator&()用于求当前区域与指定参数的交集，返回值为相交的区域。</para>
	<para>BRegion::operator&=()用于将当前区域与指定参数进行交集操作，返回值为操作后的当前区域本身引用。</para>
	<para>BRegion::operator|()用于求当前区域与指定参数的并集，返回值为联合的区域。</para>
	<para>BRegion::operator|=()用于将当前区域与指定参数进行并集操作，返回值为操作后的当前区域本身引用。</para>
</section>
</document>
-----------------------------------------------------------------------------*/
BRegion&
BRegion::operator=(const BRegion &from)
{
	MakeEmpty();

	if (from.fRects.CountItems() > 0) {
		for (int32 i = 0; i < from.fRects.CountItems(); i++) {
			BRect *r = (BRect*)from.fRects.ItemAt(i);
			BRect *nr = ((!r || r->IsValid() == false) ? NULL : new BRect(*r));
			if (!nr || fRects.AddItem(nr) == false) {
				if (nr) delete nr;
				MakeEmpty();
				break;
			} else {
				fFrame = (fFrame.IsValid() ? (fFrame | *r) : *r);
			}
		}
	}

	return *this;
}


/* --------------------------------------------------------------------------
<document lang="zh_CN.UTF-8">
<section id="EREGION_FUNCTION_FRAME">
	<title>区域描述的最大矩形范围</title>
	<programlisting>
BRect BRegion::Frame() const
	</programlisting>
	<para>返回值为该区域描述的最大矩形范围，当区域无效时返回值亦无效。</para>
</section>
</document>
-----------------------------------------------------------------------------*/
BRect
BRegion::Frame() const
{
	return fFrame;
}


/* --------------------------------------------------------------------------
<document lang="zh_CN.UTF-8">
<section id="EREGION_FUNCTION_RECT_AT">
	<title>区域描述的组成矩形</title>
	<programlisting>
BRect BRegion::RectAt(int32 <emphasis>index</emphasis>) const
	</programlisting>
	<itemizedlist>
		<listitem><para><emphasis>index</emphasis>是组成矩形的位置索引。
		<footnote><para>其值应界于0和BRegion::CountRects() - 1之间。</para></footnote>
		</para></listitem>
	</itemizedlist>
	<para>返回值为该区域第index+1个组合矩形，当index处于无效范围时返回值亦无效。</para>
</section>
</document>
-----------------------------------------------------------------------------*/
BRect
BRegion::RectAt(int32 index) const
{
	BRect *r = (BRect*)fRects.ItemAt(index);
	return(r ? *r : BRect());
}


/* --------------------------------------------------------------------------
<document lang="zh_CN.UTF-8">
<section id="EREGION_FUNCTION_COUNT_RECTS">
	<title>区域描述的组成矩形数量</title>
	<programlisting>
int32 BRegion::CountRects() const
	</programlisting>
	<para>返回值为该区域描述的组合矩形数量。
		<footnote><para>一个区域范围描述由多个矩形成员组成，
				BRegion::CountRects()主要用于统计
				组成矩形的数量以便调用BRegion::RectAt()。</para></footnote>
	</para>
</section>
</document>
-----------------------------------------------------------------------------*/
int32
BRegion::CountRects() const
{
	return fRects.CountItems();
}


/* --------------------------------------------------------------------------
<document lang="zh_CN.UTF-8">
<section id="EREGION_FUNCTION_MODIFICATION">
	<title>区域描述的修改与设定</title>
	<programlisting>
void BRegion::Set(BRect <emphasis>singleBound</emphasis>)
void BRegion::MakeEmpty()

bool BRegion::Include(BRect <emphasis>rect</emphasis>)
bool BRegion::Include(const BRegion *<emphasis>region</emphasis>)

bool BRegion::Exclude(BRect <emphasis>rect</emphasis>)
bool BRegion::Exclude(const BRegion *<emphasis>region</emphasis>)

void BRegion::OffsetBy(float <emphasis>dx</emphasis>, float <emphasis>dy</emphasis>)
void BRegion::OffsetBy(BPoint <emphasis>pt</emphasis>)
BRegion& BRegion::OffsetBySelf(float <emphasis>dx</emphasis>, float <emphasis>dy</emphasis>)
BRegion& BRegion::OffsetBySelf(BPoint <emphasis>pt</emphasis>)
BRegion BRegion::OffsetByCopy(float <emphasis>dx</emphasis>, float <emphasis>dy</emphasis>)
BRegion BRegion::OffsetByCopy(BPoint <emphasis>pt</emphasis>)

void BRegion::Scale(float <emphasis>scaling</emphasis>)
BRegion& BRegion::ScaleSelf(float <emphasis>scaling</emphasis>)
BRegion BRegion::ScaleCopy(float <emphasis>scaling</emphasis>)
	</programlisting>
	<itemizedlist>
		<listitem><para><emphasis>singleBound</emphasis>是一个矩形的区域范围描述。
			<footnote><para>如果该矩形为无效矩形时会导致当前区域范围描述无效。</para></footnote>
		</para></listitem>
		<listitem><para><emphasis>region</emphasis>是已有的区域范围描述。
		</para></listitem>
		<listitem><para><emphasis>rect</emphasis>是已有的一个矩形区域范围。
		</para></listitem>
		<listitem><para><emphasis>dx</emphasis>是相对的偏移X方向量。
		</para></listitem>
		<listitem><para><emphasis>dy</emphasis>是相对的偏移Y方向量。
		</para></listitem>
		<listitem><para><emphasis>pt</emphasis>是相对的偏移X和Y方向量的点组合。
			<footnote><para>pt.x和pt.y相对应上述的dx和dy。</para></footnote>
		</para></listitem>
		<listitem><para><emphasis>scaling</emphasis>是缩放的比例。
			<footnote><para>scaling不能小于0。</para></footnote>
		</para></listitem>
	</itemizedlist>
	<para>BRegion::Set()用于将当前区域描述赋值成一个单独的矩形区域。
		<footnote><para>如果该矩形为无效矩形时会导致当前区域范围描述无效。</para></footnote>
	</para>
	<para>BRegion::MakeEmpty()用于清除当前区域描述的所有组成矩形。</para>
	<para>BRegion::Include()用于将参数指定的区域试图加入到当前区域描述中，成功时返回值为true，否则为false。</para>
	<para>BRegion::Exclude()用于将参数指定的区域试图从当前区域描述中排除，成功时返回值为true，否则为false。</para>
	<para>BRegion::OffsetBy()用于将当前区域描述的所有组成矩形都按指定相对位移进行偏移。</para>
	<para>BRegion::OffsetBySelf()用于将当前区域描述的所有组成矩形都按指定相对位移进行偏移，返回值为偏移后的当前区域本身引用。</para>
	<para>BRegion::OffsetByCopy()用于求当前区域描述的所有组成矩形都按指定相对位移进行偏移的结果，返回值为偏移后的区域描述。</para>
	<para>BRegion::Scale()用于将当前区域描述的所有组成矩形都按指定比例进行放缩。</para>
	<para>BRegion::ScaleSelf()用于将当前区域描述的所有组成矩形都按指定比例进行放缩，返回值为放缩后的当前区域本身引用。</para>
	<para>BRegion::ScaleCopy()用于求当前区域描述的所有组成矩形都按指定比例进行放缩的结果，返回值为放缩后的区域描述。</para>
</section>
</document>
-----------------------------------------------------------------------------*/
void
BRegion::Set(BRect rect)
{
	MakeEmpty();

	if (rect.IsValid()) {
		BRect *r = new BRect(rect);

		if (r) {
			if (fRects.AddItem(r))
				fFrame = rect;
			else
				delete r;
		}
	}
}


void
BRegion::MakeEmpty()
{
	for (int32 i = 0; i < fRects.CountItems(); i++) {
		BRect *r = (BRect*)fRects.ItemAt(i);
		if (r) delete r;
	}

	fRects.MakeEmpty();
	fFrame = BRect();
}


// return value must free by "delete[]"
inline BRect* rect_exclude(BRect s, BRect r, int8 *nRects)
{
	if (!r.IsValid() || !s.IsValid() || !nRects) return NULL;

	BRect iR = (s & r);
	if (!iR.IsValid() || iR == s) {
		*nRects = 0;
		return NULL;
	}

	BRect *rects = new BRect[4];
	if (!rects) {
		*nRects = -1;
		return NULL;
	}

	rects[0].Set(min_c(s.left, iR.left), min_c(s.top, iR.top), max_c(s.right, iR.right), max_c(s.top, iR.top)); // top
	rects[1].Set(min_c(s.left, iR.left), min_c(s.bottom, iR.bottom), max_c(s.right, iR.right), max_c(s.bottom, iR.bottom)); // bottom
	rects[2].Set(min_c(s.left, iR.left), max_c(s.top, iR.top), max_c(s.left, iR.left), min_c(s.bottom, iR.bottom)); // left
	rects[3].Set(min_c(s.right, iR.right), max_c(s.top, iR.top), max_c(s.right, iR.right), min_c(s.bottom, iR.bottom)); // right

	int8 count = 4;
	for (int8 i = 0; i < count; i++) {
		if ((rects[i].Width() == 0 && s.Width() > 0) || (rects[i].Height() == 0 && s.Height() > 0) || !rects[i].IsValid()) {
			for (int8 j = i; j < count - 1; j++) rects[j] = rects[j + 1];
			count--;
			i--;
		}
	}

	*nRects = count;

	if (count > 0) return rects;

	delete[] rects;
	return NULL;
}


bool
BRegion::Include(BRect rect)
{
	if (rect.IsValid() == false) return false;

	BRect *rects = &rect;
	int32 nRects = 1;

	for (int32 i = 0; i < fRects.CountItems(); i++) {
		BRect *r = (BRect*)fRects.ItemAt(i);

		BRect **rs = ((!r || r->IsValid() == false) ? NULL : new BRect*[nRects]);
		int8 *nrs = ((!r || r->IsValid() == false) ? NULL : new int8[nRects]);
		int32 nNeeded = 0;
		bool haveNew = false;

		if (rs && nrs) {
			for (int32 j = 0; j < nRects; j++) {
				if (r->Intersects(rects[j])) {
					rs[j] = rect_exclude(rects[j], *r, &(nrs[j]));
					if (nrs[j] < 0 || (rs[j] == NULL && nrs[j] > 0) || nNeeded >= B_MAXINT32 - nrs[j]) {
						nNeeded = -(j + 1);
						break;
					} else {
						haveNew = true;
						nNeeded += nrs[j];
					}
				} else {
					rs[j] = &(rects[j]);
					nrs[j] = 1;
					nNeeded++;
				}
			}
		}

		if (!rs || !nrs || nNeeded < 0) {
			if (rs && nrs) for (int32 j = 0; j <= -(nNeeded + 1); j++) if (rs[j] != NULL && rs[j] != &(rects[j])) delete[] rs[j];
			if (rs) delete[] rs;
			if (nrs) delete[] nrs;

			if (rects != &rect) delete[] rects;
			return false;
		} else if (nNeeded == 0 && haveNew) {
			for (int32 j = 0; j < nRects; j++) if (rs[j] != NULL && rs[j] != &(rects[j])) delete[] rs[j];
			delete[] rs;
			delete[] nrs;

			if (rects != &rect) delete[] rects;
			return true;
		}

		BRect *newRects = ((nNeeded > 0 && haveNew) ? new BRect[nNeeded] : NULL);
		BRect *tmp = newRects;
		for (int32 j = 0; j < nRects; j++) {
			if (tmp && nrs[j] > 0) {
				memcpy(tmp, rs[j], sizeof(BRect) * (size_t)nrs[j]);
				tmp += nrs[j];
			}
			if (rs[j] != NULL && rs[j] != &(rects[j])) delete[] rs[j];
		}

		delete[] rs;
		delete[] nrs;

		if (newRects) {
			if (rects != &rect) delete[] rects;
			rects = newRects;
			nRects = nNeeded;
		}
	}

	int32 oldLength = fRects.CountItems();
	BRect oldFrame = fFrame;

	for (int32 i = 0; i < nRects; i++) {
		BRect *r = new BRect(rects[i]);
		if (!r || r->IsValid() == false || fRects.AddItem(r) == false) {
			if (r) delete r;
			if (rects != &rect) delete[] rects;

			if (fRects.CountItems() > oldLength) {
				for (int32 i = oldLength; i < fRects.CountItems(); i++) {
					r = (BRect*)fRects.ItemAt(i);
					if (r) delete r;
				}
				fRects.RemoveItems(oldLength, -1);
			}
			fFrame = oldFrame;

			return false;
		}
		fFrame = (fFrame.IsValid() ? (fFrame | *r) : *r);
	}

	if (rects != &rect) delete[] rects;
	return true;
}


bool
BRegion::Include(const BRegion *region)
{
	if (region == NULL || region->CountRects() <= 0) return false;

	int32 oldLength = fRects.CountItems();
	BRect oldFrame = fFrame;

	for (int32 i = 0; i < region->CountRects(); i++) {
		BRect r = region->RectAt(i);
		if (r.IsValid() == false || Include(r) == false) {
			if (fRects.CountItems() > oldLength) {
				for (int32 i = oldLength; i < fRects.CountItems(); i++) {
					BRect *ar = (BRect*)fRects.ItemAt(i);
					if (ar) delete ar;
				}
				fRects.RemoveItems(oldLength, -1);
			}
			fFrame = oldFrame;
			return false;
		}
	}

	return true;
}


bool
BRegion::Exclude(BRect r)
{
	if (Intersects(r) == false) return true;

	int32 offset = 0;
	BRect rect;
	bool retVal = true;

	BRegion aRegion(*this);
	if (aRegion.CountRects() != CountRects() || aRegion.Frame() != Frame()) return false;

	while (offset < aRegion.fRects.CountItems()) {
		rect = aRegion.RectAt(offset);
		if (rect.IsValid() == false) {
			retVal = false;
			break;
		}
		if (rect.Intersects(r) == false) {
			offset++;
			continue;
		}

		int8 nrs = 0;
		BRect *rs = rect_exclude(rect, r, &nrs);
		if (nrs < 0 || (rs == NULL && nrs > 0)) {
			if (rs) delete[] rs;
			retVal = false;
			break;
		}

		for (int8 i = 0; i < nrs; i++) {
			BRect *tr = new BRect(rs[i]);
			if (!tr || tr->IsValid() == false || aRegion.fRects.AddItem(tr, offset) == false) {
				if (tr) delete tr;
				retVal = false;
				break;
			}
			offset++;
		}

		if (rs) delete[] rs;
		if (!retVal) break;

		BRect *ar = (BRect*)aRegion.fRects.RemoveItem(offset);
		if (!ar) {
			retVal = false;
			break;
		}
		delete ar;

		if (nrs == 0) break; // here the "rect == r", so we break
	}

	if (retVal) {
		MakeEmpty();
		if (fRects.AddList(&(aRegion.fRects))) aRegion.fRects.MakeEmpty();
		for (int32 i = 0; i < fRects.CountItems(); i++) {
			BRect *r = (BRect*)fRects.ItemAt(i);
			fFrame = (fFrame.IsValid() ? (fFrame | *r) : *r);
		}
	}

	return retVal;
}


bool
BRegion::Exclude(const BRegion *region)
{
	if (Intersects(region) == false) return true;

	BRegion aRegion(*this);
	if (aRegion.CountRects() != CountRects() || aRegion.Frame() != Frame()) return false;

	bool retVal = true;

	for (int32 i = 0; i < region->CountRects(); i++) {
		BRect r = region->RectAt(i);
		if (aRegion.Exclude(r) == false) {
			retVal = false;
			break;
		}
	}

	if (retVal) {
		MakeEmpty();
		if (fRects.AddList(&(aRegion.fRects))) aRegion.fRects.MakeEmpty();
		for (int32 i = 0; i < fRects.CountItems(); i++) {
			BRect *r = (BRect*)fRects.ItemAt(i);
			fFrame = (fFrame.IsValid() ? (fFrame | *r) : *r);
		}
	}

	return retVal;
}


void
BRegion::OffsetBy(float dx, float dy)
{
	if (fRects.CountItems() <= 0) return;

	for (int32 i = 0; i < fRects.CountItems(); i++) {
		BRect *r = (BRect*)fRects.ItemAt(i);
		r->OffsetBy(dx, dy);
	}
	fFrame.OffsetBy(dx, dy);
}


void
BRegion::OffsetBy(BPoint pt)
{
	OffsetBy(pt.x, pt.y);
}


BRegion&
BRegion::OffsetBySelf(float dx, float dy)
{
	OffsetBy(dx, dy);
	return *this;
}


BRegion&
BRegion::OffsetBySelf(BPoint pt)
{
	OffsetBy(pt.x, pt.y);
	return *this;
}


BRegion
BRegion::OffsetByCopy(float dx, float dy)
{
	BRegion aRegion(*this);
	aRegion.OffsetBy(dx, dy);
	return aRegion;
}


BRegion
BRegion::OffsetByCopy(BPoint pt)
{
	BRegion aRegion(*this);
	aRegion.OffsetBy(pt.x, pt.y);
	return aRegion;
}


/* --------------------------------------------------------------------------
<document lang="zh_CN.UTF-8">
<section id="EREGION_FUNCTION_QUERY">
	<title>区域描述的查询</title>
	<programlisting>
bool BRegion::Intersects(BRect <emphasis>rect</emphasis>) const;
bool BRegion::Intersects(float <emphasis>left</emphasis>, float <emphasis>top</emphasis>, float <emphasis>right</emphasis>, float <emphasis>bottom</emphasis>) const;
bool BRegion::Intersects(const BRegion *<emphasis>region</emphasis>) const;

bool BRegion::Contains(BPoint <emphasis>pt</emphasis>) const;
bool BRegion::Contains(float <emphasis>x</emphasis>, float <emphasis>y</emphasis>) const;
bool BRegion::Contains(BRect <emphasis>rect</emphasis>) const;
bool BRegion::Contains(float <emphasis>left</emphasis>, float <emphasis>top</emphasis>, float <emphasis>right</emphasis>, float <emphasis>bottom</emphasis>) const;
bool BRegion::Contains(const BRegion *<emphasis>region</emphasis>) const;

void BRegion::PrintToStream() const;
	</programlisting>
	<itemizedlist>
		<listitem><para><emphasis>region</emphasis>是已有的区域范围描述。
		</para></listitem>
		<listitem><para><emphasis>rect</emphasis>是已有的一个矩形区域范围。
		</para></listitem>
		<listitem><para><emphasis>left top right bottom</emphasis>分别是已有的一个矩形区域范围的左、上、右、下坐标值。
		</para></listitem>
		<listitem><para><emphasis>pt</emphasis>是一个点。
		</para></listitem>
		<listitem><para><emphasis>x y</emphasis>分别是一个点的X、Y方向坐标值。
		</para></listitem>
	</itemizedlist>
	<para>BRegion::Intersects()用于查询参数指定的矩形、区域描述与当前区域描述是否相交，有相交时返回true，否则返回false。</para>
	<para>BRegion::Contains()用于查询参数指定的点、矩形或区域描述是否在当前区域描述之内，被其完全包含时返回true，否则返回false。</para>
	<para>BRegion::PrintToStream()用于将当前区域描述的所有组成矩形内容打印到标准输出流中。
		<footnote><para>BRegion::PrintToStream()主要用于调试，无其它特殊效用。</para></footnote>
	</para>
</section>
</document>
-----------------------------------------------------------------------------*/
bool
BRegion::Intersects(BRect rect) const
{
	return Intersects(rect.left, rect.top, rect.right, rect.bottom);
}


bool
BRegion::Intersects(float l, float t, float r, float b) const
{
	if (fFrame.Intersects(l, t, r, b) == false) return false;

	for (int32 i = 0; i < fRects.CountItems(); i++) {
		BRect *rect = (BRect*)fRects.ItemAt(i);
		if (!rect || rect->IsValid() == false) return false;
		if (rect->Intersects(l, t, r, b)) return true;
	}

	return false;
}


bool
BRegion::Intersects(const BRegion *region) const
{
	if (!region || fFrame.Intersects(region->fFrame) == false) return false;

	for (int32 i = 0; i < fRects.CountItems(); i++) {
		BRect *rect = (BRect*)fRects.ItemAt(i);
		if (!rect || rect->IsValid() == false) return false;

		for (int32 j = 0; j < region->fRects.CountItems(); j++) {
			BRect *ar = (BRect*)region->fRects.ItemAt(j);
			if (!ar || ar->IsValid() == false) return false;
			if (ar->Intersects(*rect)) return true;
		}
	}

	return false;
}


bool
BRegion::Contains(BPoint pt) const
{
	return Contains(pt.x, pt.y);
}


bool
BRegion::Contains(float x, float y) const
{
	if (fFrame.Contains(x, y) == false) return false;

	for (int32 i = 0; i < fRects.CountItems(); i++) {
		BRect *r = (BRect*)fRects.ItemAt(i);
		if (!r || r->IsValid() == false) return false;
		if (r->Contains(x, y)) return true;
	}

	return false;
}


bool
BRegion::Contains(BRect rect) const
{
	BRegion aRegion(rect);
	return Contains(&aRegion);
}


bool
BRegion::Contains(float l, float t, float r, float b) const
{
	return Contains(BRect(l, t, r, b));
}


bool
BRegion::Contains(const BRegion *region) const
{
	if (!region || fFrame.Contains(region->fFrame) == false) return false;

	BRegion aRegion(*region);
	if (aRegion.CountRects() != region->CountRects() || aRegion.Frame() != region->Frame()) return false;

	if (aRegion.Exclude(this) == false || aRegion.fRects.CountItems() > 0) return false;

	return true;
}


void
BRegion::PrintToStream() const
{
	fFrame.PrintToStream();
	ETK_OUTPUT("\n");
	for (int32 i = 0; i < fRects.CountItems(); i++) {
		BRect *r = (BRect*)fRects.ItemAt(i);
		if (r) r->PrintToStream();
		if (i < fRects.CountItems() - 1) ETK_OUTPUT(", ");
	}
}


BRegion
BRegion::operator&(BRect r) const
{
	BRegion aRegion(*this);
	if (aRegion.CountRects() != CountRects() || aRegion.Frame() != Frame()) aRegion.MakeEmpty();

	aRegion &= r;

	return aRegion;
}


BRegion
BRegion::operator|(BRect r) const
{
	BRegion aRegion(*this);
	if (aRegion.CountRects() != CountRects() || aRegion.Frame() != Frame()) aRegion.MakeEmpty();

	aRegion |= r;

	return aRegion;
}


BRegion&
BRegion::operator&=(BRect r)
{
	if (fFrame.Intersects(r) == false) MakeEmpty();
	else {
		int32 offset = 0;
		while (offset < fRects.CountItems()) {
			BRect *rect = (BRect*)fRects.ItemAt(offset);
			if (!rect || rect->IsValid() == false) {
				MakeEmpty();
				break;
			}

			*rect &= r;
			if (rect->IsValid() == false) {
				rect = (BRect*)fRects.RemoveItem(offset);
				if (!rect) {
					MakeEmpty();
					break;
				}
				delete rect;
				continue;
			}

			offset++;
		}
	}

	fFrame = BRect();
	for (int32 i = 0; i < fRects.CountItems(); i++) {
		BRect *rect = (BRect*)fRects.ItemAt(i);
		fFrame = (fFrame.IsValid() ? (fFrame | *rect) : *rect);
	}

	return *this;
}


BRegion&
BRegion::operator|=(BRect r)
{
	Include(r);
	return *this;
}


BRegion
BRegion::operator&(const BRegion &region) const
{
	BRegion aRegion(*this);
	if (aRegion.CountRects() != CountRects() || aRegion.Frame() != Frame()) aRegion.MakeEmpty();

	aRegion &= region;

	return aRegion;
}


BRegion
BRegion::operator|(const BRegion &region) const
{
	BRegion aRegion(*this);
	if (aRegion.CountRects() != CountRects() || aRegion.Frame() != Frame()) aRegion.MakeEmpty();

	aRegion |= region;

	return aRegion;
}


BRegion&
BRegion::operator&=(const BRegion &region)
{
	if (fFrame.Intersects(region.fFrame) == false) MakeEmpty();
	else {
		operator&=(region.fFrame);
		if (region.CountRects() > 1) {
			BRegion aRegion(region.fFrame);
			aRegion.Exclude(&region);
			Exclude(&aRegion);
		}
	}

	return *this;
}


BRegion&
BRegion::operator|=(const BRegion &region)
{
	Include(&region);
	return *this;
}


void
BRegion::Scale(float scaling)
{
	if (fRects.CountItems() <= 0 || scaling < 0 || scaling == 1) return;

	if (scaling > 0) {
		for (int32 i = 0; i < fRects.CountItems(); i++) {
			BRect *r = (BRect*)fRects.ItemAt(i);
			r->left *= scaling;
			r->top *= scaling;
			r->right *= scaling;
			r->bottom *= scaling;
		}

		fFrame.left *= scaling;
		fFrame.top *= scaling;
		fFrame.right *= scaling;
		fFrame.bottom *= scaling;
	} else {
		Set(BRect(0, 0, 0, 0));
	}
}


BRegion&
BRegion::ScaleSelf(float scaling)
{
	Scale(scaling);
	return *this;
}


BRegion
BRegion::ScaleCopy(float scaling)
{
	BRegion aRegion(*this);
	aRegion.Scale(scaling);
	return aRegion;
}

