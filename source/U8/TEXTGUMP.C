// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -vi-
// name: TEXTGUMP.C

#include <dos.h>
#include <mem.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "..\UI\NEWGUMP.H"
#include "BASECAM.H"
#include "CDESC.H"
#include "CRECT.H"
#include "DISPATCH.H"
#include "MENU.H"
#include "SYSTEM.H"
#include "TEXTGUMP.H"

// Text mode screen: 80 columns of character and attribute bytes.
#define TEXT_ROW	160

#define HIDE_MOUSE()	_AX = 2; geninterrupt(0x33)
#define SHOW_MOUSE()	_AX = 1; geninterrupt(0x33)

char *doubleFrame = "\311\273\310\274\315\272";
char *singleFrame = "\332\277\300\331\304\263";

WindowGump::WindowGump(NewGumpId id, NewGump *parent, Rect r, char color, unsigned char show, WindowFrame *frame) :
	NewGump(id, parent, 0)
{
	rect = r;
	cursorX = 0;
	cursorY = 0;
	this->color = color;
	this->frame = frame;
	shown = 0;
	if (show)
	{
		shown = 1;
		theBaseCamera->textVideoMode();
		refresh();
	}
}

void WindowGump::draw(short, short)
{
	if (!shown)
	{
		shown = 1;
		theBaseCamera->textVideoMode();
	}
}

void WindowGump::clear(void)
{
	char line[TEXT_ROW];
	memset(line, color, sizeof(line));
	for (char *p = line + TEXT_ROW - 2; p >= line; p -= 2)
		*p = ' ';
	int x = get_dx();
	int y = get_dy();
	char far *screen = (char far *)MK_FP(TextSelector, y * TEXT_ROW + x * 2);
	int bytes = rect.width() * 2;
	int rows = rect.height();
	for (int i = 0; i < rows; i++, screen += TEXT_ROW)
		memcpy(screen, line, bytes);
}

void WindowGump::gotoxy(short x, short y)
{
	if (x > rect.f_04 || x < 0 || y > rect.f_06 || y < 0)
		return;
	cursorX = x;
	cursorY = y;
	unsigned char column = get_dx() + x;
	unsigned char row = get_dy() + y;
	asm {
		mov ah, 2
		xor bx, bx
		mov dl, column
		mov dh, row
		int 10h
	}
}

void WindowGump::print(char *format, ...)
{
	char *s;
	int x;
	int y;
	char far *screen;
	va_list args;
	HIDE_MOUSE();
	if (cursorX < 0 || cursorX >= rect.width() || cursorY < 0 || cursorY >= rect.height())
		return;
	FORMAT_WORKSTRING(format, args);
	int len = strlen(WorkString);
	if (rect.width() < cursorX + len)
		len = rect.width() - cursorX;
	s = WorkString;
	x = get_dx() + cursorX;
	y = get_dy() + cursorY;
	cursorX += len;
	screen = (char far *)MK_FP(TextSelector, y * TEXT_ROW + x * 2);
	memset(screen, color, len * 2);
	while (len > 0)
	{
		*screen = *s;
		screen += 2;
		len--;
		s++;
	}
	gotoxy(cursorX, cursorY);
	SHOW_MOUSE();
}

void WindowGump::print(char c)
{
	HIDE_MOUSE();
	if (cursorX < 0 || cursorX >= rect.width() || cursorY < 0 || cursorY >= rect.height())
		return;
	if (rect.width() < cursorX + 1)
		return;
	int x = get_dx() + cursorX;
	int y = get_dy() + cursorY;
	cursorX++;
	char far *screen = (char far *)MK_FP(TextSelector, y * TEXT_ROW + x * 2);
	screen[1] = color;
	screen[0] = c;
	gotoxy(cursorX, cursorY);
	SHOW_MOUSE();
}

void WindowGump::deactivate(void)
{
	if (frame)
	{
		frame->setFrameChars(singleFrame);
		frame->refresh();
	}
}

void WindowGump::activate(void)
{
	if (frame)
	{
		frame->setFrameChars(doubleFrame);
		frame->refresh();
		gotoxy(cursorX, cursorY);
	}
}

void WindowGump::moveCursor(int x, int y)
{
	asm {
		mov ah, 2
		mov bh, 0
		mov dl, byte ptr x
		mov dh, byte ptr y
		int 10h
	}
}

WindowFrame::WindowFrame(NewGumpId id, NewGump *parent, Rect r, char *chars, char color) :
	WindowGump(id, parent, r, color, 0, 0)
{
	rect = r;
	frameChars = chars;
}

void WindowFrame::draw(short x, short y)
{
	WindowGump::draw(x, y);
	int right = rect.width() - 1;
	int bottom = rect.height() - 1;
	int i;
	gotoxy(0, 0);
	print(frameChars[0]);
	gotoxy(right, 0);
	print(frameChars[1]);
	gotoxy(0, bottom);
	print(frameChars[2]);
	gotoxy(right, bottom);
	print(frameChars[3]);
	gotoxy(1, 0);
	for (i = 1; i < right; i++)
		print(frameChars[4]);
	gotoxy(1, bottom);
	for (i = 1; i < right; i++)
		print(frameChars[4]);
	for (i = 1; i < bottom; i++)
	{
		gotoxy(0, i);
		print(frameChars[5]);
		gotoxy(right, i);
		print(frameChars[5]);
	}
}

Scroller::Scroller(NewGumpId id, NewGump *parent, Rect r, char color, WindowFrame *frame) :
	WindowGump(id, parent, r, color, 1, frame)
{
	scrollX = 0;
	scrollY = 0;
}

int Scroller::isVis(short x, short y)
{
	return x >= scrollX && x < scrollX + rect.width() && y >= scrollY && y < scrollY + rect.height();
}

TextMenuItemGump &operator+(TextMenuItemGump &item, TextPullDownGump &pullDown)
{
	item.addPullDown((PullDownGump *)&pullDown);
	return item;
}

TextPullDownGump &operator+(TextMenuItemGump &first, TextMenuItemGump &second)
{
	TextPullDownGump *pullDown = new TextPullDownGump;
	pullDown->addMenuItem(&first);
	pullDown->addMenuItem(&second);
	return *pullDown;
}

TextPullDownGump &operator+(TextPullDownGump &pullDown, TextMenuItemGump &item)
{
	pullDown.addMenuItem(&item);
	return pullDown;
}

TextMenuBarGump &operator+(TextMenuBarGump &bar, TextMenuItemGump &item)
{
	bar.addMenuItem(&item);
	return bar;
}

TextPullDownGump &operator+(TextPullDownGump &pullDown, TextMenuLineGump &line)
{
	pullDown.addMenuItem(&line);
	return pullDown;
}

TextPullDownGump &operator+(TextMenuItemGump &item, TextMenuLineGump &line)
{
	TextPullDownGump *pullDown = new TextPullDownGump;
	pullDown->addMenuItem(&item);
	pullDown->addMenuItem(&line);
	return *pullDown;
}

TextMenuLineGump::TextMenuLineGump(void) :
	PureMenuItemGump(0, 0, 0)
{
	f_38 = f_3a = 0;
	int height;
	int width;
	getDim(width, height);
	rect = Rect(0, 0, width - 1, 0);
}

void TextMenuLineGump::draw(short x, short y)
{
	Rect r = rect;
	r.moveto(x, y);
	WindowGump window(0, 0, r, 0x70, 0, 0);
	window.gotoxy(0, 0);
	char line[80];
	memset(line, '\304', sizeof(line));
	line[r.width()] = 0;
	window.print(line);
}

void TextMenuLineGump::getDim(int &width, int &height)
{
	width = 0;
	height = 1;
}

TextMenuItemGump::TextMenuItemGump(char *text, int id, unsigned char enabled) :
	PureMenuItemGump(text, id, enabled)
{
	f_38 = 2;
	f_3a = 2;
	int height;
	int width;
	getDim(width, height);
	rect = Rect(0, 0, width - 1, 0);
}

void TextMenuItemGump::getDim(int &width, int &height)
{
	height = 1;
	width = strlen(text) + f_3a + f_38;
}

void TextMenuItemGump::draw(short x, short y)
{
	char color = 0x70;
	if (!f_3c)
		color = 0x78;
	else if (f_3d)
		color = 0x20;
	WindowGump window(0, 0, rect, color, 0, 0);
	window.moveto(x, y);
	char format[20];
	memset(format, ' ', f_3a);
	sprintf(format + f_3a, "%%-%ds", rect.width() - f_3a);
	window.print(format, text);
	if (newGumpList.getHead() && !parent->newGumpId.get())
	{
		window.gotoxy(window.rect.f_04 - 2, 0);
		window.print('\020');
	}
}

void TextCheckMenuItemGump::commandMouseLeft(Event &event)
{
	if (!f_3c)
		return;
	if (event.isSingle())
	{
		PureMenuItemGump::commandMouseLeft(event);
		return;
	}
	if (event.isRelease())
	{
		if (contains(event) && f_3d)
			*checked = !*checked;
		PureMenuItemGump::commandMouseLeft(event);
	}
}

void TextCheckMenuItemGump::draw(short x, short y)
{
	char color = 0x70;
	if (!f_3c)
		color = 0x78;
	else if (f_3d)
		color = 0x20;
	WindowGump window(0, 0, rect, color, 0, 0);
	window.moveto(x, y);
	char format[20];
	memset(format, ' ', f_3a);
	if (*checked)
		format[0] = '\373';
	sprintf(format + f_3a, "%%-%ds", rect.width() - f_3a);
	window.print(format, text);
}

TextPullDownGump::TextPullDownGump(void) :
	TextMenuGump(0)
{
	rect = Rect(0, 0, 1, 1);
}

void TextPullDownGump::addMenuItem(PureMenuItemGump *item)
{
	if (!item)
		return;
	int height;
	int width;
	item->getDim(width, height);
	item->parent = this;
	item->rect = Rect(1, 0, width, 0);
	item->moveto(1, rect.height() - 1);
	rect.f_06++;
	if (rect.width() < item->rect.width())
	{
		int grow = item->rect.width() - rect.width();
		rect.f_04 += grow + 2;
		NewGump *kid = 0;
		while (newGumpList.traverse((DoubleLink *&)kid))
		{
			kid->rect.f_04 = item->rect.f_04;
			NewGump *pullDown = kid->newGumpList.getHead();
			if (pullDown)
				pullDown->moveto(kid->rect.width() - 1, 0);
		}
	}
	else
		item->rect.f_04 = rect.width() - 2;
	attach(item);
	NewGump *pullDown = item->newGumpList.getHead();
	if (pullDown)
		pullDown->moveto(item->rect.width() - 1, 0);
}

void TextPullDownGump::commandPrivate(Event &event)
{
	switch (event.data)
	{
	case 1:
		event.to = parent;
		send(event);
		send(Event(EVENT_PRIVATE, 0, 0, 2, this, parent, 0));
	}
}

void TextPullDownGump::draw(short x, short y)
{
	Rect r = rect;
	r.moveto(x, y);
	WindowFrame frame(0, 0, r, singleFrame, 0x70);
	frame.refresh();
}

TextMenuBarGump::TextMenuBarGump(unsigned char id, NewGump *parent) :
	TextMenuGump(parent)
{
	newGumpId = NewGumpId(3, id);
	rect = Rect(0, 0, 0, 0);
}

void TextMenuBarGump::draw(short x, short y)
{
	char line[80];
	memset(line, ' ', rect.width());
	WindowGump window(0, 0, rect, 0x78, 0, 0);
	window.moveto(x, y);
	window.print(line);
}

void TextMenuBarGump::addMenuItem(PureMenuItemGump *item)
{
	if (!item)
		return;
	int width;
	int height;
	item->getDim(width, height);
	item->parent = this;
	item->rect = Rect(1, 0, width, 0);
	item->moveto(rect.width(), 0);
	rect.f_04 += item->rect.width();
	attach(item);
	NewGump *pullDown = item->newGumpList.getHead();
	if (pullDown)
		pullDown->moveto(0, item->rect.height());
}

void TextMenuBarGump::commandPrivate(Event &event)
{
	switch (event.data)
	{
	case 1:
		event.to = parent;
		event.from = this;
		event.data = event.x;
		send(event);
		clearSelect();
		setMovement(0);
	}
}
