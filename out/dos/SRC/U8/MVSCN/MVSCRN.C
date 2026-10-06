// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: MVSCN\MVSCRN.C

#include "CVPORT.H"
#include "MVSCRN.H"

#define ABS(n) ((n) < 0 ? -(n) : (n))

// Picks the scroll routine for the direction of (dx, dy).
void MoveScreen(Vport *v, short dx, short dy)
{
	if (dx == 0)
	{
		if (dy == 0)
			return;
		if (dy > 0)
			MoveScreenUp(v, dy);
		else
			MoveScreenDown(v, ABS(dy));
	}
	else if (dy != 0)
	{
		if (dx > 0)
		{
			if (dy > 0)
				MoveScreenUpRight(v, dx, dy);
			else
				MoveScreenDownRight(v, dx, ABS(dy));
		}
		else
		{
			if (dy > 0)
				MoveScreenUpLeft(v, ABS(dx), dy);
			else
				MoveScreenDownLeft(v, ABS(dx), ABS(dy));
		}
	}
	else
	{
		if (dx > 0)
			MoveScreenRight(v, dx);
		else
			MoveScreenLeft(v, ABS(dx));
	}
}
