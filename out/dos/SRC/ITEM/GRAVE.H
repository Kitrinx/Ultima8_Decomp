#ifndef GRAVE_H
#define GRAVE_H

#include "ITEM.H"
#include "IRGUMP.H"
#include "SBUTTON.H"
#include "CAFONT.H"
#include "CRECT.H"

class Event;
class NewGump;

// Shows the inscription of a gravestone or plaque, centred line by line.
class GraveGump : public ItemRelativeGump, public GumpShape
{
public:
	FontType fontType;
	char *text;
	unsigned offset;		// start of the line on show
	int lineCount;
	unsigned textLength;
	int graveShape;
	Rect f_68;
	unsigned char mouseDown;

	GraveGump(NewGump *, short, FontType, Referent, char *);
	virtual ~GraveGump(void);
	virtual void commandMouseLeft(Event &);
	virtual void commandPrivate(Event &);
	virtual void commandKeyboard(Event &);
	virtual void draw(short, short);
	Boolean moreToCome(void);
	Boolean nextLine(void);
};

class Grave : public Item
{
public:
	unsigned read(short, char *);
};

class Plaque : public Item
{
public:
	unsigned read(short, char *);
};

#endif
