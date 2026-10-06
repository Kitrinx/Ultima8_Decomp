// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: KTEXTED.C

#include <string.h>
#include <mem.h>
#include <ctype.h>
#include "..\UI\NEWGUMP.H"
#include "CRECT.H"
#include "CVPORT.H"
#include "CGLOBVP.H"
#include "CAFONT.H"
#include "DISPATCH.H"
#include "MENU.H"
#include "KTEXTED.H"

// Inline in the shared headers when this file was built.
inline Point::Point(void) {}
inline Rect::Rect(void) {}
inline NewGumpId::NewGumpId(unsigned char type, unsigned char id) { instance = (type << 8) | id; f_02 = ~instance; f_04 = 0; }

// Set while a save description is being edited and nothing has been typed.
unsigned char IDidNotTypeInAnything;

// A four-line, 25-column editor over a caller's 104-byte buffer.
KillerTextEditGump::KillerTextEditGump(unsigned char id, NewGump *owner, char *buffer, Rect r) :
	NewGump(NewGumpId(6, id), owner, 0x22)
{
	IDidNotTypeInAnything = 1;
	setExclusive(0);
	text = buffer;
	backup = new char[104];
	memcpy(backup, text, 104);
	rect = r;
	font = new CacheFont((FontType)4);
	line = col = 0;
	for (int i = 0; i < 4; i++)
		if (backup[i * 26])
		{
			line = i;
			col = strlen(backup + i * 26);
		}
	if (line || col)
		IDidNotTypeInAnything = 0;
	registerHotKeys(0xffff, 0);
	parent->restore();
}

KillerTextEditGump::~KillerTextEditGump(void)
{
	if (backup)
		delete backup;
	if (font)
		delete font;
}

void KillerTextEditGump::clearText(void)
{
	for (int i = 0; i < 104; i++)
		text[i] = 0;
}

void KillerTextEditGump::nullLine(short l)
{
	nullToEnd(l, 0);
}

int KillerTextEditGump::getLen(short l)
{
	int len = 0;
	while (text[l * 26 + len])
		len++;
	return len;
}

// Pads line `l` with spaces out to column `c`.
void KillerTextEditGump::insertTo(short l, short c)
{
	int len = getLen(l);
	while (len < c)
	{
		text[l * 26 + len] = ' ';
		len++;
	}
	text[l * 26 + len] = 0;
}

int KillerTextEditGump::findLastSpace(short l)
{
	int len = getLen(l);
	if (!len)
		return 0;
	for (; len; len--)
		if (text[l * 26 + len] == ' ')
		{
			for (len--; len; len--)
				if (text[l * 26 + len] != ' ')
					return ++len;
			return 0;
		}
	return 0;
}

int KillerTextEditGump::findFirstSpace(short l)
{
	int i = 0;
	int len = getLen(line);
	for (; i < len; i++)
		if (text[l * 26 + i] == ' ')
			for (i++; i; i++)
				if (text[l * 26 + i] != ' ')
					return --i;
	return 0;
}

// Adds a character at the cursor, wrapping the last word onto the next line.
void KillerTextEditGump::output(char c)
{
	if (col >= 25)
	{
		int space = findLastSpace(line);
		int j = 0;
		if (space)
		{
			if (line == 3)
				return;
			int wrap = space;
			space++;
			if (c == ' ')
			{
				nullToEnd(line, col);
				line++;
				col = 0;
				return;
			}
			for (; space != 25; space++, j++)
				text[line * 26 + j + 26] = text[line * 26 + space];
			nullToEnd(line, wrap);
		}
		else if (line == 3)
			return;
		text[line * 26 + col] = 0;
		line++;
		col = j;
	}
	text[line * 26 + col++] = c;
	IDidNotTypeInAnything = 0;
}

void KillerTextEditGump::deleteChar(short l, short c)
{
	if (c < 25)
	{
		for (int i = col; i < 25; i++)
			text[l * 26 + i] = text[l * 26 + i + 1];
		text[l * 26 + i] = 0;
		printLine(l);
	}
}

void KillerTextEditGump::printLine(short)
{
}

void KillerTextEditGump::printText(void)
{
	for (int i = 0; i < 4; i++)
		printLine(i);
}

void KillerTextEditGump::nullToEnd(short l, short c)
{
	for (int i = c; i < 26; i++)
		text[l * 26 + i] = 0;
}

void KillerTextEditGump::commandMouseLeft(Event &event)
{
	if (event.isDouble())
	{
		clrExclusive();
		send(Event(EVENT_PRIVATE, 0, 0, 32, this, parent, 0));
		killMyself();
	}
}

void KillerTextEditGump::commandKeyboard(Event &event)
{
	int key = event.data;
	switch (key)
	{
	case 27:
		restore();
		clrExclusive();
		send(Event(EVENT_PRIVATE, 0, 0, 31, this, parent, 0));
		memcpy(text, backup, 104);
		killMyself();
		return;
	case 13:
		restore();
		clrExclusive();
		send(Event(EVENT_PRIVATE, 0, 0, 32, this, parent, 0));
		killMyself();
		return;
	case 8:
		if (--col < 0)
		{
			if (--line < 0)
			{
				line++;
				col = 0;
			}
			else
			{
				col = getLen(line);
				deleteChar(line, col);
			}
		}
		else
			deleteChar(line, col);
		break;
	case 0x14d:		// Right
		if (++col >= 26)
		{
			col = 0;
			if (++line == 4)
				line--;
		}
		break;
	case 0x14b:		// Left
		if (--col < 0)
		{
			if (--line < 0)
			{
				line++;
				col = 0;
			}
			else
				col = 25;
		}
		break;
	case 0x148:		// Up
		if (--line < 0)
			line++;
		break;
	case 0x150:		// Down
		if (++line >= 4)
			line--;
		break;
	case 0x147:		// Home
		col = 0;
		break;
	case 0x14f:		// End
		col = getLen(line);
		break;
	case 9:
		col = col / 4 * 4;
		col += 4;
		if (col >= 26)
			col = 26;
		break;
	default:
		if (isalnum(key) || ispunct(key) || key == ' ')
		{
			if (getLen(line) < col)
				;
			insertTo(line, col);
			output(key);
		}
	}
	parent->restore();
}

void KillerTextEditGump::draw(short x, short y)
{
	for (int i = 0; i < 4; i++)
		font->printf(x, y + 17 + i * 6, text + i * 26);
	DrawRectangle(GlobalVport::global_ptr, x + col * 4, y + line * 6 + 16, x + col * 4 + 3, y + line * 6 + 20, GumpColorMap[0]);
}

UnkSymbolEditor::UnkSymbolEditor(unsigned char id, NewGump *parent, char *buffer, unsigned char size, unsigned char width) :
	TextEditGump(id, parent, buffer, size, width, 0, 0)
{
}

void UnkSymbolEditor::commandKeyboard(Event &event)
{
	if (isalnum(event.data) || event.data == 8)
		TextEditGump::commandKeyboard(event);
}

InputInt::InputInt(unsigned char id, NewGump *parent, char *buffer) :
	TextEditGump(id, parent, buffer, 5, 40, 0, 0)
{
}

void InputInt::commandKeyboard(Event &event)
{
	if (event.data < '0' || event.data > '9')
		return;
	TextEditGump::commandKeyboard(event);
}
