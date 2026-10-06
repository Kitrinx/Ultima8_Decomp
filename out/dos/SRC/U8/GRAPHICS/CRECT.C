// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: GRAPHICS\CRECT.C

#include "CRECT.H"

inline Rect::Rect(void) {}
inline Point::Point(void) {}
inline int Rect::width(void) { return f_04 - f_00 + 1; }
inline int Rect::height(void) { return f_06 - f_02 + 1; }

void Rect::moveto(short x, short y)
{
	short dx = x - f_00;
	short dy = y - f_02;
	moverel(dx, dy);
}

unsigned char Rect::moverel(short dx, short dy)
{
	// The shipped code moves an unused, uninitialised point too.
	Point delta;
	delta.f_00 += dx;
	delta.f_02 += dy;

	f_00 += dx;
	f_02 += dy;
	f_04 += dx;
	f_06 += dy;
	return 1;
}

int Rect::contains(Point p)
{
	return f_00 <= p.f_00 && f_04 >= p.f_00 && f_02 <= p.f_02 && f_06 >= p.f_02;
}

unsigned char Rect::contains(short x, short y)
{
	return f_00 <= x && f_04 >= x && f_02 <= y && f_06 >= y;
}

void Rect::clip(Rect &r)
{
	if (r.f_00 > f_00)
		f_00 = r.f_00;
	if (r.f_04 < f_04)
		f_04 = r.f_04;
	if (r.f_02 > f_02)
		f_02 = r.f_02;
	if (r.f_06 < f_06)
		f_06 = r.f_06;
	if (f_04 - f_00 < 0 || f_06 - f_02 < 0)
		f_00 = f_02 = f_04 = f_06 = 0;
}

unsigned char Rect::intersects(Rect &r)
{
	Rect overlap;
	overlap.f_00 = f_00 > r.f_00 ? f_00 : r.f_00;
	overlap.f_02 = f_02 > r.f_02 ? f_02 : r.f_02;
	overlap.f_04 = f_04 < r.f_04 ? f_04 : r.f_04;
	overlap.f_06 = f_06 < r.f_06 ? f_06 : r.f_06;
	return overlap.width() > 0 && overlap.height() > 0;
}
