// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: ..\ITEM\GRAVE.C

#include <string.h>
#include "..\UI\NEWGUMP.H"
#include "DISPATCH.H"
#include "CVPORT.H"
#include "CCURRVP.H"
#include "CGLOBVP.H"
#include "U8GMPSHP.H"
#include "CEXIT.H"
#include "GRAVE.H"

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

#define LINE_GAP	4

GraveGump::GraveGump(NewGump *parent, short shape, FontType font, Referent referent, char *message) :
	ItemRelativeGump((IRGumpType)4, parent, Rect(0, 0, 0, 0), referent, (IRGumpMove)1, 0, 120, 0),
	GumpShape(shape)
{
	f_1d |= 0x22;
	registerHotKeys(-1, 0);
	fontType = font;
	graveShape = shape;
	rect = get_rect();
	moveto((320 - rect.f_04) >> 1, (200 - rect.f_06) >> 1);

	textLength = strlen(message) + 1;
	text = new char[textLength];
	if (text == 0)
		outOfMemory(__FILE__, 81);	// __LINE__
	strcpy(text, message);
	mouseDown = 0;
	this->parent->newGumpList.moveToTail(this);

	lineCount = 0;
	offset = -1;
	while (moreToCome())
	{
		nextLine();
		lineCount++;
	}
	offset = -1;
	setExclusive(0);
	refresh();
}

GraveGump::~GraveGump(void)
{
	clrExclusive();
	if (text)
		delete text;
}

Boolean GraveGump::moreToCome(void)
{
	if (text == 0)
		return FALSE;
	if (offset == -1)
		return TRUE;
	if (offset + strlen(text + offset) + 1 >= textLength)
		return FALSE;
	return TRUE;
}

// Moves to the next line and ends it at its line break.
Boolean GraveGump::nextLine(void)
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
		if (*p == '*' || *p == 0)
		{
			*p = 0;
			break;
		}
	}
	return TRUE;
}

void GraveGump::commandPrivate(Event &event)
{
	if (event.data == -1)
		killMyself();
}

void GraveGump::commandKeyboard(Event &)
{
	killMyself();
}

void GraveGump::commandMouseLeft(Event &event)
{
	if (event.isSingle())
	{
		mouseDown = 1;
		return;
	}
	if (event.isRelease() && mouseDown)
	{
		mouseDown = 0;
		killMyself();
		return;
	}
	if (event.isDouble())
		killMyself();
}

void GraveGump::draw(short x, short y)
{
	int w;
	int h;

	CacheFont font(fontType);
	GumpShape::draw(x, y);
	Rect textRect = U8GumpShapeDrawer::rects[graveShape - 1];
	textRect.moveto(x + textRect.f_00, y + textRect.f_02);
	Vport vport;
	vport = *GlobalVport::global_ptr;
	vport.rect = textRect;
	vport.rect.clip(GlobalVport::global_ptr->rect);

	// Centre the block of lines vertically.
	font.dim('A', w, h);
	int textY = (textRect.f_06 - textRect.f_02 + 1 - (lineCount * h + lineCount * LINE_GAP)) >> 1;
	offset = -1;
	{
		CurrentVport current(&vport);
		while (moreToCome())
		{
			nextLine();

			// Trim the spaces at both ends of the line.
			unsigned len = strlen(text + offset);
			int start;
			for (start = offset; text[start] == ' ' && start - offset < len; start++)
				;
			int end;
			for (end = offset + len - 1; text[end] == ' ' && end != start; end--)
				;
			if (start < end)
			{
				char saved = 0;
				if (text[end + 1])
				{
					saved = text[end + 1];
					text[end + 1] = 0;
				}
				font.Font::dim(text + start, w, h);
				w = (textRect.f_04 - textRect.f_00 + 1 - w) >> 1;
				font.printf(textRect.f_00 + w, textRect.f_02 + textY + font.getBaseLine(), text + start);
				if (saved)
					text[end + 1] = saved;
				textY += h + LINE_GAP;
			}
		}
	}
}

unsigned Grave::read(short shape, char *message)
{
	if (message)
	{
		GraveGump *gump = new GraveGump(Dispatcher::base[Dispatcher::baseSP], shape, (FontType)11, referent, message);
		return gump->f_4e;
	}
}

unsigned Plaque::read(short shape, char *message)
{
	if (message)
	{
		GraveGump *gump = new GraveGump(Dispatcher::base[Dispatcher::baseSP], shape, (FontType)10, referent, message);
		return gump->f_4e;
	}
}
