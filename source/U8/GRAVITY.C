// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: GRAVITY.C

#include <dos.h>
#include <stdlib.h>
#include "NPC.H"
#include "ITEMDATA.H"
#include "ITEMFIND.H"
#include "TYPE.H"
#include "KERNEL.H"
#include "PRIORITY.H"
#include "ERROR.H"
#include "COLBUF.H"
#include "MOTRAK.H"
#include "STATREST.H"
#include "SCRATCHM.H"
#include "DSFXMAN.H"
#include "GRAVITY.H"

// Inline in the shared headers when this file was built.
inline void killAnimProcesses(unsigned short ref)
{
	PriorityClass all(0x21);
	Kernel::killProcess(ref, PT_FIRST, all);
}

ItemSupport::ItemSupport(unsigned short item, unsigned char strict)
{
	Npc npc(item);
	figureInfo(ItemData::typeArray[npc.referent], npc.getLoc(), strict, item);
}

// Looks at what lies under each corner of an item of the given type at the point;
// strict counts only things whose top is exactly at the point's height.
void ItemSupport::figureInfo(unsigned short type, WorldPoint &point, unsigned char strict, unsigned short self)
{
	int x;
	int y;
	int z;
	int xSize;
	int ySize;
	int zSize;
	int top;
	int x1;
	int y1;
	int x2;
	int y2;
	int midX1;
	int midX2;
	int midY1;
	int midY2;
	TypeFlag flag;
	unsigned volume;
	unsigned blockers;
	Boolean touching = 0;
	Item item;

	x = point.x;
	y = point.y;
	flag = GlobalTypes.typeFlags[type];
	flag.getWorldSize(xSize, ySize, zSize);
	z = point.z;
	if (z <= 0)
	{
		flags = 0x1f;
		return;
	}
	flags = 0;
	volume = CollisionBuffer::volumeFlags(flag);
	x1 = x - xSize + 1;
	y1 = y - ySize + 1;
	x2 = x;
	y2 = y;
	midX1 = (unsigned)(x1 + x2) >> 1;
	midX2 = (unsigned)(x1 + x2 + 1) >> 1;
	midY1 = (unsigned)(y1 + y2) >> 1;
	midY2 = (unsigned)(y1 + y2 + 1) >> 1;
	{
		AreaItemFinder finder(x1, y1, x2 + 480, y2 + 480);
		for (; finder.found(); finder.findNext())
		{
			item.set(finder.referent);
			x = ItemData::xArray[item.referent];
			y = ItemData::yArray[item.referent];
			if (item.referent == self)
				continue;
			if (x < x1 || y < y1 || ItemData::zArray[item.referent] > z)
				continue;
			flag = GlobalTypes.typeFlags[ItemData::typeArray[item.referent]];
			flag.getWorldSize(xSize, ySize, zSize);
			top = ItemData::zArray[item.referent] + zSize;
			if (x - xSize + 1 > x2 || y - ySize + 1 > y2)
				continue;
			if (top != z && (strict == FALSE || top > z || z - 9 > top))
				continue;
			blockers = CollisionBuffer::volumeFlags(flag) & 0x7fff;
			if (self && !(CollisionBuffer::surfaceFlags(flag) & 4))
				blockers &= ~1;
			if (!(blockers & volume))
				continue;
			if (top == z)
				touching = 1;
			if (x - xSize + 1 <= midX1)
			{
				if (y - ySize + 1 <= midY1)
					flags |= SUPPORT_WEST_NORTH;
				if (y >= midY2)
					flags |= SUPPORT_WEST_SOUTH;
			}
			if (x >= midX2)
			{
				if (y - ySize + 1 <= midY1)
					flags |= SUPPORT_EAST_NORTH;
				if (y >= midY2)
					flags |= SUPPORT_EAST_SOUTH;
			}
		}
	}
	if ((flags & SUPPORT_WEST_NORTH && flags & SUPPORT_EAST_SOUTH) ||
		(flags & SUPPORT_WEST_SOUTH && flags & SUPPORT_EAST_NORTH))
		flags |= SUPPORTED;
	if (strict && !touching)
		flags = 0;
}

GravityTracker::GravityTracker(unsigned short item, short x, short y, short z, short g, short ticks, WorldPoint *point)
{
	Kernel::setIdString(pid, "Gravity");
	setRef(item);
	setProcessType((ProcessType)0x203);
	vx = x;
	vy = y;
	vz = z;
	gravity = g;
	lifetime = ticks;
	dest = point;
	sliding = 0;
	grabbed = 0;
	Item(item).andStatus(~0x400);
	Npc npc(ref);
	if (npc.isNpc())
	{
		if (npc.isFalling())
			halt(__FILE__, 320);	// __LINE__
		Npc::npcData[npc.referent].falling = 1;
		npc.setFallZ(npc.getZ());
	}
}

// A falling NPC lands: hurt by long falls, otherwise it gets up.
GravityTracker::~GravityTracker(void)
{
	Npc npc(ref);
	if (npc.getZ() < 251 && npc.isNpc())
	{
		Npc::npcData[npc.referent].falling = 0;
		if (!isFailed() && !grabbed)
		{
			unsigned char startZ = npc.getFallZ();
			int fall = (startZ - npc.getZ()) / 8;
			if (npc.isDead())
			{
				Process *anim = Kernel::getLinearExecutor(ref, (ProcessType)0xf0);
				if (anim)
					Kernel::checkSynchronization(anim);
			}
			else if (fall >= 10)
			{
				int damage;
				if (fall >= 13)
					damage = npc.getHp();
				else
					damage = (fall - 9) << 1;
				npc.receiveHit(0, npc.getDir(), damage, 0x180);
				playSFX(51, 250);
			}
			else if (npc.getLastAnimSet() != ANIM_DIE)
			{
				killAnimProcesses(npc.referent);
				npc.doAnim(ANIM_LAND, 8, 0, 0);
				if (npc.isInCombat())
					npc.doAnim(ANIM_COMBAT_STAND, 8, 1, 0);
			}
		}
	}
}

void GravityTracker::process(void)
{
	MotionTracker tracker;
	int x;
	int y;
	int z;
	Npc item(ref);
	unsigned blocked;
	Boolean landed = 0;
	Boolean stepped;

	if (item.getZ() >= 251)
	{
		Kernel::killProcess(ref, (ProcessType)6, PriorityClass(0x21));
		return;
	}
	do
	{
		if (ItemData::statusArray[item.referent] & CONTAINED)
		{
			pop(0);
			return;
		}
		if (dest)
		{
			int dist = abs(ItemData::zArray[item.referent] - dest->z) +
				(abs(ItemData::xArray[item.referent] - dest->x) +
				abs(ItemData::yArray[item.referent] - dest->y));
			if (dist < 50)
			{
				pop(0);
				return;
			}
		}
		if (!item.isInFastArea())
		{
			if (dest)
				pop(0);
			return;
		}
		if (vz == 0 || (ItemData::statusArray[item.referent] & 0x4400))
		{
			if (vx || vy)
				item.grab();
			else
			{
				for (SurfaceItemFinder above(item.referent, "$", 1, 1); above.found(); above.findNext())
					above.fall();
			}
		}
		item.andStatus(~0x400);
		x = ItemData::xArray[item.referent] + vx;
		y = ItemData::yArray[item.referent] + vy;
		z = ItemData::zArray[item.referent] + vz;
		vz -= gravity;
		blocked = 0;
		if (move())
			return;
		stepped = 0;
		if (!tracker.tryMove(ref, x, y, z, 4, 0))
		{
			afterMove(tracker.hitItem);
			blocked = tracker.blocked;
			if (blocked)
				item.orStatus(0x400);
			if (blocked & BLOCKED_Z)
			{
				if (ItemData::statusArray[item.referent] & 0x4000)
				{
					tracker.tryMove(ref, x, y, tracker.point.z, 4, 0);
					blocked = tracker.blocked | BLOCKED_Z;
				}
				stepped = 1;
			}
		}
		if (dest)
			item.move(tracker.point);
		else
		{
			tracker.makeItSo();
			if (isPhantom())
				return;
		}
		if (tracker.offMap)
			vx = vy = 0;
		if (blocked)
		{
			int unused;		// gives this block its own stack scope

			if (dest)
			{
				pop(0);
				return;
			}
			if (!sliding)
			{
				if (ItemData::statusArray[item.referent] & 0x4000)
				{
					int r = urandom(20);
					if (r < 4)
					{
						if (vx < 0)
							vx++;
						else if (vx > 0)
							vx--;
						else
							vx = 0;
					}
					else if (r < 8)
					{
						if (vy < 0)
							vy++;
						else if (vy > 0)
							vy--;
						else
							vy = 0;
					}
					else if (r == 8)
						vx++;
					else if (r == 9)
						vx--;
					else if (r == 10)
						vy++;
					else if (r == 11)
						vy--;
					if (!(ItemData::statusArray[item.referent] & 0x400))
						blocked &= ~(BLOCKED_X | BLOCKED_Y);
				}
				if (!(ItemData::statusArray[item.referent] & 0x4000) || (blocked & (BLOCKED_X | BLOCKED_Y)))
				{
					if (vx < 0)
						vx++;
					vx >>= 2;
					if (vy < 0)
						vy++;
					vy >>= 2;
				}
			}
			if (!(blocked & BLOCKED_Z))
			{
				if (vz < 0)
					vz++;
				vz >>= 2;
			}
			if (blocked & BLOCKED_X)
			{
				if (!sliding && item.isNpc())
					vx = 0;
				else
					vx = -vx;
			}
			if (blocked & BLOCKED_Y)
			{
				if (!sliding && item.isNpc())
					vy = 0;
				else
					vy = -vy;
			}
			if (landed)
				sliding = 0;
			if (blocked & BLOCKED_Z)
			{
				vz += gravity;
				landed = 0;
				if (vz <= 0)
				{
					ItemSupport support(ref, 0);
					support = ItemSupport(ref, 0);
					if (support.onlyNorth())
						vy += 4;
					if (support.onlyEast())
						vx -= 4;
					if (support.onlySouth())
						vy -= 4;
					if (support.onlyWest())
						vx += 4;
					landed = support.flags & SUPPORTED;
				}
				if (ItemData::statusArray[item.referent] & 0x4400)
				{
				if (landed)
				{
					if (stepped && item.isNpc())
					{
						pop(0);
						return;
					}
					if (vz < 0)
					{
						vz = -vz;
						vz >>= 2;
					}
					else
					{
						vz >>= 2;
						vz = -vz;
					}
					sliding = 0;
					if (ItemData::statusArray[item.referent] & 0x4000)
					{
						if (vz < gravity)
							vz = 0;
						else
						{
							vx += urandom(vz);
							vx -= urandom(vz);
							vy += urandom(vz);
							vy -= urandom(vz);
						}
						if (abs(vx) < 4 && abs(vy) < 4 && vz < gravity)
						{
							pop(0);
							return;
						}
					}
					else if (vz < gravity)
					{
						pop(0);
						return;
					}
				}
				else
				{
					if (abs(vx) < 16)
					{
						if (vx > 0)
							vx += 4;
						else if (vx < 0)
							vx -= 4;
						else
							urandom(2) ? vx += 4 : vx -= 4;
					}
					if (abs(vy) < 16)
					{
						if (vy > 0)
							vy += 4;
						else if (vy < 0)
							vy -= 4;
						else
							urandom(2) ? vy += 4 : vy -= 4;
					}
					vz = 0;
					sliding = 1;
				}
				}
				else
				{
					pop(0);
					return;
				}
			}
			if (landed && impact())
			{
				pop(0);
				return;
			}
		}
		if (lifetime && --lifetime == 0)
		{
			pop(0);
			return;
		}
	} while (dest);
}

unsigned char GravityTracker::impact(void)
{
	MotionTracker tracker;
	Npc item(ref);

	if (vz * gravity <= 0)
	{
		if (!tracker.tryMove(ref, WorldPoint(ItemData::xArray[item.referent], ItemData::yArray[item.referent], ItemData::zArray[item.referent] + vz), 4, 0))
			return TRUE;
	}
	return FALSE;
}

void GravityTracker::bump(short x, short y, short z)
{
	vx += x;
	vy += y;
	vz += z;
}

void GravityTracker::cancel(void)
{
	Item item(ref);
	MotionTracker tracker;

	tracker.tryMove(item.referent, item.getX(), item.getY(), 0, 4, 0);
	tracker.makeItSo();
	if (isPhantom())
		return;
	pop(0);
}

unsigned char GravityTrackerCling::move(void)
{
	unsigned char buttons;
	Boolean caught = 0;

	_AX = 3;
	geninterrupt(0x33);
	buttons = _BL;
	if (buttons & 2)
	{
		Npc npc(ref);
		WorldPoint to;
		WorldPoint from;
		int x = npc.getX();
		int y = npc.getY();
		unsigned char z = avatar.getZ() & 0xf8;
		char dir = avatar.getDir();
		Process *waiter = new ResultProcess;
		AnimPrimitive *climb;
		int i;

		hitItem = 0;
		for (i = 0; i < 5; i++)
		{
			if (i == 0)
				to.set(x + x8add[dir] * 64, y + y8add[dir] * 64, z);
			else if (i == 1)
				to.set(x + x8add[dir] * 32, y + y8add[dir] * 32, z);
			else if (i == 2)
				to.set(x + x8add[dir] * 32, y + y8add[dir] * 32, z);
			else if (i == 3)
				to.set(x + x8add[dir] * 16, y + y8add[dir] * 16, z);
			else
				to.set(x, y, z);
			from = to;
			climb = new Climb(npc.referent, 5, 8, &to);
			climb->then(waiter);
			climb->process();
			if (!waiter->dependencyFailed())
			{
				ItemSupport support;
				support.figureInfo(npc.getType(), WorldPoint(to.x, to.y, to.z), 1, npc.referent);
				if (support.onlyEast() || support.onlyWest() || support.onlySouth() || support.onlyNorth())
				{
					npc.move(from.x, from.y, from.z);
					grabbed = 1;
					Kernel::resetRef(npc.referent, (ProcessType)0xf0);
					caught = 1;
					pop(0);
					npc.doAnim(ANIM_HANG, 8, 0, 0);
					npc.setLastAnimSet(ANIM_HANG);
					break;
				}
			}
		}
		waiter->pop(0);
	}
	return caught;
}

void GravityTrackerCling::afterMove(unsigned short hit)
{
	Item item(ref);
	if (item.isAvatar())
		hitItem = hit;
}

int Item::hurl(short x, short y, short z, short gravity)
{
	GravityTracker *tracker = (GravityTracker *)Kernel::findValidProcess(referent, (ProcessType)0x203);
	if (tracker)
		tracker->bump(x, y, z);
	else if (isAvatar())
		tracker = new GravityTrackerCling(referent, x, y, z, gravity);
	else
		tracker = new GravityTracker(referent, x, y, z, gravity, 200, 0);
	return tracker->pid;
}

void Item::fall(void)
{
	if (!(getStatus() & 0x1000))
	{
		TypeFlag flag = GlobalTypes.typeFlags[getType()];
		if (!flag.fixed && flag.weight)
			hurl(0, 0, 0, 4);
	}
}
