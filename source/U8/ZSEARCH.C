// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: ZSEARCH.C

#include "CEXIT.H"
#include "FILESPEC.H"
#include "SCRATCHM.H"
#include "FASTFLEX.H"
#include "UPROCESS.H"
#include "UNITFILE.H"

// Starts a usecode loop: over the items in container `range` when item
// is -1, within `range` of item otherwise, or (type 1) on a surface.

int Searcher::findFirst(char *script, short slot, short range, short item, short scriptSize, SearchType type)
{
	local = slot;
	Item origin(item);
	int found;
	Referent ref;
	if (finder)
		delete finder;
	if (type == 1)
	{
		int what;
		if (range != -1 && item != -1)
		{
			what = 3;
			ref = range;
		}
		else
		{
			if (range != -1)
			{
				what = 1;
				ref = range;
			}
			if (item != -1)
			{
				what = 2;
				ref = item;
			}
		}
		finder = new SurfaceItemFinder(ref, script, scriptSize, what);
	}
	else if (item == -1)
	{
		if (type == 0)
			finder = new RecursiveContainerItemFinder(range, script, scriptSize);
		else
			finder = new ContainerItemFinder(range, script, scriptSize);
	}
	else
	{
		int x = origin.getX();
		int y = origin.getY();
		if (type == 0)
			finder = new RecursiveAreaItemFinder(x - range, y - range, x + range, y + range, script, scriptSize);
		else
			finder = new AreaItemFinder(x - range, y - range, x + range, y + range, script, scriptSize);
	}
	found = *(Referent *)((char *)data + data->bp + local) = found = finder->referent;
	finder->findNext();
	return found;
}

// Stores the current item in the loop variable and moves on.
int Searcher::findNext(void)
{
	int found;
	found = *(Referent *)((char *)data + data->bp + local) = found = finder->referent;
	finder->findNext();
	return found;
}

Searcher::~Searcher(void)
{
	if (finder)
		delete finder;
}
