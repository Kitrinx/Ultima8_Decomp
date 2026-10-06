#ifndef GRAVITY_H
#define GRAVITY_H

#include "..\GLIB\PROCESS.H"
#include "ANIMPRIM.H"
#include "ITEM.H"
#include "TYPE.H"

// ItemSupport::flags: which corners of the footprint rest on something.
#define SUPPORT_EAST_NORTH	0x01
#define SUPPORT_EAST_SOUTH	0x02
#define SUPPORT_WEST_SOUTH	0x04
#define SUPPORT_WEST_NORTH	0x08
#define SUPPORTED			0x10	// two opposite corners: the item stands

// What an item standing at a point would rest on.
class ItemSupport
{
public:
	unsigned char flags;

	ItemSupport(void) {}
	ItemSupport(unsigned short, unsigned char);
	void figureInfo(unsigned short, WorldPoint &, unsigned char, unsigned short);
	unsigned char isSupported(void) { return flags & SUPPORTED; }
	Boolean onlyNorth(void) { return (flags & 9) && !(flags & 6); }
	Boolean onlyEast(void) { return (flags & 3) && !(flags & 0xc); }
	Boolean onlySouth(void) { return (flags & 6) && !(flags & 9); }
	Boolean onlyWest(void) { return (flags & 0xc) && !(flags & 3); }
};

// Moves a thrown or falling item each tick until it comes to rest.
class GravityTracker : public Process
{
public:
	int vx;
	int vy;
	int vz;
	int gravity;
	unsigned char sliding;
	int lifetime;			// ticks left, or 0 for no limit
	unsigned char grabbed;	// caught hold of a ledge
	WorldPoint *dest;		// set when only testing the throw

	GravityTracker(unsigned short, short, short, short, short, short, WorldPoint *);
	virtual void process(void);
	virtual ~GravityTracker(void);
	virtual unsigned char move(void) { return 0; }
	virtual void afterMove(unsigned short) {}
	virtual unsigned char impact(void);
	void bump(short, short, short);
	void cancel(void);
};

// The Avatar's fall: holding the right mouse button grabs a ledge in reach.
class GravityTrackerCling : public GravityTracker
{
public:
	unsigned hitItem;

	GravityTrackerCling(unsigned short npc, short vx, short vy, short vz, short gravity) :
		GravityTracker(npc, vx, vy, vz, gravity, 200, 0) { hitItem = 0; }
	virtual unsigned char move(void);
	virtual void afterMove(unsigned short);
	void setHitItem(unsigned short item) { hitItem = item; }
};

// Climbs onto something 16 to 72 high; height counts in steps of 8 from 1.
class Climb : public AnimPrimitive
{
public:
	Climb(unsigned short npc, int height, char dir, WorldPoint *dest = 0) :
		AnimPrimitive(npc, (AnimSet)(height + 17), dir, dest, 0, 0) {}
};

// Waits on another process, which then leaves its result here.
class ResultProcess : public Process
{
public:
	virtual void process(void) { pop(0); }
};

#endif
