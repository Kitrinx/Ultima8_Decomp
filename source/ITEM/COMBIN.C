// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r- -y
// name: ..\ITEM\COMBIN.C

#include "ITEM.H"
#include "ITEMCACH.H"
#include "ITEMFIND.H"
#include "TYPE.H"
#include "CONTAIN.H"
#include "COMBIN.H"

// Quantity items and reagent piles of the same type merge into one stack.
#define MAX_QUANTITY	666

Boolean Combinable::combine(Referent otherReferent)
{
	Combinable other(otherReferent);

	int family = GlobalTypes.typeFlags[getType()].family;
	if (getType() == ItemData::typeArray[other.referent] &&
		(family == QUAN_FAMILY || family == REAGENT_FAMILY))
	{
		// A reagent pile's frame shows its size, so piles only merge
		// when their frames fall in the same size group.

		if (family == REAGENT_FAMILY)
		{
			int otherFrame = other.getFrame();
			int frame = getFrame();

			switch (getType())
			{
			case 395:
				if (otherFrame < 5) otherFrame = 0;
				else if (otherFrame == 7) otherFrame = 6;
				else if (otherFrame == 11 || otherFrame == 12) otherFrame = 10;
				else if (otherFrame == 15) otherFrame = 14;
				else if (otherFrame > 16) otherFrame = 16;
				if (frame < 5) frame = 0;
				else if (frame == 7) frame = 6;
				else if (frame == 11 || frame == 12) frame = 10;
				else if (frame == 15) frame = 14;
				else if (frame > 16) frame = 16;
				break;
			case 398:
				if (otherFrame == 1) otherFrame = 0;
				else if (otherFrame >= 3 && otherFrame <= 5) otherFrame = 2;
				else if (otherFrame >= 7 && otherFrame <= 9) otherFrame = 6;
				else if (otherFrame >= 11 && otherFrame <= 13) otherFrame = 10;
				else if (otherFrame >= 15 && otherFrame <= 17) otherFrame = 14;
				else if (otherFrame == 19 || otherFrame == 20) otherFrame = 18;
				if (frame == 1) frame = 0;
				else if (frame >= 3 && frame <= 5) frame = 2;
				else if (frame >= 7 && frame <= 9) frame = 6;
				else if (frame >= 11 && frame <= 13) frame = 10;
				else if (frame >= 15 && frame <= 17) frame = 14;
				else if (frame == 19 || frame == 20) frame = 18;
				break;
			}

			if (otherFrame != frame)
			{
				return FALSE;
			}
		}
		int total = getQuantity() + other.getQuantity();
		if (total <= MAX_QUANTITY)
		{
			setQuantity(total);
			other.destroy();
		}
		else
		{
			setQuantity(MAX_QUANTITY);
			other.setQuantity(total - MAX_QUANTITY);
		}
		return TRUE;
	}
	return FALSE;
}

// Moves this item into a container, then merges it with the first
// matching stack already inside.

Boolean Combinable::give(Referent container)
{
	if (Container(container).can_hold(referent))
	{
		push();
		pop(container);
		RecursiveContainerItemFinder finder(container);
		while (finder.found())
		{
			if (combine(finder.referent))
				break;

			finder.findNext();
		}
		return TRUE;
	}
	return FALSE;
}
