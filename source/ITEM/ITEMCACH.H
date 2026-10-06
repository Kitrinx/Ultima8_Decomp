#ifndef ITEMCACH_H
#define ITEMCACH_H

#include "..\GLIB\PROCESS.H"
#include "ITEM.H"
#include "ITEMDATA.H"

class FixedMapFile;
class MapFile;
class WorldPoint;
class OneItem;

// The regions to keep loaded around the camera: a diamond in region
// coordinates, bounded on rx + ry and on rx - ry.
class RegionRectangle
{
public:
	int minSum;
	int maxSum;
	int minDiff;
	int maxDiff;

	RegionRectangle(void) { minSum = 0; maxSum = 0; minDiff = 0; maxDiff = 0; }
	Boolean isNeeded(short, short);
	void calculateNeeded(unsigned short, unsigned short);
};

class Scheduler : public Process
{
public:
	unsigned npcNum;		// next NPC to schedule
	unsigned time;

	Scheduler(unsigned short);
	virtual void process(void);
};

class ItemCache : public ItemData
{
public:
	~ItemCache(void);
	static void prependToRegion(unsigned short, unsigned short, char *, short);
	static void loadRegion(short, short);
	static void dumpRegion(short, short);
	static void dumpMapLevel(unsigned char, unsigned char);
	static void loadMapLevel(short);
	static void saveNpcs(void);
	static void loadNpcs(void);
	static void save(char *);
	static Boolean load(void);
	static void goTo(unsigned short, unsigned short, short);
	static void reloadMapLevel(void);
	static void init(void);
	static int findPrevious(unsigned short);
	static void unlinkFromContainer(unsigned short);
	static void unlinkFromRegion(unsigned short);
	static void unlinkFromFree(unsigned short);
	static void linkToFree(unsigned short);
	static void unlinkFromVoid(unsigned short);
	static void destroy(unsigned short);
	static Boolean coordToLoadedCoord(unsigned short, unsigned short, unsigned short &, unsigned short &);
	static void push(unsigned short);
	static void pop(unsigned short, unsigned short, unsigned char);
	static void pop(unsigned short);
	static void pop(void);
	static void pop(WorldPoint &);
	static void popToEnd(unsigned short);
	static void popToLunch(unsigned char);
	static int popEthereal(void);
	static int create(void);
	static int createNpc(void);
	static int countFreeNodes(void);
	static int countFreeNpcs(void);
	static void verifyFailed(short);
	static void dumpCache(void);
	static int mapIn;			// map level loaded, -1 for none
	static unsigned short (*regionStartHash)[64];
	static unsigned short (*itemCount)[64];
	static MapFile *itemFile;
	static FixedMapFile *fixedItemFile;
	static unsigned char expandingGlobs;
	static RegionRectangle regionsIn;
	static unsigned firstFree;
	static unsigned firstFreeNpc;
	static unsigned lastFree;
	static unsigned lastFreeNpc;
	static int freeCount;
	static unsigned freeNpcCount;
	static unsigned etherealTop;
	static int etherealCount;
};

int getEtherealTop(void);
void initItemCache(void);
void setCantSaveGameFlag(void);

extern char U7FileName[9];
extern char ItemCacheFileName[13];

#endif
