// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -vi-
// name: TEXTEDIT.C

#include <string.h>
#include <mem.h>
#include <ctype.h>
#include "..\UI\NEWGUMP.H"
#include "CRECT.H"
#include "CVPORT.H"
#include "CGLOBVP.H"
#include "FONT.H"
#include "DISPATCH.H"
#include "MENU.H"
#include "TEXTEDIT.H"

// Inline in the shared headers when this file was built.
inline NewGumpId::NewGumpId(unsigned char type, unsigned char id) { instance = (type << 8) | id; f_02 = ~instance; f_04 = 0; }
inline Rect::Rect(short x1, short y1, short x2, short y2) { set(x1, y1, x2, y2); }
inline int Rect::width(void) { return f_04 - f_00 + 1; }
inline int Rect::height(void) { return f_06 - f_02 + 1; }
inline char Event::isShift(void) { return (data >> 16) & 3; }

TextEditGump *TextEditGump::focus = 0;

TextEditGump::TextEditGump(unsigned char id, NewGump *parent, char *buffer, unsigned char size, unsigned char width, char *initial, unsigned char next) :
	NewGump(NewGumpId(6, id), parent, 0x22)
{
	text = buffer;
	maxLen = size;
	lastChar = ' ';
	tabNext = next;
	if (initial)
		strcpy(text, initial);
	int height = GlobalFont::global_ptr->height(' ');
	rect = Rect(0, 0, width, height + 2);
	reset(1, 0);
	findScrollPos();
	registerHotKeys(0xffff, 0);
	getFocus();
}

TextEditGump::~TextEditGump(void)
{
	if (focus == this)
		focus = 0;
}

void TextEditGump::reset(unsigned char toEnd, char *newText)
{
	if (newText)
		text = newText;
	len = strlen(text);
	if (toEnd)
	{
		cursor = len;
		selStart = 0;
		selEnd = cursor;
	}
	else
	{
		if (cursor > len)
			cursor = len;
		selStart = 0;
		selEnd = 0;
	}
	scroll = 0;
}

void TextEditGump::draw(short x, short y)
{
	DrawRectangle(GlobalVport::global_ptr, x, y, x + rect.width() - 1, y + rect.height() - 1, GumpColorMap[0]);
	DrawBox(GlobalVport::global_ptr, x + 1, y + 1, x + rect.width() - 2, y + rect.height() - 2, GumpColorMap[5]);
	findScrollPos();
	if (focus == this && selStart < selEnd)
	{
		int start = findLoc(selStart);
		int end = findLoc(selEnd);
		DrawBox(GlobalVport::global_ptr, x + start + 1, y + 1, x + end, y + rect.height() - 2, GumpColorMap[8]);
	}
	GlobalFont::global_ptr->printf(x + 3, y + rect.height() - 2, rect.width() - 3, text + scroll);
	if (focus == this)
	{
		int at = findLoc(cursor);
		DrawLine(GlobalVport::global_ptr, x + at, y + 2, x + at, y + rect.height() - 2, GumpColorMap[0]);
	}
}

// Pixel offset of character `pos` from the left edge.
int TextEditGump::findLoc(int pos)
{
	int width;
	if (scroll >= pos || len < pos)
		width = 0;
	else
	{
		char saved = text[pos];
		text[pos] = 0;
		int height;
		GlobalFont::global_ptr->Font::dim(text + scroll, width, height);
		text[pos] = saved;
	}
	return width + 2;
}

// Character nearest to pixel offset `x`.
int TextEditGump::findChar(short x)
{
	int height, width;
	int i = scroll;
	int prev = 0;
	int at = 2;
	while (at < x && len > i)
	{
		GlobalFont::global_ptr->dim(text[i], width, height);
		prev = at;
		at += width;
		i++;
	}
	if (len == i)
		return len;
	if (x - prev <= at - x)
		i--;
	if (i < 0)
		return 0;
	return i;
}

void TextEditGump::findScrollPos(void)
{
	if (scroll > cursor)
	{
		scroll = cursor;
		return;
	}
	while (findLoc(cursor) > rect.width() - 3)
		scroll++;
}

void TextEditGump::commandMouseLeft(Event &event)
{
	if (event.isSingle())
	{
		TextEditGump *old = focus;
		getFocus();
		if (old && !old->parent->isInvisible())
			old->refresh();
		parent->newGumpList.moveToTail(this);
		f_1d |= 9;
		setExclusive(0);
		commandMouseMovement(event);
		return;
	}
	if (event.isRelease())
	{
		f_1d &= ~9;
		cursor = findChar(getRelX(event));
		clrExclusive();
		refresh();
	}
}

void TextEditGump::commandMouseMovement(Event &event)
{
	int pos = cursor;
	if (contains(event))
		pos = findChar(getRelX(event));
	else
	{
		if (event.x > rect.f_04)
		{
			if (len > pos)
			{
				pos++;
				if (findLoc(pos) == 0)
					scroll++;
			}
		}
		else if (event.x < rect.f_00)
		{
			if (pos > 0)
			{
				pos--;
				if (scroll > pos)
					scroll = pos;
			}
		}
		else
			return;
	}
	if (event.isShift())
	{
		if (selEnd > pos)
			selStart = pos;
		else
			selEnd = pos;
	}
	else
		selEnd = selStart = pos;
	cursor = pos;
	refresh();
}

void TextEditGump::commandNoEvents(Event &event)
{
	commandMouseMovement(event);
}

void TextEditGump::commandKeyboard(Event &event)
{
	if (focus != this)
		return;
	Event reply(EVENT_PRIVATE, (char)event.data, 0, 3, this, parent, 0);
	switch (event.data)
	{
	case 9:
	case 13:
		loseFocus();
		reply.data = 1;
		break;
	case 0x153:		// Del
		if (selStart < selEnd)
		{
			len -= selEnd - selStart;
			memmove(text + selStart, text + selEnd, strlen(text + selEnd) + 1);
			cursor = selStart;
		}
		else if (cursor < len)
		{
			memmove(text + cursor, &text[cursor + 1], strlen(text + cursor) + 1);
			len--;
		}
		break;
	case 8:			// Backspace
		if (cursor > 0)
		{
			memmove(&text[cursor - 1], text + cursor, strlen(text + cursor) + 1);
			cursor--;
			len--;
		}
		break;
	case 0x14d:		// Right
		if (cursor < len)
		{
			cursor++;
			if (findLoc(cursor) == 0)
				scroll++;
		}
		break;
	case 0x14b:		// Left
		if (cursor > 0)
		{
			cursor--;
			if (cursor < scroll)
				scroll = cursor;
		}
		break;
	case 0x14f:		// End
		cursor = len;
		break;
	case 0x147:		// Home
		cursor = 0;
		break;
	default:
		if (event.data <= 0x100)
		{
			lastChar = selStart == 0 ? ' ' : text[selStart - 1];
			if (cursor == 0 || lastChar == ' ')
				event.data = toupper(event.data);
			if (selStart < selEnd)
			{
				len -= selEnd - selStart;
				memmove(text + selStart, text + selEnd, strlen(text + selEnd) + 1);
				cursor = selStart;
			}
			if (strlen(text) < maxLen)
			{
				memmove(&text[cursor + 1], text + cursor, strlen(text + cursor) + 1);
				text[cursor] = event.data;
				lastChar = event.data;
				cursor++;
				len++;
				reply.data = 2;
			}
		}
	}
	send(reply);
	selEnd = selStart = cursor;
	refresh();
}

void TextEditGump::getFocus(void)
{
	focus = this;
	parent->newGumpList.moveToTail(this);
}

// Passes the focus to the next text field, or drops it.
void TextEditGump::loseFocus(void)
{
	TextEditGump *next = this;
	if (tabNext)
	{
		do
			parent->newGumpList.hasType(6, (NewGump *&)next);
		while (!next);
		if (!parent->isInvisible())
			parent->refresh();
		next->getFocus();
		next->reset(1, 0);
		next->refresh();
		return;
	}
	focus = 0;
	if (!parent->isInvisible())
		parent->refresh();
}
