// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: EVENT\CMPOINT.C

#include "..\MISC\CEXIT.H"
#include "SYSTIMER.H"
#include "CMPOINT.H"
#include "DISPATCH.H"

// Inline in the shared headers when this file was built.
inline MouseButtonStatus::MouseButtonStatus(void) {}
inline MouseEvent::MouseEvent(void) { _action = 0; }

unsigned MousePointer::old_x = 0;
unsigned MousePointer::old_y = 0;
unsigned char MousePointer::visible = 0;
unsigned MousePointer::catSP = 0;
CatState MousePointer::cat[5];

MousePointer::MousePointer(void) :
	MouseHandler(1)
{
	visible = 0;
	old_x = 0;
	old_y = 0;
	for (int i = 0; i < 5; i++)
		cat[i] = CAT_FREE;
}

void MousePointer::pushCat(CatState state)
{
	if (catSP < 4)
		cat[++catSP] = state;
	else
		halt(__FILE__, 51);
}

void MousePointer::popCat(void)
{
	if (catSP > 0)
		catSP--;
}
