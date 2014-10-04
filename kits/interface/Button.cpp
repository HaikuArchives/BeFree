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
 * File: Button.cpp
 * Description: BButton --- A view like button for control in window
 *
 * --------------------------------------------------------------------------*/

/* --------------------------------------------------------------------------
<document lang="zh_CN.UTF-8">
<documentinfo>
	<title>按钮视图</title>
</documentinfo>
<xref linkend="EBUTTON_DESCRIPTION" /><para></para>
<xref linkend="EBUTTON_FUNCTIONS" />
</document>
-----------------------------------------------------------------------------*/

/* --------------------------------------------------------------------------
<document lang="zh_CN.UTF-8">
<section id="EBUTTON_DESCRIPTION">
	<title>BButton类描述</title>
	<para>声明所在：<emphasis>&lt;interface/Button.h&gt;</emphasis></para>
	<para>链 接 库：<emphasis>libetkxx</emphasis></para>
	<para>派生关系：<emphasis>BButton --- BControl --- BView --- BHandler --- BArchivable</emphasis></para>
	<para>          <emphasis>                     |-- BInvoker</emphasis></para>
	<para>BButton是一个按钮视图类，主要用于在窗口中显示按钮以便程序的交互操作。</para>
</section>

<section id="EBUTTON_FUNCTIONS">
	<title>BButton类成员函数</title>
	<xref linkend="EBUTTON_FUNCTION_CONSTRUCT" /><para></para>
	<xref linkend="EBUTTON_FUNCTION_LABEL" /><para></para>
	<xref linkend="EBUTTON_FUNCTION_CONTENT_FRAME" /><para></para>
	<xref linkend="EBUTTON_FUNCTION_DRAW_CONTENT" />
</section>
</document>
-----------------------------------------------------------------------------*/

#include <add-ons/theme/ThemeEngine.h>

#include "Window.h"
#include "Button.h"


/* --------------------------------------------------------------------------
<document lang="zh_CN.UTF-8">
<section id="EBUTTON_FUNCTION_CONSTRUCT">
	<title>构造函数</title>
	<programlisting>
BButton::BButton(BRect <emphasis>frame</emphasis>,
                 const char *<emphasis>name</emphasis>,
                 const char *<emphasis>label</emphasis>,
                 BMessage *<emphasis>message</emphasis>,
                 uint32 <emphasis>resizeMode = B_FOLLOW_LEFT |B_FOLLOW_TOP</emphasis>,
                 uint32 <emphasis>flags = B_WILL_DRAW |B_NAVIGABLE</emphasis>)
virtual BButton::~BButton()
	</programlisting>
	<itemizedlist>
		<listitem><para><emphasis>frame</emphasis>是视图在父视图的轮廓定义矩形。
			<footnote><para>视图轮廓矩形必须为有效矩形，否则视图将成为不可见。
					除非你通过MoveTo或ResizeTo成员函数变更视图轮廓，
					那么视图才得以有可能显示。</para></footnote>
		</para></listitem>
		<listitem><para><emphasis>name</emphasis>是视图的名字。
			<footnote><para>视图名字用于BView::FindView函数，可以指定为NULL。</para></footnote>
		</para></listitem>
		<listitem><para><emphasis>label</emphasis>是按钮的标签文本。
		</para></listitem>
		<listitem><para><emphasis>message</emphasis>是按钮按下后要产生的消息。
			<footnote><para>此消息为视图解构时自动销毁，程序中不应在视图创建后对其进行销毁。</para></footnote>
		</para></listitem>
		<listitem><para><emphasis>resizeMode</emphasis>是当父视图变换大小时该视图的缩放模式。
			<footnote><para>视图缩放模式详BView类。</para></footnote>
		</para></listitem>
		<listitem><para><emphasis>flags</emphasis>是视图的事件选择标记。
			<footnote><para>视图事件选择标记详BView类。</para></footnote>
		</para></listitem>
	</itemizedlist>
</section>
</document>
-----------------------------------------------------------------------------*/
BButton::BButton(BRect frame, const char *name, const char *label,
                 BMessage *message, uint32 resizeMode, uint32 flags)
		: BControl(frame, name, label, message, resizeMode, flags), fInsided(false),
		fMousePushed(false), fFocusFlash(0), fRunner(NULL)
{
}


BButton::~BButton()
{
}


/* --------------------------------------------------------------------------
<document lang="zh_CN.UTF-8">
<section id="EBUTTON_FUNCTION_LABEL">
	<title>按钮标签</title>
	<programlisting>
virtual void BButton::SetLabel(const char *<emphasis>label</emphasis>)
const char* BControl::Label() const
	</programlisting>
	<itemizedlist>
		<listitem><para><emphasis>label</emphasis>是按钮的标签文本。
			<footnote><para>SetLabel()函数被调用时，按钮视图自动发送区域
					作废消息给窗口(如果有的话)，以便在下次更新时
					更新其内容。</para></footnote>
		</para></listitem>
	</itemizedlist>
	<para>BButton::SetLabel()用于设置按钮的标签内容。</para>
	<para>BControl::Label()函数返回按钮标签内容的字符串。</para>
</section>
</document>
-----------------------------------------------------------------------------*/
void
BButton::SetLabel(const char *label)
{
	BControl::SetLabel(label);
	Invalidate();
}


void
BButton::GetPreferredSize(float *width, float *height)
{
	e_theme_engine *theme = get_current_theme_engine();
	if (theme == NULL || theme->get_button_preferred_size == NULL) return;

	theme->get_button_preferred_size(theme, this, width, height, Label());
}


/* --------------------------------------------------------------------------
<document lang="zh_CN.UTF-8">
<section id="EBUTTON_FUNCTION_CONTENT_FRAME">
	<title>可用包容区域</title>
	<programlisting>
BRect BButton::ContentFrame() const
	</programlisting>
	<para>BButton::ContentFrame()返回按钮扣除边界后的可用包容区域。
		<footnote><para>可以用于派生其它类时定位要绘画的图像、文字等起点。</para></footnote>
	</para>
</section>
</document>
-----------------------------------------------------------------------------*/
BRect
BButton::ContentFrame() const
{
	BRect rect = Frame().OffsetToSelf(B_ORIGIN);

	e_theme_engine *theme = get_current_theme_engine();
	if (!(theme == NULL || theme->get_button_border_margins == NULL)) {
		float l, t, r, b;
		theme->get_button_border_margins(theme, this, &l, &t, &r, &b);
		rect.left += l;
		rect.top += t;
		rect.right -= r;
		rect.bottom -= b;
	}

	return rect;
}


void
BButton::Draw(BRect updateRect)
{
	e_theme_engine *theme = get_current_theme_engine();
	if (theme == NULL) return;

	bool fPushed = (IsEnabled() ? Value() == B_CONTROL_ON : false);
	BRect rect = Frame().OffsetToSelf(B_ORIGIN);

	PushState();

	ConstrainClippingRegion(updateRect);

	if (theme->draw_button_border != NULL)
		theme->draw_button_border(theme, this, rect, fPushed, fInsided, fFocusFlash);

	if (IsFocusChanging()) {
		PopState();
		return;
	}

	if (theme->clear_button_content != NULL)
		theme->clear_button_content(theme, this, rect, fPushed, fInsided, fFocusFlash);

	BRect contentFrame = ContentFrame();
	ConstrainClippingRegion(contentFrame);
	MovePenTo(contentFrame.LeftTop());
	DrawContent();

	PopState();
}


/* --------------------------------------------------------------------------
<document lang="zh_CN.UTF-8">
<section id="EBUTTON_FUNCTION_DRAW_CONTENT">
	<title>缺省绘制标签函数</title>
	<programlisting>
void BButton::DrawContent()
	</programlisting>
	<para>BButton::DrawContent()用于绘制标签，派生类的继承函数可类似下面做法：</para>
	<programlisting>
void MyButton::DrawContent()
{
        BPoint penLocation = ContentFrame().LeftBottom();
        penLocation.y -= 8;

        MovePenTo(penLocation);
        DrawString("PREFIX:");

        MovePenBy(10, 0);
        BButton::DrawContent();
}
	</programlisting>
	<para>上述示例中最后一个MovePenBy调用后视图的笔位置将
用于决定缺省绘制标签时文字的<emphasis>基线</emphasis>起点。</para>
</section>
</document>
-----------------------------------------------------------------------------*/
void
BButton::DrawContent()
{
	if (Label() == NULL) return;
	e_theme_engine *theme = get_current_theme_engine();
	if (theme == NULL || theme->draw_button_label == NULL) return;
	theme->draw_button_label(theme, this, Frame().OffsetToSelf(B_ORIGIN), Label(),
	                         (IsEnabled() ? Value() == B_CONTROL_ON : false), fInsided, 0);
}


void
BButton::DetachedFromWindow()
{
	fInsided = false;
	fMousePushed = false;
	BControl::DetachedFromWindow();
}


void
BButton::MouseDown(BPoint where)
{
	if (!IsEnabled() || !QueryCurrentMouse(true, B_PRIMARY_MOUSE_BUTTON)) return;

	BRect rect = VisibleBounds();
	if (!rect.Contains(where)) return;
	if (Value() == B_CONTROL_ON) return;

	if (!fMousePushed) fMousePushed = true;

	if (SetPrivateEventMask(B_POINTER_EVENTS, B_LOCK_WINDOW_FOCUS) != B_OK) {
		SetValueNoUpdate(B_CONTROL_ON);
		Invalidate();
		Window()->UpdateIfNeeded();
		e_snooze(50000);
		fMousePushed = false;
		SetValueNoUpdate(B_CONTROL_OFF);
		Invalidate();
		Invoke();
	} else {
		SetValueNoUpdate(B_CONTROL_ON);
		Invalidate();
	}
}


void
BButton::MouseUp(BPoint where)
{
	if (!fMousePushed) return;

	fMousePushed = false;

	if (Value() == B_CONTROL_ON) {
		SetValueNoUpdate(B_CONTROL_OFF);
		Invalidate();

		Invoke();
	}
}


void
BButton::MouseMoved(BPoint where, uint32 code, const BMessage *a_message)
{
	if (!IsEnabled()) return;

	if (code == B_ENTERED_VIEW) {
		bool update = false;

		if (!fInsided) {
			fInsided = true;
			update = true;
		}

		if (Value() != B_CONTROL_ON && fMousePushed) {
			SetValueNoUpdate(B_CONTROL_ON);
			update = true;
		}

		if (update) Invalidate();

	} else if (code == B_EXITED_VIEW) {
		bool update = false;

		if (fInsided) {
			fInsided = false;
			update = true;
		}

		if (Value() == B_CONTROL_ON && fMousePushed) {
			SetValueNoUpdate(B_CONTROL_OFF);
			update = true;
		}

		if (update) Invalidate();
	}
}


void
BButton::KeyDown(const char *bytes, int32 numBytes)
{
	if (!IsEnabled() || !IsFocus() || numBytes != 1) return;

	if (Value() == B_CONTROL_ON) return;

	if (!(bytes[0] == B_ENTER || bytes[0] == B_SPACE)) return;

	fMousePushed = false;

	if (SetPrivateEventMask(B_KEYBOARD_EVENTS, B_LOCK_WINDOW_FOCUS) != B_OK) {
		SetValueNoUpdate(B_CONTROL_ON);
		Invalidate();
		Window()->UpdateIfNeeded();
		e_snooze(50000);
		SetValueNoUpdate(B_CONTROL_OFF);
		Invalidate();
		Invoke();
	} else {
		SetValueNoUpdate(B_CONTROL_ON);
		Invalidate();
	}
}


void
BButton::KeyUp(const char *bytes, int32 numBytes)
{
	if (Value() != B_CONTROL_ON || fMousePushed) return;

	SetValueNoUpdate(B_CONTROL_OFF);

	Invalidate();

	Invoke();
}


void
BButton::SetFont(const BFont *font, uint8 mask)
{
	BControl::SetFont(font, mask);
	Invalidate();
}


void
BButton::WindowActivated(bool state)
{
	if (fMousePushed || Value() == B_CONTROL_ON) {
		fMousePushed = false;
		if (Value() == B_CONTROL_ON) {
			SetValueNoUpdate(B_CONTROL_OFF);
			Invalidate();
		}
	}

#if 0
	if (IsFocus() == false) return;

	e_theme_engine *theme = get_current_theme_engine();
	if (theme == NULL ||
	        theme->should_button_do_focus_flash == NULL ||
	        theme->should_button_do_focus_flash(theme, this) == 0) {
		BControl::SetFlags(Flags() & ~B_PULSE_NEEDED);
	}

	if (state)
		BControl::SetFlags(Flags() |B_PULSE_NEEDED);
	else
		BControl::SetFlags(Flags() & ~B_PULSE_NEEDED);
#endif
}


void
BButton::MakeFocus(bool focusState)
{
	if (focusState != IsFocus()) {
		BControl::MakeFocus(focusState);
		fFocusFlash = 0;

#if 0
		if (IsFocus() && Window()->IsActivate()) {
			e_theme_engine *theme = get_current_theme_engine();
			if (theme == NULL || theme->should_button_do_focus_flash == NULL) return;
			if (theme->should_button_do_focus_flash(theme, this) == 0) return;

			BControl::SetFlags(Flags() |B_PULSE_NEEDED);
		} else {
			BControl::SetFlags(Flags() & ~B_PULSE_NEEDED);
		}
#endif
	}
}


#if 0
void
BButton::SetFlags(uint32 flags)
{
	flags &= ~B_PULSE_NEEDED;
	BControl::SetFlags(flags);
}


void
BButton::Pulse()
{
	if (Window() == NULL || IsFocus() == false) return;

	e_theme_engine *theme = get_current_theme_engine();
	if (theme == NULL || theme->should_button_do_focus_flash == NULL) return;

	int8 shouldFlash = theme->should_button_do_focus_flash(theme, this);
	if (shouldFlash == 0) return;

	Window()->DisableUpdates();

	bool fPushed = (IsEnabled() ? Value() == B_CONTROL_ON : false);
	BRect rect = Frame().OffsetToSelf(B_ORIGIN);

	fFocusFlash++;

	PushState();

	ConstrainClippingRegion(rect);

	if (theme->draw_button_border != NULL && (shouldFlash &B_THEME_FOCUS_FLASH_BORDER))
		theme->draw_button_border(theme, this, rect, fPushed, fInsided, fFocusFlash);

	if (theme->clear_button_content != NULL && (shouldFlash &B_THEME_FOCUS_FLASH_CONTENT)) {
		theme->clear_button_content(theme, this, rect, fPushed, fInsided, fFocusFlash);

		BRect contentFrame = ContentFrame();
		ConstrainClippingRegion(contentFrame);
		MovePenTo(contentFrame.LeftTop());
		DrawContent();
	}

	PopState();

	Invalidate(Bounds(), false);

	Window()->EnableUpdates();
}
#endif

