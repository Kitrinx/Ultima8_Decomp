// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: ..\ITEM\GLOB.C

#include <mem.h>
#include "CFILE.H"
#include "FILESPEC.H"
#include "SCRATCHM.H"
#include "CACHE.H"
#include "ERROR.H"
#include "DLIST.H"
#include "ITEM.H"
#include "ITEMCACH.H"
#include "TYPE.H"
#include "GLOB.H"

// Inline in the shared headers when this file was built.
inline unsigned char BaseFile::is_valid(void) { return handle != -1; }
inline BaseFile::~BaseFile(void) { if (is_valid()) close(); }
inline Index::Index(void) { size = 0; offset = 0; }
inline Point::Point(void) {}
inline void Point::set(short x, short y) { f_00 = x; f_02 = y; }
inline Rect::Rect(void) {}
inline void Rect::set(short x1, short y1, short x2, short y2) { Point::set(x1, y1); f_04 = x2; f_06 = y2; }
inline Rect::Rect(short x1, short y1, short x2, short y2) { set(x1, y1, x2, y2); }
inline unsigned char Rect::intersects(Rect &r)
{
	return f_00 <= r.f_04 && f_02 <= r.f_06 && f_04 >= r.f_00 && f_06 >= r.f_02;
}

#define REGION_SHIFT	9

// One item of a glob, placed relative to the glob's region.
struct GlobRecord
{
	unsigned char x;		// in 2-unit steps
	unsigned char y;
	unsigned char z;
	unsigned short type;
	unsigned char frame;
};

// A glob egg: the item that stands in for a glob until it is expanded.
class GlobEgg : public Item
{
public:
	GlobEgg(Item item) { referent = item.referent; }
};

// The map region holding (x, y).
inline void worldToRegion(unsigned short x, unsigned short y, int &rx, int &ry)
{
	rx = x >> REGION_SHIFT;
	ry = y >> REGION_SHIFT;
}

// The world corner of region (rx, ry).
inline void regionOrigin(int rx, int ry, unsigned &x, unsigned &y)
{
	x = rx << REGION_SHIFT;
	y = ry << REGION_SHIFT;
}

GlobFile *GlobExpander::globFile = 0;
GlobExpander *globExpander = 0;
unsigned GlobExpander::startHandle;
unsigned GlobExpander::globsIn[MAX_GLOBS_IN];

GlobFile::GlobFile(void) :
	FlexFile(fileSpec(0, StaticDir, globFileName, 0), (OpenMode)0, 3072)
{
}

GlobExpander::GlobExpander(void) :
	CacheHandler((CacheNodeType)2)
{
	startHandle = CacheHandler::reserve(MAX_GLOBS_IN);
	CacheNodePtr::attach(this, (CacheNodeType)2);
	memset(globsIn, 0xff, sizeof(globsIn));
	globFile = new GlobFile;
	if (globFile == 0)
		outOfMemory(__FILE__, 201);	// __LINE__
}

GlobExpander::~GlobExpander(void)
{
	if (globFile)
	{
		delete globFile;
		globFile = 0;
	}
}

long GlobExpander::getSizeFromDisk(unsigned short glob)
{
	Index index;
	globFile->getIndex(glob, index);
	return index.size;
}

// The glob's data in the cache, read from disk if it is not there yet.
void *GlobExpander::get(unsigned short glob)
{
	unsigned handle = FREE_HANDLE;
	unsigned long node = 0;
	int i;

	for (i = 0; i < MAX_GLOBS_IN; i++)
	{
		if (globsIn[i] == glob)
		{
			handle = startHandle + i;
			break;
		}
	}
	if (handle != FREE_HANDLE)
	{
		node = handles[handle];
		CacheNodePtr(node - NODE_HEADER).makeMru();
	}
	else
	{
		for (i = 0; i < MAX_GLOBS_IN; i++)
		{
			if (globsIn[i] == 0xffff)
			{
				handle = startHandle + i;
				globsIn[i] = glob;
				break;
			}
		}
		if (i >= MAX_GLOBS_IN)
		{
			handle = CacheNodePtr::deallocateLru((CacheNodeType)2);
			globsIn[handle - startHandle] = glob;
		}
		long size = getSizeFromDisk(glob);
		if (size < 2)
		{
			// An empty glob still holds its item count.
			size = 2;
			node = CacheNodePtr::allocate(size, handle, (CacheNodeType)2);
			*(int *)CacheNodePtr::getPtr(node) = 0;
		}
		else
		{
			node = CacheNodePtr::allocate(size, handle, (CacheNodeType)2);
			globFile->readRecord(glob, CacheNodePtr::getPtr(node));
		}
		if (node == 0)
			halt(__FILE__, 293);	// __LINE__
	}
	return CacheNodePtr::getPtr(node);
}

void GlobExpander::cacheOut(unsigned long, unsigned handle)
{
	globsIn[handle - startHandle] = 0xffff;
}

void GlobExpander::expand(Item item, unsigned char now)
{
	GlobEgg egg(item.referent);
	int globNum = egg.getGlobNum();
	expand(globNum, ItemData::xArray[egg.referent], ItemData::yArray[egg.referent],
		ItemData::zArray[egg.referent], now);
}

// Creates the glob's items around (x, y, z). Unless asked to do it now, a
// glob far from the screen is only queued on its region.

int GlobExpander::expand(unsigned short globNum, unsigned short x, unsigned short y, unsigned char z,
	unsigned char now)
{
	OneItem *items;
	GlobRecord *rec;
	OneItem *p;
	Item item;
	int i;
	int count;
	int rx;
	int ry;
	unsigned x0;
	unsigned y0;
	char *data;

	data = (char *)get(globNum);
	count = *(int *)data;
	items = new OneItem[count];
	p = items;
	data = (char *)get(globNum);
	worldToRegion(x, y, rx, ry);
	regionOrigin(rx, ry, x0, y0);
	x0++;
	y0++;
	if (count > ItemCache::freeCount)
		halt(__FILE__, 351);	// __LINE__
	rec = (GlobRecord *)(data + 2);
	for (i = 0; i < count; i++)
	{
		p->x = rec->x * 2 + x0;
		p->y = rec->y * 2 + y0;
		p->z = rec->z + z;
		p->type = rec->type;
		if (p->type >= NUM_TYPES)
			halt(__FILE__, 364);	// __LINE__
		p->frame = rec->frame;
		p->flags = 0x82;
		p->quality = 0;
		p->npcNum = 0;
		p++;
		rec++;
	}

	if (!now)
	{
		int sx;
		int sy;
		Rect area(0, 0, 0, 0);
		Rect screen(0, 0, 0, 0);
		sx = ShadowSquareLC;
		sy = ShadowSquareTC;
		screen = Rect(sx, sy, sx + 383, sy + 319);
		sx = (x0 >> 2) - (y0 >> 2);
		sy = (x0 >> 3) + (y0 >> 3);
		area = Rect(sx - 240, sy - 250, sx + 239, sy + 127);
		if (area.intersects(screen))
			now = TRUE;
	}
	if (now)
	{
		p = items;
		for (i = 0; i < count; i++)
		{
			if (item.create(p->type, p->frame))
			{
				item.pop(p->x, p->y, p->z);
				item.orStatus(0x82);
				p++;
			}
		}
	}
	else
		ItemCache::prependToRegion(rx, ry, (char *)items, count);
	delete items;
	return count;
}

void GlobExpander::preload(int globNum)
{
	GlobRecord *rec;
	int i;
	int count;
	char *data;

	data = (char *)get(globNum);
	count = *(int *)data;
	for (i = 0; i < count; i++)
	{
		data = (char *)get(globNum);
		rec = (GlobRecord *)(data + 2) + i;
		ShapePreloader::add(rec->type, rec->frame);
	}
}

void GlobExpander::flushGlob(unsigned short globNum)
{
	int i;

	for (i = 0; i < MAX_GLOBS_IN; i++)
	{
		if (globsIn[i] == globNum)
		{
			CacheNodePtr(handles[i + startHandle] - NODE_HEADER).free();
			globsIn[i] = 0xffff;
			return;
		}
	}
}

int GlobExpander::countItems(int globNum)
{
	int *data = (int *)get(globNum);
	return *data;
}

void initGlobExpander(void)
{
	globExpander = new GlobExpander;
	if (globExpander == 0)
		outOfMemory(__FILE__, 471);	// __LINE__
}

void uninitGlobExpander(void)
{
	if (globExpander)
		delete globExpander;
}
