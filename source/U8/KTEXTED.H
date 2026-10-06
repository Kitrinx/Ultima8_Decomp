#ifndef KTEXTED_H
#define KTEXTED_H

#include "DISPATCH.H"
#include "TEXTEDIT.H"
#include "CRECT.H"

class Event;
class Rect;

class CacheFont;

// The save description editor.
class KillerTextEditGump : public NewGump
{
public:
	Rect f_34;
	CacheFont *font;
	char *text;
	char *backup;		// restored by Escape
	int line;
	int col;

	KillerTextEditGump(unsigned char, NewGump *, char *, Rect);
	virtual ~KillerTextEditGump(void);
	virtual void commandMouseLeft(Event &);
	virtual void commandKeyboard(Event &);
	virtual void draw(short, short);
	void clearText(void);
	void nullLine(short);
	int getLen(short);
	void insertTo(short, short);
	int findLastSpace(short);
	int findFirstSpace(short);
	void output(char);
	void deleteChar(short, short);
	void printLine(short);
	void printText(void);
	void nullToEnd(short, short);
};

class UnkSymbolEditor : public TextEditGump
{
public:
	UnkSymbolEditor(unsigned char, NewGump *, char *, unsigned char, unsigned char);
	virtual void commandKeyboard(Event &);
};

class InputInt : public TextEditGump
{
public:
	InputInt(unsigned char, NewGump *, char *);
	virtual void commandKeyboard(Event &);
};

extern unsigned char IDidNotTypeInAnything;

#endif
