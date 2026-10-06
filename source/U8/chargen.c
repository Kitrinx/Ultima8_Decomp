// flags: -P -2 -Z -G -k-
// name: chargen.c

#include <dos.h>
#include <phapi.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "ERROR.H"
#include "chargen.H"

extern "C" unsigned char *getRomFontPtr(int);

unsigned short textBufferSeg;
CharacterGenerator charGen;

CharacterGenerator::CharacterGenerator(void)
{
	unsigned short seg;

	smallFont = 0;
	font = getRomFontPtr(2);
	fgColor = 15;
	bgColor = 0;
	DosMapRealSeg(0xb800, 8000L, &textBufferSeg);
	DosMapRealSeg(0xa000, 64000L, &seg);
	setSeg(seg);
}

void CharacterGenerator::setSeg(unsigned short seg)
{
	screen = (unsigned char *)MK_FP(seg, 0);
}

// Glyphs are 8x14; the small font keeps every other column and drops the top and bottom rows.
void CharacterGenerator::writeChar(short x, short y, char c)
{
	int row = 14;
	unsigned char *glyph = font + c * 14;
	unsigned char *dest = screen + x + y * 320;

	while (row--)
	{
		unsigned char bits = *glyph++;
		unsigned char mask = 0x80;
		if (smallFont && (row == 13 || row == 0))
			continue;
		while (mask)
		{
			if (bits & mask)
				*dest++ = fgColor;
			else
				*dest++ = bgColor;
			if (smallFont)
				mask >>= 2;
			else
				mask >>= 1;
		}
		if (smallFont)
			dest += 316;
		else
			dest += 312;
	}
}

void CharacterGenerator::writeString(short x, short y, char *s)
{
	short startX = x;
	short startY = y;
	int line = 0;

	while (*s)
	{
		if (*s == '\n')
		{
			line++;
			x = startX;
			y = startY;
			if (smallFont)
				y += line * 12;
			else
				y += line * 14;
			s++;
		}
		else
		{
			writeChar(x, y, *s++);
			if (smallFont)
				x += 4;
			else
				x += 8;
		}
	}
}

void CharacterGenerator::makeString(short x, short y, char *format, ...)
{
	char buffer[100];
	va_list ap;

	va_start(ap, format);
	vsprintf(buffer, format, ap);
	if (strlen(buffer) > 99)
		halt(__FILE__, 138);	// __LINE__
	writeString(x, y, buffer);
}
