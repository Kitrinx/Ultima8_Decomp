// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: ..\ITEM\PRELOAD.C

#include <string.h>
#include <mem.h>
#include "CACHE.H"
#include "ITEM.H"
#include "ITEMCACH.H"
#include "GLOB.H"
#include "SHAPHAND.H"
#include "CFILE.H"
#include "FEXIST.H"
#include "FILESPEC.H"
#include "CEXIT.H"
#include "SCRATCHM.H"
#include "STATREST.H"
#include "UPROCESS.H"
#include "TYPE.H"

// A handle on an item found in the map's region lists.
class MapItem : public Item
{
public:
	MapItem(Referent r) { referent = r; }
};

FrameSet *ShapePreloader::type[2];

void FrameSet::init(void)
{
	memset(bits, 0, sizeof(bits));
}

void FrameSet::add(unsigned short frame)
{
	bits[frame >> 3] |= 1 << (frame & 7);
}

char FrameSet::contains(unsigned short frame)
{
	return bits[frame >> 3] & (1 << (frame & 7));
}

char ShapePreloader::init(void)
{
	char ok = TRUE;
	for (int i = 0; i < 2; i++)
	{
		type[i] = new FrameSet[NUM_TYPES / 2];
		if (!type[i])
			ok = FALSE;
	}
	return ok;
}

char ShapePreloader::uninit(void)
{
	char ok = TRUE;
	for (int i = 0; i < 2; i++)
	{
		if (type[i])
			delete type[i];
		type[i] = 0;
	}
	return ok;
}

FrameSet *ShapePreloader::getFrameSet(unsigned short shape)
{
	if (shape < NUM_TYPES)
		return &type[shape >> 10][shape & 0x3ff];
	return 0;
}

void ShapePreloader::add(unsigned short shape, unsigned short frame)
{
	FrameSet *set;
	if ((set = getFrameSet(shape)) != 0)
		set->add(frame);
}

char ShapePreloader::contains(unsigned short shape, unsigned short frame)
{
	FrameSet *set;
	if ((set = getFrameSet(shape)) != 0)
		return set->contains(frame);
	return FALSE;
}

inline Item itemOf(Referent r) { return MapItem(r); }

char ShapePreloader::make(void)
{
	int y, x, glob;
	Referent r;
	Item item;
	if (init())
	{
		for (x = 0; x < 64; x++)
		{
			for (y = 0; y < 64; y++)
			{
				r = ItemCache::regionStartHash[y][x];
				while (r)
				{
					item = itemOf(r);
					add(ItemData::typeArray[item.referent], item.getFrame());
					glob = item.getGlobNum();
					if (GlobalTypes.typeFlags[ItemData::typeArray[item.referent]].family == GLOBEGG_FAMILY && glob)
						globExpander->preload(glob);
					r = ItemData::nextArray[item.referent];
				}
			}
		}
		return TRUE;
	}
	return FALSE;
}

void ShapePreloader::preload(unsigned char)
{
	if (make())
	{
		for (int shape = 0; shape < NUM_TYPES; shape++)
			for (int frame = 0; frame < NUM_FRAMES; frame++)
				if (contains(shape, frame))
					shapeHandler->get(shape, frame);
	}
	uninit();
}
