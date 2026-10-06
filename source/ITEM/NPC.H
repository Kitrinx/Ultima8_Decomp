#ifndef NPC_H
#define NPC_H

#include "..\GLIB\PROCESS.H"
#include "..\POLIB\INC\FLEX.H"
#include "ANIMCACH.H"
#include "ANIMPRIM.H"
#include "INIT.H"
#include "ITEM.H"
#include "CRECT.H"

class BaseFile;
class NpcDataFile;
class WorldPoint;

enum Activity
{
	Activity_0
};

// One NPC's 49-byte record in npcdata.dat.
struct NpcData
{
	char str;
	char dex;
	char intelligence;
	unsigned char hp;
	unsigned char dir;
	unsigned lastAnimSet;
	unsigned char frameHi;	// high byte of the frame number
	unsigned char animFrame;	// current frame of the running animation
	unsigned char fallZ;	// height a fall started from
	unsigned char pad_0a;
	unsigned char alignment;	// own alignment in the low nibble, enemies' in the high
	unsigned char pad_0c[15];
	unsigned invincible : 1;
	unsigned ascending : 1;
	unsigned descending : 1;
	unsigned animLock : 1;
	unsigned flags1B_4 : 1;
	unsigned flags1B : 11;
	Referent equip[8];
	short mana;
	unsigned inUse : 1;	// slot taken by a live NPC
	unsigned falling : 1;
	unsigned firstStep : 1;	// first half of a two-step animation done
	unsigned inCombat : 1;
	unsigned dead : 1;
	unsigned flags2F_5 : 1;
	unsigned combatPending : 1;
	unsigned balancing : 1;	// the Avatar teeters on an edge
	unsigned airWalk : 1;
	unsigned swinging : 1;	// the Avatar is recovering from a swing
	unsigned immortal : 1;
	unsigned withstandDeath : 1;
	unsigned feignDeath : 1;
	unsigned stunned : 1;
	unsigned poisoned : 1;
	unsigned pathfinding : 1;
};

class Npc : public Item
{
public:
	Npc(void) {}
	Npc(Referent r) { referent = r; }

	static void init(void);
	static void uninit(void);
	static void save(void);
	static void load(void);
	Boolean isBusy(void);
	int doAnim(AnimSet, short, short, unsigned char);
	int getNumTypes(Type, unsigned short);
	void setTarget(Referent);
	int getTarget(void);
	int areEnemiesNear(void);
	Boolean isInCombat(void);
	void setAlignment(unsigned char);
	unsigned char getAlignment(void);
	void setEnemyAlignment(unsigned char);
	unsigned char getEnemyAlignment(void);
	Boolean isDead(void);
	void setDead(void);
	void clrDead(void);
	int pathfind(unsigned short, unsigned short, unsigned short, unsigned short);
	int pathfind(unsigned short, unsigned short);
	static int getNpcSlot(void);
	void freeNpcSlot(void);
	void setAirWalkEnabled(Boolean);
	Boolean getAirWalkEnabled(void);
	Boolean isImmortal(void);
	void setImmortal(void);
	void clrImmortal(void);
	Boolean isFeignDeath(void);
	void setFeignDeath(void);
	void clrFeignDeath(void);
	Boolean isStunned(void);
	void setStunned(void);
	void clrStunned(void);
	Boolean isPoisoned(void);
	void setPoisoned(void);
	void clrPoisoned(void);
	Boolean isPathfinding(void);
	void setPathfinding(void);
	void clrPathfinding(void);
	Boolean isInvincible(void);
	void setInvincible(void);
	void clrInvincible(void);
	Boolean isAscending(void);
	void setAscending(void);
	void clrAscending(void);
	Boolean isDescending(void);
	void setDescending(void);
	void clrDescending(void);
	Boolean isAnimLock(void);
	void setAnimLock(void);
	void clrAnimLock(void);
	Boolean isBlocking(void);
	Boolean isWithstandDeath(void);
	void setWithstandDeath(void);
	void clrWithstandDeath(void);
	Boolean isEnemy(Referent);
	Boolean simpleCreate(Type, Frame);
	Boolean create(Type, Frame);
	void insertNpcData(void);
	Boolean legal_create(Type, Frame, WorldPoint &);
	Boolean legal_create(Type, Frame, unsigned short, unsigned short, unsigned short);
	Referent getEquip(short);
	Boolean setEquip(short, Referent);
	void unEquip(short);
	void freeEquip(Referent);
	int findEquip(Referent);
	void clearEquip(void);
	void teleport(unsigned short, unsigned short, unsigned char, unsigned char);
	int getMap(void);
	int setActivity(unsigned long);
	void cSetActivity(Activity);
	int schedule(unsigned long);
	unsigned char getDamageAmount(void);
	unsigned char getArmorClass(void);
	char getAttackingDex(void);
	char getDefendingDex(void);
	int getDefenseType(void);
	int setInCombat(void);
	void clrInCombat(void);
	void callForHelp(void);
	char getStr(void);
	char getDex(void);
	char getInt(void);
	unsigned char getHp(void);
	short getMana(void);
	void setStr(char);
	void setDex(char);
	void setInt(char);
	void setHp(unsigned char);
	void setMana(short);
	unsigned char getDir(void);
	int getLastAnimSet(void);
	static NpcDataFile *npcFile;
	static unsigned char namefile[4];
	static NpcData *npcData;

	Boolean isInFastArea(void) { return (getStatus() & FAST_AREA) ? TRUE : FALSE; }
	Boolean isAtZ254(void) { return getZ() == 254; }

	// Set while combat is about to start; cleared when it does.
	unsigned char isCombatPending(void) { return ((NpcData *)npcData)[referent].combatPending; }
	unsigned char getFrameHi(void) { return ((NpcData *)npcData)[referent].frameHi; }
	void setFrameHi(unsigned char hi) { ((NpcData *)npcData)[referent].frameHi = hi; }
	unsigned char isFalling(void) { return ((NpcData *)npcData)[referent].falling; }
	unsigned char getFallZ(void) { return ((NpcData *)npcData)[referent].fallZ; }
	void setFallZ(unsigned char z) { ((NpcData *)npcData)[referent].fallZ = z; }
	unsigned char isFirstStep(void) { return ((NpcData *)npcData)[referent].firstStep; }
	unsigned char isFlag2F_5(void) { return ((NpcData *)npcData)[referent].flags2F_5; }
	unsigned char isBalancing(void) { return ((NpcData *)npcData)[referent].balancing; }
	unsigned char isSwinging(void) { return ((NpcData *)npcData)[referent].swinging; }
	void setAnimFrame(unsigned char frame) { ((NpcData *)npcData)[referent].animFrame = frame; }
	void setLastAnimSet(int anim) { ((NpcData *)npcData)[referent].lastAnimSet = anim; }
	void setDir(char dir) { ((NpcData *)npcData)[referent].dir = dir; }
};

class Avatar : public Npc
{
public:
	int strengthPoints;		// progress toward the next point of each stat
	int intelligencePoints;
	int dexterityPoints;
	char name[32];
	int sex;
	unsigned char f_2a;
	unsigned char f_2b;
	unsigned char cheater;
	unsigned char canCheat;
	short inStasis;
	long randomSeed;
	unsigned char music;
	unsigned char pad_35[1];
	unsigned f_36;
	char language;
	char f_39;
	unsigned char effectiveStr;
	unsigned char f_3b;
	Point f_3c;
	Point f_40;
	Point f_44;
	unsigned char f_48;
	unsigned char soundOn;
	unsigned char musicOn;
	unsigned char pad_4b[4];

	Avatar(void);
	char getEffectiveStr(void);
	void setEffectiveStr(int);
	int setInCombat(void);
	void clrInCombat(void);
	void init(void);
	void accumulateStrength(short);
	void accumulateIntelligence(short);
	void accumulateDexterity(short);
	Boolean setLanguage(Language);
	void load(void);
	void save(char *);
	static char LanguageDescChar[15];

	int getLanguageChar(void) { return LanguageDescChar[language]; }
};

class ClearFeignDeath : public Process
{
public:
	ClearFeignDeath(unsigned short);
	virtual void process(void);
};

class DeathLoader : public Process
{
public:
	Boolean dead;	// set when the avatar dies

	DeathLoader(void);
	virtual void process(void);
	virtual void load(BaseFile *);
};

class BlockEnd : public AnimPrimitive
{
public:
	BlockEnd(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)60, dir, 0, 0, 0) {}
};

class BlockStart : public AnimPrimitive
{
public:
	BlockStart(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)59, dir, 0, 0, 0) {}
};

class Kick : public AnimPrimitive
{
public:
	Kick(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)58, dir, 0, 0, 0) {}
};

class Burn : public AnimPrimitive
{
public:
	Burn(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)57, dir, 0, 0, 0) {}
};

class Drown : public AnimPrimitive
{
public:
	Drown(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)56, dir, 0, 0, 0) {}
};

class Work : public AnimPrimitive
{
public:
	Work(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)55, dir, 0, 0, 0) {}
};

class GiveItem : public AnimPrimitive
{
public:
	GiveItem(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)54, dir, 0, 0, 0) {}
};

class Talk : public AnimPrimitive
{
public:
	Talk(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)53, dir, 0, 0, 0) {}
};

class SitUp : public AnimPrimitive
{
public:
	SitUp(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)52, dir, 0, 0, 0) {}
};

class SitDown : public AnimPrimitive
{
public:
	SitDown(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)51, dir, 0, 0, 0) {}
};

class KneelUp : public AnimPrimitive
{
public:
	KneelUp(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)50, dir, 0, 0, 0) {}
};

class KneelDown : public AnimPrimitive
{
public:
	KneelDown(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)49, dir, 0, 0, 0) {}
};

class Fidget2 : public AnimPrimitive
{
public:
	Fidget2(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)48, dir, 0, 0, 0) {}
};

class Fidget : public AnimPrimitive
{
public:
	Fidget(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)47, dir, 0, 0, 0) {}
};

class LedgeClimb : public AnimPrimitive
{
public:
	LedgeClimb(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)46, dir, 0, 0, 0) {}
};

class LedgeSwing : public AnimPrimitive
{
public:
	LedgeSwing(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)45, dir, 0, 0, 0) {}
};

class FallRecDmg : public AnimPrimitive
{
public:
	FallRecDmg(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)44, dir, 0, 0, 0) {}
};

class FallRecHurt : public AnimPrimitive
{
public:
	FallRecHurt(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)43, dir, 0, 0, 0) {}
};

class EdgeBalance : public AnimPrimitive
{
public:
	EdgeBalance(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)42, dir, 0, 0, 0) {}
};

class AltitudeFall : public AnimPrimitive
{
public:
	AltitudeFall(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)41, dir, 0, 0, 0) {}
};

class SwingWeapon2 : public AnimPrimitive
{
public:
	SwingWeapon2(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)39, dir, 0, 0, 0) {}
};

class HighReach : public AnimPrimitive
{
public:
	HighReach(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)38, dir, 0, 0, 0) {}
};

class MidReach : public AnimPrimitive
{
public:
	MidReach(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)37, dir, 0, 0, 0) {}
};

class LowReach : public AnimPrimitive
{
public:
	LowReach(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)36, dir, 0, 0, 0) {}
};

class Kneel : public AnimPrimitive
{
public:
	Kneel(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)35, dir, 0, 0, 0) {}
};

class Bow : public AnimPrimitive
{
public:
	Bow(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)34, dir, 0, 0, 0) {}
};

class LookRight : public AnimPrimitive
{
public:
	LookRight(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)33, dir, 0, 0, 0) {}
};

class LookLeft : public AnimPrimitive
{
public:
	LookLeft(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)32, dir, 0, 0, 0) {}
};

class Cast3 : public AnimPrimitive
{
public:
	Cast3(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)31, dir, 0, 0, 0) {}
};

class Cast2 : public AnimPrimitive
{
public:
	Cast2(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)30, dir, 0, 0, 0) {}
};

class Cast1 : public AnimPrimitive
{
public:
	Cast1(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)29, dir, 0, 0, 0) {}
};

class ExitCast : public AnimPrimitive
{
public:
	ExitCast(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)28, dir, 0, 0, 0) {}
};

class EnterCast : public AnimPrimitive
{
public:
	EnterCast(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)27, dir, 0, 0, 0) {}
};

class NinjaFlip : public AnimPrimitive
{
public:
	NinjaFlip(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)18, dir, 0, 0, 0) {}
};

class FallRecovery : public AnimPrimitive
{
public:
	FallRecovery(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)16, dir, 0, 0, 0) {}
};

class CombatStand : public AnimPrimitive
{
public:
	CombatStand(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)15, dir, 0, 0, 0) {}
};

class GetHit : public AnimPrimitive
{
public:
	GetHit(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)13, dir, 0, 0, 0) {}
};

class CarefulStep : public AnimPrimitive
{
public:
	CarefulStep(Referent npc, char dir, WorldPoint *dest = 0, Referent target = 0) :
		AnimPrimitive(npc, (AnimSet)12, dir, dest, target, 0) {}
};

class RunningLeap : public AnimPrimitive
{
public:
	RunningLeap(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)10, dir, 0, 0, 0) {}
};

class Retreat : public AnimPrimitive
{
public:
	Retreat(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)9, dir, 0, 0, 0) {}
};

class Advance : public AnimPrimitive
{
public:
	Advance(unsigned short npc, char dir, WorldPoint *dest = 0, unsigned short target = 0) :
		AnimPrimitive(npc, ANIM_ADVANCE, dir, dest, target, 0) {}
};

class SwingWeapon : public AnimPrimitive
{
public:
	SwingWeapon(Referent npc, char dir, WorldPoint *dest = 0, Referent target = 0) :
		AnimPrimitive(npc, (AnimSet)7, dir, dest, 0, target) {}
};

class SheathWeapon : public AnimPrimitive
{
public:
	SheathWeapon(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)6, dir, 0, 0, 0) {}
};

class DrawWeapon : public AnimPrimitive
{
public:
	DrawWeapon(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)5, dir, 0, 0, 0) {}
};

class VertLeap : public AnimPrimitive
{
public:
	VertLeap(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)3, dir, 0, 0, 0) {}
};

class Stand : public AnimPrimitive
{
public:
	Stand(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)2, dir, 0, 0, 0) {}
};

class Run : public AnimPrimitive
{
public:
	Run(Referent npc, char dir, WorldPoint *dest = 0, Referent target = 0) :
		AnimPrimitive(npc, (AnimSet)1, dir, dest, target, 0) {}
};

class Walk : public AnimPrimitive
{
public:
	Walk(unsigned short npc, char dir, WorldPoint *dest = 0, unsigned short target = 0) :
		AnimPrimitive(npc, ANIM_WALK, dir, dest, target, 0) {}
};

class NpcDataFile : public FlexFile
{
public:
	NpcDataFile(void) {}
};

void initNpcs(void);
void uninitNpcs(void);
void SetAvatarInCombat(void);
void ClrAvatarInCombat(void);
int IsAvatarInCombat(void);
int getName(void);
int getSex(void);
int getAvatarInStasis(void);
void setAvatarInStasis(short);
void AccumulateStrength(short);
void AccumulateIntelligence(short);
void AccumulateDexterity(short);
char AvatarCanCheat(void);
void MakeAvatarACheater(void);
void realAvatarDeath(void);
char avatarDotMusic(void);
int avatarDotGetLanguage(void);
Boolean avatarDotIsInCombat(void);

extern char *NpcDataFileName;
extern char *NpcNameFileName;
extern char *AvatarFileName;
extern DeathLoader *theDeathLoader;
extern unsigned char avatarSettingCombat;
extern Avatar avatar;

#endif
