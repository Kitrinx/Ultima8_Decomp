// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: ANIM.C

#include <conio.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NPC.H"
#include "ITEMDATA.H"
#include "ITEMFIND.H"
#include "TYPE.H"
#include "KERNEL.H"
#include "DISPATCH.H"
#include "TARGGUMP.H"
#include "CAMERA.H"
#include "MOTRAK.H"
#include "PATHFIND.H"
#include "GRAVITY.H"
#include "COMBATBR.H"
#include "PROFILER.H"
#include "HARDWARE.H"
#include "SPANKER.H"
#include "CPROTMEM.H"
#include "GRANTPEA.H"
#include "INTER.H"
#include "STATREST.H"
#include "SHAPHAND.H"
#include "DSFXMAN.H"
#include "WAIT.H"
#include "ANIM.H"

extern "C" void *profilePtr = 0;
extern "C" unsigned long nextProfileSlot = 0;
char targetedJump = 0;		// the next jump aims at avysDropZone
static Profiler *profiler = 0;
WorldPoint avysDropZone;

#define max(a, b)	(((a) > (b)) ? (a) : (b))

void waitForKey(void)
{
	charGen.makeString(0, 188, "Press a key.");
	while (kbhit())
		getch();
	getch();
}

char *getAvatarStatus(char *status)
{
	if (avatar.isImmortal())
		strcpy(status, "immortal");
	else if (avatar.isInvincible())
		strcpy(status, "invincible");
	else
		strcpy(status, "normal");
	return status;
}

unsigned char createItem(unsigned short shape, unsigned short frame, unsigned short x, unsigned short y, unsigned char z)
{
	if (canExistAt(shape, x, y, z, 1, 0, 0))
	{
		Item item;
		if (item.create(shape, frame))
		{
			item.pop(x, y, z);
			item.setStatus(item.getStatus() | 0x80);
			return TRUE;
		}
	}
	return FALSE;
}

void f1key(void)
{
}

void f2key(void)
{
}

// Debug: sets the Avatar's dexterity, strength and intelligence.
void f3key(void)
{
	int value;

	gotoxy(1, 23);
	printf("Enter dex (0-25):  ");
	scanf("%d", &value);
	if (value >= 0 && value <= 25)
		avatar.setDex(value);
	gotoxy(1, 24);
	printf("Enter str (0-25):  ");
	scanf("%d", &value);
	if (value >= 0 && value <= 25)
		avatar.setStr(value);
	gotoxy(1, 25);
	printf("Enter int (0-25):  ");
	scanf("%d", &value);
	if (value >= 0 && value <= 25)
		avatar.setInt(value);
}

// Debug: sets one picked NPC on another.
void f4key(void)
{
	Item attacker;
	Item victim;

	Dispatch(new TargetGump(&attacker, 0));
	Dispatch(new TargetGump(&victim, 0));
	if (attacker.isAvatar())
		return;
	if (attacker.isNpc() && victim.isNpc())
	{
		Npc npc(attacker.referent);
		CombatBrain *brain;
		if (npc.isInCombat())
		{
			brain = (CombatBrain *)Kernel::findValidProcess(npc.referent, (ProcessType)0xf2);
			if (brain)
			{
				brain->setTarget(victim.referent);
				return;
			}
			npc.clrInCombat();
			npc.setInCombat();
			brain = (CombatBrain *)Kernel::findValidProcess(npc.referent, (ProcessType)0xf2);
			brain->setTarget(victim.referent);
			return;
		}
		npc.setInCombat();
		brain = (CombatBrain *)Kernel::findValidProcess(npc.referent, (ProcessType)0xf2);
		brain->setTarget(victim.referent);
	}
}

// Debug: sends a picked NPC to a picked item or place.
void f5key(void)
{
	Item npc;
	Item target;
	WorldPoint point;
	unsigned char picked;

	Dispatch(new TargetGump(&npc, 0));
	Dispatch(new TargetGump(&point, &target, &picked, 0));
	if (npc.isNpc())
	{
		if (target.isNpc())
		{
			if (canGetThere(npc.referent, target.referent, 12))
				new PathFinder(npc.referent, target.referent, 12, 0);
		}
		else if (picked)
		{
			if (canGetThere(npc.referent, target.referent, 12))
				new PathFinder(npc.referent, target.referent, 12, 0);
		}
	}
}

static void toggleInterruptDebug(void)
{
	if (Kernel::interruptDebug)
		Kernel::interruptDebug = 0;
	else
		Kernel::interruptDebug = 1;
	charGen.makeString(0, 176, "Interrupt debugging is now %3s.", Kernel::interruptDebug ? "ON" : "OFF");
	waitForKey();
}

void shiftF8Key(void)
{
}

void ctrlF8Key(void)
{
}

void altF8Key(void)
{
}

// Jumps forward, or climbs onto whatever is in front of the Avatar.
void jumpAvatar(char dir, char type, Point *point)
{
	if (!avatar.isBusy())
	{
		if (avatar.getDir() == dir)
		{
			Boolean climbed = 0;
			if (type == 1 || ((type == 2 || type == 3) && !avatar.getAirWalkEnabled()))
			{
				int height;
				WorldPoint to;
				AnimPrimitive *climb;
				Process *waiter;
				ItemSupport support;

				waiter = new ResultProcess;

				for (height = 2; height <= 9; height++)
				{
					to.set(avatar.getX(), avatar.getY(), avatar.getZ());
					climb = new Climb(avatar.referent, height, 8, &to);
					climb->then(waiter);
					climb->process();
					if (!waiter->dependencyFailed())
					{
						support.figureInfo(avatar.getType(), WorldPoint(to.x, to.y, to.z), 1, avatar.referent);
						if (support.flags & SUPPORTED)
						{
							avatar.doAnim((AnimSet)(height + 17), 8, 0, 0);
							climbed = 1;
							break;
						}
					}
				}
				waiter->pop(0);
				if (climbed)
				{
					AccumulateDexterity(height * 2);
					AccumulateStrength(height);
				}
				else if (type == 1)
					avatar.doAnim(ANIM_JUMP_UP, 8, 0, 0);
				else
				{
					if (point)
					{
						unsigned short x;
						unsigned short y;
						unsigned char z;
						FastItem target;
						unsigned char onItem;

						ScreenToWorldCoords(point->f_00, point->f_02, x, y, z, &target, &onItem);
						x += 32;
						y += 32;
						WorldPoint dest(x, y, z);
						if (!onItem)
							target.referent = 0;
						int dx = abs(dest.x - avatar.getX());
						int dy = abs(dest.y - avatar.getY());
						int dist = max(dx, dy) >> 5;
						Boolean reachable = avatar.getStr() >= dist && target.isValid() &&
							GlobalTypes.getTypeFlag(ItemData::typeArray[target.referent]).land;
						if (!reachable)
						{
							avatar.doAnim(ANIM_LOOK_LEFT, 8, 0, 0);
							avatar.doAnim(ANIM_STAND, 8, 1, 0);
							avatar.doAnim(ANIM_LOOK_RIGHT, 8, 2, 0);
							avatar.doAnim(ANIM_STAND, 8, 3, 0);
						}
						else
						{
							avysDropZone = dest;
							targetedJump = 1;
						}
					}
					avatar.doAnim(ANIM_JUMP, 8, 0, 0);
				}
			}
			else
				avatar.doAnim(ANIM_AIRWALK_JUMP, 8, 0, 0);
			avatar.doAnim(ANIM_STAND, 8, 10000, 0);
		}
		else
			turnToFace(1, avatar.getDir(), dir, 1);
	}
}

// Steps carefully; at an edge the Avatar first balances, and steps on the second try.
void carefulStepAvatar(char dir, char)
{
	AnimPrimitive *anim;

	if (!avatar.isBusy())
	{
		if (avatar.isBalancing() && avatar.getDir() == dir)
		{
			Npc::npcData[avatar.referent].balancing = 0;
			avatar.doAnim(ANIM_STEP, dir, 0, 0);
			anim = (AnimPrimitive *)Kernel::findValidProcess(1, PT_FIRST);
			if (anim)
			{
				anim->trySideways = 0;
				return;
			}
		}
		else
		{
			WorldPoint to;
			Process *waiter;
			AnimPrimitive *test;
			AnimResult res;

			to.set(avatar.getX(), avatar.getY(), avatar.getZ());
			waiter = new ResultProcess;
			test = new CarefulStep(avatar.referent, dir, &to);
			test->then(waiter);
			test->process();
			res = AnimResult(waiter->result);
			if (res.fell)
			{
				Npc::npcData[avatar.referent].balancing = 1;
				if (avatar.doAnim(ANIM_EDGE_BALANCE, dir, 0, 0))
					avatar.doAnim(ANIM_EDGE_BALANCE, dir, 1, 0);
			}
			else
			{
				Npc::npcData[avatar.referent].balancing = 0;
				avatar.doAnim(ANIM_STEP, dir, 0, 0);
			}
			waiter->pop(0);
		}
	}
}

AvatarSwingToggler::AvatarSwingToggler(unsigned short npc)
{
	Kernel::setIdString(pid, "SwingToggler");
	setProcessType((ProcessType)0x21f);
	setRef(npc);
	Npc::npcData[avatar.referent].swinging = 1;
}

void AvatarSwingToggler::pop(long value)
{
	Npc::npcData[avatar.referent].swinging = 0;
	Process::pop(value);
}

void AvatarSwingToggler::fail(long value)
{
	Npc::npcData[avatar.referent].swinging = 0;
	Process::fail(value);
}

// Swings the weapon unless the Avatar is still recovering, or already swung twice.
void avatarSwing(unsigned char second)
{
	if (!avatar.isSwinging())
	{
		Boolean swung = 0;
		if (Kernel::isProcessTypeActive(1, PT_FIRST))
		{
			for (AnimPrimitive *anim = (AnimPrimitive *)Kernel::findValidProcess(1, PT_FIRST); Kernel::isProcess(anim);
				anim = (AnimPrimitive *)Kernel::findNextValidProcess())
			{
				if (anim->anim == ANIM_ATTACK || anim->anim == ANIM_KICK)
				{
					if (swung)
						return;
					swung = 1;
				}
			}
		}
		if (avatar.doAnim(second ? ANIM_ATTACK : ANIM_KICK, 8, 3, 0))
		{
			int dex = avatar.getDex();
			Wait *wait;
			avatar.doAnim(ANIM_COMBAT_STAND, 8, 10000, 0);
			AccumulateStrength(rndRange(1, 2));
			AccumulateDexterity(rndRange(2, 3));
			if (dex >= 25)
				wait = 0;
			else
			{
				dex = 25 - dex;
				wait = new Wait(dex, avatar.referent);
			}
			if (wait)
			{
				Process *toggler = new AvatarSwingToggler(avatar.referent);
				wait->then(toggler);
				wait->start();
			}
		}
	}
}

// Debug help screen.
void ctrlF1Key(void)
{
	char status[60];

	charGen.makeString(0, 92, "Avatar AC:  %-2d  Status:  %-10s  Dead:  %-3s  Spd Limit=%c    \n",
		avatar.getArmorClass(), getAvatarStatus(status), avatar.isDead() ? "Yes" : "No", avatar.f_48 ? 'Y' : 'N');
	charGen.makeString(0, 104, "F1=Anim on/off  CF1=Help      SF1=Monster AF1=Unk debug         ");
	charGen.makeString(0, 116, "F2=Anim display CF2=Debugger  SF2=Weapons AF2=Function Profiler ");
	charGen.makeString(0, 128, "F3=Avatar stats CF3=Info      SF3=Bombs   AF3=Garbage Collect   ");
	charGen.makeString(0, 140, "F4=Combat       CF4=Restart   SF4=Immortl AF4=Avatar restore    ");
	charGen.makeString(0, 152, "F5=Pathfind     CF5=Profiler  SF5=Armor   AF5=Avatar dead       ");
	charGen.makeString(0, 164, "F6=Int Debug    CF6=          SF6=        AF6=                  ");
	charGen.makeString(0, 176, "");
	waitForKey();
}

void ctrlF2Key(void)
{
	asm int 3;
}

void ctrlF3Key(void)
{
	ProcessMonitor *monitor = (ProcessMonitor *)Kernel::findValidProcess(0, (ProcessType)0x225);
	if (monitor->isOnRealTime())
		monitor->turnOff();
	else
		monitor->turnOn();
}

// Debug: ends every fight near the Avatar and restarts the kernel.
void ctrlF4Key(void)
{
	AreaItemFinder finder;

	for (finder.init(avatar.getX() - 0x800u, avatar.getY() - 0x800u, avatar.getX() + 0x800, avatar.getY() + 0x800, "$", 1);
		finder.found(); finder.findNext())
	{
		if (finder.isNpc())
		{
			Npc npc(finder.referent);
			if (npc.isInCombat() && !npc.isAvatar() && !npc.isDead())
				npc.clrInCombat();
		}
	}
	Kernel::restart();
}

void ctrlF5Key(void)
{
	if (!profiler)
	{
		profiler = new Profiler;
		profiler->start();
	}
	else if (profiler->isOnRealTime())
		profiler->stop();
	else
		profiler->start();
	charGen.makeString(0, 176, "Profiling is now %3s.", profiler->isOnRealTime() ? "ON" : "OFF");
	waitForKey();
}

// Debug: creates a monster by number.
void shiftF1Key(void)
{
	int choice;
	unsigned monsters[18] = {142, 411, 119, 120, 83, 96, 214, 509, 413, 357, 574, 807, 806, 707, 76, 808, 824, 377};

	charGen.makeString(0, 140, "0=Ghoul 1=Ghost 2=Skeleton 3=Torax 4=Troll 5=Demon 6=Golem 7=Seeker 8=GHead ");
	charGen.makeString(0, 152, "9=Troodle 10=Guard 11=Spider 12=Rat 13=Peasant 14=Kith 15=InvisiMan");
	charGen.makeString(0, 164, "16=Bug 17=Twister");
	charGen.makeString(0, 176, "Enter no. (99 for none):");
	gotoxy(1, 24);
	printf("-->  ");
	scanf("%d", &choice);
	if (choice >= 0 && choice <= 18)
		createNpc(monsters[choice]);
}

// Debug: drops one of each weapon around the Avatar.
void shiftF2Key(void)
{
	unsigned _ss *weapon;
	int tries;
	unsigned weapons[16] = {540, 541, 542, 420, 419, 418, 417, 815, 822, 816, 821, 820, 819, 818, 817, 0};

	for (weapon = weapons; *weapon; weapon++)
		for (tries = 0; tries < 10; tries++)
			if (createItem(*weapon, 0, avatar.getX() + rndRange(-400, 400), avatar.getY() + rndRange(-400, 400), avatar.getZ()))
				break;
}

// Debug: drops bombs and the like around the Avatar.
void shiftF3Key(void)
{
	int tries;
	unsigned bombs[7] = {579, 0, 579, 4, 750, 0, 0xffff};
	int i;
	int j;

	for (i = 0; bombs[i] != 0xffff; i += 2)
		for (tries = 0; tries < 10; tries++)
			if (createItem(bombs[i], bombs[i + 1], avatar.getX() + rndRange(-300, 300), avatar.getY() + rndRange(-300, 300), avatar.getZ()))
				break;
	unsigned more[5] = {592, 0, 592, 4, 0xffff};
	for (j = 0; more[j] != 0xffff; j += 2)
		for (tries = 0; tries < 10; tries++)
			if (createItem(more[j], more[j + 1], avatar.getX() + rndRange(-300, 300), avatar.getY() + rndRange(-300, 300), avatar.getZ()))
				break;
}

// Debug: cycles the Avatar through immortal, invincible and normal.
void shiftF4Key(void)
{
	char status[60];

	if (avatar.isImmortal())
	{
		avatar.clrImmortal();
		avatar.setInvincible();
	}
	else if (avatar.isInvincible())
		avatar.clrInvincible();
	else
		avatar.setImmortal();
	charGen.makeString(0, 176, "Avatar status:  %s.", getAvatarStatus(status));
	waitForKey();
}

// Debug: drops every second frame of each armor around the Avatar.
void shiftF5Key(void)
{
	int i;
	int tries;
	int frame;
	unsigned shape;
	int frames;
	int width;
	int height;
	unsigned armors[13] = {539, 532, 531, 523, 530, 841, 64, 842, 844, 845, 843, 823, 0xffff};

	for (i = 0; armors[i] != 0xffff; i++)
	{
		shape = armors[i];
		shapeHandler->getInfo(shape, width, height, frames);
		for (frame = 0; frame < frames; frame += 2)
			for (tries = 0; tries < 10; tries++)
				if (createItem(armors[i], frame, avatar.getX() + rndRange(-400, 400), avatar.getY() + rndRange(-400, 400), avatar.getZ()))
					break;
	}
}

void altF1Key(void)
{
	unkDebug = !unkDebug;
}

void altF2Key(void)
{
	Spanky::toggleProfiler();
	charGen.makeString(0, 176, "Function profiling=%s.", Spanky::profiling ? "on" : "off");
	waitForKey();
}

void altF3Key(void)
{
	ProtMemoryManager::garbageCollect(0);
}

void altF4Key(void)
{
	avatar.setHp(avatar.getStr() * 2);
	avatar.setMana(avatar.getInt() * 2);
}

void altF5Key(void)
{
	if (avatar.isDead())
	{
		avatar.clrDead();
		avatar.clrAnimLock();
	}
	else
		avatar.setDead();
	charGen.makeString(0, 176, "Avatar dead=%s.", avatar.isDead() ? "yes" : "no");
	waitForKey();
}

// Clicking both buttons 16 times in the top left corner with Ctrl-Alt-Shift held toggles cheating.
void handleTracker(unsigned short x, unsigned short y, unsigned short buttons)
{
	static int count = 0;
	unsigned char shift;
	unsigned char keys = 0x0e;

	_AH = 2;
	geninterrupt(0x16);
	shift = _AL;
	if ((shift & keys) == keys)
	{
		if (x <= 3 && y <= 3)
		{
			if ((buttons & 3) == 3 && ++count == 16)
			{
				count = 0;
				playSFX(218, 100);
				avatar.canCheat = !avatar.canCheat;
			}
		}
		else if (x == 638 && y == 0)
		{
			count = 0;
			asm int 3;
		}
		else
			count = 0;
	}
	else
		count = 0;
}

// Uses every item of type 592 near the point.
void igniteChaos(unsigned short x, unsigned short y, unsigned char)
{
	AreaItemFinder finder;

	for (finder.init(x - 160, y - 160, x + 160, y + 160, "$", 1); finder.found(); finder.findNext())
		if (finder.Item::getType() == 592)
			finder.use();
}

// Turns an NPC from dir to newDir a step at a time; returns TRUE if it had to turn.
unsigned char turnToFace(unsigned short ref, char dir, char newDir, unsigned char force)
{
	Npc npc(ref);
	Boolean clockwise;
	Boolean combat = npc.isInCombat() ? 1 : 0;
	int d;
	int steps;
	char left = (dir - 1) & 7;
	char right = (dir + 1) & 7;
	int stand = combat ? ANIM_COMBAT_STAND : ANIM_STAND;
	int lastAnim = npc.getLastAnimSet();
	int turn;

	if (newDir != dir && (force || (newDir != left && newDir != right)))
	{
		if (newDir == ((dir + 1) & 7) || newDir == ((dir + 2) & 7) ||
			newDir == ((dir + 3) & 7) || newDir == ((dir + 4) & 7))
			clockwise = 1;
		else
			clockwise = 0;
		steps = 0;
		d = dir;
		do
		{
			steps++;
			d = (d + (clockwise ? 1 : -1)) & 7;
		} while (newDir != d);
		steps++;
		if (lastAnim == ANIM_RUN || lastAnim == ANIM_RUNNING_LEAP)
			npc.doAnim(ANIM_WALK, dir, 10000, 0);
		if (combat)
			turn = stand;
		else if (clockwise)
			turn = ANIM_LOOK_RIGHT;
		else
			turn = ANIM_LOOK_LEFT;
		if (clockwise)
			for (d = 0; d < steps - 1; d++)
				npc.doAnim((AnimSet)turn, (dir + d) & 7, 10000, 0);
		else
			for (d = 0; d < steps - 1; d++)
				npc.doAnim((AnimSet)turn, (dir - d) & 7, 10000, 0);
		if (clockwise)
			dir = (dir + d) & 7;
		else
			dir = (dir - d) & 7;
		if (combat)
			npc.doAnim(ANIM_COMBAT_STAND, dir, 10000, 0);
		else
			npc.doAnim(ANIM_STAND, dir, 10000, 0);
		return TRUE;
	}
	if (force)
	{
		Boolean needStand = 0;
		if (combat)
		{
			if (lastAnim != ANIM_COMBAT_STAND)
				needStand = 1;
		}
		else if (lastAnim != ANIM_STAND)
			needStand = 1;
		if (needStand)
		{
			if (combat)
				npc.doAnim(ANIM_COMBAT_STAND, dir, 10000, 0);
			else
				npc.doAnim(ANIM_STAND, dir, 10000, 0);
			return TRUE;
		}
	}
	return FALSE;
}
