// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: TARGGUMP.C

#include "..\UI\NEWGUMP.H"
#include "CRECT.H"
#include "DISPATCH.H"
#include "ITEM.H"
#include "TYPE.H"
#include "CAMERA.H"
#include "U8POINT.H"
#include "TARGGUMP.H"

// Inline in the shared headers when this file was built.
inline void Point::set(short x, short y) { f_00 = x; f_02 = y; }
inline void Rect::set(short x1, short y1, short x2, short y2) { Point::set(x1, y1); f_04 = x2; f_06 = y2; }
inline NewGumpId::NewGumpId(unsigned i) { instance = i; f_02 = ~instance; f_04 = 0; }
inline void NewGump::moveto(short x, short y) { rect.moveto(x, y); }

// A full-screen gump that waits for a click on the world.
void TargetGump::init(void)
{
	rect.set(0, 0, 319, 199);
	moveto(0, 0);
	U8MousePointer::pushMode((PointerModes)0x22, 1);
	clicks = 0;
	registerHotKeys(27, 0);
}

TargetGump::TargetGump(Item *target, NewGump *parent) :
	NewGump(NewGumpId(0), parent, 3)
{
	init();
	item = target;
	*item = 0;
	toScreen = 0;
	findItem = 1;
}

TargetGump::TargetGump(WorldPoint *where, Item *target, unsigned char *found, NewGump *parent) :
	NewGump(NewGumpId(0), parent, 3)
{
	init();
	item = target;
	*item = 0;
	toScreen = 0;
	point = where;
	flag = found;
	findItem = 0;
}

TargetGump::TargetGump(unsigned short *x, unsigned short *y, NewGump *parent) :
	NewGump(NewGumpId(0), parent, 3)
{
	init();
	toScreen = 1;
	xp = x;
	yp = y;
	findItem = 0;
}

TargetGump::~TargetGump(void)
{
	U8MousePointer::popMode();
	if (parent)
		send(Event(EVENT_PRIVATE, 0, 0, clicks, this, parent, 0));
}

void TargetGump::commandMouseLeft(Event &event)
{
	if (event.isSingle())
		clicks++;
	if (event.isRelease() && clicks)
	{
		if (findItem)
			item->findTarget(getRelX(event), getRelY(event));
		else if (toScreen)
		{
			FastItem found;
			unsigned char z;
			unsigned char hit;
			ScreenToWorldCoords(event.x, event.y, *xp, *yp, z, &found, &hit);
		}
		else
			ScreenToWorldCoords(event.x, event.y, (unsigned short &)point->x, (unsigned short &)point->y, point->z, (FastItem *)item, flag);
		killMyself();
	}
}

void TargetGump::commandKeyboard(Event &event)
{
	if (event.data == 27)
		killMyself();
}
