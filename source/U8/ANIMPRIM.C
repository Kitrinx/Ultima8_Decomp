// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: ANIMPRIM.C

#include "NPC.H"
#include "TYPE.H"
#include "KERNEL.H"
#include "ERROR.H"
#include "PRISM.H"
#include "ITEMFIND.H"
#include "MOTRAK.H"
#include "COMBATBR.H"
#include "ANIMCALL.H"
#include "STATREST.H"
#include "SCRATCHM.H"
#include "DSFXMAN.H"
#include "WAIT.H"
#include "GRAVITY.H"
#include "ANIMCACH.H"
#include "ANODE.H"
#include "ASTREAM.H"
#include "ANIMPRIM.H"

// One monster type's 13-byte record in DataTable::data.
struct MonsterRecord
{
	unsigned char pad_00[9];
	unsigned damageType;
	unsigned defenseType;
};

#define monsterRecord	((MonsterRecord *)DataTable::data)

// Damage that lands whether or not the weapon reaches.
#define DAMAGE_ANYWHERE	0x200

// Builds a test move of the given kind; only walking, running and stepping are tried.
#define NEW_MOVE(move, d, point) \
	if ((move) == ANIM_WALK) \
		test = new Walk(ref, d, point); \
	else if ((move) == ANIM_RUN) \
		test = new Run(ref, d, point); \
	else \
		test = new CarefulStep(ref, d, point)

AnimPrimitive::AnimPrimitive(unsigned short npcRef, AnimSet a, char d, WorldPoint *point, unsigned short x, unsigned short targetRef)
{
	Kernel::setIdString(pid, "AnimPrimitive");
	setRef(npcRef);
	setProcessType((ProcessType)0xf0);
	setLinearProcess();
	anim = a;
	dest = point;
	dir = d;
	started = 0;
	stopMoving = 0;
	f_3d = x;
	target = targetRef;
	collided = 0;
	result.clear();
	blocked = 0;
	standAfter = 1;
	trySideways = 1;
	failNext = 0;
	intFlags |= 1;
	if (dest)
		legalStore();
	Npc npc(npcRef);
	AnimStreamHeader header;
	header = AnimCache::getAnimStreamHeader(npc.getType(), anim);
	frames = header.frames;
}

// The Avatar, walking or running into something, first tries slower moves, then
// sidestepping a little to either side, before giving up.
void AnimPrimitive::initialize(void)
{
	Npc npc(ref);
	AnimStreamHeader header;

	started = 1;
	if (dir == 8)
		dir = npc.getDir();
	Npc::npcData[npc.referent].flags2F_5 = 0;
	if (!dest && (anim == ANIM_WALK || anim == ANIM_RUN || anim == ANIM_STEP) &&
		npc.isAvatar() && !npc.isPathfinding() && trySideways)
	{
		int x = npc.getX();
		int y = npc.getY();
		unsigned char z = npc.getZ();
		int j;
		int i;
		WorldPoint to;
		WorldPoint start;
		Process *waiter = new ResultProcess;
		char offsets[6] = {16, 16, 32, 32, 48, 48};
		SuperMotionTracker tracker;
		AnimResult res;
		char side;
		Boolean done = 0;
		AnimPrimitive *test;
		int first;
		int moves[3] = {ANIM_RUN, ANIM_WALK, ANIM_STEP};
		int move;

		start.set(x, y, z);
		if (anim == ANIM_RUN)
			first = 0;
		else if (anim == ANIM_WALK)
			first = 1;
		else
			first = 2;
		for (i = first; i < 3 && !done; i++)
		{
			move = moves[i];
			to = start;
			NEW_MOVE(move, dir, &to);
			test->then(waiter);
			test->process();
			res = AnimResult(waiter->result);
			if (res.done)
				done = 1;
			if (!done && move == ANIM_STEP)
			{
				char left = (dir - 1) & 7;
				char right = (dir + 1) & 7;
				for (j = 0; j < 2; j++)
				{
					to = start;
					test = new CarefulStep(ref, j ? left : right, &to);
					test->then(waiter);
					test->process();
					res = AnimResult(waiter->result);
					if (res.done)
					{
						dir = j ? left : right;
						done = 1;
						break;
					}
				}
			}
			if (!done)
			{
				for (j = 0; j < 4; j++)
				{
					if ((int)offsets[j] >= 0) x += 0;	// no-op that still compiles to a test
					if (j & 1)
						side = (dir + 2) & 7;
					else
						side = (dir + 6) & 7;
					to.set(x + x8add[side] * offsets[j], y + y8add[side] * offsets[j], z);
					tracker.init();
					if (tracker.tryMove(1, start, to, 1, ref, 0))
					{
						NEW_MOVE(move, dir, &to);
						test->then(waiter);
						test->process();
						res = AnimResult(waiter->result);
						if (res.done)
						{
							to.set(x + x8add[side] * offsets[j], y + y8add[side] * offsets[j], z);
							tracker.makeItSo();
							done = 1;
							break;
						}
					}
				}
			}
			if (!done && move != ANIM_STEP)
			{
				char left = (dir - 1) & 7;
				char right = (dir + 1) & 7;
				for (j = 0; j < 2; j++)
				{
					to = start;
					NEW_MOVE(move, j ? left : right, &to);
					test->then(waiter);
					test->process();
					res = AnimResult(waiter->result);
					if (res.done)
					{
						dir = j ? left : right;
						done = 1;
						break;
					}
				}
			}
			if (!done)
			{
				for (j = 4; j < 6; j++)
				{
					if ((int)offsets[j] >= 0) x += 0;	// no-op that still compiles to a test
					if (j & 1)
						side = (dir + 2) & 7;
					else
						side = (dir + 6) & 7;
					to.set(x + x8add[side] * offsets[j], y + y8add[side] * offsets[j], z);
					tracker.init();
					if (tracker.tryMove(1, start, to, 1, ref, 0))
					{
						NEW_MOVE(move, dir, &to);
						test->then(waiter);
						test->process();
						res = AnimResult(waiter->result);
						if (res.done)
						{
							to.set(x + x8add[side] * offsets[j], y + y8add[side] * offsets[j], z);
							tracker.makeItSo();
							done = 1;
							break;
						}
					}
				}
			}
		}
		if (i == 3 && (Boolean)Npc::npcData[npc.referent].flags1B_4)
		{
			to = start;
			NEW_MOVE(anim, dir, &to);
			test->then(waiter);
			test->process();
			res = AnimResult(waiter->result);
			if (!res.fell)
				anim = ANIM_STAND;
		}
		else
			anim = (AnimSet)move;
		if (!waiter->isPhantom())
			waiter->pop(0);
	}
	if (anim != ANIM_STAND)
		Npc::npcData[npc.referent].flags1B_4 = 0;
	header = AnimCache::getAnimStreamHeader(npc.getType(), anim);
	frames = header.frames;
	figureStartingPhase();
	npc.setLastAnimSet(anim);
	npc.setDir(dir);
}

void AnimPrimitive::figureStartingPhase(void)
{
	Npc npc(ref);
	AnimStreamHeader header;

	header = AnimCache::getAnimStreamHeader(npc.getType(), anim);
	if (header.twoStep)
	{
		if (npc.isFirstStep())
		{
			if (header.looping)
				npc.setAnimFrame(frames - 1);
			else
				npc.setAnimFrame(0);
		}
		else
			npc.setAnimFrame(frames >> 1);
	}
	else if (header.looping)
	{
		if (npc.getLastAnimSet() == anim)
			npc.setAnimFrame(1);
		else
			npc.setAnimFrame(0);
	}
	else
		npc.setAnimFrame(0);
}

void AnimPrimitive::process(void)
{
	Npc npc(ref);
	AnimNode node;
	AnimStreamHeader header;
	SuperMotionTracker tracker;
	int type = npc.getType();
	char first;
	Boolean moved;
	int moveFlags;
	unsigned char frame;
	Boolean hitBlocker;
	Boolean clear;
	Referent hit;
	WorldPoint next;
	WorldPoint start;
	WorldPoint here;

	if (!npc.isInFastArea() || (!dest && npc.isFalling()))
		return;
	if (started)
		first = 0;
	else
	{
		first = 1;
		initialize();
		if (isPhantom())
			return;
	}
	if (failNext)
	{
		fail(0);
		return;
	}
	header = AnimCache::getAnimStreamHeader(type, anim);
	if (header.hanging)
		npc.orStatus(0x1000);
	else
		npc.andStatus(~0x1000);
	do
	{
		if (first)
		{
			first = 0;
			frame = Npc::npcData[npc.referent].animFrame;
		}
		else
			frame = Npc::npcData[npc.referent].animFrame + 1;
		if (header.twoStep)
		{
			if (npc.isFirstStep())
			{
				if ((frame == frames >> 1 && frames != 3) || (frame == 2 && frames == 3))
				{
					Npc::npcData[npc.referent].firstStep = 0;
					pop(0);
					return;
				}
				if (frames == frame)
				{
					if (header.looping)
						Npc::npcData[npc.referent].animFrame = 1;
					else
						Npc::npcData[npc.referent].animFrame = 0;
					frame = Npc::npcData[npc.referent].animFrame;
				}
			}
			else if (header.looping && frame == frames - 1)
			{
				AnimPrimitive *next = (AnimPrimitive *)Kernel::getLinearExecutor(ref, processType);
				next = (AnimPrimitive *)Kernel::getNextLinear();
				if (!Kernel::isProcess(next) || next->anim != anim)
				{
					Npc::npcData[npc.referent].firstStep = 1;
					pop(0);
					return;
				}
			}
		}
		if (frames == frame)
		{
			if (frames != 1)
				Npc::npcData[npc.referent].firstStep = 1;
			pop(0);
			return;
		}
		npc.setAnimFrame(frame);
		AnimCache::getNode(type, anim, dir, Npc::npcData[npc.referent].animFrame, &node);
		if (dest)
			here = *dest;
		else
			here = npc.getLoc();
		if (node.flipped)
			npc.setStatus(npc.getStatus() | FLIPPED);
		else
			npc.setStatus(npc.getStatus() & ~FLIPPED);
		if (here.z + node.deltaZ < 0)
			node.deltaZ = -here.z;
		next = getPoint(here, dir, node);
		if (npc.isInCombat())
		{
			Npc enemy(npc.getTarget());
			if (enemy.isValid() && header.hanging)
			{
				if (npc.getZ() > enemy.getZ())
					next.z = next.z - 1;
				else if (npc.getZ() < enemy.getZ())
					next.z = next.z + 1;
			}
		}
		if (blocked == FALSE)
		{
			start = next;
			if (node.onGround)
				moveFlags = 0x11;
			else
				moveFlags = 4;
			hit = 0;
			if (header.unstoppable)
				moved = 1;
			else
				moved = 0;
			tracker.init();
			if (dest)
			{
				if (node.onGround)
				{
					clear = tracker.tryMove(type, *dest, next, moveFlags, ref, f_3d);
					if (!clear && node.onGround && !tracker.hitItem)
					{
						next = start;
						clear = tracker.tryMove(type, *dest, next, 4, ref, f_3d);
						if (next.z > start.z)
							clear = 0;
					}
					if (clear)
						next = tracker.point;
					else
						next = here;
				}
				else
					clear = tracker.isClear(type, *dest, next, moveFlags, ref, 0, f_3d);
				hit = tracker.hitItem;
				*dest = next;
				if (tracker.targetFirst)
					result.reachedTarget = 1;
				if (clear)
					moved = 1;
				else if (tracker.targetFirst && !header.unstoppable)
				{
					blocked = 1;
					continue;
				}
			}
			else if (stopMoving)
				next = here;
			else if (node.onGround)
			{
				clear = npc.superlegal_move(next, 1, moveFlags, f_3d, &hitBlocker, &hit);
				if (isPhantom())
					return;
				if (!clear && node.onGround && !hit)
				{
					next = start;
					clear = npc.superlegal_move(next, 1, 4, f_3d, &hitBlocker, &hit);
					if (isPhantom())
						return;
				}
				if (clear)
				{
					next = npc.getLoc();
					moved = 1;
				}
				else
				{
					next = start;
					if (hitBlocker)
					{
						result.reachedTarget = 1;
						pop(0);
						return;
					}
					if (header.unstoppable)
						stopMoving = 1;
					else if (!hit)
						moved = 1;
					else
					{
						collided = 1;
						collideWithItem(hit);
						if (isPhantom())
							return;
					}
				}
			}
			else
			{
				clear = tracker.isClear(type, here, next, moveFlags, ref, 0, f_3d);
				hit = tracker.hitItem;
				if (clear)
				{
					npc.grab();
					moved = 1;
					tracker.makeItSo();
					if (isPhantom())
						return;
				}
				else
				{
					collided = 1;
					collideWithItem(tracker.hitItem);
					if (isPhantom())
						return;
					if (tracker.targetFirst)
					{
						result.reachedTarget = 1;
						pop(0);
						return;
					}
					next = here;
					if (header.unstoppable)
						stopMoving = 1;
				}
			}
			if (isPhantom())
				halt(__FILE__, 811);	// __LINE__
			if (moved)
			{
				if (!dest)
				{
					npc.setFrame(node.frame);
					if (node.sfx)
						playSFX(node.sfx, 100, npc.referent);
					if (node.callUsecode)
					{
						npc.calledFromAnim();
						if (isPhantom())
							return;
					}
					if (node.special)
					{
						generalAnimCall(ref);
						if (isPhantom())
							return;
					}
				}
				if (!npc.isFlag2F_5())
					weaponCheck(&header, &next, &node);
				if (isPhantom())
					return;
				if (!dest && header.repeat && (avatar.f_48 || anim != ANIM_RUN))
				{
					Wait *wait = new Wait(header.repeat, ref);
					wait->then(this);
					wait->notifyOfDeath(pid);
					wait->start();
				}
				if (node.onGround)
				{
					if (checkGravity(here, next, 0))
						return;
					if (isPhantom())
						return;
				}
			}
			else
			{
				if (dest)
				{
					fail(0);
					return;
				}
				if (!header.hanging)
					checkGravity(here, next, hit);
				failFunction();
				return;
			}
		}
	} while (dest);
}

WorldPoint AnimPrimitive::getPoint(WorldPoint point, char d, AnimNode node)
{
	int step = node.deltaDir * 4;
	WorldPoint p(point.x + x8add[d] * step, point.y + y8add[d] * step, point.z + node.deltaZ);
	return p;
}

void AnimPrimitive::interruptHandler(unsigned long message, unsigned long)
{
	switch (message)
	{
	case 1:
		flags &= ~0x40;
	}
}

// Keeps what a test move may change, so it can be put back.
void AnimPrimitive::legalStore(void)
{
	Npc npc(ref);

	savedFirstStep = Npc::npcData[npc.referent].firstStep;
	savedLastAnimSet = npc.getLastAnimSet();
	savedDir = npc.getDir();
	savedFlipped = (npc.getStatus() & FLIPPED) ? 1 : 0;
	savedFlags2F_5 = Npc::npcData[npc.referent].flags2F_5;
	savedAnimFrame = Npc::npcData[npc.referent].animFrame;
}

void AnimPrimitive::legalRevert(void)
{
	Npc npc(ref);

	if (savedFirstStep)
		Npc::npcData[npc.referent].firstStep = 1;
	else
		Npc::npcData[npc.referent].firstStep = 0;
	npc.setLastAnimSet(savedLastAnimSet);
	npc.setDir(savedDir);
	if (savedFlipped)
		npc.setStatus(npc.getStatus() | FLIPPED);
	else
		npc.setStatus(npc.getStatus() & ~FLIPPED);
	if (savedFlags2F_5)
		Npc::npcData[npc.referent].flags2F_5 = 1;
	else
		Npc::npcData[npc.referent].flags2F_5 = 0;
	npc.setAnimFrame(savedAnimFrame);
}

void AnimPrimitive::pop(long)
{
	AnimSet lastAnim = anim;
	Npc npc(ref);
	Boolean testing = dest ? 1 : 0;

	cleanUp();
	result.done = 1;
	Process::pop(*(long *)&result);
	if (!testing)
	{
		AnimStreamHeader header;
		header = AnimCache::getAnimStreamHeader(npc.getType(), lastAnim);
		if (header.destroyActor)
		{
			npc.destroy();
			return;
		}
		if (header.flag40 && standAfter)
			npc.doAnim(lastAnim, 8, 0, 0);
	}
}

void AnimPrimitive::fail(long)
{
	Process::fail(*(long *)&result);
	if (!Kernel::restarting)
	{
		AnimStreamHeader header;
		Npc npc(ref);
		Npc::npcData[npc.referent].flags1B_4 = 1;
		header = AnimCache::getAnimStreamHeader(npc.getType(), anim);
		cleanUp();
		if (!dest && header.destroyActor)
			npc.destroy();
	}
}

void AnimPrimitive::cleanUp(void)
{
	if (dest)
		legalRevert();
}

void AnimPrimitive::failFunction(void)
{
	AnimStreamHeader header;
	Npc npc(ref);
	int count;

	header = AnimCache::getAnimStreamHeader(npc.getType(), anim);
	npc.setLastAnimSet(61);
	count = Kernel::getNumProcesses(ref, (ProcessType)0xf0);
	if (collided && (header.looping2 || header.flag20))
	{
		char hitDir = dir;
		Kernel::resetRef(ref, (ProcessType)0xf0);
		if (header.looping2)
		{
			if (OneIn(3))
				npc.receiveHit(0, hitDir, 1, 0x80);
			return;
		}
		npc.receiveHit(0, hitDir, 1, 0x80);
		return;
	}
	if (count == 1)
	{
		if (npc.isInCombat())
			npc.doAnim(ANIM_COMBAT_STAND, 8, 1, 0);
		else
			npc.doAnim(ANIM_STAND, 8, 1, 0);
	}
	fail(0);
}

// Hits the first NPC, then the first other item, that the swing reaches.
void AnimPrimitive::weaponCheck(AnimStreamHeader *header, WorldPoint *point, AnimNode *node)
{
	Npc npc(ref);

	if (header->attack && node->attackRange)
	{
		AreaItemFinder finder;
		Prism weapon;
		Prism body;
		TypeFlag flag;
		int pass;
		int xSize;
		int ySize;
		int zSize;
		int x1;
		int y1;
		int x2;
		int y2;
		int x;
		int y;
		unsigned char z;
		char npcDir;
		unsigned damageFlags;
		int monster;

		npcDir = npc.getDir();
		flag = GlobalTypes.getTypeFlag(npc.getType());
		flag.getWorldSize(xSize, ySize, zSize);
		weapon.set(point->x - xSize + 1, point->y - ySize + 1, point->z, point->x, point->y, point->z + zSize - 1);
		weapon.moveRel(x8add[npcDir] * 32 * node->attackRange, y8add[npcDir] * 32 * node->attackRange, 0);
		x1 = weapon.x;
		y1 = weapon.y;
		x2 = weapon.x2;
		y2 = weapon.y2;
		x1 -= 480;
		y1 -= 480;
		x2 += 480;
		y2 += 480;
		for (pass = 0; pass < 2; pass++)
		{
			for (finder.init(x1, y1, x2, y2, "$", 1); finder.found(); finder.findNext())
			{
				if (!finder.isFixed() && (pass == 1 || finder.isNpc()) && ref != finder.referent)
				{
					damageFlags = 0;
					monster = DataTable::getMonsterRecord(npc.getType());
					if (monster)
						damageFlags = monsterRecord[monster].damageType;
					x = finder.getX();
					y = finder.getY();
					z = finder.getZ();
					flag = GlobalTypes.getTypeFlag(finder.Item::getType());
					flag.getWorldSize(xSize, ySize, zSize);
					body.set(x - xSize + 1, y - ySize + 1, z, x, y, z + zSize - 1);
					if (weapon.intersects(&body) || (damageFlags & DAMAGE_ANYWHERE))
					{
						Npc::npcData[npc.referent].flags2F_5 = 1;
						result.hit = 1;
						if (target == finder.referent)
							result.hitTarget = 1;
						Npc victim(finder.referent);
						Boolean isNpc = victim.isNpc();
						Boolean dead = 0;
						Boolean doHit = 0;
						if (isNpc && victim.isDead())
							dead = 1;
						if (pass == 0)
						{
							if (isNpc && !dead)
								doHit = 1;
						}
						else if (!isNpc)
							doHit = 1;
						if (doHit)
						{
							if (dest)
								return;
							int damageType = npc.getDamageType();
							victim.receiveHit(ref, 8, 0, damageType);
							hitAnimCall(ref, victim.referent);
							return;
						}
					}
				}
			}
		}
	}
}

void AnimPrimitive::collideWithItem(unsigned short)
{
}

// Lets an unsupported NPC fall; returns FALSE when it stands on something.
unsigned char AnimPrimitive::checkGravity(WorldPoint from, WorldPoint to, unsigned short hit)
{
	ItemSupport support;
	Npc npc(ref);
	int type = npc.getType();

	if (dest)
		support.figureInfo(type, *dest, 1, ref);
	else if (collided)
		support.figureInfo(type, from, 1, ref);
	else
		support.figureInfo(type, to, 1, ref);
	if (!support.isSupported())
	{
		if (dest)
		{
			result.fell = 1;
			fail(0);
		}
		else
		{
			npc.hurl(to.x - from.x, to.y - from.y, to.z - from.z, 2);
			if (npc.isAvatar())
			{
				GravityTrackerCling *gravity = (GravityTrackerCling *)Kernel::findValidProcess(ref, (ProcessType)0x203);
				if (gravity)
					gravity->setHitItem(hit);
				else
					halt(__FILE__, 1323);	// __LINE__
			}
		}
		return TRUE;
	}
	return FALSE;
}
