// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: CONTRECT.C

#include "SYSTIMER.H"
#include "CRECT.H"

inline Point::Point(void) {}

#include "CONTRECT.H"

long getCurrentTimerTick(void)
{
	return systemTimer->ticks;
}

// Indexed by container gump number.
ContGumpRect contGumpRect[20] =
{
	ContGumpRect(0, 0, 0, 0),
	ContGumpRect(80, 43, 23, 34),
	ContGumpRect(76, 49, 15, 14),
	ContGumpRect(66, 59, 20, 17),
	ContGumpRect(81, 39, 10, 22),
	ContGumpRect(0, 0, 0, 0),
	ContGumpRect(127, 39, 10, 51),
	ContGumpRect(90, 22, 4, 7),
	ContGumpRect(89, 36, 7, 26),
	ContGumpRect(67, 45, 8, 17),
	ContGumpRect(101, 45, 8, 4),
	ContGumpRect(87, 45, 3, 42),
	ContGumpRect(78, 43, 6, 5),
	ContGumpRect(60, 47, 22, 14),
	ContGumpRect(0, 0, 0, 0),
	ContGumpRect(0, 0, 0, 0),
	ContGumpRect(0, 0, 0, 0),
	ContGumpRect(0, 0, 0, 0),
	ContGumpRect(0, 0, 0, 0)
};
