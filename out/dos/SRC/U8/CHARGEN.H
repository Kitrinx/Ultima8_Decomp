#ifndef CHARGEN_H
#define CHARGEN_H

#include "FLAG.H"
#include "PRIORITY.H"

// Writes text with the ROM font straight into the 320x200 screen, for debug output.
class CharacterGenerator
{
public:
	unsigned char *font;
	unsigned char *screen;
	unsigned char smallFont;
	unsigned char fgColor;
	unsigned char bgColor;

	CharacterGenerator(void);
	void setSeg(unsigned short);
	void writeChar(short, short, char);
	void writeString(short, short, char *);
	void makeString(short, short, char *, ...);
};

extern unsigned short textBufferSeg;
extern CharacterGenerator charGen;

#endif
