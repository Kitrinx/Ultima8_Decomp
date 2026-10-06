// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: ..\ITEM\ITEMFIND.C

#include <mem.h>
#include "ITEM.H"
#include "ITEMCACH.H"
#include "TYPE.H"
#include "ERROR.H"
#include "ITEMFIND.H"

#define REGION_SHIFT	9
#define REGION_SIZE		512
#define REGION_MASK		0xfe00

#define STACK_SIZE		32

// A handle on another item met during the search.
class FoundItem : public Item
{
public:
	FoundItem(Referent r) { referent = r; }
	Referent getNext(void) { return ItemData::nextArray[referent]; }
	unsigned char getZ(void) { return ItemData::zArray[referent]; }
};

// The map region holding (x, y).
inline void worldToRegion(unsigned short x, unsigned short y, int &rx, int &ry)
{
	rx = x >> REGION_SHIFT;
	ry = y >> REGION_SHIFT;
}

// A criteria string may start with its own length, flagged by len == -1.
ItemFinder::ItemFinder(Referent start, char *s, unsigned short len)
{
	if (len == -1)
		len = *s++;
	first = start;
	memmove(criteria, s, len);
	restartSearch();
}

void ItemFinder::restartSearch(void)
{
	referent = first;
	if (found() && !meetsSearchCriteria())
		findNext();
}

void ItemFinder::findNext(void)
{
	while (found())
	{
		referent = ItemData::nextArray[referent];
		if (found() && meetsSearchCriteria())
			break;
	}
}

// Runs the criteria program, a postfix expression ending in '$', on the
// current item.

unsigned char ItemFinder::meetsSearchCriteria(void)
{
	Boolean result = TRUE;
	Boolean match;
	Boolean overflow = FALSE;
	Boolean underflow = FALSE;
	int i;
	int c;
	int sp = 0;
	int pc = 0;
	int value;
	unsigned short *list;
	int stack[STACK_SIZE];

	do
	{
		switch (c = criteria[pc++])
		{
		case 1:
			if (sp < STACK_SIZE)
				stack[sp++] = TRUE;
			else
				overflow = TRUE;
			break;
		case 0:
			if (sp < STACK_SIZE)
				stack[sp++] = FALSE;
			else
				overflow = TRUE;
			break;
		case '%':	// a literal word follows
			if (sp < STACK_SIZE)
			{
				value = *(short *)&criteria[pc];
				pc = pc + 2;
				stack[sp++] = value;
			}
			else
				overflow = TRUE;
			break;
		case '@':
			if (sp < STACK_SIZE)
				stack[sp++] = getType();
			else
				overflow = TRUE;
			break;
		case ':':
			if (sp < STACK_SIZE)
			{
				if (ItemData::typeArray[referent] < NUM_TYPES)
					stack[sp++] = GlobalTypes.typeFlags[ItemData::typeArray[referent]].family;
				else
					stack[sp++] = 10;
			}
			else
				overflow = TRUE;
			break;
		case '`':
			if (sp < STACK_SIZE)
				stack[sp++] = getFrame();
			else
				overflow = TRUE;
			break;
		case '?':
			if (sp < STACK_SIZE)
				stack[sp++] = ItemData::statusArray[referent];
			else
				overflow = TRUE;
			break;
		case '*':
			if (sp < STACK_SIZE)
				stack[sp++] = ItemData::qArray[referent];
			else
				overflow = TRUE;
			break;
		case '#':
			if (sp < STACK_SIZE)
				stack[sp++] = referent;
			else
				overflow = TRUE;
			break;
		case '=':
			if (sp > 1)
			{
				stack[sp - 2] = stack[sp - 2] == stack[sp - 1];
				sp--;
			}
			else
				underflow = TRUE;
			break;
		case '>':
			if (sp > 1)
			{
				stack[sp - 2] = stack[sp - 2] > stack[sp - 1];
				sp--;
			}
			else
				underflow = TRUE;
			break;
		case '<':
			if (sp > 1)
			{
				stack[sp - 2] = stack[sp - 2] < stack[sp - 1];
				sp--;
			}
			else
				underflow = TRUE;
			break;
		case ')':
			if (sp > 1)
			{
				stack[sp - 2] = stack[sp - 2] >= stack[sp - 1];
				sp--;
			}
			else
				underflow = TRUE;
			break;
		case '(':
			if (sp > 1)
			{
				stack[sp - 2] = stack[sp - 2] <= stack[sp - 1];
				sp--;
			}
			else
				underflow = TRUE;
			break;
		case '&':
			if (sp > 1)
			{
				stack[sp - 2] = stack[sp - 2] & stack[sp - 1];
				sp--;
			}
			else
				underflow = TRUE;
			break;
		case '+':
			if (sp > 1)
			{
				stack[sp - 2] = stack[sp - 2] | stack[sp - 1];
				sp--;
			}
			else
				underflow = TRUE;
			break;
		case '!':
			if (sp)
				stack[sp - 1] = ~stack[sp - 1];
			else
				underflow = TRUE;
			break;
		}
		if (c == '$')
			break;

		// 'A' to '_' list 1 to 31 types, 'a' up are lists of frames.
		if (c > '@' && c < '`')
		{
			list = (unsigned short *)&criteria[pc];
			match = FALSE;
			for (i = 0; i < c - '@'; i++)
			{
				if (list[i] == ItemData::typeArray[referent])
				{
					match = TRUE;
					break;
				}
			}
			if (sp < STACK_SIZE)
				stack[sp++] = match;
			else
				overflow = TRUE;
			pc += sizeof(short) * (c - '@');
		}
		else if (c > '`' && c < 0x80)
		{
			list = (unsigned short *)&criteria[pc];
			match = FALSE;
			for (i = 0; i < c - '`'; i++)
			{
				if (getFrame() == list[i])
				{
					match = TRUE;
					break;
				}
			}
			if (sp < STACK_SIZE)
				stack[sp++] = match;
			else
				overflow = TRUE;
			pc += sizeof(short) * (c - '`');
		}
	} while (pc < 32);

	if (overflow)
		halt(__FILE__, 319);	// __LINE__
	if (underflow)
		halt(__FILE__, 326);	// __LINE__
	if (sp)
		result = stack[--sp] != 0;
	return result;
}

RegionItemFinder::RegionItemFinder(unsigned short x, unsigned short y, char *s, unsigned short len)
{
	if (len == -1)
		len = *s++;
	gotoRegion(x, y);
	memmove(criteria, s, len);
	restartSearch();
}

void RegionItemFinder::gotoRegion(unsigned short x, unsigned short y)
{
	int rx;
	int ry;

	worldToRegion(x, y, rx, ry);
	first = ItemCache::regionStartHash[rx][ry];
}

void AreaItemFinder::init(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2,
	char *s, unsigned short len)
{
	if (len == -1)
		len = *s++;
	if ((short)x1 < 0)
		x1 = 0;
	if ((short)y1 < 0)
		y1 = 0;
	if ((short)x2 < 0)
		x2 = 0x7fff;
	if ((short)y2 < 0)
		y2 = 0x7fff;
	curX = left = x1;
	curY = top = y1;
	right = x2;
	bottom = y2;
	memmove(criteria, s, len);
	gotoRegion(curX, curY);
	referent = first;
	if (!found())
		gotoNextList();
	if (found() && (!inArea() || !meetsSearchCriteria()))
		findNext();
}

// Searches one whole map region.
void AreaItemFinder::init(short rx, short ry, char *s, unsigned short len)
{
	unsigned short x1 = rx << REGION_SHIFT;
	unsigned short y1 = ry << REGION_SHIFT;
	unsigned short x2 = x1 + REGION_SIZE - 1;
	unsigned short y2 = y1 + REGION_SIZE - 1;
	init(x1, y1, x2, y2, s, len);
}

void AreaItemFinder::gotoNextList(void)
{
	while (!found())
	{
		curX += REGION_SIZE;
		if ((curX & REGION_MASK) > (right & REGION_MASK))
		{
			curX = left;
			curY += REGION_SIZE;
			if ((curY & REGION_MASK) > (bottom & REGION_MASK))
				return;
		}
		gotoRegion(curX, curY);
		referent = first;
	}
}

// Also walks into the contents of every container in the area.
void RecursiveAreaItemFinder::findNext(void)
{
	while (found())
	{
		if (getContents())
			referent = getContents();
		else
		{
			while ((ItemData::statusArray[referent] & 8) && !ItemData::nextArray[referent])
				referent = getContainer();
			referent = ItemData::nextArray[referent];
			if (!found())
				gotoNextList();
		}
		if (found() && inArea() && meetsSearchCriteria())
			break;
	}
}

Boolean RecursiveAreaItemFinder::inArea(void)
{
	unsigned x = getX();
	unsigned y = getY();
	return left <= x && right >= x && top <= y && bottom >= y;
}

void ContainerItemFinder::init(Referent container, char *s, unsigned short len)
{
	if (len == -1)
		len = *s++;
	first = Item(container).getContents();
	memmove(criteria, s, len);
	restartSearch();
}

void RecursiveContainerItemFinder::init(Referent top, char *s, unsigned short len)
{
	if (len == -1)
		len = *s++;
	container = top;
	first = Item(top).getContents();
	memmove(criteria, s, len);
	restartSearch();
}

void RecursiveContainerItemFinder::findNext(void)
{
	Referent next;

	while (found())
	{
		next = ItemData::nextArray[referent];
		if (!next && getContainer() != container)
			next = FoundItem(getContainer()).getNext();
		referent = getContents();
		if (!found())
			referent = next;
		if (found() && meetsSearchCriteria())
			break;
	}
}

// Finds the items touching the top or bottom face of a surface.
void SurfaceItemFinder::init(Referent r, char *s, unsigned short len, unsigned char what)
{
	int left;
	int top;
	int xd;
	int yd;
	int zd;

	surface = r;
	this->what = what;
	where = 0;
	if (r)
	{
		FoundItem item(surface);
		TypeFlag flags = GlobalTypes.typeFlags[ItemData::typeArray[item.referent]];
		xd = flags.xSize * 32;
		yd = flags.ySize * 32;
		zd = flags.zSize * 8;
		zd == 0 && !flags.fixed ? zd = 1 : 0;
		x = ItemData::xArray[item.referent];
		y = ItemData::yArray[item.referent];
		left = x - xd + 1;
		top = y - yd + 1;
		z = ItemData::zArray[item.referent] + zd;
		AreaItemFinder::init(left, top, x + 480, y + 480, s, len);
		return;
	}
	referent = 0;
}

Boolean SurfaceItemFinder::inArea(void)
{
	if (found() && surface != referent && AreaItemFinder::inArea())
	{
		int xd;
		int yd;
		int zd;
		TypeFlag flags = GlobalTypes.typeFlags[ItemData::typeArray[referent]];
		xd = flags.xSize * 32;
		yd = flags.ySize * 32;
		zd = flags.zSize * 8;
		zd == 0 && !flags.fixed ? zd = 1 : 0;
		if ((what & SURFACE_ON) && ItemData::zArray[referent] == z &&
			ItemData::xArray[referent] - xd + 1 <= x &&
			ItemData::yArray[referent] - yd + 1 <= y)
		{
			where = SURFACE_ON;
			return TRUE;
		}
		if ((what & SURFACE_UNDER) &&
			FoundItem(surface).getZ() == ItemData::zArray[referent] + zd &&
			ItemData::xArray[referent] - xd + 1 <= x &&
			ItemData::yArray[referent] - yd + 1 <= y)
		{
			where = SURFACE_UNDER;
			return TRUE;
		}
	}
	where = 0;
	return FALSE;
}
