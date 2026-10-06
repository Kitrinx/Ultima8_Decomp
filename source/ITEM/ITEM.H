#ifndef ITEM_H
#define ITEM_H

#include "..\GLIB\PROCESS.H"
#include "ANIMPRIM.H"
#include "DISPATCH.H"

class Event;
class WorldPoint;

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

typedef unsigned char Boolean;
typedef unsigned short Referent;
typedef unsigned short Type;
typedef unsigned short Frame;
typedef unsigned short Quality;
typedef short Quantity;

// Item status bits (Origin's usecode names where known).
#define IN_DISPLAY_LIST		0x0001
#define DISPOSABLE			0x0002
#define OWNED				0x0004
#define CONTAINED			0x0008
#define INVISIBLE			0x0010
#define FLIPPED				0x0020
#define FAST_ONLY			0x0080
#define CONTAINER_GUMP_OPEN	0x0100
#define ETHEREAL			0x0800
#define FAST_AREA			0x2000

class Item
{
public:
	Referent referent;

	Item(void) { referent = 0; }
	Item(Referent r) { referent = r; }
	Item &operator=(Item item) { referent = item.referent; return *this; }
	Boolean isValid(void) { return referent != 0; }
	void set(Referent r) { referent = r; }
	Boolean isAvatar(void) { return referent == 1; }
	// z 254 marks an item taken off the map.
	Boolean isOffMap(void) { return getZ() == 0xfe; }
	Boolean isFastOnly(void) { return (getStatus() & FAST_ONLY) ? TRUE : FALSE; }

	int hurl(short, short, short, short);
	void fall(void);
	void getScreenXY(unsigned short &, unsigned short &);
	int getX(void);
	int getY(void);
	int getGumpX(void);
	int getGumpY(void);
	void setGumpXY(short, short);
	unsigned char getZ(void);
	int getCX(void);
	int getCY(void);
	char getCZ(void);
	void setX(unsigned short);
	void setY(unsigned short);
	void setZ(unsigned char);
	WorldPoint getLoc(void);
	int getNext(void);
	int getWeight(void);
	int getWeightIncludingContents(void);
	int getVolume(void);
	int getCapacity(void);
	int getSurfaceWeight(void);
	void setNext(unsigned short);
	int getType(void);
	void setType(unsigned short);
	int getFrame(void);
	void setFrame(unsigned short);
	void setNpcNum(short);
	void setNpcArray(short);
	int getNpcArray(void);
	void setMapArray(short);
	int getMapArray(void);
	int getQ(void);
	int getQuality(void);
	void setQuality(short);
	int getUnkEggType(void);
	void setUnkEggType(short);
	int getQuantity(void);
	void setQuantity(short);
	int getContents(void);
	int getContainer(void);
	int getRootContainer(void);
	int getGlobNum(void);
	void setGlobNum(unsigned);
	void setQ(unsigned);
	void setContents(unsigned short);
	int getFamily(void);
	char getTypeFlag(short);
	int getStatus(void);
	Boolean legal_create(Type, Frame, WorldPoint &);
	Boolean legal_create(Type, Frame, unsigned short, unsigned short, unsigned short);
	Boolean legal_create(Type, Frame, Referent, Quantity);
	Boolean create(Type, Frame);
	void pop(unsigned short, unsigned short, unsigned char);
	void pop(unsigned short);
	void pop(void);
	void pop(WorldPoint &);
	void popToEnd(unsigned short);
	void push(void);
	void destroy(void);
	void destroyContents(void);
	void removeContents(void);
	void move(unsigned short, unsigned short, unsigned char);
	void move(unsigned short);
	Boolean legal_move(WorldPoint &, unsigned short, unsigned short);
	Boolean legal_move(Referent &, unsigned short);
	Boolean superlegal_move(WorldPoint &, unsigned short, unsigned short, unsigned short, Boolean *, Referent *);
	void move(WorldPoint &);
	void move(char);
	char getDirToCoords(unsigned short, unsigned short);
	int getDirFromCoords(unsigned short, unsigned short);
	int getDirFromItem(Referent);
	char getDirToItem(Referent);
	void getFootpad(short &, short &, short &);
	Boolean touches(Referent);
	Boolean overlaps(Referent);
	Boolean overlapsXY(Referent);
	Boolean isOn(Referent);
	Boolean isCompletelyOn(Referent);
	Boolean isAbove(Referent);
	Boolean isUnder(Referent);
	Boolean ascend(short);
	void getPoint(WorldPoint &);
	Boolean isNpc(void);
	Boolean isInNpc(void);
	void touch(void);
	void setStatus(short);
	void orStatus(short);
	void andStatus(short);
	int look(void);
	int use(void);
	int hit(Referent, short);
	int gotHit(Referent, short);
	int anim(void);
	int bark(char *);
	void grab(void);
	int release(void);
	int equip(void);
	int unequip(void);
	int cachein(void);
	int combine(void);
	int calledFromAnim(void);
	int enterFastArea(void);
	int leaveFastArea(void);
	int cast(unsigned short);
	int justMoved(void);
	int animGetHit(unsigned short);
	int guardianBark(int);
	int AvatarStoleSomething(Referent);
	Boolean findTarget(int, int);
	void getSliderInput(int, int, int);
	void ask(unsigned short);
	void openGump(short);
	void closeGump(void);
	Boolean isGumpOpen(void);
	int getDamageType(void);
	void receiveHit(Referent, char, short, unsigned short);
	Boolean isExplosive(void);
	Boolean isFixed(void);
	int explode(void);
	Boolean canReach(Referent, short, const WorldPoint *, Boolean);
	Boolean canReach(Referent, short);
	void preload(void);
	int getRange(Referent);
	int getRange(Type, unsigned short, unsigned short);
	int getQHi(void);
	void setQHi(short);
	int getQLo(void);
	void setQLo(short);
	void shoot(WorldPoint &, short, short);
};

// The item found under a screen point.
class FastItem : public Item
{
public:
	FastItem(void) {}
	FastItem(Referent r) { referent = r; }
};

class ReleaseTrigger : public Process
{
public:
	ReleaseTrigger(Referent);
	virtual void process(void);
};

class SpeechProcess : public Process
{
public:
	SpeechProcess(void);
	virtual void process(void);
};

class Enliven : public Process
{
public:
	Enliven(Referent);
	virtual void process(void);
};

class WaitForKeyGump : public NewGump
{
public:
	unsigned f_34;

	WaitForKeyGump(unsigned short);
	virtual void commandKeyboard(Event &);
};

class DeathGumpProcess : public Process
{
public:
	DeathGumpProcess(void);
	virtual void process(void);
};

class AvatarDeathWatch : public Process
{
public:
	unsigned char f_2a;
	NewGump *gump;

	AvatarDeathWatch(void);
	virtual void process(void);
	void setGump(NewGump *g) { gump = g; }
};

class UnstunNpc : public Process
{
public:
	UnstunNpc(Referent);
	virtual void process(void) { pop(0); }
	virtual void pop(long);
	virtual void fail(long);
};

class GetUp : public AnimPrimitive
{
public:
	GetUp(void) : AnimPrimitive(0, (AnimSet)0, 0, 0, 0, 0) {}
	GetUp(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)4, dir, 0, 0, 0) {}
};

class FallDown : public AnimPrimitive
{
public:
	FallDown(void) : AnimPrimitive(0, (AnimSet)0, 0, 0, 0, 0) {}
	FallDown(Referent npc, char dir) : AnimPrimitive(npc, (AnimSet)14, dir, 0, 0, 0) {}
};

void checkReferent(unsigned short, char *, short);
void selectiveProcessKill(Referent);
Boolean killAllButDeathAnim(Referent, Boolean, Boolean);
void checkForVictoryMusic(void);

extern unsigned dirToXOffset[8];
extern unsigned dirToYOffset[8];
extern char *AvatarDeathYell[3];

#endif
