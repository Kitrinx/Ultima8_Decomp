#ifndef BARK_H
#define BARK_H

#include "..\GLIB\PROCESS.H"
#include "CAFONT.H"
#include "DISPATCH.H"
#include "IRGUMP.H"

class BarkGump;
class BaseFile;
class Event;
class Rect;

// Debug process: outlines the screen places barks have taken.
class DorkProcess : public Process
{
public:
	virtual void process(void);
};

// Speech or text shown over an item, a page at a time, placed to keep clear of other barks.
class BarkGump : public ItemRelativeGump
{
public:
	FontType fontType;
	char *text;
	int rectIndex;		// slot taken in rects, or -1
	unsigned offset;	// start of the page shown, or -1 before the first
	unsigned char f_60;
	unsigned screenX;		// where the item is on screen
	unsigned screenY;
	unsigned textLength;
	Rect itemRect;		// the item's own shape on screen

	BarkGump(NewGump *, unsigned short, char *, unsigned char, unsigned short);
	virtual ~BarkGump(void);
	virtual void commandMouseLeft(Event &);
	virtual void commandPrivate(Event &);
	virtual void draw(short, short);
	virtual void killMyself(void);
	virtual void move(MovedState);
	void findPlace(void);
	void removeRect(void);
	void nextPage(void);
	static void load(BaseFile *);
	static void save(BaseFile *);
	static int numBarks;
	static Rect rects[30];
};

// One keyword of an InputGump.
class InputElementGump : public NewGump
{
public:
	char *text;

	InputElementGump(NewGump *, char *, Rect *);
	virtual ~InputElementGump(void);
	virtual void commandMouseLeft(Event &);
	virtual void draw(short, short);
};

// Offers the Avatar the keywords of a usecode list to choose from.
class InputGump : public BarkGump
{
public:
	Rect f_6f;

	InputGump(unsigned short, unsigned short);
	virtual void commandMouseLeft(Event &) {}
	virtual void commandPrivate(Event &);
	virtual void draw(short, short);
	virtual void killMyself(void);
};

void BarkTest(void);
unsigned short ask(unsigned short, unsigned short);
void f6key(void);

extern char StringHeader[3];
extern Process *barkTest;

#endif
