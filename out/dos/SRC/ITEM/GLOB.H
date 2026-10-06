#ifndef GLOB_H
#define GLOB_H

#include "..\POLIB\INC\FLEX.H"
#include "..\XCACHE\HANDLER.H"
#include "TYPE.H"

class Item;

#define MAX_GLOBS_IN	512

class GlobFile : public FlexFile
{
public:
	GlobFile(void);
};

// An item as stored in a glob or the map file.
class OneItem : public WorldPoint
{
public:
	unsigned short type;
	unsigned char frame;
	unsigned short flags;
	unsigned short quality;
	unsigned char npcNum;
	unsigned char mapNum;
	unsigned short next_object;
};

// Loads globs (prefabricated groups of items) into the cache and turns
// them into real items in the map.
class GlobExpander : public CacheHandler
{
public:
	GlobExpander(void);
	~GlobExpander(void);
	virtual void cacheOut(unsigned long, unsigned);
	long getSizeFromDisk(unsigned short);
	void *get(unsigned short);
	void expand(Item, unsigned char);
	int expand(unsigned short, unsigned short, unsigned short, unsigned char, unsigned char);
	void preload(int);
	void flushGlob(unsigned short);
	int countItems(int);
	static GlobFile *globFile;
	static unsigned startHandle;
	static unsigned globsIn[MAX_GLOBS_IN];	// glob loaded in each handle
};

void initGlobExpander(void);
void uninitGlobExpander(void);

extern GlobExpander *globExpander;

#endif
