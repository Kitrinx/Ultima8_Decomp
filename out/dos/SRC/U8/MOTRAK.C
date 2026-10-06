// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r- -y
// name: MOTRAK.C

#include <stdlib.h>
#include "ITEM.H"
#include "ITEMDATA.H"
#include "ITEMCACH.H"
#include "ITEMFIND.H"
#include "TYPE.H"
#include "COMBIN.H"
#include "COLBUF.H"
#include "REFSTORE.H"
#include "MOTRAK.H"

// Usecode events queued when one item runs into another.
#define HIT_EVENT		((ProcessType)0x20b)
#define GOT_HIT_EVENT	((ProcessType)0x20c)

#define MAX_Z			250

// Status bit of an item drawn mirrored, which swaps its x and y size.
#define ITEM_FLIPPED	0x0020

// A shape's size in world units, as CollisionBuffer measures it.
inline void footprint(TypeFlag &type, int &xd, int &yd, int &zd)
{
	xd = type.xSize * 32;
	yd = type.ySize * 32;
	zd = type.zSize * 8;
	if (zd == 0 && !type.fixed)
		zd = 1;
}

MotionTracker::MotionTracker(void)
{
	collisions = 0;
	item = 0;

	targetFirst = FALSE;
	targetHit = FALSE;
}

MotionTracker::~MotionTracker(void)
{
	initCollisionList();
}

void MotionTracker::init(void)
{
	initCollisionList();

	targetFirst = FALSE;
	targetHit = FALSE;
}

void MotionTracker::initCollisionList(void)
{
	CollisionNode *node;

	while (collisions)
	{
		node = collisions;
		collisions = collisions->next;
		delete node;
	}
}

unsigned char MotionTracker::can_create(unsigned short type, const WorldPoint &p, unsigned char flipped)
{
	TypeFlag shape = GlobalTypes.typeFlags[type];
	footprint(shape, xd, yd, zd);
	if (flipped)
	{
		int t = xd;
		xd = yd;
		yd = t;
	}

	volumeMask = CollisionBuffer::volumeFlags(shape);

	int hit = CollisionBuffer::scanArea(p.x - xd + 1, p.y - yd + 1, p.x, p.y, p.z, p.z + zd - 1);

	return (volumeMask & hit) == 0;
}

void MotionTracker::collision(unsigned short other, short x, short y, short z, unsigned char blocking)
{
	if (blocking)
	{
		if (abs(x) < abs(hitDx) || abs(y) < abs(hitDy) || abs(z) < abs(hitDz))
		{
			if (target == other)
			{
				targetHit = TRUE;
				if (!hitItem)
					targetFirst = TRUE;
			}
			else
				targetFirst = FALSE;

			hitItem = other;
			hitDx = x;
			hitDy = y;
			hitDz = z;
		}
	}

	CollisionNode *node = new CollisionNode;
	if (node)
	{
		node->next = collisions;
		collisions = node;
		node->item = other;
		node->dx = x;
		node->dy = y;
		node->dz = z;
	}
}

unsigned char MotionTracker::isClear(unsigned short type, const WorldPoint &from, short x, short y, short z, unsigned char flags, unsigned short mover, short volume, unsigned short goal)
{
	int left, top;
	unsigned right, bottom;
	int edge;

	MotionTrackerInternalFlags info;
	info.clear = TRUE;
	info.needSupport = !(flags & MOVE_NO_SUPPORT);
	info.badSurface = FALSE;
	info.supported = FALSE;
	info.damaging = FALSE;

	TypeFlag shape = GlobalTypes.typeFlags[type];
	if (type == 0)
	{
		shape.xSize = shape.ySize = shape.zSize = 0;
		shape.fixed = 0;
		shape.solid = 1;
		shape.damaging = shape.roof = 0;
	}

	item = mover;
	target = goal;
	offMap = FALSE;
	floorGap = 0;

	footprint(shape, xd, yd, zd);
	if (mover && (CollisionItem(mover).getStatus() & ITEM_FLIPPED))
	{
		int t = xd;
		xd = yd;
		yd = t;
	}

	if (volume)
		volumeMask = volume;
	else
		volumeMask = CollisionBuffer::volumeFlags(shape) & ~COLLIDE_ANY;

	if (shape.damaging)
		info.damaging = TRUE;

	surfaceMask = COLLIDE_SEA | COLLIDE_LAND;
	blocked = 0;
	if (flags & MOVE_ON_LAND)
		surfaceMask &= ~COLLIDE_LAND;
	if (flags & MOVE_ON_SEA)
		surfaceMask &= ~COLLIDE_SEA;
	if (flags & MOVE_STEP_DOWN)
		floorGap = 9;

	point = WorldPoint(x, y, z);

	dx = hitDx = info.dx = x - from.x;
	dy = hitDy = info.dy = y - from.y;
	dz = hitDz = info.dz = z - from.z;
	hitItem = 0;

	if (from.z > MAX_Z)
		return FALSE;

	if (x < 0)
	{
		point.x = info.dx < 0 ? 0 : 0x7fff;
		blocked |= BLOCKED_X;
		offMap = TRUE;
		info.clear = FALSE;
	}
	if (y < 0)
	{
		point.y = info.dy < 0 ? 0 : 0x7fff;
		blocked |= BLOCKED_Y;
		offMap = TRUE;
		info.clear = FALSE;
	}
	if (z < 0 || z > MAX_Z)
	{
		point.z = z < 0 ? 0 : MAX_Z;
		blocked |= BLOCKED_Z;
		info.clear = FALSE;
	}

	// Both ends of the move must lie in the loaded part of the map.
	unsigned short lx, ly;
	if (!ItemCache::coordToLoadedCoord(from.x - xd + 1, from.y - yd + 1, lx, ly) ||
		!ItemCache::coordToLoadedCoord(from.x, from.y - yd + 1, lx, ly) ||
		!ItemCache::coordToLoadedCoord(from.x - xd + 1, from.y, lx, ly) ||
		!ItemCache::coordToLoadedCoord(from.x, from.y, lx, ly))
	{
		point = from;
		return FALSE;
	}
	if (!ItemCache::coordToLoadedCoord(x - xd + 1, y - yd + 1, lx, ly) ||
		!ItemCache::coordToLoadedCoord(x, y - yd + 1, lx, ly) ||
		!ItemCache::coordToLoadedCoord(x - xd + 1, y, lx, ly) ||
		!ItemCache::coordToLoadedCoord(x, y, lx, ly))
	{
		point = from;
		blocked |= BLOCKED_X | BLOCKED_Y;
		offMap = TRUE;
		return FALSE;
	}

	// Items are anchored at their far corner, so search up to 480 past the move.
	left = from.x - xd;
	top = from.y - yd;
	right = from.x + 480;
	bottom = from.y + 480;

	if ((edge = x - xd) < left)
		left = edge;
	if ((edge = y - yd) < top)
		top = edge;
	if ((edge = x + 480) > right)
		right = edge;
	if ((edge = y + 480) > bottom)
		bottom = edge;

	if (left < 0) left = 0;
	if (top < 0) top = 0;
	if ((int)right < 0) right = 0x7fff;
	if ((int)bottom < 0) bottom = 0x7fff;

	info.to.x = x;
	info.to.y = y;
	info.to.z = z;
	info.from = from;

	for (AreaItemFinder finder(left, top, right, bottom); finder.found();
		finder.findNext())
	{
		if (finder.referent != mover)
			check(finder.referent, info);
	}

	if (hitItem)
	{
		point.x = hitDx + from.x;
		point.y = hitDy + from.y;
		point.z = hitDz + from.z;
		if (point.z > MAX_Z)
			point.z = hitDz + from.z < 0 ? 0 : MAX_Z;
	}

	return info.clear && (info.needSupport ? info.supported && !info.badSurface : TRUE);
}

unsigned char MotionTracker::tryMove(unsigned short mover, short x, short y, short z, unsigned char flags, unsigned short goal)
{
	CollisionItem it(mover);
	item = mover;
	target = goal;

	if (flags == 0)
		flags = 1;

	return isClear(ItemData::typeArray[it.referent], it.getLoc(), x, y, z, flags, mover, 0, target);
}

unsigned char MotionTracker::tryMove(unsigned short type, const WorldPoint &from, const WorldPoint &to, unsigned char flags, unsigned short mover, unsigned short goal)
{
	item = mover;
	target = goal;

	if (flags == 0)
		flags = 1;

	return isClear(type, from, to, flags, mover, 0, target);
}

void MotionTracker::makeItSo(void)
{
	int force;
	int most;
	CollisionNode *node;

	if (item)
	{
		if (hitItem)
		{
			int ax = abs(dx >> 2);
			int ay = abs(dy >> 2);
			int az = abs(dz);

			most = ax > ay ? ax : ay;
			most = most > az ? most : az;
			force = (ax + ay + az + most) >> 1;
		}

		Item(item).move(point.x, point.y, point.z);

		int count = 0;
		if (hitItem)
			count += 2;

		for (node = collisions; node; node = node->next)
			if (node->item != hitItem && (abs(node->dx) <= abs(hitDx) || abs(node->dy) <= abs(hitDy) || abs(node->dz) <= abs(hitDz)))
				count += 2;

		RefStorage storage(count);

		if (hitItem)
		{
			int family = GlobalTypes.typeFlags[CollisionItem(item).getType()].family;
			// Two stacks of the same thing merge instead of colliding.
			if ((family == QUAN_FAMILY || family == REAGENT_FAMILY) &&
				CollisionItem(hitItem).getType() == CollisionItem(item).getType())
			{
				((Combinable &)Item(item)).combine(hitItem);
			}
			else
			{
				storage.set(new StorageData(&storage, HIT_EVENT, item, hitItem, force));
				storage.set(new StorageData(&storage, GOT_HIT_EVENT, hitItem, item, force));
			}
		}

		for (node = collisions; node; node = node->next)
		{
			if (hitItem != node->item &&
				(abs(node->dx) <= abs(hitDx) || abs(node->dy) <= abs(hitDy) || abs(node->dz) <= abs(hitDz)))
			{
				storage.set(new StorageData(&storage, HIT_EVENT, item, node->item, 0));
				storage.set(new StorageData(&storage, GOT_HIT_EVENT, node->item, item, 0));
			}
		}

		item = 0;
		if (count)
			storage.execute();
	}
}

unsigned char SuperMotionTracker::tryMove(unsigned short mover, short x, short y, short z, unsigned char flags, unsigned short goal)
{
	CollisionItem it(mover);
	return tryMove(ItemData::typeArray[it.referent], it.getLoc(), WorldPoint(x, y, z), flags, mover, goal);
}

unsigned char SuperMotionTracker::tryMove(unsigned short type, const WorldPoint &from, const WorldPoint &to, unsigned char flags, unsigned short mover, unsigned short goal)
{
	stepZ = 0;
	item = mover;
	target = goal;

	if (flags == 0)
		flags = 1;

	if (to.z == from.z)
	{
		if (MotionTracker::tryMove(type, from, to, flags | MOVE_STEP_DOWN, mover, target))
		{
			stepZ = -floorGap;
			if (floorGap && isClear(type, to, to.x, to.y, to.z + stepZ, flags, mover, 0, 0))
				point.z = to.z + stepZ;
			return TRUE;
		}

		if (flags & MOVE_STEP_UP)
		{
			if (target && targetFirst)
				return FALSE;

			// Try stepping up 8, then 9.
			WorldPoint step;
			step.x = from.x;
			step.y = from.y;
			step.z = from.z + 8;
			if (MotionTracker::tryMove(type, from, step, MOVE_NO_SUPPORT, mover, target))
			{
				if (isClear(type, step, to.x, to.y, to.z + 8, flags | MOVE_STEP_DOWN, mover, 0, 0))
				{
					stepZ = 8 - floorGap;
					point.z = to.z + stepZ;
					return TRUE;
				}
				stepZ = 0;
			}

			step.z = from.z + 9;
			if (MotionTracker::tryMove(type, from, step, MOVE_NO_SUPPORT, mover, target))
			{
				if (isClear(type, step, to.x, to.y, to.z + 9, flags | MOVE_STEP_DOWN, mover, 0, 0))
				{
					stepZ = 9;
					point.z = to.z + stepZ;
					return TRUE;
				}
				stepZ = 0;
				return FALSE;
			}
		}
	}
	else
		return MotionTracker::tryMove(type, from, to, flags, mover, target);

	return FALSE;
}

char canExistAt(unsigned short type, unsigned short x, unsigned short y, unsigned short z, unsigned char flags, unsigned short mover, short volume)
{
	MotionTracker tracker;
	WorldPoint from, to;

	from.set(x, y, z);
	to = from;

	if (tracker.can_create(type, to, 0))
		return tracker.isClear(type, from, to, flags, mover, volume, 0);

	return 0;
}
