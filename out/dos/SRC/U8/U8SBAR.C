// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: U8SBAR.C

#include "..\UI\NEWGUMP.H"
#include "CRECT.H"
#include "DISPATCH.H"
#include "MENU.H"
#include "SBUTTON.H"
#include "U8GMPSHP.H"
#include "U8SBAR.H"

// Inline in the shared headers when this file was built.
inline Point::Point(void) {}
inline void Point::set(short x, short y) { f_00 = x; f_02 = y; }
inline Rect::Rect(void) {}
inline void Rect::set(short x1, short y1, short x2, short y2) { Point::set(x1, y1); f_04 = x2; f_06 = y2; }
inline Rect::Rect(short x1, short y1, short x2, short y2) { set(x1, y1, x2, y2); }
inline int Rect::width(void) { return f_04 - f_00 + 1; }
inline int Rect::height(void) { return f_06 - f_02 + 1; }
inline NewGumpId::NewGumpId(unsigned i) { instance = i; f_02 = ~instance; f_04 = 0; }
inline NewGumpId::NewGumpId(unsigned char type, unsigned char id) { instance = (type << 8) | id; f_02 = ~instance; f_04 = 0; }
inline unsigned char NewGumpId::getInstance(void) { return instance & 0xff; }
inline void NewGump::moveto(short x, short y) { rect.moveto(x, y); }
inline GumpShape::GumpShape(int shape) { _frame = 0; this->shape = shape; }
inline Rect GumpShape::get_rect(void) { return drawer->get_rect(shape, _frame); }
inline void GumpShape::draw(int x, int y) { drawer->draw(x, y, shape, _frame); }

void U8RightActivatorButton::draw(short x, short y)
{
	GumpShape::draw(x, y);
}

void U8LeftActivatorButton::draw(short x, short y)
{
	GumpShape::draw(x, y);
}

U8Thumb::U8Thumb(NewGump *parent, Rect r, long resolution) :
	NewGump(NewGumpId(3), parent, 2), GumpShape(45)
{
	rect = r;
	thumb = 0;
	dragging = 0;
	int size = rect.height() - 1;
	thumbRect = Rect(0, 0, size, size);
	setResolution(resolution);
}

void U8Thumb::setResolution(int steps)
{
	long range = rect.width();
	int width = thumbRect.width();
	range -= width;
	if (steps)
		steps--;
	if (steps > 0)
	{
		resolution = steps;
		scale = (range << 16) / resolution;
	}
	else
	{
		scale = 0;
		thumb = 0;
	}
}

void U8Thumb::draw(short x, short y)
{
	if (scale)
		GumpShape::draw(x + thumbRect.f_00, y);
}

void U8Thumb::commandMouseMovement(Event &event)
{
	long x = getRelX(event) - (thumbRect.width() >> 1);
	if (x < 0)
		x = 0;
	if (scale)
	{
		long t = (x << 16) / scale;
		setThumb((unsigned)t);
		send(Event(EVENT_PRIVATE, 0, 0, 1, this, parent, 0));
	}
}

void U8Thumb::commandMouseLeft(Event &event)
{
	if (event.isSingle() && !dragging)
	{
		setExclusive(this);
		f_1d |= 8;
		dragging = 1;
		commandMouseMovement(event);
	}
	else if (event.isRelease() && dragging)
	{
		dragging = 0;
		clrExclusive();
		f_1d &= ~8;
		send(Event(EVENT_PRIVATE, 0, 0, 1, this, parent, 0));
	}
}

void U8Thumb::setThumb(long t)
{
	if (thumb == t)
		return;
	if (!scale)
	{
		thumb = 0;
		return;
	}
	long range = rect.width();
	long width = thumbRect.width();
	range -= width;
	if (t > resolution)
		t = resolution;
	else if (t < 0)
		t = 0;
	int x = (t * scale) >> 16;
	if (x < 0)
		x = 0;
	if (x > range)
		x = range;
	thumb = t;
	thumbRect.moveto(x, rect.f_02);
}

U8ScrollBar::U8ScrollBar(unsigned char id, NewGump *parent, int length, int steps, short x, short y) :
	NewGump(NewGumpId(4, id), parent, EVENT_PRIVATE)
{
	int right = length;
	rect = Rect(0, 0, length, 9);
	U8RightActivatorButton *rightButton;
	U8LeftActivatorButton *left;
	Rect buttonRect(0, 0, rect.height() - 1, rect.height() - 1);
	left = new U8LeftActivatorButton(1, this, buttonRect, GumpColorMap[5]);
	rightButton = new U8RightActivatorButton(2, this, buttonRect, GumpColorMap[5]);
	buttonRect = rightButton->get_rect();
	left->moveto(0, 0);
	rightButton->moveto(rect.f_04 - buttonRect.width() + 1, 0);
	right -= buttonRect.width() - 1;
	pos = new U8Thumb(this, Rect(buttonRect.width() + 1, 0, right, rect.height() - 1), steps);
	rect.moveto(x, y);
}

void U8ScrollBar::commandPrivate(Event &event)
{
	switch (event.from->newGumpId.getInstance())
	{
	case 1:
		pos->decThumb(1);
		refresh();
		break;
	case 2:
		pos->incThumb(1);
		pos->refresh();
		refresh();
		break;
	}
	send(Event(EVENT_PRIVATE, pos->getThumb(), 0, 1, this, parent, 0));
}

void U8ScrollBar::setPos(int p)
{
	pos->setThumb(p);
}

int U8ScrollBar::getPos(void)
{
	return pos->getThumb();
}

void U8ScrollBar::setRange(int range)
{
	pos->setResolution(range);
}
