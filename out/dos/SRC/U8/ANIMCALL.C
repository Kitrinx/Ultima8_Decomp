// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: ANIMCALL.C

#include <stdlib.h>
#include "NPC.H"
#include "KERNEL.H"
#include "WAIT.H"
#include "SPRITE.H"
#include "PAL.H"
#include "DSFXMAN.H"
#include "SCRATCHM.H"
#include "URANDOM.H"
#include "DIST.H"
#include "MOTRAK.H"
#include "ITEMFIND.H"
#include "COMBATBR.H"
#include "SHAPHAND.H"
#include "ANIMCALL.H"

// One monster type's 13-byte record in DataTable::data.
struct MonsterRecord
{
	unsigned char wealth;
	unsigned char pad_01[12];
};

#define monsterRecord	((MonsterRecord *)DataTable::data)

// Makes a new item, popping it into a container if one is given.
int createItem(unsigned short type, unsigned short frame, unsigned short container)
{
	Item item;

	if (item.create(type, frame))
	{
		item.setStatus(item.getStatus() | FAST_ONLY);
		if (container)
			item.pop(container);
		return item.referent;
	}
	return 0;
}

// Called by an npc's animation: special monster moves, weapon swings and footsteps.
void generalAnimCall(unsigned short victim)
{
	Npc npc(victim);
	int animSet = npc.getLastAnimSet();
	int type = npc.getType();
	unsigned char handled = 0;

	switch (type)
	{
	case 1:
		if (animSet == 5 || animSet == 6)
		{
			Item weapon;

			weapon = npc.getEquip(5);
			if (weapon.isValid())
				playSFX(OneIn(2) ? 0x51 : 0x52, 100, victim);
			handled = 1;
		}
		break;
	case 413:
		handled = 1;
		if (dist(npc.getX(), npc.getY(), npc.getZ(), avatar.getX(), avatar.getY(), avatar.getZ()) <= 96)
		{
			npc.setDead();
			npc.explode();
		}
		break;
	case 411:
		handled = 1;
		if (npc.getLastAnimSet() == 7)
		{
			if ((unsigned)npc.getNumTypes(413, 1536) < 6)
			{
				int pid;
				CombatBrain *brain;
				Npc ghost(0);
				char dir = npc.getDir();

				if (ghost.create(413, 0))
				{
					ghost.pop(npc.getX() + x8add[dir] * 32, npc.getY() + y8add[dir] * 32, npc.getZ() + 32);
					ghost.setStatus(ghost.getStatus() | FAST_ONLY);
					pid = ghost.setInCombat();
					brain = (CombatBrain *)Kernel::getProcess(pid);
					brain->setTarget(npc.getTarget());
				}
			}
		}
		else if (npc.getMap() != 54 && (unsigned)npc.getNumTypes(142, 2048) < 3)
		{
			unsigned tries;
			int x, y;
			unsigned char z = npc.getZ();

			for (tries = 0; tries < 5; tries++)
			{
				x = npc.getX() + rndRange(-768, 768);
				y = npc.getY() + rndRange(-768, 768);
				if (canExistAt(142, x, y, z, 1, 0, 0))
					break;
			}
			if (tries < 5)
			{
				Npc spawn(0);

				if (spawn.create(142, 268))
				{
					spawn.pop(x, y, z);
					spawn.setStatus(spawn.getStatus() | FAST_ONLY);
					spawn.setTarget(npc.getTarget());
					spawn.doAnim(4, 0, 0, 0);
				}
			}
		}
		break;
	}

	// Footsteps, by the floor under the npc.
	if (handled == FALSE && npc.isNpc() && (victim != 1 || avatar.f_36))
	{
		int sound;
		SurfaceItemFinder floor(victim, "$", 1, 2);
		unsigned char volume;

		if (!floor.isValid())
			return;
		switch ((Type)floor.Item::getType())
		{
		case 3:
		case 4:
		case 9:
		case 11:
		case 92:
		case 94:
			sound = 0x2b;
			break;
		case 161:
		case 162:
		case 163:
		case 164:
			if (animSet == 1)
				sound = 0x91;
			else
				sound = 0x99;
			break;
		case 126:
		case 128:
			sound = 0xcd;
			break;
		default:
			if (animSet == 1)
				sound = 0x90;
			else
				sound = 0x97;
			break;
		}
		if (sound == 0xcd)
			createSprite(475, 0, 7, 7, 1, 1, npc.getX(), npc.getY(), npc.getZ());
		if (animSet == 1)
			volume = 200;
		else
			volume = 128;
		playSFX(sound, volume, npc.referent);
		changePitchSFX(sound, urandom(3000) - 1500);
	}
}

// Called when an npc dies: leaves its remains and treasure.
void deathAnimCall(unsigned short victim)
{
	Npc npc(victim);
	Type type = npc.getType();
	Item item;
	int drops = 0;
	int record;
	unsigned wealth;
	unsigned count;
	unsigned i;
	unsigned j;
	int width, depth, frames;

	if (npc.isFastOnly())
		npc.destroyContents();

	switch (type)
	{
	case 509:
		for (i = 0; i < 5; i++)
		{
			int w, d, f;
			int x, y, z;

			shapeHandler->getInfo(458, w, d, f);
			x = rndRange(-4, 2);
			x *= 32;
			x += npc.getX();
			y = rndRange(-4, 2);
			y *= 32;
			y += npc.getY();
			z = urandom(7);
			z *= 8;
			z += npc.getZ();
			item = createItem(458, urandom(f), 0);
			if (item.isValid())
			{
				item.pop(x, y, z);
				item.hurl(rndRange(-25, 25), rndRange(-25, 25), urandom(10) + 10, 2);
			}
		}
		break;
	case 119:
		{
			Process *p = Kernel::findValidProcess(victim, (ProcessType)0x229);
			if (!p)
				drops = 0x5f;
		}
		break;
	case 83:
		drops = 0x5f;
		break;
	case 214:
		drops = 3;
		break;
	case 357:
		drops = 7;
		break;
	case 574:
		drops = 0x1f;
		break;
	case 411:
		if (npc.getMap() == 54)
		{
			if (item.create(821, 0))
				item.pop(npc.referent);
		}
		drops = 0x43;
		break;
	case 120:
		drops = 0x20;
		break;
	case 187:
	case 191:
	case 192:
	case 200:
	case 315:
	case 316:
		drops = 0xc0;
		break;
	case 269:
		drops = 0x41;
		break;
	case 96:
		drops = 0x102;
		if (victim == 57)
			drops |= 0x80;
		break;
	}

	if (drops == 0)
		return;

	record = DataTable::getMonsterRecord(npc.getType());
	if (record)
		wealth = monsterRecord[record].wealth;
	else
		wealth = 0;

	if ((drops & 1) && OneIn(2))
	{
		item = createItem(143, 0, victim);
		if (item.isValid())
			item.setQuantity(wealth / 5 + urandom(wealth / 4 + 1));
	}
	if ((drops & 2) && OneIn(3))
	{
		i = wealth / 25 + 2;
		if (i > 3)
			i = 3;
		count = urandom(i) + 1;
		for (i = 0; i < count; i++)
			createItem(592, OneIn(2) ? 0 : 4, victim);
	}
	if ((drops & 4) && OneIn(3))
	{
		count = urandom(wealth / 10 + 2);
		for (i = 0; i < count; i++)
			createItem(579, OneIn(2) ? 0 : 4, victim);
	}
	if (drops & 0x10)
	{
		// Pairs of type and percent chance, ending at type 0.
		unsigned table[15] = {
			540, 7, 541, 5, 542, 3, 420, 2, 419, 1, 418, 15, 417, 9, 0
		};

		for (i = 0; table[i]; i += 2)
		{
			if (random(100) < table[i + 1])
			{
				item = createItem(table[i], 0, victim);
				if (item.isValid() && OneIn(2))
					break;
			}
		}
	}
	if (drops & 8)
		;	// no drop for this bit
	if (drops & 0x20)
		createItem(537, 2, victim);
	if (drops & 0x80)
	{
		i = random(9);
		for (j = 0; j < i; j++)
			createItem(398, random(14), victim);
		i = random(5);
		for (j = 0; j < i; j++)
			createItem(398, rndRange(14, 20), victim);
		if (random(100) < 80)
		{
			if (random(100) < 40)
			{
				item = createItem(397, 0, victim);
				if (item.isValid())
				{
					item.setQLo(rndRange(3, 6));
					item.setQHi(rndRange(1, 4));
				}
			}
			else
				createItem(397, 15, victim);
		}
		if (random(100) < 60)
		{
			if (random(100) < 20)
			{
				item = createItem(397, 3, victim);
				if (item.isValid())
				{
					item.setQLo(rndRange(3, 6));
					item.setQHi(rndRange(1, 7));
				}
			}
			else
				createItem(397, 16, victim);
		}
		if (random(100) < 20)
		{
			if (random(100) < 10)
			{
				item = createItem(397, 9, victim);
				if (item.isValid())
				{
					item.setQLo(rndRange(1, 2));
					item.setQHi(rndRange(10, 11));
				}
			}
			else
				createItem(397, 18, victim);
		}
		if (random(100) < 50)
		{
			item = createItem(397, 12, victim);
			if (item.isValid())
			{
				item.setQHi(rndRange(10, 11));
				if (item.getQHi() <= 2)
					item.setQLo(rndRange(3, 6));
				else
					item.setQLo(1);
			}
		}
		else
			createItem(397, 19, victim);
	}
	if (drops & 0x100)
	{
		if (random(100) < 80)
			createItem(398, 18, victim);
		if (random(100) < 70)
			createItem(398, 19, victim);
		for (i = 0; i < 2; i++)
		{
			if (random(100) < 50)
				createItem(398, 20, victim);
		}
	}
	if ((drops & 0x40) && OneIn(2))
	{
		shapeHandler->getInfo(766, width, depth, frames);
		createItem(766, random(frames), victim);
	}
}

ItemCreator::ItemCreator(unsigned short item, unsigned short type, unsigned short frame,
	unsigned short x, unsigned short y, unsigned char z)
{
	setRef(item);
	setProcessType((ProcessType)0x234);
	Kernel::setIdString(pid, "ItemCreator");
	this->x = x;
	this->y = y;
	this->z = z;
	this->type = type;
	this->frame = frame;
}

void ItemCreator::process(void)
{
	Item item;

	item = createItem(type, frame, 0);
	if (item.isValid())
		item.pop(x, y, z);
	pop(item.referent);
}

ItemDestroyer::ItemDestroyer(unsigned short item)
{
	setRef(item);
	setProcessType((ProcessType)0x232);
	Kernel::setIdString(pid, "ItemDestroyer");
}

void ItemDestroyer::process(void)
{
	if (ref)
	{
		Item item(ref);
		item.destroy();
	}
	else
		fail(0);
}

// Takes the item made by the process we waited on.
void ItemDestroyer::interruptHandler(unsigned long event, unsigned long pid)
{
	switch (event)
	{
	case 1:
		if (!ref)
		{
			Process *p = Kernel::getProcess(pid);
			setRef(p->getResult());
		}
		break;
	}
}

FutureAnim::FutureAnim(unsigned short type, unsigned short firstFrame, unsigned short lastFrame,
	unsigned short x, unsigned short y, unsigned char z)
{
	Kernel::setIdString(pid, "FutureAnim");
	started = 0;
	this->type = type;
	this->firstFrame = firstFrame;
	this->lastFrame = lastFrame;
	this->x = x;
	this->y = y;
	this->z = z;
}

// Makes the item, then steps it a frame every two ticks up to lastFrame.
void FutureAnim::process(void)
{
	if (started)
	{
		Item item(ref);

		if (item.getFrame() < lastFrame)
		{
			Wait *wait = new Wait(2, ref);
			item.setFrame(item.getFrame() + 1);
			wait->then(this);
			wait->start();
		}
		else
			pop(ref);
	}
	else
	{
		Item item;

		item = createItem(type, 0, 0);
		if (item.isValid())
		{
			item.pop(x, y, z);
			started = 1;
			setRef(item.referent);
		}
		else
			fail(0);
	}
}

// Sounds and effects when an npc is hit by a weapon.
void hitAnimCall(unsigned short victim, unsigned short attacker)
{
	Npc npc(victim);
	Npc hitter(attacker);
	int type = npc.getType();
	Item weapon;
	int sound;
	int volume = 100;

	switch (type)
	{
	case 1:
		if (avatar.getLastAnimSet() == 0x3a)
			break;
		weapon = npc.getEquip(5);
		if (weapon.isValid())
		{
			sound = 0;
			switch (weapon.getType())
			{
			case 815:
				sound = 0x17;
				break;
			case 816:
				if (hitter.isNpc() && hitter.isDead())
				{
					int thunder[3] = { 0x5b, 0x5e, 0x60 };
					sound = thunder[random(3)];
					LightningBolt();
					volume = 250;
				}
				break;
			case 817:
				{
					char dir = npc.getDir();
					int x = npc.getX();
					x += x8add[dir] * 96;
					int y = npc.getY();
					y += y8add[dir] * 96;
					unsigned char z = npc.getZ();

					if (createSprite(480, 0, 9, 2, x, y, z))
					{
						RoundTimer *timer;
						ItemCreator *creator;
						Wait *wait;
						ItemDestroyer *destroyer1, *destroyer2;
						int sounds[2] = { 0x21, 0x65 };

						sound = sounds[random(2)];
						timer = new RoundTimer(3, 0);
						creator = new ItemCreator(0, 400, 0, x, y, z);
						wait = new Wait(90, 0);
						destroyer1 = new ItemDestroyer(0);
						destroyer2 = new ItemDestroyer(0);
						FutureAnim *anim = new FutureAnim(381, 0, 9, x, y, z);
						wait->intFlags |= 4;
						destroyer1->intFlags |= 1;
						destroyer2->intFlags |= 1;
						creator->notifyOfDeath(destroyer1->pid);
						anim->notifyOfDeath(destroyer2->pid);
						timer->then(creator)->then(wait)->then(destroyer1)->then(anim)->then(destroyer2);
					}
				}
				break;
			}
			if (sound)
				playSFX(sound, volume, victim);
		}
		break;
	case 413:
		npc.setDead();
		npc.explode();
		break;
	}
}
