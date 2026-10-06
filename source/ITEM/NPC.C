// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r- -y
// name: ..\ITEM\NPC.C

#include <mem.h>
#include "..\UI\NEWGUMP.H"
#include "ANIMCACH.H"
#include "CFILE.H"
#include "ERROR.H"
#include "SCRATCHM.H"
#include "FILESPEC.H"
#include "INIT.H"
#include "ITEMDATA.H"
#include "ITEMCACH.H"
#include "ITEMFIND.H"
#include "NPC.H"
#include "TYPE.H"
#include "COMBATBR.H"
#include "PATHFIND.H"
#include "WAIT.H"
#include "DSFXMAN.H"
#include "MOTRAK.H"
#include "STATREST.H"
#include "YAMM.H"
#include "CAMERA.H"
#include "CONTGUMP.H"
#include "UPROCESS.H"
#include "LOITER.H"
#include "WPNCACHE.H"
#include "STDINT.H"
#include "chargen.H"
#include "U8POINT.H"
#include "FEXIST.H"
#include "MAIN.H"
#include "PAL.H"
#include "PREAMBLE.H"
#include "ANIM.H"
#include "ANIMCLAS.H"
#include "GRAVITY.H"
#include <string.h>

// Inline in the shared headers when this file was built.
inline Index::Index(void) { size = 0; offset = 0; }
inline SharedFile::SharedFile(void) {}
inline FlexFile::FlexFile(void) {}
inline unsigned char BaseFile::is_valid(void) { return handle != -1; }
inline BaseFile::~BaseFile(void) { if (is_valid()) close(); }
inline Point::Point(void) {}

inline Item itemOf(Referent r) { return Item(r); }

// One monster type's 13-byte record in DataTable::data.
struct MonsterRecord
{
	unsigned char minHp;
	unsigned char maxHp;
	unsigned char minDex;
	unsigned char maxDex;
	unsigned char minDamage;
	unsigned char maxDamage;
	unsigned char armorClass;
	unsigned char alignment;
	unsigned char pad_08[3];
	short defenseType;
};

#define monsterRecord	((MonsterRecord *)DataTable::data)
#define MAX_NPCS	256
#define NPC_DATA_SIZE	(MAX_NPCS * sizeof(NpcData))

char *NpcDataFileName = "npcdata.dat";
char *NpcNameFileName = "npcnames.dat";
char *AvatarFileName = "avatar.dat";
NpcDataFile *Npc::npcFile = 0;
unsigned char Npc::namefile[4] = {0, 0, 0, 0};
DeathLoader *theDeathLoader = 0;
NpcData *Npc::npcData;
Avatar avatar;

void initNpcs(void)
{
	Npc::init();
}

void uninitNpcs(void)
{
	Npc::uninit();
}

void Npc::init(void)
{
	npcData = new NpcData[MAX_NPCS];
	if (!npcData)
	{
		halt(__FILE__, 213);	// __LINE__
	}
	npcFile = new NpcDataFile;
	if (!npcFile)
	{
		halt(__FILE__, 219);	// __LINE__
	}

	// The avatar is always item 1.
	ItemData::typeArray[1] = 1;
	ItemData::frameArray[1] = 0;
	ItemData::statusArray[1] = 0x40;
	ItemData::zArray[1] = 0;
	ItemData::qArray[1] = 0;
	ItemData::npcNumArray[1] = 1;

	load();
}

void Npc::uninit(void)
{
	if (npcData)
	{
		delete npcData;
		npcData = 0;
	}
	if (npcFile)
	{
		delete npcFile;
		npcFile = 0;
	}
}

void Npc::save(void)
{
	long length = 0x314d;
	Index index;
	long offset = 0;

	npcFile->init(fileSpec(0, GamedatDir, NpcDataFileName, 0), ReadWrite, 1);
	npcFile->getIndex(0, index);
	npcFile->changeRecordLen(0, index, length);
	npcFile->writeRecord(0, npcData, NPC_DATA_SIZE, offset, 0);
	npcFile->close();
}

void Npc::load(void)
{
	Index index;

	npcFile->init(fileSpec(0, GamedatDir, NpcDataFileName, 0), ReadOnly, 1);
	npcFile->readRecord(0, npcData, NPC_DATA_SIZE, 0);
	npcFile->close();
}

unsigned char Npc::isBusy(void)
{
	return Kernel::isProcessTypeActive(referent, (ProcessType)0xf0);
}

int Npc::doAnim(AnimSet animSet, short dir, short maxAnims, unsigned char stopOthers)
{
	unsigned count;
	AnimPrimitive *anim = 0;
	Referent npc = referent;

	if (isAtZ254())
		return 0;
	if (isAnimLock())
		return 0;
	if (!AnimCache::isAnimInExistence(getType(), animSet))
		return 0;

	// maxAnims limits how many animations may already be queued.
	if (maxAnims == 0)
		count = isBusy();
	else
		count = Kernel::getNumLinearProcesses(npc, (ProcessType)0xf0);
	if (count > maxAnims)
		return 0;

	if (animSet != ANIM_STAND && animSet != ANIM_STEP && animSet != ANIM_EDGE_BALANCE)
		npcData[referent].balancing = 0;
	if (stopOthers)
		Kernel::resetRef(referent, (ProcessType)0xf0);

	switch (animSet)
	{
	case 0:
		anim = new Walk(npc, dir);
		break;
	case 1:
		anim = new Run(npc, dir);
		break;
	case 2:
		anim = new Stand(npc, dir);
		break;
	case 3:
		anim = new VertLeap(npc, dir);
		break;
	case 4:
		anim = new GetUp(npc, dir);
		break;
	case 5:
		anim = new DrawWeapon(npc, dir);
		break;
	case 6:
		anim = new SheathWeapon(npc, dir);
		break;
	case 7:
		anim = new SwingWeapon(npc, dir);
		break;
	case 8:
		anim = new Advance(npc, dir);
		break;
	case 9:
		anim = new Retreat(npc, dir);
		break;
	case 10:
		anim = new RunningLeap(npc, dir);
		break;
	case 12:
		anim = new CarefulStep(npc, dir);
		break;
	case 13:
		anim = new GetHit(npc, dir);
		break;
	case 14:
		anim = new FallDown(npc, dir);
		break;
	case 15:
		anim = new CombatStand(npc, dir);
		break;
	case 16:
		anim = new FallRecovery(npc, dir);
		break;
	case 17:
		if (targetedJump)
		{
			anim = new StandingLeap(npc, dir, &avysDropZone);
			targetedJump = 0;
		}
		else
			anim = new StandingLeap(npc, dir, 0);
		break;
	case 18:
		if (npcData[referent].airWalk)
			anim = new NinjaFlip(npc, dir);
		break;
	case 19:
	case 20:
	case 21:
	case 22:
	case 23:
	case 24:
	case 25:
	case 26:
		anim = new Climb(npc, animSet - 17, dir);
		break;
	case 27:
		anim = new EnterCast(npc, dir);
		break;
	case 28:
		anim = new ExitCast(npc, dir);
		break;
	case 29:
		anim = new Cast1(npc, dir);
		break;
	case 30:
		anim = new Cast2(npc, dir);
		break;
	case 31:
		anim = new Cast3(npc, dir);
		break;
	case 32:
		anim = new LookLeft(npc, dir);
		break;
	case 33:
		anim = new LookRight(npc, dir);
		break;
	case 34:
		anim = new Bow(npc, dir);
		break;
	case 35:
		anim = new Kneel(npc, dir);
		break;
	case 36:
		anim = new LowReach(npc, dir);
		break;
	case 37:
		anim = new MidReach(npc, dir);
		break;
	case 38:
		anim = new HighReach(npc, dir);
		break;
	case 39:
		anim = new SwingWeapon2(npc, dir);
		break;
	case 41:
		anim = new AltitudeFall(npc, dir);
		break;
	case 42:
		anim = new EdgeBalance(npc, dir);
		break;
	case 43:
		anim = new FallRecHurt(npc, dir);
		break;
	case 44:
		anim = new FallRecDmg(npc, dir);
		break;
	case 45:
		anim = new LedgeSwing(npc, dir);
		break;
	case 46:
		anim = new LedgeClimb(npc, dir);
		break;
	case 47:
		anim = new Fidget(npc, dir);
		break;
	case 48:
		anim = new Fidget2(npc, dir);
		break;
	case 49:
		anim = new KneelDown(npc, dir);
		break;
	case 50:
		anim = new KneelUp(npc, dir);
		break;
	case 51:
		anim = new SitDown(npc, dir);
		break;
	case 52:
		anim = new SitUp(npc, dir);
		break;
	case 53:
		anim = new Talk(npc, dir);
		break;
	case 54:
		anim = new GiveItem(npc, dir);
		break;
	case 55:
		anim = new Work(npc, dir);
		break;
	case 56:
		anim = new Drown(npc, dir);
		break;
	case 57:
		anim = new Burn(npc, dir);
		break;
	case 58:
		anim = new Kick(npc, dir);
		break;
	case 59:
		anim = new BlockStart(npc, dir);
		break;
	case 60:
		anim = new BlockEnd(npc, dir);
		break;
	default:
		halt(__FILE__, 641);	// __LINE__
	}

	if (anim)
	{
		if (isAvatar())
			headShaker->animationUsed();
		return anim->pid;
	}
	return 0;
}

int Npc::getNumTypes(Type type, unsigned short range)
{
	AreaItemFinder finder;
	int count = 0;

	finder.init(getX() - range, getY() - range, getX() + range, getY() + range, "$", 1);
	while (finder.found())
	{
		if (finder.Item::getType() == type)
		{
			Npc npc(finder.referent);
			if (!npc.isDead())
				count++;
		}
		finder.findNext();
	}
	return count;
}

void Npc::setTarget(Referent target)
{
	CombatBrain *brain;

	if (isInCombat())
		brain = (CombatBrain *)Kernel::findValidProcess(referent, (ProcessType)0xf2);
	else
		brain = (CombatBrain *)Kernel::getProcess(setInCombat());

	if (brain)
		brain->setTarget(target);
	else
		traceGet((TraceLevel)50, "N %d, T %d no brain target.", referent, getType());
}

int Npc::getTarget(void)
{
	CombatBrain *brain;

	if (Kernel::isProcessTypeActive(referent, (ProcessType)0xf2))
	{
		brain = (CombatBrain *)Kernel::findValidProcess(referent, (ProcessType)0xf2);
		return brain->getTarget();
	}
	return 0;
}

int Npc::areEnemiesNear(void)
{
	AreaItemFinder finder;
	int count = 0;

	finder.init(getX() - 0x800U, getY() - 0x800U, getX() + 0x800U, getY() + 0x800U, "$", 1);
	while (finder.found())
	{
		if (finder.isNpc())
		{
			Npc npc(finder.referent);
			if (npc.isInCombat() && !npc.isDead() && !npc.isWithstandDeath())
				count++;
		}
		finder.findNext();
	}
	return count != 0;
}

Boolean Npc::isInCombat(void)
{
	return npcData[referent].inCombat;
}

void Npc::setAlignment(unsigned char alignment)
{
	npcData[referent].alignment = npcData[referent].alignment & 0xf0;
	npcData[referent].alignment |= alignment & 0x0f;
}

unsigned char Npc::getAlignment(void)
{
	return npcData[referent].alignment & 0x0f;
}

void Npc::setEnemyAlignment(unsigned char alignment)
{
	npcData[referent].alignment = npcData[referent].alignment & 0x0f;
	npcData[referent].alignment |= alignment & 0xf0;
}

unsigned char Npc::getEnemyAlignment(void)
{
	return npcData[referent].alignment & 0xf0;
}

Boolean Npc::isDead(void)
{
	if (referent && referent < MAX_NPCS)
		return npcData[referent].dead;
	return TRUE;
}

void Npc::setDead(void)
{
	npcData[referent].dead = 1;
}

void Npc::clrDead(void)
{
	npcData[referent].dead = 0;
}

int Npc::pathfind(unsigned short x, unsigned short y, unsigned short z, unsigned short range)
{
	PathFinder *finder;

	finder = new PathFinder(referent, 0, WorldPoint(x, y, z), range, 0);
	return finder->pid;
}

int Npc::pathfind(Referent item, unsigned short range)
{
	PathFinder *finder;

	finder = new PathFinder(referent, item, range, 0);
	return finder->pid;
}

int Npc::getNpcSlot(void)
{
	for (int i = 1; i < MAX_NPCS; i++)
	{
		if (!npcData[i].inUse)
		{
			npcData[i].inUse = 1;
			return i;
		}
	}
	return 0;
}

void Npc::freeNpcSlot(void)
{
	npcData[referent].inUse = 0;
}

void Npc::setAirWalkEnabled(Boolean enabled)
{
	npcData[referent].airWalk = enabled;
}

Boolean Npc::getAirWalkEnabled(void)
{
	return npcData[referent].airWalk;
}

Boolean Npc::isImmortal(void)
{
	return npcData[referent].immortal;
}

void Npc::setImmortal(void)
{
	npcData[referent].immortal = 1;
	clrInvincible();
}

void Npc::clrImmortal(void)
{
	npcData[referent].immortal = 0;
}

ClearFeignDeath::ClearFeignDeath(Referent npc)
{
	Kernel::setIdString(pid, "ClearFeignDeath");
	setProcessType((ProcessType)0x243);
	setRef(npc);
}

void ClearFeignDeath::process(void)
{
	Npc npc(ref);
	npc.clrFeignDeath();
	playSFX(0x3b, 100, ref);
	pop(0);
}

Boolean Npc::isFeignDeath(void)
{
	return npcData[referent].feignDeath;
}

void Npc::setFeignDeath(void)
{
	if (!isFeignDeath())
	{
		Wait *wait = new Wait(1800, referent);
		Process *clear = new ClearFeignDeath(referent);
		wait->then(clear);
		wait->start();
		doAnim(ANIM_DIE, 8, 10000, 1);
		doAnim(ANIM_STAND_UP, 8, 10000, 0);
		npcData[referent].feignDeath = 1;
	}
}

void Npc::clrFeignDeath(void)
{
	npcData[referent].feignDeath = 0;
}

Boolean Npc::isStunned(void)
{
	return npcData[referent].stunned;
}

void Npc::setStunned(void)
{
	npcData[referent].stunned = 1;
}

void Npc::clrStunned(void)
{
	npcData[referent].stunned = 0;
}

Boolean Npc::isPoisoned(void)
{
	return npcData[referent].poisoned;
}

void Npc::setPoisoned(void)
{
	npcData[referent].poisoned = 1;
}

void Npc::clrPoisoned(void)
{
	npcData[referent].poisoned = 0;
}

Boolean Npc::isPathfinding(void)
{
	return npcData[referent].pathfinding;
}

void Npc::setPathfinding(void)
{
	npcData[referent].pathfinding = 1;
}

void Npc::clrPathfinding(void)
{
	npcData[referent].pathfinding = 0;
}

Boolean Npc::isInvincible(void)
{
	return npcData[referent].invincible;
}

void Npc::setInvincible(void)
{
	npcData[referent].invincible = 1;
	clrImmortal();
}

void Npc::clrInvincible(void)
{
	npcData[referent].invincible = 0;
}

Boolean Npc::isAscending(void)
{
	return npcData[referent].ascending;
}

void Npc::setAscending(void)
{
	npcData[referent].ascending = 1;
}

void Npc::clrAscending(void)
{
	npcData[referent].ascending = 0;
}

Boolean Npc::isDescending(void)
{
	return npcData[referent].descending;
}

void Npc::setDescending(void)
{
	npcData[referent].descending = 1;
}

void Npc::clrDescending(void)
{
	npcData[referent].descending = 0;
}

Boolean Npc::isAnimLock(void)
{
	return npcData[referent].animLock;
}

void Npc::setAnimLock(void)
{
	npcData[referent].animLock = 1;
}

void Npc::clrAnimLock(void)
{
	npcData[referent].animLock = 0;
}

Boolean Npc::isBlocking(void)
{
	int animSet = getLastAnimSet();
	return animSet == ANIM_BLOCK_START || animSet == ANIM_BLOCK_END;
}

Boolean Npc::isWithstandDeath(void)
{
	return npcData[referent].withstandDeath;
}

void Npc::setWithstandDeath(void)
{
	npcData[referent].withstandDeath = 1;
}

void Npc::clrWithstandDeath(void)
{
	npcData[referent].withstandDeath = 0;
}

Boolean Npc::isEnemy(Referent other)
{
	Npc npc(other);

	unsigned char enemies = getEnemyAlignment() >> 4;
	unsigned char alignment = npc.getAlignment();
	return (enemies & alignment) ? TRUE : FALSE;
}

Boolean Npc::simpleCreate(Type type, Frame frame)
{
	referent = ItemCache::createNpc();
	if (referent)
	{
		ItemData::typeArray[referent] = type;
		ItemData::frameArray[referent] = frame;
		ItemData::npcNumArray[referent] = referent;
		ItemData::mapNumArray[referent] = ItemCache::mapIn;
		setFrameHi(frame >> 8);
	}
	return referent != 0;
}

Boolean Npc::create(Type type, Frame frame)
{
	if (simpleCreate(type, frame))
		insertNpcData();
	return referent != 0;
}

void Npc::insertNpcData(void)
{
	int monster;
	int frame = getFrame();

	memset(&npcData[referent], 0, sizeof(NpcData));
	setFrame(frame);

	monster = DataTable::getMonsterRecord(getType());
	if (!monster)
		traceGet((TraceLevel)50, "Def mon rec for T %d.\n", getType());
	setHp(rndRange(monsterRecord[monster].minHp, monsterRecord[monster].maxHp));
	setDex(rndRange(monsterRecord[monster].minDex, monsterRecord[monster].maxDex));
	setAlignment(monsterRecord[monster].alignment & 0x0f);
	setEnemyAlignment(monsterRecord[monster].alignment & 0xf0);
}

Boolean Npc::legal_create(Type type, Frame frame, WorldPoint &loc)
{
	if (type >= NUM_TYPES)
		return FALSE;
	MotionTracker tracker;
	if (tracker.can_create(type, loc, 0))
	{
		create(type, frame);
		pop(loc.x, loc.y, loc.z);
		return TRUE;
	}
	return FALSE;
}

Boolean Npc::legal_create(Type type, Frame frame, unsigned short x, unsigned short y, unsigned short z)
{
	if (type >= NUM_TYPES)
		return FALSE;
	WorldPoint loc(x, y, z);
	MotionTracker tracker;
	if (tracker.can_create(type, loc, 0))
	{
		create(type, frame);
		pop(loc.x, loc.y, loc.z);
		return TRUE;
	}
	return FALSE;
}

Referent Npc::getEquip(short slot)
{
	Referent *equip = npcData[referent].equip;
	return equip[slot];
}

Boolean Npc::setEquip(short slot, Referent item)
{
	if (!getEquip(slot))
	{
		Item thing(item);
		if (thing.getContainer() != referent)
		{
			thing.push();
			thing.pop(referent);
		}
		Npc(item).setZ(slot);
		thing.orStatus(0x200);
		npcData[referent].equip[slot] = item;
		return TRUE;
	}
	return FALSE;
}

void Npc::unEquip(short slot)
{
	npcData[referent].equip[slot] = 0;
}

void Npc::freeEquip(Referent item)
{
	Referent *equip = npcData[referent].equip;

	for (int i = 0; i < 8; i++)
		if (equip[i] == item)
			equip[i] = 0;
}

int Npc::findEquip(Referent item)
{
	Referent *equip = npcData[referent].equip;

	for (int i = 0; i < 8; i++)
		if (equip[i] == item)
			break;
	return i;
}

void Npc::clearEquip(void)
{
	Referent *equip = npcData[referent].equip;

	for (int i = 0; i < 8; i++)
		equip[i] = 0;
}

void Npc::teleport(unsigned short x, unsigned short y, unsigned char z, unsigned char map)
{
	if (ItemCache::mapIn && ItemData::mapNumArray[referent] == ItemCache::mapIn)
	{
		if (map != ItemCache::mapIn)
			leaveFastArea();
		ItemCache::push(referent);
		ItemCache::popToLunch(0);
	}

	ItemData::xArray[referent] = x;
	ItemData::yArray[referent] = y;
	ItemData::zArray[referent] = z;
	ItemData::mapNumArray[referent] = map;

	if (map == ItemCache::mapIn)
	{
		ItemCache::push(referent);
		ItemCache::pop();
	}

	if (isAvatar() && Camera::centeredOn == 1)
	{
		Camera::move_to(x, y, z, map);
		if (theStatGump)
		{
			((NewGump *)theStatGump)->visibilityStack = 0;
			((NewGump *)theStatGump)->refresh();
		}
	}
}

int Npc::getMap(void)
{
	return ItemData::mapNumArray[referent];
}

int Npc::setActivity(unsigned long activity)
{
	unsigned short pid;

	if (spawnUnk(this, 8, 3, pid, activity))
		return pid;
	cSetActivity((Activity)activity);
	return 0;
}

void Npc::cSetActivity(Activity activity)
{
	Kernel::resetRef(referent, (ProcessType)0xf0);
	if (isInCombat())
		clrInCombat();

	switch (activity)
	{
	case 0:
		new Loiter(referent, -1);
		break;
	case 1:
		setInCombat();
		break;
	case 2:
		doAnim(ANIM_STAND, 8, 0, 0);
		break;
	default:
		halt(__FILE__, 1346);	// __LINE__
	}
}

int Npc::schedule(unsigned long time)
{
	unsigned short pid;

	if (isDead())
		return 0;

	if (!isInFastArea() && isInCombat())
		clrInCombat();
	if (spawnUnk(this, 0x100, 8, pid, time))
		return pid;
	return 0;
}

unsigned char Npc::getDamageAmount(void)
{
	unsigned char damage;

	if (isAvatar())
	{
		int str = getStr();

		if (getLastAnimSet() == ANIM_KICK)
		{
			// A kick: magic boots add to it.
			Item boots = avatar.getEquip(4);
			Boolean magicBoots = FALSE;

			if (boots.isValid() && boots.getType() == 845)
				magicBoots = TRUE;
			damage = urandom((str >> 1) + 1) + 4;
			if (magicBoots)
				damage += 3;
		}
		else
		{
			Item weapon = getEquip(5);

			if (weapon.isValid())
			{
				int modifier = WeaponCache::getDamageModifier(weapon.getType());
				int base = WeaponCache::getBaseDamage(weapon.getType());

				str /= 5;
				modifier = urandom(modifier + 1);
				return str + base + modifier;
			}
			damage = urandom((str >> 1) + 1) + 1;
		}
		return damage;
	}
	else
	{
		int monster = DataTable::getMonsterRecord(getType());
		if (monster)
			damage = rndRange(monsterRecord[monster].minDamage, monsterRecord[monster].maxDamage);
		else
		{
			traceGet((TraceLevel)50, "T %u - no mon rec.\n\r", getType());
			damage = 1;
		}
		return damage;
	}
}

unsigned char Npc::getArmorClass(void)
{
	int i;
	unsigned j;
	int k;
	int monster;
	Referent equip;
	Type type;
	Item armour;
	unsigned index;
	char ac = 0;

	if (isAvatar())
	{
		char slots[5] = {0, 1, 2, 3, 4};
		char *bonuses[12] = {
			ArmorData::shieldBonus, ArmorData::leggingBonus, ArmorData::helmetBonus,
			ArmorData::armourBonus, ArmorData::armGuardsBonus, ArmorData::magArmorBonus,
			ArmorData::magArmorTwoBonus, ArmorData::magShieldBonus, ArmorData::magArmsBonus,
			ArmorData::magLegsBonus, ArmorData::magHelmBonus, ArmorData::clothesBonus
		};
		Type armourTypes[12] = {
			539, 532, 531, 523, 530, 841,
			64, 842, 844, 845, 843, 823
		};
		unsigned char bonus;

		for (i = 0; i < 5; i++)
		{
			equip = getEquip(slots[i]);
			if (equip)
			{
				armour = itemOf(equip);
				type = armour.getType();
				index = (Frame)armour.getFrame() >> 1;
				for (k = 0; k < 12; k++)
				{
					if (armourTypes[k] == type)
						break;
				}
				if (k == 12)
					halt(__FILE__, 1482);	// __LINE__
				for (j = 0; j <= index; j++)
				{
					bonus = bonuses[k][j];
					if (bonus == 0x80)
						halt(__FILE__, 1489);	// __LINE__
				}

				switch (type)
				{
				case 539:
					ac += ArmorData::shieldBonus[index];
					break;
				case 532:
					ac += ArmorData::leggingBonus[index];
					break;
				case 531:
					ac += ArmorData::helmetBonus[index];
					break;
				case 523:
					ac += ArmorData::armourBonus[index];
					break;
				case 530:
					ac += ArmorData::armGuardsBonus[index];
					break;
				case 841:
					ac += ArmorData::magArmorBonus[index];
					break;
				case 64:
					ac += ArmorData::magArmorTwoBonus[index];
					break;
				case 842:
					ac += ArmorData::magShieldBonus[index];
					break;
				case 844:
					ac += ArmorData::magArmsBonus[index];
					break;
				case 845:
					ac += ArmorData::magLegsBonus[index];
					break;
				case 843:
					ac += ArmorData::magHelmBonus[index];
					break;
				case 823:
					ac += ArmorData::clothesBonus[index];
					break;
				}
			}
		}

		armour = itemOf(getEquip(5));
		if (armour.isValid())
			ac += WeaponCache::getArmorClassBonus(armour.getType());
		if (ac < 0)
			ac = 0;
		return ac;
	}

	monster = DataTable::getMonsterRecord(getType());
	if (monster)
		return monsterRecord[monster].armorClass;
	return 0;
}

char Npc::getAttackingDex(void)
{
	Item weapon = getEquip(5);
	int dex = getDex();

	if (weapon.isValid())
		dex += WeaponCache::getDexAttackBonus(weapon.getType());
	return dex;
}

char Npc::getDefendingDex(void)
{
	Item weapon = getEquip(5);
	int dex = getDex();

	if (isBlocking())
		dex += getDex() / 5;
	if (weapon.isValid())
		dex += WeaponCache::getDexDefendBonus(weapon.getType());
	return dex;
}

int Npc::getDefenseType(void)
{
	int monster;
	int defense = 0;

	if (isAvatar())
	{
		Item shield = getEquip(0);
		if (shield.isValid() && shield.getType() == 842)
			defense |= 8;
	}
	else
	{
		monster = DataTable::getMonsterRecord(getType());
		if (monster)
			defense = monsterRecord[monster].defenseType;
	}
	return defense;
}

int Npc::setInCombat(void)
{
	unsigned short pid = 0;

	if (isAvatar())
	{
		if (!avatarSettingCombat)
			avatar.setInCombat();
	}
	else
	{
		Kernel::killProcess(referent, (ProcessType)6, PriorityClass(0x21));
		int spell = cast(0);
		CombatBrain *brain = new CombatBrain(referent, 0);
		pid = brain->pid;
		if (spell)
			Kernel::getProcess(spell)->then(brain);
	}
	npcData[referent].inCombat = 1;
	return pid;
}

void Npc::clrInCombat(void)
{
	npcData[referent].inCombat = 0;
	if (isAvatar())
	{
		if (!avatarSettingCombat)
			avatar.clrInCombat();
	}
	else
	{
		CombatBrain *brain = (CombatBrain *)Kernel::findValidProcess(referent, (ProcessType)0xf2);
		if (brain)
			brain->pop(0);
	}
}

void Npc::callForHelp(void)
{
	AreaItemFinder finder;
	Referent target;
	unsigned char alignment;
	unsigned char otherAlignment;
	Type type;
	Type otherType;

	alignment = getAlignment();
	type = getType();
	finder.init(getX() - 0x400U, getY() - 0x400U, getX() + 0x400U, getY() + 0x400U, "$", 1);
	target = getTarget();
	while (finder.found())
	{
		if (finder.isNpc() && finder.referent != referent && !finder.isAvatar())
		{
			Npc npc(finder.referent);

			otherAlignment = npc.getAlignment();
			if (otherAlignment == alignment && !npc.isDead() && npc.referent != target)
			{
				otherType = npc.getType();
				if (type == 574)
				{
					if (otherType == 574)
						npc.setTarget(target);
				}
				else if (otherType != 707 && otherType != 835 && otherType != 836 && otherType != 708)
					npc.setTarget(target);
			}
		}
		finder.findNext();
	}
}

unsigned char avatarSettingCombat = 0;

int Avatar::setInCombat(void)
{
	avatarSettingCombat = TRUE;
	int pid = Npc::setInCombat();
	avatarSettingCombat = FALSE;

	playCombatMusic(98);
	avatar.doAnim(ANIM_DRAW_WEAPON, 8, 10000, 0);
	U8MousePointer::popMode();
	U8MousePointer::pushMode((PointerModes)25, 1);
	return pid;
}

void Avatar::clrInCombat(void)
{
	avatarSettingCombat = TRUE;
	Npc::clrInCombat();
	avatarSettingCombat = FALSE;

	if (!isCombatPending())
		restoreMusic();
	avatar.doAnim(ANIM_SHEATH_WEAPON, 8, 10000, 0);
	U8MousePointer::popMode();
	U8MousePointer::pushMode((PointerModes)0, 1);
}

void SetAvatarInCombat(void)
{
	avatar.setInCombat();
}

void ClrAvatarInCombat(void)
{
	avatar.clrInCombat();
}

int IsAvatarInCombat(void)
{
	return avatar.isInCombat();
}

char Npc::getStr(void) { return npcData[referent].str; }
char Npc::getDex(void) { return npcData[referent].dex; }
char Npc::getInt(void) { return npcData[referent].intelligence; }
unsigned char Npc::getHp(void) { return npcData[referent].hp; }
short Npc::getMana(void) { return npcData[referent].mana; }

void Npc::setStr(char str) { npcData[referent].str = str; }
void Npc::setDex(char dex) { npcData[referent].dex = dex; }
void Npc::setInt(char intelligence) { npcData[referent].intelligence = intelligence; }
void Npc::setHp(unsigned char hp) { npcData[referent].hp = hp; }
void Npc::setMana(short mana) { npcData[referent].mana = mana; }

int getName(void)
{
	return Yamm::flimFlam(avatar.name);
}

int getSex(void)
{
	return avatar.sex;
}

int getAvatarInStasis(void)
{
	return avatar.inStasis;
}

void setAvatarInStasis(short inStasis)
{
	avatar.inStasis = inStasis;
	if (avatar.inStasis && avatar.isInCombat() && !avatar.isDead())
		avatar.clrInCombat();
}

unsigned char Npc::getDir(void)
{
	return npcData[referent].dir;
}

int Npc::getLastAnimSet(void)
{
	return npcData[referent].lastAnimSet;
}

Avatar::Avatar(void) : Npc(1)
{
	init();
}

void Avatar::init(void)
{
	strengthPoints = 0;
	intelligencePoints = 0;
	dexterityPoints = 0;

	sex = 1;
	f_2a = 0;
	f_2b = 0;
	cheater = 0;
	canCheat = 0;
	inStasis = 0;
	f_3c.f_00 = 2;
	f_3c.f_02 = 2;
	randomSeed = 0;
	music = 0xff;
	f_36 = 1;

	language = ENGLISH;
	f_48 = 1;
	f_39 = 5;

	effectiveStr = 0;
	f_3b = 0;
}

// Each use of a stat earns points; enough points, or a lucky roll, raise it by one.
#define STAT_POINTS	650
#define MAX_STAT	25

void Avatar::accumulateStrength(short points)
{
	if (avatar.getStr() < MAX_STAT)
	{
		strengthPoints += points;
		if (strengthPoints >= STAT_POINTS || OneIn(STAT_POINTS - strengthPoints))
		{
			strengthPoints = 0;
			npcData[referent].str++;
			playSFX(0x36, 250);
		}
	}
}

void Avatar::accumulateIntelligence(short points)
{
	if (avatar.getInt() < MAX_STAT)
	{
		intelligencePoints += points;
		if (intelligencePoints >= STAT_POINTS || OneIn(STAT_POINTS - intelligencePoints))
		{
			intelligencePoints = 0;
			npcData[referent].intelligence++;
			playSFX(0x36, 250);
		}
	}
}

void Avatar::accumulateDexterity(short points)
{
	if (avatar.getDex() < MAX_STAT)
	{
		dexterityPoints += points;
		if (dexterityPoints >= STAT_POINTS || OneIn(STAT_POINTS - dexterityPoints))
		{
			dexterityPoints = 0;
			npcData[referent].dex++;
			playSFX(0x36, 250);
		}
	}
}

void AccumulateStrength(short points)
{
	avatar.accumulateStrength(points);
}

void AccumulateIntelligence(short points)
{
	avatar.accumulateIntelligence(points);
}

void AccumulateDexterity(short points)
{
	avatar.accumulateDexterity(points);
}

char AvatarCanCheat(void)
{
	return avatar.canCheat;
}

void MakeAvatarACheater(void)
{
	avatar.cheater = 1;
}

char Avatar::LanguageDescChar[15] = {'E', 'F', 'G', 'S', 'I', 'R', 'D', 'H', 'J', 'A', 'C', 'P', 'B', 'N', 'M'};

Boolean Avatar::setLanguage(Language lang)
{
	char name[13];

	// The intro movie's first letter names its language.
	strcpy(name, introFileName);
	name[0] = LanguageDescChar[lang];
	if (!FileExists(fileSpec(0, StaticDir, name, 0)))
		return FALSE;

	language = lang;
	reloadUnk();
	return TRUE;
}

void Avatar::load(void)
{
	if (!FileExists(fileSpec(0, GamedatDir, AvatarFileName, 0)))
		return;

	BaseFile file(fileSpec(0, GamedatDir, AvatarFileName, 0), ReadOnly);
	file.read((char *)&avatar.strengthPoints, sizeof(Avatar) - sizeof(Npc));
	file.close();
	SetRandomSeed(avatar.randomSeed);
}

void Avatar::save(char *dir)
{
	if (!dir)
		dir = GamedatDir;

	BaseFile file;
	file.forceOpen(fileSpec(0, dir, AvatarFileName, 0), ReadWrite);
	avatar.randomSeed = GetRandomSeed();
	file.write((char *)&avatar.strengthPoints, sizeof(Avatar) - sizeof(Npc));
	file.close();
}

void realAvatarDeath(void)
{
	theDeathLoader->dead = TRUE;
}

DeathLoader::DeathLoader(void)
{
	Kernel::setIdString(pid, "DeathLoader");
	setProcessType((ProcessType)0x231);
	setDaemon();
	dead = FALSE;
	setRef(0);
}

void DeathLoader::process(void)
{
	if (!inGameMode)
		return;
	if (dead)
	{
		dead = FALSE;
		if (Camera::screenInverted)
			Camera::invertScreen(0);

		Process *old = Kernel::findValidProcess(0, (ProcessType)0x206);
		if (old)
			old->fail(0);

		FadeProcess *fade = new FadeProcess((PaletteNum)0, 0x7fff, 0);
		Process *menu = new MainMenuProcess;
		fade->then(menu);
	}
}

void DeathLoader::load(BaseFile *file)
{
	Process::load(file);
	theDeathLoader = this;
}

char avatarDotMusic(void)
{
	return avatar.music;
}

int avatarDotGetLanguage(void)
{
	return avatar.getLanguageChar();
}

Boolean avatarDotIsInCombat(void)
{
	return avatar.isInCombat();
}
