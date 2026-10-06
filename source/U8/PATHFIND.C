// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: PATHFIND.C

#include "NPC.H"
#include "TYPE.H"
#include "KERNEL.H"
#include "ERROR.H"
#include "DIST.H"
#include "ANIM.H"
#include "GRAVITY.H"
#include "PATHFIND.H"

PathFinder::PathFinder(unsigned short npcRef, unsigned short targetRef, WorldPoint where, unsigned short steps, unsigned char test)
{
	Kernel::setIdString(pid, "PathFinder");
	Npc npc(npcRef);
	Item targetItem(targetRef);
	TypeFlag flag;

	if (npc.isPathfinding())
	{
		Kernel::killProcess(npcRef, (ProcessType)0x204, PriorityClass(0x21));
		traceGet((TraceLevel)50, "R %d, T %d - 2 pf's.\n", npc.referent, npc.getType());
	}
	npc.setPathfinding();
	setRef(npcRef);
	setProcessType((ProcessType)0x204);
	dest = where;
	testOnly = test;
	dir = 8;
	lastDir = 8;
	sweeping = 0;
	maxSteps = steps;
	this->steps = 0;
	target = targetRef;
	arrived = 0;
	targetSolid = 0;
	closeEnough = -1;
	bestDist = dist(npc.getX(), npc.getY(), dest.x, dest.y);
	if (targetItem.isValid())
	{
		flag = GlobalTypes.getTypeFlag(targetItem.getType());
		if (flag.zSize && flag.solid)
			targetSolid = 1;
	}
	if (!targetSolid)
		closeEnough = 64;
}

PathFinder::PathFinder(unsigned short npcRef, unsigned short targetRef, unsigned short steps, unsigned char test)
{
	Kernel::setIdString(pid, "PathFinder");
	Npc npc(npcRef);
	Item targetItem(targetRef);
	TypeFlag flag;

	if (npc.isPathfinding())
	{
		Kernel::killProcess(npcRef, (ProcessType)0x204, PriorityClass(0x21));
		traceGet((TraceLevel)50, "R %d, T %d - 2 pf's.\n", npc.referent, npc.getType());
	}
	npc.setPathfinding();
	setRef(npcRef);
	setProcessType((ProcessType)0x204);
	dest = targetItem.getLoc();
	testOnly = test;
	dir = 8;
	lastDir = 8;
	sweeping = 0;
	maxSteps = steps;
	this->steps = 0;
	target = targetRef;
	arrived = 0;
	targetSolid = 0;
	closeEnough = -1;
	bestDist = dist(npc.getX(), npc.getY(), dest.x, dest.y);
	if (targetItem.isValid())
	{
		flag = GlobalTypes.getTypeFlag(targetItem.getType());
		if (flag.zSize && flag.solid)
			targetSolid = 1;
	}
	if (!targetSolid)
		closeEnough = 64;
}

void PathFinder::process(void)
{
	WorldPoint here;
	Npc npc(ref);
	AnimPrimitive *anim;
	char faceDir;
	Item targetItem(target);

	if (!npc.isInFastArea())
		return;
	if (npc.isBusy())
	{
		int count = Kernel::getNumProcesses(ref, (ProcessType)0xf0);
		AnimStreamHeader header;
		if (count == 1)
		{
			AnimPrimitive *current = (AnimPrimitive *)Kernel::getLinearExecutor(ref, (ProcessType)0xf0);
			header = AnimCache::getAnimStreamHeader(npc.getType(), current->anim);
			if (header.flag40)
				current->standAfter = 0;
		}
		return;
	}
	if (arrived)
	{
		pop(1);
		return;
	}
	if (targetItem.isValid() && targetSolid)
		dest = targetItem.getLoc();
	here.set(npc.getX(), npc.getY(), npc.getZ());
	// A test-only finder walks the whole path in one call.
	goto start;
	for (; testOnly; steps++)
	{
start:
		if (dest.x == here.x && dest.y == here.y)
		{
			if (dest.z == here.z)
			{
				pop(1);
				return;
			}
			fail(0);
			return;
		}
		char oldDir = dir;
		dir = getLegalDir(&here);
		lastDir = oldDir;
		if (dir == 8)
		{
			fail(0);
			return;
		}
		if (dir == -1)
		{
			if (testOnly)
			{
				pop(1);
				return;
			}
			arrived = 1;
		}
		else if (steps == maxSteps)
		{
			fail(0);
			return;
		}
		if (!testOnly)
		{
			if (arrived)
				faceDir = tryDir;
			else
				faceDir = dir;
			turnToFace(ref, npc.getDir(), faceDir, arrived ? 1 : 0);
			if (npc.isInCombat())
				anim = new Advance(ref, faceDir);
			else
				anim = new Walk(ref, faceDir);
			anim->then(this);
		}
	}
}

void PathFinder::end(void)
{
	Npc npc(ref);
	char npcDir;
	unsigned char turned;

	npc.clrPathfinding();
	if (!testOnly && !Kernel::restarting && !npc.isAtZ254() && !npc.isDead())
	{
		npcDir = npc.getDir();
		if (target)
			turned = turnToFace(ref, npcDir, npc.getDirToItem(target), 1);
		else
			turned = turnToFace(ref, npcDir, npc.getDirToCoords(dest.x, dest.y), 1);
		if (!turned)
		{
			if (npc.isInCombat())
				npc.doAnim(ANIM_COMBAT_STAND, 8, 0, 0);
			else
				npc.doAnim(ANIM_STAND, 8, 0, 0);
		}
	}
}

void PathFinder::pop(long value)
{
	end();
	Process::pop(value);
}

void PathFinder::fail(long value)
{
	end();
	Process::fail(value);
}

// Returns the direction of the next step from pt and moves pt there;
// -1 when close enough to the goal, 8 when every direction is blocked.
char PathFinder::getLegalDir(WorldPoint *pt)
{
	int i;
	int d;
	int start;
	Npc npc(ref);
	char goalDir;
	WorldPoint test;
	char sweep[8];
	char order[8];
	AnimPrimitive *anim;
	AnimResult res;

	goalDir = getDirCoordToCoord(pt->x, pt->y, dest.x, dest.y);
	d = dist(pt->x, pt->y, dest.x, dest.y);
	if (bestDist > d)
	{
		bestDist = d;
		test = *pt;
		if (npc.isInCombat())
			anim = new Advance(ref, goalDir, &test, target);
		else
			anim = new Walk(ref, goalDir, &test, target);
		anim->notifyOfDeath(pid);
		anim->process();
		if (!dependencyFailed())
			sweeping = 0;
	}
	start = 0;
	if (sweeping)
	{
		sweep[0] = (dir + (sweepLeft ? -1 : 1) + 4) & 7;
		for (i = 1; i < 8; i++)
			sweep[i] = (sweep[i - 1] + (sweepLeft ? -1 : 1)) & 7;
		if (dir != lastDir)
			start = 1;
	}
	else
	{
		for (i = 0; i < 8; i++)
		{
			char turn[8] = {0, -1, 1, -2, 2, -3, 3, -4};
			order[i] = (goalDir + turn[i]) & 7;
		}
	}
	for (i = start; i < 8; i++)
	{
		if (sweeping)
			tryDir = sweep[i];
		else
			tryDir = order[i];
		test = *pt;
		if (npc.isInCombat())
			anim = new Advance(ref, tryDir, &test, target);
		else
			anim = new Walk(ref, tryDir, &test, target);
		anim->notifyOfDeath(pid);
		anim->process();
		res = AnimResult(result);
		if (targetSolid)
		{
			if (res.reachedTarget)
				return -1;
		}
		else if (dist(dest.x, dest.y, test.x, test.y) <= closeEnough)
			return -1;
		if (!dependencyFailed())
			break;
	}
	if (dependencyFailed())
		return 8;
	if (tryDir == goalDir)
		;
	else if (!sweeping)
	{
		sweeping = 1;
		if (i & 1)
			sweepLeft = 1;
		else
			sweepLeft = 0;
	}
	*pt = test;
	return tryDir;
}

// Runs a test-only PathFinder at once and reports whether it got there.
char canGetThere(unsigned short npcRef, unsigned short targetRef, unsigned short steps)
{
	char ok;
	Process *waiter = new ResultProcess;
	Npc npc(npcRef);
	PathFinder *finder;

	if (npc.isPathfinding())
		halt(__FILE__, 465);	// __LINE__
	finder = new PathFinder(npcRef, targetRef, steps, 1);
	finder->then(waiter);
	finder->process();
	ok = !waiter->dependencyFailed();
	waiter->pop(0);
	return ok;
}

char canGetThere(unsigned short npcRef, unsigned short targetRef, WorldPoint where, unsigned short steps)
{
	char ok;
	Process *waiter = new ResultProcess;
	Npc npc(npcRef);
	PathFinder *finder;

	if (npc.isPathfinding())
		halt(__FILE__, 487);	// __LINE__
	finder = new PathFinder(npcRef, targetRef, where, steps, 1);
	finder->then(waiter);
	finder->process();
	ok = !waiter->dependencyFailed();
	waiter->pop(0);
	return ok;
}
