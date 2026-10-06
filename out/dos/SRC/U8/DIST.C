// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: DIST.C

#include <stdlib.h>
#include "DIST.H"
#include "SCRATCHM.H"

unsigned char signToDirTable[9] = {7, 6, 5, 0, 8, 4, 1, 2, 3};
char x8add[9] = {0, 1, 1, 1, 0, -1, -1, -1, 0};
char y8add[10] = {-1, -1, 0, 1, 1, 1, 0, -1, 0, 0};

// Chessboard distance between two map points.
int dist(short x1, short y1, short x2, short y2)
{
	int dx, dy;

	dx = abs(x1 - x2);
	dy = abs(y1 - y2);
	return dx > dy ? dx : dy;
}

int dist(short x1, short y1, unsigned char z1, short x2, short y2, unsigned char z2)
{
	int dx, dy, dz, d;

	dx = abs(x1 - x2);
	dy = abs(y1 - y2);
	dz = abs(z1 - z2);
	d = dx > dy ? dx : dy;
	d = d > dz ? d : dz;
	return d;
}

// One of eight directions, 0 being north and counting clockwise.
char getDirCoordToCoord(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2)
{
	int dx = x1 - x2;
	int dy = y1 - y2;
	int adx = abs(dx);
	int ady = abs(dy);

	if (adx / 2 <= ady && adx * 2 >= ady)
	{
		if (dx > 0 && dy > 0)
			return 7;
		if (dx < 0 && dy < 0)
			return 3;
		if (dx > 0 && dy < 0)
			return 5;
		return 1;
	}
	if (adx * 2 <= ady)
	{
		if (dy >= 0)
			return 0;
		return 4;
	}
	if (dx >= 0)
		return 6;
	return 2;
}
