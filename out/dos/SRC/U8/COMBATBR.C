// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: COMBATBR.C

#include "KERNEL.H"
#include "NPC.H"
#include "ITEMFIND.H"
#include "PATHFIND.H"
#include "ANIM.H"
#include "ANIMPRIM.H"
#include "WAIT.H"
#include "STATREST.H"
#include "LOITER.H"
#include "GRAVITY.H"
#include "ASTREAM.H"
#include "ANIMCACH.H"
#include "PRIORITY.H"
#include "DIST.H"

#include "COMBATBR.H"

// One monster type's 13-byte record in DataTable::data.
struct MonsterRecord
{
	unsigned char pad_00[8];
	unsigned char flags;
	unsigned damageType;
	unsigned defenseType;
};

#define monsterRecord	((MonsterRecord *)DataTable::data)
#define DEFENSE_UNDEAD	0x10

TypeChanger::TypeChanger(unsigned short item, unsigned short type)
{
	Kernel::setIdString(pid, "TypeChanger");
	setProcessType((ProcessType)0x237);
	setRef(item);
	newType = type;
}

void TypeChanger::process(void)
{
	Npc npc(ref);

	npc.setType(newType);
	npc.setFrame(0);
	npc.setDir(0);
	pop(0);
}

// Undead and type 96 see through feigned death.
unsigned char isFeignable(unsigned short type)
{
	int monster = DataTable::getMonsterRecord(type);

	if (monster && (monsterRecord[monster].defenseType & DEFENSE_UNDEAD))
		return 1;
	if (type == 96)
		return 1;
	return 0;
}

WaitForVictim::WaitForVictim(unsigned short item, short range)
{
	Kernel::setIdString(pid, "WaitForVictim");
	setProcessType((ProcessType)0x21e);
	intFlags = 1;
	victimFound = 0;
	setRef(item);
	this->range = range;
	Wait *wait = new Wait(rndRange(25, 45) * 30, ref);
	wait->notifyOfDeath(pid);
	waitPid = wait->pid;
	wait->start();
}

void WaitForVictim::process(void)
{
	CombatBrain *brain = (CombatBrain *)Kernel::findValidProcess(ref, (ProcessType)0xf2);

	if (brain->hasTarget() || victimFound)
	{
		Npc npc(ref);
		npc.clrDead();
		if (!victimFound)
			Kernel::getProcess(waitPid)->fail(0);
		pop(0);
	}
	else
		brain->selectTarget(range);
}

void WaitForVictim::interruptHandler(unsigned long, unsigned long)
{
	victimFound = 1;
}

CombatBrain::CombatBrain(unsigned short item, unsigned short arg)
{
	Kernel::setIdString(pid, "CombatBrain");
	setProcessType((ProcessType)0xf2);
	intFlags = 1;
	setRef(item);
	f_34 = 0;
	f_35 = 0;
	f_36 = 1;
	shiftWaitPid = 0;
	Npc npc(item);
	changeling = npc.getType() == 357;
	target = arg;
	fixedTarget = 0;
}

int CombatBrain::getTarget(void)
{
	Npc npc(target);
	Npc t(target);

	if (t.isValid() && !t.isDead() && t.isInFastArea() && (!t.isFeignDeath() || !isFeignable(npc.getType())))
		return target;
	return 0;
}

void CombatBrain::setTarget(unsigned short t)
{
	f_36 = 0;
	if (fixedTarget == 0)
		lockTarget(t);
	target = t;
}

void CombatBrain::lockTarget(unsigned short t)
{
	fixedTarget = t;
}

unsigned char CombatBrain::selectTarget(short range)
{
	AreaItemFinder finder;
	Referent self = ref;
	Npc npc(self);
	unsigned char enemies = npc.getEnemyAlignment() >> 4;

	finder.init(npc.getX() - range, npc.getY() - range, npc.getX() + range, npc.getY() + range, "$", 1);
	while (finder.found())
	{
		if (finder.isNpc())
		{
			Npc cand(finder.referent);
			if (!cand.isDead() && (!cand.isFeignDeath() || !isFeignable(npc.getType())) && cand.referent != self &&
				((enemies & cand.getAlignment()) || (npc.getType() == 509 && cand.getType() == 708)) &&
				fixedTarget != cand.referent)
			{
				setTarget(cand.referent);
				return 1;
			}
		}
		finder.findNext();
	}
	return 0;
}

unsigned char CombatBrain::canHitTarget(void)
{
	Npc npc(ref);
	WorldPoint loc = npc.getLoc();
	AnimResult res;
	SwingWeapon *swing = new SwingWeapon(ref, npc.getDir(), &loc, target);

	swing->notifyOfDeath(pid);
	swing->process();
	res = result;
	if (res.hitTarget)
		return 1;
	if (npc.getType() == 411 && OneIn(10) && (unsigned)npc.getNumTypes(413, 1024) < 3)
		return 1;
	return 0;
}

void CombatBrain::attackTarget(void)
{
	if (isFacingTarget())
	{
		Npc npc(ref);
		SwingWeapon *swing = new SwingWeapon(ref, npc.getDir());
		int dex = npc.getDex();
		Wait *wait;

		if (dex >= 25)
			wait = 0;
		else
		{
			dex = (25 - dex) * 3;
			wait = new Wait(dex, ref);
		}
		f_36 = 0;
		swing->then(this);
		if (wait)
		{
			wait->then(this);
			wait->start();
		}
	}
	else
		faceTarget();
}

void CombatBrain::gotoTarget(void)
{
	PathFinder *path = new PathFinder(ref, target, 12, 0);

	path->then(this);
	path->notifyOfDeath(pid);
	f_36 = 0;
}

unsigned char CombatBrain::isFacingTarget(void)
{
	Npc npc(ref);
	Npc t(target);
	char dir = npc.getDirToCoords(t.getX(), t.getY());

	if (npc.getDir() == dir)
		return 1;
	return 0;
}

unsigned char CombatBrain::faceTarget(void)
{
	Npc npc(ref);

	f_36 = 0;
	return turnToFace(ref, npc.getDir(), npc.getDirToItem(target), 1);
}

// Changelings (type 357) shift between a hidden form and a fighting one.
void CombatBrain::process(void)
{
	Npc npc(ref);
	unsigned distance;
	unsigned char farAway = 0;

	if (npc.isBusy())
	{
		int n = Kernel::getNumProcesses(ref, (ProcessType)0xf0);
		AnimStreamHeader header;
		if (n == 1)
		{
			AnimPrimitive *anim = (AnimPrimitive *)Kernel::getLinearExecutor(ref, (ProcessType)0xf0);
			header = AnimCache::getAnimStreamHeader(npc.getType(), anim->anim);
			if (header.flag40)
			{
				anim->standAfter = 0;
				return;
			}
		}
	}
	else
	{
		distance = dist(avatar.getX(), avatar.getY(), npc.getX(), npc.getY());
		if (distance >= 0x800)
			farAway = 1;
		if (changeling && npc.getType() == 1 && shiftWaitPid == 0)
		{
			Kernel::killProcessNonSpecifiedPTypes(ref, PriorityClass(0xe0), 0xf2, PT_END);
			npc.setType(357);
			npc.setFrame(0);
			Climb *shift = new Climb(ref, 6, 0);
			shift->then(this);
			return;
		}
		if (hasTarget() && (npc.getType() != 357 || (!OneIn(8) && !farAway)))
		{
			int monster = DataTable::getMonsterRecord(npc.getType());
			if (monster && (monsterRecord[monster].flags & 1) && OneIn(10))
			{
				wander(0);
				return;
			}
			if (canHitTarget())
			{
				attackTarget();
				return;
			}
			Npc t(target);
			if (!(t.getStatus() & INVISIBLE) || npc.getType() == 411 || npc.getType() == 413)
				gotoTarget();
			else
				wander(1);
			return;
		}
		if (npc.getType() == 357)
		{
			Kernel::killProcessNonSpecifiedPTypes(ref, PriorityClass(0xe0), 0xf2, PT_END);
			if (farAway && npc.getMap() != 43)
			{
				Climb *shift1 = new Climb(ref, 3, 0);
				Climb *shift2 = new Climb(ref, 4, 0);
				WaitForVictim *wait = new WaitForVictim(ref, 160);
				shift1->then(wait)->then(shift2)->then(this);
				npc.setDead();
				return;
			}
			Climb *shift = new Climb(ref, 5, 0);
			TypeChanger *changer = new TypeChanger(ref, 1);
			Wait *wait = new Wait(rndRange(5, 25) * 30, ref);
			shift->then(changer)->then(this);
			shiftWaitPid = wait->pid;
			wait->notifyOfDeath(pid);
			wait->start();
			return;
		}
		if (!selectTarget(600))
			wander(1);
	}
}

// Falls back to the fixed target when the current one is gone.
unsigned char CombatBrain::hasTarget(void)
{
	if (target == 0)
	{
		if (fixedTarget == 0)
			return 0;
		target = fixedTarget;
	}
	Npc npc(ref);
	Npc t(target);
	if (t.isDead() || !t.isInFastArea() || (t.isFeignDeath() && isFeignable(npc.getType())))
	{
		if (target == fixedTarget)
		{
			target = 0;
			fixedTarget = 0;
			return 0;
		}
		target = fixedTarget;
		return hasTarget();
	}
	return 1;
}

// Loiters or waits a while, or else fidgets.
void CombatBrain::wander(unsigned char move)
{
	Npc npc(ref);
	unsigned char hasFidget1, hasFidget2;
	int type = npc.getType();

	f_36 = 1;
	hasFidget1 = AnimCache::isAnimInExistence(type, (AnimSet)0x2f);
	hasFidget2 = AnimCache::isAnimInExistence(type, (AnimSet)0x30);
	if (move && ((hasFidget1 == 0 && hasFidget2 == 0) || OneIn(2)))
	{
		if (OneIn(2))
		{
			(new Loiter(ref, 1))->then(this);
			return;
		}
		Wait *wait = new Wait(rndRange(2, 3) * 30, ref);
		wait->then(this);
		wait->start();
		return;
	}
	if (hasFidget1 && (hasFidget2 == 0 || OneIn(2)))
		npc.doAnim((AnimSet)0x2f, 8, 5, 0);
	else if (hasFidget2)
		npc.doAnim((AnimSet)0x30, 8, 5, 0);
}

void CombatBrain::interruptHandler(unsigned long event, unsigned long data)
{
	if (!Kernel::restarting)
	{
		Npc npc(ref);
		if (!npc.isOffMap())
		{
			switch (event)
			{
			case 1:
				if (shiftWaitPid == data)
				{
					shiftWaitPid = 0;
					return;
				}
				Process *proc = Kernel::getProcess(data);
				if (proc->processType == 0x204 && hasTarget() && (dependencyFailed() || !canHitTarget()) &&
					npc.getType() != 413)
					wander(0);
				break;
			}
		}
	}
}
