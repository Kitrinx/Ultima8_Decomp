#ifndef BOOK_H
#define BOOK_H

#include "ITEM.H"
#include "IRGUMP.H"
#include "SBUTTON.H"
#include "CAFONT.H"
#include "CRECT.H"

class Event;
class NewGump;

// Shows a book's text two pages at a time, or a scroll's one page at a time.
class BookGump : public ItemRelativeGump, public GumpShape
{
public:
	FontType fontType;
	char *text;
	unsigned offset;		// start of the page on show
	unsigned char f_62;
	unsigned textLength;
	int isScroll;
	Rect f_67;
	unsigned char mouseDown;

	BookGump(NewGump *, char, short, Referent, char *);
	virtual ~BookGump(void);
	virtual void commandMouseLeft(Event &);
	virtual void commandPrivate(Event &);
	virtual void commandKeyboard(Event &);
	virtual void draw(short, short);
	Boolean moreToCome(void);
	Boolean nextPage(void);
};

class Book : public Item
{
public:
	unsigned read(char *);
};

class Scroll : public Item
{
public:
	unsigned read(char *);
};

extern Rect bookrect;
extern Rect scrollrect;

#endif
