// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: LOITER.C

#include "NPC.H"
#include "TYPE.H"
#include "ANIMCACH.H"
#include "PATHFIND.H"
#include "WAIT.H"
#include "STATREST.H"
#include "LOITER.H"

// count is the number of walks to take, -1 for no limit.
Loiter::Loiter(unsigned short item, short count)
{
	Kernel::setIdString(pid, "Loiter");
	setRef(item);
	setProcessType((ProcessType)0x205);
	intFlags |= 1;
	this->count = count;
}

// Walks to a random spot near the npc.
void Loiter::process(void)
{
	Npc npc(ref);

	if (count == -1 || count-- != 0)
	{
		WorldPoint dest;
		dest.x = npc.getX() + rndRange(-320, 320);
		dest.y = npc.getY() + rndRange(-320, 320);
		dest.z = npc.getZ();
		PathFinder *path = new PathFinder(ref, 0, dest, 12, 0);
		path->then(this);
		path->notifyOfDeath(pid);
	}
	else
		pop(0);
}

// After each walk: wait a while, or fidget.
void Loiter::interruptHandler(unsigned long event, unsigned long)
{
	Npc npc(ref);
	int pid1, pid2 = 0;
	unsigned char hasFidget1, hasFidget2;
	int type = npc.getType();

	if (!(npc.isOffMap() || npc.isDead()))
	{
		if (Kernel::restarting)
			return;
		switch (event)
		{
		case 1:
			hasFidget1 = AnimCache::isAnimInExistence(type, (AnimSet)0x2f);
			hasFidget2 = AnimCache::isAnimInExistence(type, (AnimSet)0x30);
			if (OneIn(2) || (hasFidget1 == 0 && hasFidget2 == 0))
			{
				Wait *wait = new Wait(rndRange(4, 7) * 30, ref);
				wait->then(this);
				wait->start();
				return;
			}
			if (hasFidget1 || hasFidget2)
			{
				if ((hasFidget1 && OneIn(2)) || hasFidget2 == 0)
				{
					pid1 = npc.doAnim((AnimSet)0x2f, 8, 5, 0);
					if (pid1)
						pid2 = npc.doAnim((AnimSet)0x2f, 8, 5, 0);
				}
				else
				{
					pid1 = npc.doAnim((AnimSet)0x30, 8, 5, 0);
					if (pid1)
						pid2 = npc.doAnim((AnimSet)0x30, 8, 5, 0);
				}
				if (pid1)
				{
					Kernel::getProcess(pid1)->then(this);
					if (pid2)
						Kernel::getProcess(pid2)->then(this);
				}
			}
			break;
		}
	}
}
