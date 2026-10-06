#ifndef MOTRAK_H
#define MOTRAK_H

#include "ITEM.H"
#include "TYPE.H"

// Mover flags for isClear and tryMove.
#define MOVE_ON_LAND		0x01	// may stand on land
#define MOVE_ON_SEA			0x02	// may stand on water
#define MOVE_NO_SUPPORT		0x04	// needs nothing underneath
#define MOVE_STEP_DOWN		0x08	// may drop up to 9 onto a floor
#define MOVE_STEP_UP		0x10	// may climb a ledge up to 9

// Directions a move was cut short in.
#define BLOCKED_X			0x01
#define BLOCKED_Y			0x02
#define BLOCKED_Z			0x04

// One item hit along a move, and how far the mover got before touching it.
class CollisionNode
{
public:
	Referent item;
	short dx;
	short dy;
	short dz;
	CollisionNode *next;
};

// Scratch state shared by isClear and check while sweeping one move.
class MotionTrackerInternalFlags
{
public:
	short dx;
	short dy;
	short dz;
	WorldPoint from;
	WorldPoint to;
	Boolean needSupport;
	Boolean badSurface;
	Boolean supported;
	Boolean damaging;
	Boolean clear;
};

class MotionTracker
{
public:
	unsigned volumeMask;		// collision flags that block the mover
	unsigned surfaceMask;		// surfaces the mover cannot stand on
	int xd;
	int yd;
	int zd;
	unsigned char pad_0a[6];
	short hitDx;				// how far the mover gets before the first block
	short hitDy;
	short hitDz;
	short dx;					// the whole move asked for
	short dy;
	short dz;
	int floorGap;				// drop to the nearest floor below
	WorldPoint point;			// where the mover ends up
	unsigned blocked;
	Referent item;
	Referent hitItem;
	CollisionNode *collisions;
	Referent target;
	Boolean targetFirst;
	Boolean targetHit;
	Boolean offMap;

	MotionTracker(void);
	~MotionTracker(void);
	void init(void);
	void initCollisionList(void);
	unsigned char can_create(unsigned short, const WorldPoint &, unsigned char);
	void collision(unsigned short, short, short, short, unsigned char);
	unsigned char isClear(unsigned short, const WorldPoint &, short, short, short, unsigned char, unsigned short, short, unsigned short);
	unsigned char isClear(unsigned short type, const WorldPoint &from, const WorldPoint &to, unsigned char flags, unsigned short mover, short volume, unsigned short goal)
	{
		return isClear(type, from, to.x, to.y, to.z, flags, mover, volume, goal);
	}
	unsigned char tryMove(unsigned short, short, short, short, unsigned char, unsigned short);
	unsigned char tryMove(unsigned short, const WorldPoint &, const WorldPoint &, unsigned char, unsigned short, unsigned short);
	unsigned char tryMove(unsigned short mover, const WorldPoint &to, unsigned char flags, unsigned short goal)
	{
		return tryMove(mover, to.x, to.y, to.z, flags, goal);
	}
	void makeItSo(void);
	void check(unsigned short, MotionTrackerInternalFlags &);
};

// A tracker that can also step up onto ledges up to 9 high.
class SuperMotionTracker : public MotionTracker
{
public:
	int stepZ;

	unsigned char tryMove(unsigned short, short, short, short, unsigned char, unsigned short);
	unsigned char tryMove(unsigned short, const WorldPoint &, const WorldPoint &, unsigned char, unsigned short, unsigned short);
	unsigned char tryMove(unsigned short mover, const WorldPoint &to, unsigned char flags, unsigned short goal)
	{
		return tryMove(mover, to.x, to.y, to.z, flags, goal);
	}
};

char canExistAt(unsigned short, unsigned short, unsigned short, unsigned short, unsigned char, unsigned short, short);

#endif
