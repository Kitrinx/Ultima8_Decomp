// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: ..\ITEM\BOOK.C

#include <string.h>
#include "..\UI\NEWGUMP.H"
#include "DISPATCH.H"
#include "CVPORT.H"
#include "CCURRVP.H"
#include "CGLOBVP.H"
#include "U8GMPSHP.H"
#include "FORMAT.H"
#include "CEXIT.H"
#include "BOOK.H"

// Inline in the shared headers when this file was built.
inline Point::Point(void) {}
inline void Point::set(short x, short y) { f_00 = x; f_02 = y; }
inline Rect::Rect(void) {}
inline void Rect::set(short x1, short y1, short x2, short y2) { Point::set(x1, y1); f_04 = x2; f_06 = y2; }
inline Rect::Rect(short x1, short y1, short x2, short y2) { set(x1, y1, x2, y2); }
inline void NewGump::moveto(short x, short y) { rect.moveto(x, y); }
inline GumpShape::GumpShape(int shape) { _frame = 0; this->shape = shape; }
inline Rect GumpShape::get_rect(void) { return drawer->get_rect(shape, _frame); }
inline void GumpShape::draw(int x, int y) { drawer->draw(x, y, shape, _frame); }
inline CacheFont::~CacheFont(void) {}
inline GraphicYtable::GraphicYtable(void) { f_04 = 2; index = 0; }
inline Vport::Vport(void) { seg = 0; f_0f = 2; f_10 = 0; }
inline Vport::~Vport(void) { free(); }

// Text areas of the book and scroll gump shapes.
Rect bookrect(19, 15, 142, 144);
Rect scrollrect(22, 29, 226, 144);

BookGump::BookGump(NewGump *parent, char scroll, short shape, Referent referent, char *message) :
	ItemRelativeGump((IRGumpType)4, parent, Rect(0, 0, 0, 0), referent, (IRGumpMove)1, 0, 180, 0),
	GumpShape(shape)
{
	f_1d |= 2;
	registerHotKeys(27, 0);
	fontType = (FontType)9;
	CacheFont font(fontType);
	isScroll = scroll;
	rect = get_rect();
	moveto((320 - rect.f_04) >> 1, (200 - rect.f_06) >> 1);

	Rect textRect;
	if (isScroll == 0)
	{
		textRect = bookrect;
		textRect.moveto(rect.f_00 + 9, rect.f_02 + 5);
	}
	else
	{
		textRect = scrollrect;
		textRect.moveto(rect.f_00 + 22, rect.f_02 + 29);
	}
	message = printFormat(message, fontType, &textRect, 1);

	offset = -1;
	textLength = strlen(message) + 1;
	text = new char[textLength];
	if (text == 0)
		outOfMemory(__FILE__, 93);	// __LINE__
	strcpy(text, message);
	f_62 = 0;
	mouseDown = 0;
	this->parent->newGumpList.moveToTail(this);
	setExclusive(0);
	if (nextPage())
		refresh();
}

BookGump::~BookGump(void)
{
	clrExclusive();
	if (text)
		delete text;
}

Boolean BookGump::moreToCome(void)
{
	if (text == 0)
		return FALSE;
	if (offset + strlen(text + offset) + 1 >= textLength)
		return FALSE;
	return TRUE;
}

// Moves to the next page and ends it at its page break.
Boolean BookGump::nextPage(void)
{
	if (text == 0)
		return FALSE;
	if (offset == -1)
		offset = 0;
	else
	{
		offset += strlen(text + offset) + 1;
		if (offset >= textLength)
			return FALSE;
	}
	char *p = text + offset;
	while (*p++)
	{
		if (*p == -1 || *p == 0)
		{
			*p = 0;
			break;
		}
	}
	return TRUE;
}

void BookGump::commandPrivate(Event &event)
{
	if (event.data == -1)
		killMyself();
}

void BookGump::commandKeyboard(Event &event)
{
	if (event.data == 27)
		killMyself();
}

void BookGump::commandMouseLeft(Event &event)
{
	if (event.isSingle())
	{
		mouseDown = 1;
		return;
	}
	if (event.isRelease() && mouseDown)
	{
		mouseDown = 0;
		if (!moreToCome())
		{
			killMyself();
			return;
		}
		nextPage();
		if (!isScroll)
		{
			if (moreToCome())
				nextPage();
			else
			{
				killMyself();
				return;
			}
		}
		refresh();
		return;
	}
	if (event.isDouble())
		killMyself();
}

void BookGump::draw(short x, short y)
{
	CacheFont font(fontType);
	GumpShape::draw(x, y);
	Rect textRect;
	if (isScroll == 0)
	{
		textRect = bookrect;
		textRect.moveto(x + 9, y + 5);
	}
	else
	{
		textRect = scrollrect;
		textRect.moveto(x + 22, y + 29);
	}
	Vport vport;
	vport = *GlobalVport::global_ptr;
	vport.rect = textRect;
	vport.rect.clip(GlobalVport::global_ptr->rect);
	{
		CurrentVport current(&vport);
		font.printf(textRect.f_00, textRect.f_02 + font.getBaseLine(), text + offset);
	}
	if (isScroll == 1)
		return;

	// A book shows the following page on the right.
	if (moreToCome())
	{
		unsigned leftPage = offset;
		nextPage();
		textRect.moveto(x + 150, y + 5);
		vport.rect = textRect;
		vport.rect.clip(GlobalVport::global_ptr->rect);
		{
			CurrentVport current(&vport);
			font.printf(textRect.f_00, textRect.f_02 + font.getBaseLine(), text + offset);
		}
		offset = leftPage;
	}
}

unsigned Book::read(char *message)
{
	if (message)
	{
		BookGump *gump = new BookGump(Dispatcher::base[Dispatcher::baseSP], 0, 6, referent, message);
		return gump->f_4e;
	}
}

unsigned Scroll::read(char *message)
{
	if (message)
	{
		BookGump *gump = new BookGump(Dispatcher::base[Dispatcher::baseSP], 1, 19, referent, message);
		return gump->f_4e;
	}
}
