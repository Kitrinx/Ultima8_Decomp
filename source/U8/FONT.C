// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: FONT.C

#include <dos.h>
#include <stdarg.h>
#include <stdio.h>
#include "CRECT.H"
#include "CVPORT.H"
#include "CGLOBVP.H"
#include "SYSTEM.H"
#include "FONT.H"

Font *GlobalFont::main_font = 0;
Font *GlobalFont::global_ptr = 0;

Font::~Font(void)
{
}

Font::Font(void)
{
	gotoXY(0, 0);
	setXSpacing(0);
	setYSpacing(0);
}

void Font::print(unsigned char c)
{
	int width, height;
	dim(c, width, height);
	drawChar(cursorX, cursorY, c);
	if (c == '\n')
	{
		cursorY += height;
		cursorX = GlobalVport::global_ptr->getRect()->f_00;
	}
	else if (c == '\r')
		cursorX = GlobalVport::global_ptr->getRect()->f_00;
	else
		cursorX += width;
}

void Font::print(char *text)
{
	while (*text)
	{
		print(*text);
		text++;
	}
}

// Prints as much of text as fits in maxWidth; returns the count printed.
int Font::print(int maxWidth, char *text)
{
	int startX = cursorX;
	int count = 0;
	int width, height;
	while (*text)
	{
		dim(*text, width, height);
		if (cursorX + width > startX + maxWidth)
			break;
		print(*text);
		text++;
		count++;
	}
	return count;
}

void Font::dim(char *text, int &width, int &height)
{
	int lineWidth = 0;
	int maxWidth = 0;
	int totalHeight = 0;
	int charHeight = 0;
	while (*text)
	{
		dim(*text, width, charHeight);
		if (*text == '\n')
		{
			totalHeight += charHeight;
			if (lineWidth > maxWidth)
				maxWidth = lineWidth;
			lineWidth = 0;
		}
		lineWidth += width;
		text++;
	}
	totalHeight += charHeight;
	if (lineWidth > maxWidth)
		maxWidth = lineWidth;
	width = maxWidth;
	height = totalHeight;
}

int Font::height(char *text)
{
	int height, width;
	dim(text, width, height);
	return height;
}

int Font::width(char *text)
{
	int height, width;
	dim(text, width, height);
	return width;
}

// The argument list is found by hand: C++ va_start (args = ...) would give
// every later function in the file a stack frame, getRomFontPtr included.

// align: 'c' centres on the screen, 'l' starts at the left edge.
void Font::printf(int x, int y, char align, char *format, ...)
{
	va_list args;
	if (format && *format && format != (char *)WorkString)
	{
		args = (va_list)(&format + 1);
		vsprintf(WorkString, format, args);
	}
	else
		*(char *)WorkString = 0;
	int left;
	if (align == 'c')
		left = (320 - width(WorkString)) >> 1;
	else if (align == 'l')
		left = 0;
	else
		left = x;
	gotoXY(left, y);
	print(WorkString);
}

int Font::printf(int x, int y, char *format, ...)
{
	va_list args;
	if (format && *format && format != (char *)WorkString)
	{
		args = (va_list)(&format + 1);
		vsprintf(WorkString, format, args);
	}
	else
		*(char *)WorkString = 0;
	gotoXY(x, y);
	print(WorkString);
}

void Font::printf(int x, int y, int maxWidth, char *format, ...)
{
	va_list args;
	if (format && *format && format != (char *)WorkString)
	{
		args = (va_list)(&format + 1);
		vsprintf(WorkString, format, args);
	}
	else
		*(char *)WorkString = 0;
	gotoXY(x, y);
	print(maxWidth, WorkString);
}

void Font::printf(char *format, ...)
{
	va_list args;
	if (format && *format && format != (char *)WorkString)
	{
		args = (va_list)(&format + 1);
		vsprintf(WorkString, format, args);
	}
	else
		*(char *)WorkString = 0;
	print(WorkString);
}

void Font::getRect(Rect &rect, int x, int y, char *text)
{
	rect.f_00 = rect.f_02 = 0;
	dim(text, rect.f_04, rect.f_06);
	rect.moveto(x, y);
	rect.moverel(0, -getBaseLine());
}

void GlobalFont::setMainFont(Font *font)
{
	global_ptr = main_font = font;
}

CurrentFont::CurrentFont(Font *font)
{
	oldFont = GlobalFont::global_ptr;
	GlobalFont::global_ptr = font;
}

CurrentFont::~CurrentFont(void)
{
	GlobalFont::global_ptr = oldFont;
}

void Font::gotoXY(int x, int y)
{
	cursorX = x;
	cursorY = y;
}

void Font::setXSpacing(int spacing)
{
	xSpacing = spacing;
}

// Writes xSpacing, as the shipped code does.
void Font::setYSpacing(int spacing)
{
	xSpacing = spacing;
}

// Returns the address of a VGA BIOS ROM font. The caller passes the font
// number, which is read straight off the stack.
void *getRomFontPtr(void)
{
	asm enter 0, 0
	asm push es
	asm push bp
	_AH = 0x11;
	_AL = 0x30;
	asm mov bh, [bp + 6]
	geninterrupt(0x10);
	_DX = _ES;
	_AX = _BP;
	asm pop bp
	asm pop es
	asm leave
}
