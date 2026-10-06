// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: U8YESNO.C

#include <ctype.h>
#include "..\UI\NEWGUMP.H"
#include "CRECT.H"
#include "DISPATCH.H"
#include "SBUTTON.H"
#include "MENU.H"
#include "U8GMPSHP.H"
#include "ITEM.H"
#include "NPC.H"
#include "U8YESNO.H"

// Inline in the shared headers when this file was built.
inline Point::Point(void) {}
inline void Point::set(short x, short y) { f_00 = x; f_02 = y; }
inline Rect::Rect(void) {}
inline void Rect::set(short x1, short y1, short x2, short y2) { Point::set(x1, y1); f_04 = x2; f_06 = y2; }
inline Rect::Rect(short x1, short y1, short x2, short y2) { set(x1, y1, x2, y2); }
inline int Rect::width(void) { return f_04 - f_00 + 1; }
inline int Rect::height(void) { return f_06 - f_02 + 1; }
inline unsigned char NewGumpId::getInstance(void) { return instance & 0xff; }
inline void NewGump::moveto(short x, short y) { rect.moveto(x, y); }
inline GumpShape::GumpShape(int shape) { _frame = 0; this->shape = shape; }
inline Rect GumpShape::get_rect(void) { return drawer->get_rect(shape, _frame); }
inline void GumpShape::draw(int x, int y) { drawer->draw(x, y, shape, _frame); }
inline void GumpShape::set_frame(int frame) { _frame = frame; }

U8YesNoGump::U8YesNoGump(NewGumpId id, NewGump *owner, char *) :
	PanelGump(id, owner, Rect(0, 0, 5, 5), GumpColorMap[5]),
	GumpShape(17)
{
	ShapePushButton *yes = new ShapePushButton(1, this, avatar.language + 47);
	ShapePushButton *no = new ShapePushButton(0, this, avatar.language + 50);
	rect = get_rect();
	yes->moveto((rect.width() >> 1) - (yes->rect.width() + 5), rect.height() - (yes->rect.height() + 5));
	no->moveto((rect.width() >> 1) + 5, rect.height() - (no->rect.height() + 5));
	rect.moveto(160 - (rect.width() >> 1), 100 - (rect.height() >> 1));
	if (parent)
		rect.moverel(-parent->get_dx(), -parent->get_dy());
	registerHotKeys('o', 'O', 'j', 'J', 'y', 'Y', 'n', 'N', 's', 'S', 0);
	drawn = 0;
}

void U8YesNoGump::commandPrivate(Event &event)
{
	send(Event(EVENT_PRIVATE, 0, 0, event.from->newGumpId.getInstance(), this, parent, 0));
	killMyself();
}

void U8YesNoGump::commandKeyboard(Event &event)
{
	char c = toupper(event.data);
	if (c == 'Y')
		send(Event(EVENT_PRIVATE, 0, 0, c == 'Y', this, parent, 0));
	else if (c == 'J')
		send(Event(EVENT_PRIVATE, 0, 0, c == 'J', this, parent, 0));
	else if (c == 'O')
		send(Event(EVENT_PRIVATE, 0, 0, c == 'O', this, parent, 0));
	else if (c == 'S')
		send(Event(EVENT_PRIVATE, 0, 0, c == 'S', this, parent, 0));
	else
		send(Event(EVENT_PRIVATE, 0, 0, c == 'Y', this, parent, 0));
	killMyself();
}

void U8YesNoGump::draw(short x, short y)
{
	GumpShape question(18);
	question.set_frame(avatar.language);
	Rect r = question.get_rect();
	int dx = (rect.width() - r.width()) >> 1;
	GumpShape::draw(x, y);
	question.draw(x + dx, y + 5);
	if (!drawn)
		drawn = 1;
}
