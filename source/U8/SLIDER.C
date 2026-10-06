// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: SLIDER.C

#include "..\UI\NEWGUMP.H"
#include "CRECT.H"
#include "DISPATCH.H"
#include "CAFONT.H"
#include "U8GMPSHP.H"
#include "U8SBAR.H"
#include "MENU.H"
#include "SLIDER.H"

// Inline in the shared headers when this file was built.
inline Point::Point(void) {}
inline void Point::set(short x, short y) { f_00 = x; f_02 = y; }
inline void Rect::set(short x1, short y1, short x2, short y2) { Point::set(x1, y1); f_04 = x2; f_06 = y2; }
inline Rect::Rect(short x1, short y1, short x2, short y2) { set(x1, y1, x2, y2); }
inline int Rect::width(void) { return f_04 - f_00 + 1; }
inline int Rect::height(void) { return f_06 - f_02 + 1; }
inline NewGumpId::NewGumpId(unsigned i) { instance = i; f_02 = ~instance; f_04 = 0; }
inline unsigned char NewGumpId::getType(void) { return instance >> 8; }
inline void NewGump::moveto(short x, short y) { rect.moveto(x, y); }
inline GumpShape::GumpShape(int shape) { _frame = 0; this->shape = shape; }
inline Rect GumpShape::get_rect(void) { return drawer->get_rect(shape, _frame); }
inline void GumpShape::draw(int x, int y) { drawer->draw(x, y, shape, _frame); }
inline CacheFont::~CacheFont(void) {}

SliderGump::SliderGump(NewGump *parent, short, short, int minValue, int maxValue, int step, int startValue) :
	PanelGump(NewGumpId(0), parent, Rect(0, 0, 10, 10), GumpColorMap[5]),
	GumpShape(41)
{
	rect = get_rect();
	moveto((320 - rect.width()) >> 1, (200 - rect.height()) >> 1);
	min = minValue;
	max = maxValue;
	delta = step;
	if (startValue < 0)
		startValue = maxValue;
	value = startValue;
	U8ScrollBar *bar = new U8ScrollBar(0, this, 121, (max + 1 - min) / delta, 36, 17);
	bar->setPos(startValue);
	ShapePushButton *button = new ShapePushButton(0, this, 42);
	button->rect.moverel(158, 17);
	registerHotKeys(13, 0);
	refresh();
}

void SliderGump::draw(short x, short y)
{
	GumpShape::draw(x, y);
	CacheFont font((FontType)0);
	font.printf(x + 17, y + 25, "%d", value);
}

void SliderGump::commandPrivate(Event &event)
{
	int type = event.from->newGumpId.getType();
	if (type == 4)
	{
		value = event.x * delta + min;
		refresh();
	}
	else if (type == 2)
	{
		send(Event(EVENT_PRIVATE, 0, 0, value, this, parent, 0));
		killMyself();
	}
	else if (event.data == 1)
		refresh();
}

void SliderGump::commandKeyboard(Event &)
{
	send(Event(EVENT_PRIVATE, 0, 0, value, this, parent, 0));
	killMyself();
}

long getSliderInput(unsigned short, int min, int max, int delta)
{
	return Dispatch(new SliderGump(0, 130, 90, min, max, delta, -1));
}
