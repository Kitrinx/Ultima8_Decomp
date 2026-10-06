#ifndef CONTRECT_H
#define CONTRECT_H

#include "CRECT.H"

// Where a container gump draws its contents: top left and size.
class ContGumpRect
{
public:
	int x;
	int y;
	Point size;

	ContGumpRect(void) {}
	ContGumpRect(short x, short y, short w, short h)
		{ this->x = x; this->y = y; size.f_00 = w; size.f_02 = h; }
};

long getCurrentTimerTick(void);

extern ContGumpRect contGumpRect[20];

#endif
