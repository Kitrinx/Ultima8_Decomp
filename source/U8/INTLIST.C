// flags: -P -2 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: INTLIST.C

#include "..\GLIB\PROCESS.H"
#include "KERNEL.H"
#include "CFILE.H"
#include "STDINT.H"

// Keeps a poll in progress pointing at the right entry when a pid leaves the list.
void IntList::removeFromList(unsigned short pid)
{
	int index = List::removeFromList(pid);

	if (isAllocated())
	{
		if (current == index)
		{
			current--;
			changed = 1;
		}
		else if (current > index)
			current--;
		last--;
		return;
	}
	last = 0;
	current = 0;
	changed = 1;
}
