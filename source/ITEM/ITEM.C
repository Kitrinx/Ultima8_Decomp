// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r- -y
// name: ..\ITEM\ITEM.C

#include <dos.h>
#include <stdlib.h>
#include <string.h>
#include "..\UI\NEWGUMP.H"
#include "DISPATCH.H"
#include "ERROR.H"
#include "CAMERA.H"
#include "CONTGUMP.H"
#include "ITEM.H"
#include "ITEMDATA.H"
#include "TYPE.H"
#include "CONTAIN.H"
#include "ITEMFIND.H"
#include "NPC.H"
#include "DLITEM.H"
#include "ITEMCACH.H"
#include "MOTRAK.H"
#include "EGG.H"
#include "STATREST.H"
#include "DIST.H"
#include "UPROCESS.H"
#include "DSFXMAN.H"
#include "SNDMGR.H"
#include "BARK.H"
#include "SLIDER.H"
#include "WAIT.H"
#include "GRAVE.H"
#include "WPNCACHE.H"
#include "COMBATBR.H"
#include "AMUSIC.H"
#include "INIT.H"
#include "SHAPHAND.H"
#include "MISSILE.H"
#include "ANIMCALL.H"
#include "ANIMCACH.H"
#include "ASTREAM.H"
#include "ANODE.H"
#include "KERNEL.H"
#include "chargen.H"

#define CONTAINER_GUMP	0x43

#define LOBYTE(w)	((unsigned char)((w) & 0xff))
#define HIBYTE(w)	((unsigned char)((w) >> 8))

// Usecode events.
#define LOOK_EVENT		0
#define USE_EVENT		1
#define ANIM_EVENT		2
#define CACHEIN_EVENT	4
#define HIT_EVENT		5
#define GOTHIT_EVENT	6
#define RELEASE_EVENT	9
#define EQUIP_EVENT		10
#define UNEQUIP_EVENT	11
#define COMBINE_EVENT	12
#define CALLEDFROMANIM_EVENT	14
#define ENTERFASTAREA_EVENT	15
#define LEAVEFASTAREA_EVENT	16
#define CAST_EVENT		17
#define JUSTMOVED_EVENT	18
#define AVATARSTOLESOMETHING_EVENT	19
#define ANIMGETHIT_EVENT	20
#define GUARDIANBARK_EVENT	21

// Damage types.
#define DMG_NORMAL		0x0001
#define DMG_BLADE		0x0002
#define DMG_BLUNT		0x0004
#define DMG_FIRE		0x0008
#define DMG_UNDEAD		0x0010
#define DMG_MAGIC		0x0020
#define DMG_SLAYER		0x0040
#define DMG_PIERCE		0x0080
#define DMG_FALLING		0x0100

// Inline in the shared headers when this file was built.
inline Point::Point(void) {}
inline void Point::set(short x, short y) { f_00 = x; f_02 = y; }
inline void Rect::set(short x1, short y1, short x2, short y2) { Point::set(x1, y1); f_04 = x2; f_06 = y2; }
inline Rect::Rect(short x1, short y1, short x2, short y2) { set(x1, y1, x2, y2); }
inline NewGumpId::NewGumpId(unsigned i) { instance = i; f_02 = ~instance; f_04 = 0; }

// One monster type's 13-byte record in DataTable::data.
struct MonsterRecord
{
	unsigned char pad_00[9];
	unsigned damageType;
	unsigned defenseType;
};

#define monsterRecord	((MonsterRecord *)DataTable::data)

// A footprint on the map, corner (x1, y1) to corner (x2, y2).
class Box
{
public:
	int x1, y1, x2, y2;

	Box(void) { x1 = 0; y1 = 0; x2 = 0; y2 = 0; }
	Boolean overlaps(Box &b) { return x1 <= b.x2 && y1 <= b.y2 && x2 >= b.x1 && y2 >= b.y1; }
};

// The bytes of an AnimNode that receiveHit reads.
struct AnimNodeFields
{
	unsigned char frame[2];
	unsigned deltaZ : 8;
	unsigned sfx : 8;
	unsigned char flags[2];
};

inline Item itemOf(Referent r) { return Item(r); }

unsigned dirToXOffset[8] = {0xfff0, 0, 16, 16, 16, 0, 0xfff0, 0xfff0};
unsigned dirToYOffset[8] = {0xfff0, 0xfff0, 0xfff0, 0, 16, 16, 16, 0};
char *AvatarDeathYell[3] = {"Aiiieeee...", "A\x8b\x8b\x8b\x8b" "eeee...", "Aaaaarrrgh..."};

void checkReferent(Referent r, char *file, short line)
{
	if (r == 0 || r >= MAX_ITEMS)
		halt(file, line);
}

ReleaseTrigger::ReleaseTrigger(Referent item)
{
	setRef(item);
	setProcessType((ProcessType)0xf1);
}

void ReleaseTrigger::process(void)
{
	Item(ref).release();
	pop(0);
}

// A contained item shows at its slot in an open container gump,
// otherwise at its world position.
void Item::getScreenXY(unsigned short &x, unsigned short &y)
{
	if (getStatus() & CONTAINED)
	{
		Referent container = getContainer();
		NewGump *gump = 0;
		while (Dispatcher::base[Dispatcher::baseSP]->newGumpList.hasType(CONTAINER_GUMP, gump))
		{
			if (((ContainerGump *)gump)->f_52 == 0 && ((ContainerGump *)gump)->containerItem.referent == container)
			{
				x = ((NewGump *)gump->newGumpList.head)->get_dx() + getGumpX();
				y = ((NewGump *)gump->newGumpList.head)->get_dy() + getGumpY();
				return;
			}
		}
	}
	WorldToScreenCoords(getX(), getY(), (short &)x, (short &)y);
	y -= getZ();
}

// A contained item's x holds its container; the outermost one has the position.
int Item::getX(void)
{
	Referent r = referent;
	while (ItemData::statusArray[r] & CONTAINED)
		r = ItemData::xArray[r];
	return ItemData::xArray[r];
}

int Item::getY(void)
{
	Referent r = referent;
	while (ItemData::statusArray[r] & CONTAINED)
		r = ItemData::xArray[r];
	return ItemData::yArray[r];
}

// A contained item's y holds its place in the container gump.
int Item::getGumpX(void)
{
	int gumpX = 0;
	if (getStatus() & CONTAINED)
		gumpX = LOBYTE(ItemData::yArray[referent]);
	return gumpX;
}

int Item::getGumpY(void)
{
	int gumpY = 0;
	if (getStatus() & CONTAINED)
		gumpY = HIBYTE(ItemData::yArray[referent]);
	return gumpY;
}

void Item::setGumpXY(short x, short y)
{
	if (ItemData::statusArray[referent] & CONTAINED)
	{
		if (x > 127)
			x = 127;
		if (y > 255)
			y = 255;
		ItemData::yArray[referent] = (x & 0xff) + ((y & 0xff) << 8);
	}
}

unsigned char Item::getZ(void)
{
	Referent r = referent;
	while (ItemData::statusArray[r] & CONTAINED)
		r = ItemData::xArray[r];
	return ItemData::zArray[r];
}

// The centre of the item's footpad.
int Item::getCX(void)
{
	TypeFlag flags = GlobalTypes.typeFlags[ItemData::typeArray[referent]];
	return getX() - flags.xSize * 16;
}

int Item::getCY(void)
{
	TypeFlag flags = GlobalTypes.typeFlags[ItemData::typeArray[referent]];
	return getY() - flags.ySize * 16;
}

char Item::getCZ(void)
{
	TypeFlag flags = GlobalTypes.typeFlags[ItemData::typeArray[referent]];
	return getZ() + flags.zSize * 4;
}

void Item::setX(unsigned short x)
{
	ItemData::xArray[referent] = x;
}

void Item::setY(unsigned short y)
{
	ItemData::yArray[referent] = y;
}

void Item::setZ(unsigned char z)
{
	if (z >= 251)
		halt(__FILE__, 336);	// __LINE__ in the original file
	ItemData::zArray[referent] = z;
}

WorldPoint Item::getLoc(void)
{
	Referent r = referent;
	while (ItemData::statusArray[r] & CONTAINED)
		r = ItemData::xArray[r];
	WorldPoint loc;
	loc.x = ItemData::xArray[r];
	loc.y = ItemData::yArray[r];
	loc.z = ItemData::zArray[r];
	return loc;
}

int Item::getNext(void)
{
	return ItemData::nextArray[referent];
}

// Weights are in tenths; quantities and reagents weigh per ten.
int Item::getWeight(void)
{
	if (referent == dragWeightHack)
		return 0;
	if (getType() == 82 && (getStatus() & CONTAINED) && Item(getContainer()).getType() == 79)
		return 0;
	TypeFlag flags = GlobalTypes.typeFlags[ItemData::typeArray[referent]];
	int family;
	if ((family = flags.family) == QUAN_FAMILY)
		return (flags.weight * getQuantity() + 9) / 10;
	if (family == REAGENT_FAMILY)
		return flags.weight * getQuantity();
	return flags.weight * 10;
}

int Item::getWeightIncludingContents(void)
{
	if (referent == dragWeightHack)
		return 0;
	int weight = getWeight();
	if (getContents())
		weight += Container(referent).getContentsWeight();
	return weight;
}

int Item::getVolume(void)
{
	TypeFlag flags = GlobalTypes.typeFlags[ItemData::typeArray[referent]];
	if (getStatus() & INVISIBLE)
		return 0;
	int family;
	if ((family = flags.family) == QUAN_FAMILY)
		return (flags.volume * getQuantity() + 99) / 100;
	if (family == REAGENT_FAMILY)
		return (flags.volume * getQuantity() + 9) / 10;
	if (family == CONTAINER_FAMILY && flags.volume == 0)
		return 1;
	return flags.volume;
}

int Item::getCapacity(void)
{
	TypeFlag flags = GlobalTypes.typeFlags[ItemData::typeArray[referent]];
	if (flags.family == CONTAINER_FAMILY)
		return flags.volume ? flags.volume : 32;
	return 0;
}

// The weight of everything resting on top of this item.
int Item::getSurfaceWeight(void)
{
	int weight = 0;
	SurfaceItemFinder finder(referent, "$", 1, SURFACE_ON);
	while (finder.found())
	{
		weight += finder.getWeightIncludingContents();
		finder.findNext();
	}
	return weight;
}

void Item::setNext(unsigned short next)
{
	ItemData::nextArray[referent] = next;
}

int Item::getType(void)
{
	return ItemData::typeArray[referent];
}

void Item::setType(unsigned short type)
{
	ItemData::typeArray[referent] = type;
}

// An NPC's frame numbers go past 255; the high byte is kept with the NPC.
int Item::getFrame(void)
{
	int frame = ItemData::frameArray[referent];
	if (isNpc())
	{
		Npc npc(referent);
		frame |= npc.getFrameHi() << 8;
	}
	return frame;
}

void Item::setFrame(unsigned short frame)
{
	ItemData::frameArray[referent] = frame;
	if (isNpc())
	{
		Npc npc(referent);
		npc.setFrameHi(frame >> 8);
	}
	if (ItemData::statusArray[referent] & IN_DISPLAY_LIST)
	{
		if (ItemData::zArray[referent] < Camera::roof() ||
			GlobalTypes.typeFlags[ItemData::typeArray[referent]].draw)
		{
			touch();
			DL_ItemNode::delayed_pop();
		}
	}
}

void Item::setNpcNum(short num)
{
	ItemData::npcNumArray[referent] = num;
}

void Item::setNpcArray(short num)
{
	ItemData::npcNumArray[referent] = num;
}

int Item::getNpcArray(void)
{
	return ItemData::npcNumArray[referent];
}

void Item::setMapArray(short num)
{
	ItemData::mapNumArray[referent] = num;
}

int Item::getMapArray(void)
{
	return ItemData::mapNumArray[referent];
}

int Item::getQ(void)
{
	return ItemData::qArray[referent];
}

// The q field means something different in each family.
int Item::getQuality(void)
{
	if (GlobalTypes.typeFlags[ItemData::typeArray[referent]].family == QUAL_FAMILY)
		return ItemData::qArray[referent];
	return 0;
}

void Item::setQuality(short quality)
{
	if (GlobalTypes.typeFlags[ItemData::typeArray[referent]].family == QUAL_FAMILY)
		ItemData::qArray[referent] = quality;
}

int Item::getUnkEggType(void)
{
	if (GlobalTypes.typeFlags[ItemData::typeArray[referent]].family == UNKEGG_FAMILY)
		return ItemData::qArray[referent];
	return 0;
}

void Item::setUnkEggType(short type)
{
	if (GlobalTypes.typeFlags[ItemData::typeArray[referent]].family == UNKEGG_FAMILY)
		ItemData::qArray[referent] = type;
}

int Item::getQuantity(void)
{
	int family = GlobalTypes.typeFlags[ItemData::typeArray[referent]].family;
	if (family == QUAN_FAMILY || family == REAGENT_FAMILY)
		return ItemData::qArray[referent];
	return 1;
}

void Item::setQuantity(short quantity)
{
	int family = GlobalTypes.typeFlags[ItemData::typeArray[referent]].family;
	if (family == QUAN_FAMILY || family == REAGENT_FAMILY)
	{
		ItemData::qArray[referent] = quantity;
		combine();
	}
}

int Item::getContents(void)
{
	if (GlobalTypes.typeFlags[ItemData::typeArray[referent]].family == CONTAINER_FAMILY)
		return ItemData::qArray[referent];
	return 0;
}

int Item::getContainer(void)
{
	if (ItemData::statusArray[referent] & CONTAINED)
		return ItemData::xArray[referent];
	return 0;
}

int Item::getRootContainer(void)
{
	Item container(getContainer());
	if (!container.isValid())
		return 0;
	Item next(container.getContainer());
	while (next.isValid())
	{
		container.set(next.referent);
		next = itemOf(container.getContainer());
	}
	return container.referent;
}

int Item::getGlobNum(void)
{
	if (GlobalTypes.typeFlags[ItemData::typeArray[referent]].family == GLOBEGG_FAMILY)
		return ItemData::qArray[referent];
	return 0;
}

void Item::setGlobNum(unsigned glob)
{
	if (GlobalTypes.typeFlags[ItemData::typeArray[referent]].family == GLOBEGG_FAMILY)
		ItemData::qArray[referent] = glob;
}

void Item::setQ(unsigned q)
{
	ItemData::qArray[referent] = q;
}

void Item::setContents(unsigned short contents)
{
	if (GlobalTypes.typeFlags[ItemData::typeArray[referent]].family == CONTAINER_FAMILY)
		ItemData::qArray[referent] = contents;
}

int Item::getFamily(void)
{
	return GlobalTypes.typeFlags[ItemData::typeArray[referent]].family;
}

// Reads one bit of the type's flag record by number.
char Item::getTypeFlag(short bit)
{
	unsigned char *flags = (unsigned char *)&GlobalTypes.typeFlags[ItemData::typeArray[referent]];
	return (flags[bit >> 3] >> (bit & 7)) & 1;
}

int Item::getStatus(void)
{
	return ItemData::statusArray[referent];
}

Boolean Item::legal_create(Type type, Frame frame, WorldPoint &loc)
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

Boolean Item::legal_create(Type type, Frame frame, unsigned short x, unsigned short y, unsigned short z)
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

Boolean Item::legal_create(Type type, Frame frame, Referent container, Quantity quantity)
{
	if (type >= NUM_TYPES)
		return FALSE;
	Container box(container);
	if (box.can_hold(type, quantity))
	{
		create(type, frame);
		pop(container);
		return TRUE;
	}
	return FALSE;
}

Boolean Item::create(Type type, Frame frame)
{
	if (type >= NUM_TYPES)
	{
		referent = 0;
		halt(__FILE__, 902);	// __LINE__ in the original file
		return FALSE;
	}
	referent = ItemCache::create();
	ItemData::typeArray[referent] = type;
	ItemData::frameArray[referent] = frame;
	int family = Item(referent).getFamily();
	if (family == UNKEGG_FAMILY || family == TELEPORTEGG_FAMILY)
		new EggHatcher(referent);
	return TRUE;
}

void Item::pop(unsigned short x, unsigned short y, unsigned char z)
{
	ItemCache::pop(x, y, z);
}

void Item::pop(Referent container)
{
	ItemCache::pop(container);
}

void Item::pop(void)
{
	ItemCache::pop();
}

void Item::popToEnd(Referent container)
{
	ItemCache::popToEnd(container);
}

void Item::push(void)
{
	ItemCache::push(referent);
}

void Item::destroy(void)
{
	if (!referent)
		return;
	if (isNpc())
		((Npc *)this)->freeNpcSlot();
	ItemCache::destroy(referent);
}

void Item::destroyContents(void)
{
	if (getFamily() == CONTAINER_FAMILY)
		Container(referent).destroyContents();
}

void Item::removeContents(void)
{
	if (getFamily() == CONTAINER_FAMILY)
		Container(referent).removeContents();
}

void Item::move(unsigned short x, unsigned short y, unsigned char z)
{
	if (z >= 251)
		halt(__FILE__, 993);	// __LINE__ in the original file
	ItemCache::push(referent);
	ItemCache::pop(x, y, z);
	justMoved();
}

void Item::move(Referent container)
{
	ItemCache::push(referent);
	ItemCache::pop(container);
	justMoved();
}

// Bit 0 of flags: leave the item where it is when the move fails.
Boolean Item::legal_move(WorldPoint &loc, unsigned short flags, unsigned short speed)
{
	MotionTracker tracker;
	if (speed == 0)
		speed = 4;
	if (tracker.tryMove(referent, loc, speed, 0))
	{
		tracker.makeItSo();
		return TRUE;
	}
	if (!(flags & 1))
		tracker.makeItSo();
	return FALSE;
}

// Drops the item into a container at a random spot in its gump.
Boolean Item::legal_move(Referent &container, unsigned short)
{
	Container box(container);
	if (box.can_hold(referent))
	{
		move(container);
		setGumpXY(urandom(32), urandom(32));
		return TRUE;
	}
	return FALSE;
}

Boolean Item::superlegal_move(WorldPoint &loc, unsigned short flags, unsigned short speed, unsigned short goal,
	Boolean *reachedGoal, Referent *hitItem)
{
	SuperMotionTracker tracker;
	if (speed == 0)
		speed = 20;
	Boolean moved = tracker.tryMove(referent, loc, speed, goal);
	if (hitItem)
		*hitItem = tracker.hitItem;
	if (moved)
	{
		if (reachedGoal)
			*reachedGoal = FALSE;
		grab();
		tracker.makeItSo();
		return TRUE;
	}
	if (reachedGoal)
	{
		if (tracker.targetFirst)
			*reachedGoal = TRUE;
		else
			*reachedGoal = FALSE;
	}
	if (!(flags & 1))
	{
		grab();
		tracker.makeItSo();
	}
	return FALSE;
}

void Item::move(WorldPoint &loc)
{
	ItemCache::push(referent);
	ItemCache::pop(loc.x, loc.y, loc.z);
}

// One step in one of the eight directions.
void Item::move(char dir)
{
	int x = getX();
	int y = getY();
	unsigned char z = getZ();
	ItemCache::push(referent);
	ItemCache::pop(x + dirToXOffset[dir], y + dirToYOffset[dir], z);
}

char Item::getDirToCoords(unsigned short x, unsigned short y)
{
	return getDirCoordToCoord(getX(), getY(), x, y);
}

int Item::getDirFromCoords(unsigned short x, unsigned short y)
{
	return (getDirToCoords(x, y) + 4) % 8;
}

int Item::getDirFromItem(Referent other)
{
	Item item(other);
	return getDirFromCoords(item.getX(), item.getY());
}

char Item::getDirToItem(Referent other)
{
	Item item(other);
	return getDirToCoords(item.getX(), item.getY());
}

// Footpad size in 32-unit squares (z in 8-unit steps); a flipped item swaps x and y.
void Item::getFootpad(short &x, short &y, short &z)
{
	Type type = ItemData::typeArray[referent];
	TypeFlag *flags = &GlobalTypes.typeFlags[type];
	if (getStatus() & FLIPPED)
	{
		y = flags->xSize;
		x = flags->ySize;
	}
	else
	{
		x = flags->xSize;
		y = flags->ySize;
	}
	z = flags->zSize;
}

// Footpads are anchored at the item's far corner: x, y and z are the
// maximum edges, and the footpad extends back from them.

// TRUE when the two boxes meet face to face without overlapping.
Boolean Item::touches(Referent other)
{
	Boolean touching = FALSE;
	if (!(ItemData::statusArray[referent] & CONTAINED) && !(ItemData::statusArray[other] & CONTAINED))
	{
		short fx, fy, fz;
		short ox, oy, oz;
		unsigned x1, y1, x2, y2;
		unsigned char z1, z2;

		getFootpad(fx, fy, fz);
		Item(other).getFootpad(ox, oy, oz);
		x1 = ItemData::xArray[referent];
		x2 = ItemData::xArray[other];
		fx <<= 5;
		ox <<= 5;
		if (x2 - ox <= x1 && x1 - fx <= x2)
		{
			if (x2 - ox == x1 || x1 - fx == x2)
				touching = TRUE;
			y1 = ItemData::yArray[referent];
			y2 = ItemData::yArray[other];
			fy <<= 5;
			oy <<= 5;
			if (y2 - oy <= y1 && y1 - fy <= y2)
			{
				if (y2 - oy == y1 || y1 - fy == y2)
					touching = TRUE;
				z1 = ItemData::zArray[referent];
				z2 = ItemData::zArray[other];
				fz <<= 3;
				oz <<= 3;
				if (z1 <= z2 + oz && z2 <= z1 + fz)
				{
					if (z1 == z2 + oz || z2 == z1 + fz)
						touching = TRUE;
					return touching;
				}
			}
		}
	}
	return FALSE;
}

Boolean Item::overlaps(Referent other)
{
	if (!(ItemData::statusArray[referent] & CONTAINED) && !(ItemData::statusArray[other] & CONTAINED))
	{
		short fx, fy, fz;
		short ox, oy, oz;
		unsigned x1, y1, x2, y2;
		unsigned char z1, z2;

		getFootpad(fx, fy, fz);
		Item(other).getFootpad(ox, oy, oz);
		x1 = ItemData::xArray[referent];
		x2 = ItemData::xArray[other];
		fx <<= 5;
		ox <<= 5;
		if (x2 - ox < x1 && x1 - fx < x2)
		{
			y1 = ItemData::yArray[referent];
			y2 = ItemData::yArray[other];
			fy <<= 5;
			oy <<= 5;
			if (y2 - oy < y1 && y1 - fy < y2)
			{
				z1 = ItemData::zArray[referent];
				z2 = ItemData::zArray[other];
				fz <<= 3;
				oz <<= 3;
				if (z1 < z2 + oz && z2 < z1 + fz)
					return TRUE;
			}
		}
	}
	return FALSE;
}

Boolean Item::overlapsXY(Referent other)
{
	if (!(ItemData::statusArray[referent] & CONTAINED) && !(ItemData::statusArray[other] & CONTAINED))
	{
		short fx, fy, fz;
		short ox, oy, oz;
		unsigned x1, y1, x2, y2;

		getFootpad(fx, fy, fz);
		Item(other).getFootpad(ox, oy, oz);
		x1 = ItemData::xArray[referent];
		x2 = ItemData::xArray[other];
		fx <<= 5;
		ox <<= 5;
		if (x2 - ox < x1 && x1 - fx < x2)
		{
			y1 = ItemData::yArray[referent];
			y2 = ItemData::yArray[other];
			fy <<= 5;
			oy <<= 5;
			if (y2 - oy < y1 && y1 - fy < y2)
				return TRUE;
		}
	}
	return FALSE;
}

// Resting exactly on top of the other item.
Boolean Item::isOn(Referent other)
{
	if (overlapsXY(other))
	{
		short ox, oy, oz;
		Item(other).getFootpad(ox, oy, oz);
		oz <<= 3;
		if (ItemData::zArray[referent] == ItemData::zArray[other] + oz)
			return TRUE;
	}
	return FALSE;
}

// On top of the other item with the whole footpad over it.
Boolean Item::isCompletelyOn(Referent other)
{
	if (!(ItemData::statusArray[referent] & CONTAINED) && !(ItemData::statusArray[other] & CONTAINED))
	{
		short fx, fy, fz;
		short ox, oy, oz;
		unsigned x1, y1, x2, y2;

		getFootpad(fx, fy, fz);
		Item(other).getFootpad(ox, oy, oz);
		x1 = ItemData::xArray[referent];
		x2 = ItemData::xArray[other];
		fx <<= 5;
		ox <<= 5;
		if (x1 <= x2 && x1 - fx >= x2 - ox)
		{
			y1 = ItemData::yArray[referent];
			y2 = ItemData::yArray[other];
			fy <<= 5;
			oy <<= 5;
			if (y1 <= y2 && y1 - fy >= y2 - oy)
			{
				oz <<= 3;
				if (ItemData::zArray[referent] == ItemData::zArray[other] + oz)
					return TRUE;
			}
		}
	}
	return FALSE;
}

Boolean Item::isAbove(Referent other)
{
	if (overlapsXY(other))
	{
		short ox, oy, oz;
		Item(other).getFootpad(ox, oy, oz);
		oz <<= 3;
		if (ItemData::zArray[referent] >= ItemData::zArray[other] + oz)
			return TRUE;
	}
	return FALSE;
}

Boolean Item::isUnder(Referent other)
{
	if (overlapsXY(other))
	{
		short fx, fy, fz;
		getFootpad(fx, fy, fz);
		fz <<= 3;
		if (ItemData::zArray[referent] + fz <= ItemData::zArray[other])
			return TRUE;
	}
	return FALSE;
}

// Raises (or lowers) the item by dz, carrying along everything resting
// on it. The riders are lifted into the ethereal void first, then put
// back one by one at their new height.
Boolean Item::ascend(short dz)
{
	Item item;
	int x1, y1;
	int x, y;
	int xs, ys, zs;
	int count = 0;
	TypeFlag riderFlags;
	WorldPoint loc;
	MotionTracker tracker;

	GlobalTypes.typeFlags[getType()].getWorldSize(xs, ys, zs);
	x = getX();
	y = getY();
	x1 = x - xs + 1;
	y1 = y - ys + 1;
	AreaItemFinder finder(x1, y1, x + 480, y + 480);
	while (finder.found())
	{
		item.set(finder.referent);
		finder.findNext();
		riderFlags = GlobalTypes.typeFlags[item.getType()];
		if (item.isOn(referent) && !riderFlags.fixed && riderFlags.weight)
		{
			item.push();
			count++;
		}
	}
	if (dz < 0)
		grab();
	if (!legal_move(WorldPoint(getX(), getY(), getZ() + dz), 0, 0))
	{
		while (count)
		{
			item.pop();
			count--;
		}
		return FALSE;
	}
	while (count)
	{
		item.referent = ItemCache::etherealTop;
		loc = item.getLoc();
		loc.z += dz;
		if (tracker.can_create(item.getType(), loc, 0))
			item.setZ(item.getZ() + dz);
		else
			item.fall();
		item.pop();
		count--;
	}
	return TRUE;
}

void Item::getPoint(WorldPoint &point)
{
	point.x = getX();
	point.y = getY();
	point.z = getZ();
}

// NPCs hold the low referents.
Boolean Item::isNpc(void)
{
	return referent != 0 && referent < 256;
}

Boolean Item::isInNpc(void)
{
	Referent r = referent;
	while (r && (Item(r).getStatus() & CONTAINED))
	{
		r = Item(r).getContainer();
		if (Item(r).isNpc())
			return TRUE;
	}
	return FALSE;
}

// Redraws the item where it stands.
void Item::touch(void)
{
	if (!(getStatus() & ETHEREAL))
	{
		DL_ItemNode::before_push(referent);
		DL_ItemNode::after_pop(referent);
	}
}

void Item::setStatus(short status)
{
	checkReferent(referent, __FILE__, 1523);	// __LINE__ in the original file
	ItemData::statusArray[referent] = status;
}

void Item::orStatus(short bits)
{
	ItemData::statusArray[referent] |= bits;
}

void Item::andStatus(short bits)
{
	ItemData::statusArray[referent] &= bits;
}

// The event functions run the item's usecode and return the new process.

int Item::look(void)
{
	unsigned short pid;
	if (spawnUnk(this, 1L << LOOK_EVENT, LOOK_EVENT, pid, 0))
		return pid;
	return 0;
}

// Using a dead body opens or closes its inventory.
int Item::use(void)
{
	unsigned short pid;
	if (isNpc())
	{
		Npc npc(referent);
		if (npc.isDead())
		{
			if (getStatus() & CONTAINER_GUMP_OPEN)
				closeGump();
			else
				openGump(12);
		}
		else
		{
			if (npc.isInCombat() && !npc.isAvatar())
				return 0;
			if (spawnUnk(this, 1L << USE_EVENT, USE_EVENT, pid, 0))
				return pid;
		}
	}
	else
	{
		if (spawnUnk(this, 1L << USE_EVENT, USE_EVENT, pid, 0))
			return pid;
		return 0;
	}
}

// The usecode gets the attacker in the low word and the damage type in the high word.
int Item::hit(Referent attacker, short type)
{
	long arg = type;
	unsigned short pid;
	arg <<= 16;
	arg |= attacker;
	if (spawnUnk(this, 1L << HIT_EVENT, HIT_EVENT, pid, arg))
		return pid;
	return 0;
}

int Item::gotHit(Referent attacker, short type)
{
	long arg = type;
	unsigned short pid;
	arg <<= 16;
	arg |= attacker;
	if (spawnUnk(this, 1L << GOTHIT_EVENT, GOTHIT_EVENT, pid, arg))
		return pid;
	return 0;
}

int Item::anim(void)
{
	unsigned short pid;
	if (spawnUnk(this, 1L << ANIM_EVENT, ANIM_EVENT, pid, 0))
		return pid;
	return 0;
}

SpeechProcess::SpeechProcess(void)
{
	setProcessType((ProcessType)0x246);
}

// Waits for the current speech sample to end, then plays the next one
// in the queue, skipping leading spaces.
void SpeechProcess::process(void)
{
	if (!speaking && speechEnded)
	{
		speechEnded = 0;
		int i = 2;
		while (speechQueue[i] == ' ')
			i++;
		if (speechQueue[i] == 0)
		{
			SoundManager->discard(255);
			pop(0);
			return;
		}
		if (!playSpeech(*(short *)speechQueue, speechQueue + 2))
		{
			SoundManager->discard(255);
			pop(0);
		}
	}
}

// Speaks the text if there is a sample for it, otherwise shows it
// over the item for a time that grows with its length.
int Item::bark(char *text)
{
	if (referent == 666 ||
		((ItemData::statusArray[referent] & IN_DISPLAY_LIST) && ItemData::zArray[referent] < Camera::roof()) ||
		(ItemData::statusArray[referent] & CONTAINED) ||
		GlobalTypes.typeFlags[ItemData::typeArray[referent]].draw)
	{
		if (playSpeech(referent, text))
		{
			SpeechProcess *speech = new SpeechProcess;
			return speech->pid;
		}
		if (referent == 666)
			return 0;
		int lines = strlen(text) / 32;
		if (lines == 0)
			lines++;
		int duration = (9 - (char)avatar.f_39 + 2) * lines >> 1;
		BarkGump *gump = new BarkGump(Dispatcher::base[Dispatcher::baseSP], referent, text, 1, duration);
		return gump->f_4e;
	}
	return 0;
}

// Things resting on this item lose their support: loose ones fall at
// once, the rest are watched until the usecode lets go of them.
void Item::grab(void)
{
	SurfaceItemFinder finder(referent, "$", 1, SURFACE_ON | SURFACE_UNDER);
	while (finder.found())
	{
		if (finder.where == SURFACE_ON)
			finder.fall();
		else if (!Kernel::isProcessTypeActive(finder.referent, (ProcessType)0xf1))
			new ReleaseTrigger(finder.referent);
		finder.findNext();
	}
}

int Item::release(void)
{
	unsigned short pid;
	if (spawnUnk(this, 1L << RELEASE_EVENT, RELEASE_EVENT, pid, 0))
		return pid;
	return 0;
}

int Item::equip(void)
{
	unsigned short pid;
	if (spawnUnk(this, 1L << EQUIP_EVENT, EQUIP_EVENT, pid, 0))
		return pid;
	return 0;
}

int Item::unequip(void)
{
	unsigned short pid;
	if (spawnUnk(this, 1L << UNEQUIP_EVENT, UNEQUIP_EVENT, pid, 0))
		return pid;
	return 0;
}

int Item::cachein(void)
{
	unsigned short pid;
	if (!ItemCache::mapIn)
		return 0;
	if (spawnUnk(this, 1L << CACHEIN_EVENT, CACHEIN_EVENT, pid, 0))
		return pid;
	return 0;
}

int Item::combine(void)
{
	unsigned short pid;
	if (spawnUnk(this, 1L << COMBINE_EVENT, COMBINE_EVENT, pid, 0))
		return pid;
	return 0;
}

int Item::calledFromAnim(void)
{
	unsigned short pid;
	if (spawnUnk(this, 1L << CALLEDFROMANIM_EVENT, CALLEDFROMANIM_EVENT, pid, 0))
		return pid;
	return 0;
}

// Runs when the item comes near enough to the Avatar to be simulated.
int Item::enterFastArea(void)
{
	unsigned short pid = 0;
	setStatus(getStatus() | FAST_AREA);
	if (isNpc())
	{
		if (Npc(referent).isDead())
			return 0;
	}
	int family = getFamily();
	if (family == UNKEGG_FAMILY || (family == TELEPORTEGG_FAMILY && getFrame() == 0))
		new EggHatcher(referent);
	if (!ItemCache::mapIn)
		return 0;
	if (isNpc())
	{
		Npc npc(referent);
		if (npc.isInCombat())
			npc.setInCombat();
	}
	if (isNpc() || GlobalTypes.typeFlags[ItemData::typeArray[referent]].noisy)
	{
		if (spawnUnk(this, 1L << ENTERFASTAREA_EVENT, ENTERFASTAREA_EVENT, pid, 0))
			return pid;
	}
	return 0;
}

int Item::leaveFastArea(void)
{
	Process *hatcher;
	unsigned short pid;
	setStatus(getStatus() & ~FAST_AREA);
	int family = getFamily();
	if (isNpc())
		Kernel::killProcess(referent, PT_ANY, PriorityClass(0x21));
	if (family == UNKEGG_FAMILY || (family == TELEPORTEGG_FAMILY && getFrame() == 0))
	{
		hatcher = Kernel::findValidProcess(referent, (ProcessType)0x20f);
		if (hatcher)
			hatcher->fail(0);
	}
	if (getStatus() & CONTAINER_GUMP_OPEN)
		use();
	if (!ItemCache::mapIn)
		return 0;
	if (isNpc())
	{
		if (Npc(referent).isDead())
			return 0;
	}
	if (isNpc() || GlobalTypes.typeFlags[ItemData::typeArray[referent]].noisy)
	{
		if (spawnUnk(this, 1L << LEAVEFASTAREA_EVENT, LEAVEFASTAREA_EVENT, pid, 0))
			return pid;
	}
	return 0;
}

int Item::cast(unsigned short spell)
{
	unsigned short pid;
	if (spawnUnk(this, 1L << CAST_EVENT, CAST_EVENT, pid, spell))
		return pid;
	return 0;
}

int Item::justMoved(void)
{
	unsigned short pid;
	if (spawnUnk(this, 1L << JUSTMOVED_EVENT, JUSTMOVED_EVENT, pid, 0))
		return pid;
	return 0;
}

int Item::animGetHit(unsigned short attacker)
{
	unsigned short pid;
	if (spawnUnk(this, 1L << ANIMGETHIT_EVENT, ANIMGETHIT_EVENT, pid, attacker))
		return pid;
	return 0;
}

// Runs the Avatar's guardian bark usecode.
int Item::guardianBark(int num)
{
	unsigned short pid;
	Item avatar(1);
	long arg = num;
	if (spawnUnk(&avatar, 1L << GUARDIANBARK_EVENT, GUARDIANBARK_EVENT, pid, arg))
		return pid;
	return 0;
}

int Item::AvatarStoleSomething(Referent thing)
{
	unsigned short pid;
	if (isNpc())
	{
		if (Npc(referent).isDead())
			return 0;
	}
	if (spawnUnk(this, 1L << AVATARSTOLESOMETHING_EVENT, AVATARSTOLESOMETHING_EVENT, pid, thing))
		return pid;
	return 0;
}

// Finds the item under a screen position, in an open container or on the map.
Boolean Item::findTarget(int x, int y)
{
	if (ContainerGump::findTarget(x, y, referent))
		return referent != 0;
	int target = DisplayList::find_target(x, y);
	DL_ItemNode *node = (DL_ItemNode *)MK_FP(DisplayListNodeSeg, target);
	if (target && node->nodetype() == 2)
	{
		referent = node->referent;
		return TRUE;
	}
	referent = 0;
	return FALSE;
}

void Item::getSliderInput(int min, int max, int step)
{
	::getSliderInput(referent, min, max, step);
}

void Item::ask(unsigned short answers)
{
	::ask(referent, answers);
}

void Item::openGump(short shape)
{
	TypeFlag flags = GlobalTypes.typeFlags[getType()];
	if (flags.family == CONTAINER_FAMILY)
	{
		if (!(getStatus() & CONTAINER_GUMP_OPEN))
		{
			new ContainerGump(*Dispatcher::base, referent, shape);
			return;
		}
		if (getStatus() & 0x8000)
		{
			ItemRelativeGump::notifyMoved(referent, (MovedState)0);
			ItemRelativeGump::notifyMoved(referent, (MovedState)1);
		}
	}
}

// Closes this container's gump, and the gumps of containers inside it.
void Item::closeGump(void)
{
	NewGump *gump = 0;
	while ((*Dispatcher::base)->newGumpList.hasType(CONTAINER_GUMP, gump))
	{
		if (((ContainerGump *)gump)->f_52 == 0 && ((ContainerGump *)gump)->containerItem.referent == referent)
		{
			gump->killMyself();
			TypeFlag flags = GlobalTypes.typeFlags[getType()];
			if (flags.family == CONTAINER_FAMILY && !isAvatar())
			{
				Item item(getContents());
				while (item.isValid())
				{
					if (item.getStatus() & CONTAINER_GUMP_OPEN)
						item.use();
					item = itemOf(item.getNext());
				}
			}
			return;
		}
	}
}

Boolean Item::isGumpOpen(void)
{
	NewGump *gump = 0;
	while ((*Dispatcher::base)->newGumpList.hasType(CONTAINER_GUMP, gump))
	{
		if (((ContainerGump *)gump)->f_52 == 0 && ((ContainerGump *)gump)->containerItem.referent == referent)
			return TRUE;
	}
	return FALSE;
}

// Brings a dead NPC back to life.
Enliven::Enliven(Referent npc)
{
	setRef(npc);
	setProcessType((ProcessType)0x229);
}

void Enliven::process(void)
{
	Npc npc(ref);
	if (npc.getStatus() & CONTAINER_GUMP_OPEN)
		npc.use();
	npc.clrWithstandDeath();
	npc.clrDead();
	npc.insertNpcData();
	pop(0);
}

// Kills the NPC's processes except the ones that must outlive a death.
void selectiveProcessKill(Referent npc)
{
	Kernel::killProcessNonSpecifiedPTypes(npc, PriorityClass(0xe0),
		0xf0, 0x208, 0xf2, 0x21d, 0x238, 0x220, 0x243, PT_END);
}

// Fails every animation the NPC is running, except a dying one and,
// unless killStandUp is set, a standing-up one. Returns TRUE if one of
// those is still running.
Boolean killAllButDeathAnim(Referent npc, Boolean killStandUp, Boolean killAll)
{
	AnimPrimitive *proc;
	AnimPrimitive *next;
	Boolean busy = FALSE;
	if (killAll)
		Kernel::killProcessNonSpecifiedPTypes(npc, PriorityClass(0xe0), 0xf0, PT_END);
	else
		selectiveProcessKill(npc);
	proc = (AnimPrimitive *)Kernel::getLinearExecutor(npc, (ProcessType)0xf0);
	while ((Boolean)(proc != 0))
	{
		next = (AnimPrimitive *)Kernel::getNextLinear();
		if (proc->anim == ANIM_DIE)
			busy = TRUE;
		else
		{
			if (!killStandUp && proc->anim == ANIM_STAND_UP)
				busy = TRUE;
			if (killStandUp || proc->anim != ANIM_STAND_UP)
				proc->fail(0);
		}
		proc = next;
	}
	return busy;
}

// A gump that holds a process until a key is pressed.
WaitForKeyGump::WaitForKeyGump(unsigned short pid) :
	NewGump(NewGumpId(0), 0, 0x20)
{
	f_34 = pid;
	rect = Rect(0, 0, 1, 1);
	dispatcher->pushBase(this);
	registerHotKeys(0xffff, 0);
}

void WaitForKeyGump::commandKeyboard(Event &)
{
	Process *waiting = Kernel::getProcess(f_34);
	waiting->flags &= ~1;
	dispatcher->popBase();
}

// Shows the Avatar's gravestone.
DeathGumpProcess::DeathGumpProcess(void)
{
	setProcessType((ProcessType)0x242);
}

static char *GraveText[3] = {"HERE LIES*THE AVATAR*REST IN PEACE", "CY-GIST*AVATAR", "RUHE IN*FRIEDEN*AVATAR"};

void DeathGumpProcess::process(void)
{
	AvatarDeathWatch *watch;
	GraveGump *gump = new GraveGump(*Dispatcher::base, 27, (FontType)11, avatar.referent, GraveText[avatar.language]);
	gump->clrExclusive();
	watch = (AvatarDeathWatch *)Kernel::findValidProcess(1, (ProcessType)0x228);
	watch->setGump(gump);
	pop(0);
}

// Holds the game while the Avatar dies: a pause, then the gravestone,
// then a key press ends it.
AvatarDeathWatch::AvatarDeathWatch(void)
{
	setRef(1);
	setProcessType((ProcessType)0x228);
	setDaemon();
	f_2a = 0;
	gump = 0;
	setAvatarInStasis(1);
	playCombatMusic(44);
	Wait *wait = new Wait(360, ref);
	DeathGumpProcess *grave = new DeathGumpProcess;
	new WaitForKeyGump(pid);
	wait->then(grave);
	wait->start();
	flags |= PROC_SUSPENDED;
}

void AvatarDeathWatch::process(void)
{
	if (gump)
	{
		gump->killMyself();
		gump = 0;
	}
	else
		Kernel::killProcess(ref, (ProcessType)0x242, PriorityClass(0x21));
	pop(0);
	realAvatarDeath();
}

// Keeps an NPC stunned until the process ends.
UnstunNpc::UnstunNpc(Referent npc)
{
	setRef(npc);
	setProcessType((ProcessType)0x22a);
	Npc stunned(ref);
	stunned.setStunned();
}

void UnstunNpc::pop(long result)
{
	Npc stunned(ref);
	stunned.clrStunned();
	Process::pop(result);
}

void UnstunNpc::fail(long result)
{
	Npc stunned(ref);
	stunned.clrStunned();
	Process::fail(result);
}

// The Avatar's damage type comes from the weapon in hand; a monster's
// from its record.
int Item::getDamageType(void)
{
	int monster;
	int type = 0;
	if (isAvatar())
	{
		Item weapon(avatar.getEquip(5));
		if (weapon.isValid())
			type = WeaponCache::getDamageType(weapon.getType());
		else
			type = 1;
	}
	else
	{
		monster = DataTable::getMonsterRecord(getType());
		if (monster)
			type = monsterRecord[monster].damageType;
	}
	return type;
}

inline Boolean musicPlaying(void) { return TheMusicProcess != 0; }

// Plays the victory fanfare when the Avatar's last enemy falls, then
// goes back to the area music.
void checkForVictoryMusic(void)
{
	if (!musicOn || !avatar.isInCombat())
		return;
	stackPlay(109);
	if (TheMusicProcess && musicPlaying())
		((Process *)TheMusicProcess)->process();
	stackPlay(98);
	if (TheMusicProcess && musicPlaying())
		((Process *)TheMusicProcess)->process();
}

// Damage done to the item by other (0 for none), coming from direction
// dir (8 for "from the attacker"). An NPC works out armour, to-hit and
// death itself; anything else runs its gotHit usecode, or else breaks,
// explodes or is knocked away.
void Item::receiveHit(Referent other, char dir, short damage, unsigned short type)
{
	int sfx;

	if (isNpc())
	{
		Npc npc(referent);
		if (npc.isDead())
			return;
		Npc attacker(other);
		Boolean dying = FALSE;
		Boolean hurt = FALSE;
		Boolean immortal = npc.isImmortal();
		Boolean invincible = npc.isInvincible();
		int hp = npc.getHp();
		int newHp = hp;
		Process *proc;
		unsigned x = npc.getX();
		unsigned y = npc.getY();
		unsigned char z = npc.getZ();
		AnimPrimitive *anim;
		Boolean kicked = FALSE;
		Boolean stunned = npc.isStunned();
		Wait *wait;
		int monster = DataTable::getMonsterRecord(getType());
		int attackerMonster;
		int damageType;
		int defense;
		int attackerDefense;
		Boolean blockable;
		Boolean armour;
		int effect;
		int ac;
		int total;

		if (isAvatar())
			defense = npc.getDefenseType();
		else if (monster)
			defense = monsterRecord[monster].defenseType;
		else
			defense = 0;

		if (attacker.isNpc())
		{
			attackerMonster = DataTable::getMonsterRecord(attacker.getType());
			if (type)
				damageType = type;
			else
				damageType = attacker.getDamageType();
			if (damage == 0)
				damage = attacker.getDamageAmount();
			if (attacker.getLastAnimSet() == ANIM_KICK)
				kicked = TRUE;
			if (attacker.isAvatar())
				AccumulateStrength(damage >> 2);
		}
		else
			damageType = type;

		// Defences block their damage types, but fire, undead and
		// piercing damage always get through.
		armour = TRUE;
		effect = damageType & ~defense;
		if (damageType & DMG_UNDEAD)
			effect |= DMG_UNDEAD;
		if (damageType & DMG_MAGIC)
			effect |= DMG_MAGIC;
		if (damageType & DMG_PIERCE)
			effect |= DMG_PIERCE;
		if ((defense & DMG_MAGIC) && !(effect & DMG_MAGIC))
			damage = 0;
		if ((damageType & DMG_BLADE) || (damageType & DMG_BLUNT) || (damageType & DMG_NORMAL))
			blockable = TRUE;
		else
			blockable = FALSE;

		if (damage && effect)
		{
			if (!(effect & DMG_BLADE) && (defense & DMG_PIERCE) && !(effect & DMG_FIRE) && !(effect & DMG_PIERCE))
				damage >>= 1;
			if ((effect & DMG_SLAYER) && OneIn(10))
				damage = 255;
			if ((effect & DMG_UNDEAD) && (defense & DMG_UNDEAD))
				damage * 2;	// meant to double the damage
		}
		else
			damage = 0;

		if (damage)
		{
			if ((effect & DMG_PIERCE) || ((effect & DMG_SLAYER) && damage == 255))
				armour = FALSE;
			if (armour)
			{
				ac = npc.getArmorClass() * 3;
				if (npc.isBlocking() && !stunned)
					damage -= npc.getStr() / 5;
				if (effect & DMG_FIRE)
					ac >>= 1;
				if (stunned)
					ac >>= 1;
				if (ac > 100)
					ac = 100;
				ac = 100 - ac;
				total = ac * damage;
				damage = total / 100;
				if (total % 100 && random(ac) >= random(100))
					damage++;
				if (damage < 0)
					damage = 0;
			}
		}

		if (damage && !(effect & DMG_PIERCE) && blockable && attacker.isNpc())
		{
			int attackDex = attacker.getAttackingDex();
			int defendDex = npc.getDefendingDex();
			Boolean hit = FALSE;
			if (stunned || urandom(attackDex) + 3 > urandom(defendDex))
				hit = TRUE;
			if (!hit && attackerMonster && !attacker.isAvatar())
			{
				attackerDefense = monsterRecord[attackerMonster].defenseType;
				if ((attackerDefense & DMG_PIERCE) && OneIn(4))
					hit = TRUE;
			}
			if (hit)
			{
				if (attacker.isAvatar())
				{
					int gain;
					if (defendDex > attackDex)
						gain = defendDex - attackDex;
					else
						gain = 1;
					AccumulateDexterity(gain * 2);
				}
			}
			else
				damage = 0;
		}

		if (damage)
		{
			hurt = TRUE;
			if (!immortal && !invincible)
				newHp = hp - damage;
			if (dir == 8)
				dir = npc.getDirToItem(attacker.referent);
			if (npc.isWithstandDeath() && newHp <= 0)
			{
				newHp = npc.getStr() * 2;
				npc.setHp(newHp);
				playSFX(59, 250);
				npc.clrWithstandDeath();
			}
		}

		// The Avatar bleeds.
		if (damage >= 4 && isAvatar() && blockable && attacker.isValid())
		{
			char side = attacker.getDirToItem(npc.referent);
			char height = rndRange(0, 24);
			if (side >= 0 && side <= 2)
				createSprite(620, 13, 25, 1, x, y, z + height);
			else if (side >= 4 && side <= 6)
				createSprite(620, 0, 12, 1, x, y, z + height);
		}

		if (newHp <= 0)
		{
			dying = TRUE;
			npc.setHp(0);
			npc.setDead();
			deathAnimCall(referent);
			if (getType() == 119 && !(effect & DMG_FIRE))
			{
				// This one gets up again after a while.
				if (!npc.isInCombat())
					npc.setInCombat();
				proc = Kernel::findValidProcess(referent, PT_ANY);
				npc.setWithstandDeath();
				while ((Boolean)(proc != 0))
				{
					if (proc->processType == 0xf0)
					{
						anim = (AnimPrimitive *)proc;
						if (!((anim->isLinearExecutor() && anim->anim == ANIM_DIE) || anim->anim == ANIM_STAND_UP))
							anim->fail(0);
					}
					else if (proc->processType != 0xf2)
						proc->fail(0);
					proc = Kernel::findNextValidProcess();
				}
				proc = Kernel::findValidProcess(referent, (ProcessType)0xf2);
				FallDown *fall = new FallDown(referent, dir);
				Wait *delay = new Wait(rndRange(2, 25) * 30, referent);
				GetUp *getUp = new GetUp(referent, 8);
				Enliven *enliven = new Enliven(referent);
				fall->then(delay)->then(enliven)->then(getUp)->then(proc);
				delay->start();
				return;
			}
			if (killAllButDeathAnim(referent, TRUE, TRUE))
			{
				hurt = FALSE;
				dying = FALSE;
			}
			if (!npc.isAvatar())
				checkForVictoryMusic();
			if (npc.isAvatar())
			{
				new AvatarDeathWatch;
				closeAllGumps();
			}
		}
		else
		{
			npc.setHp(newHp);
			if (getType() == 1)
			{
				if (hurt)
				{
					if (killAllButDeathAnim(referent, FALSE, FALSE))
					{
						hurt = FALSE;
						dying = FALSE;
					}
					else if (damage >= 6)
						dying = TRUE;
				}
			}
			else if (npc.isInCombat())
			{
				CombatBrain *brain = (CombatBrain *)Kernel::findValidProcess(referent, (ProcessType)0xf2);
				if (brain->f_36)
				{
					selectiveProcessKill(referent);
					Kernel::killProcess(referent, (ProcessType)0xf0, PriorityClass(0x21));
					if (attacker.isNpc())
						brain->setTarget(other);
					else if (other == 0 && referent != 1)
						brain->setTarget(1);
					if (brain->getTarget() == 1)
						;
					npc.callForHelp();
				}
				else if (monster && ((monsterRecord[monster].defenseType & DMG_PIERCE) ? TRUE : FALSE))
				{
					if (hurt)
					{
						if (OneIn(10) || kicked)
						{
							if (killAllButDeathAnim(referent, FALSE, FALSE))
							{
								hurt = FALSE;
								dying = FALSE;
							}
						}
						else
						{
							hurt = FALSE;
							dying = FALSE;
							Type shape = getType();
							if (AnimCache::isAnimInExistence(shape, ANIM_STUMBLE_BACK))
							{
								AnimStreamHeader header;
								AnimNode node;
								char facing = npc.getDir();
								header = AnimCache::getAnimStreamHeader(shape, ANIM_STUMBLE_BACK);
								for (unsigned i = 0; i < header.frames; i++)
								{
									AnimCache::getNode(shape, ANIM_STUMBLE_BACK, facing, i, &node);
									if ((sfx = ((AnimNodeFields &)node).sfx) != 0)
										playSFX(sfx);
								}
							}
						}
					}
				}
				else
				{
					if (hurt && killAllButDeathAnim(referent, FALSE, FALSE))
					{
						hurt = FALSE;
						dying = FALSE;
					}
				}
			}
			else
			{
				npc.setInCombat();
				CombatBrain *brain = (CombatBrain *)Kernel::findValidProcess(referent, (ProcessType)0xf2);
				if (attacker.isNpc())
					brain->setTarget(other);
				else if (other == 0 && referent != 1)
					brain->setTarget(1);
				if (brain->getTarget() == 1)
					;
				npc.callForHelp();
			}
		}

		if (dying)
		{
			if ((effect & DMG_FALLING) && npc.isAvatar())
				npc.doAnim(ANIM_FALL_BACK, dir, 0, 0);
			else if (npc.isAvatar() && !npc.isDead())
				dying = FALSE;
			else
				npc.doAnim(ANIM_DIE, dir, 0, 0);
			// A falling Avatar who survives gets up again.
			if (dying && !npc.isDead())
			{
				npc.doAnim(ANIM_STAND_UP, 8, 1, 0);
				if (!npc.isInCombat())
					npc.doAnim(ANIM_STAND, 8, 2, 0);
			}
		}
		else if (hurt)
		{
			if (npc.getLastAnimSet() == ANIM_BLOCK_START)
			{
				// Hit while blocking: lower and raise the shield again.
				npc.doAnim(ANIM_BLOCK_END, 8, 0, 0);
				npc.doAnim(ANIM_BLOCK_START, 8, 1, 0);
				unsigned char blockSounds[3] = {20, 21, 22};
				unsigned char hitSounds[2] = {50, 51};
				unsigned char sound;
				if (damage)
					sound = hitSounds[rndRange(0, 1)];
				else
					sound = blockSounds[rndRange(0, 2)];
				playSFX(sound, 100, npc.referent);
			}
			else
			{
				int pid;
				if ((effect & DMG_FALLING) && npc.isAvatar())
				{
					npc.doAnim(ANIM_FALL_BACK, dir, 0, 0);
					npc.doAnim(ANIM_STAND_UP, 8, 1, 0);
				}
				else
					npc.doAnim(ANIM_STUMBLE_BACK, dir, 0, 0);
				if (npc.isInCombat())
					pid = npc.doAnim(ANIM_COMBAT_STAND, 8, 2, 0);
				else
				{
					pid = npc.doAnim(ANIM_STAND, 8, 2, 0);
					if (effect & DMG_FALLING)
					{
						// Dazed: looks about before getting going.
						npc.doAnim(ANIM_LOOK_LEFT, 8, 3, 0);
						npc.doAnim(ANIM_STAND, 8, 4, 0);
						npc.doAnim(ANIM_LOOK_RIGHT, 8, 5, 0);
						npc.doAnim(ANIM_STAND, 8, 6, 0);
						npc.doAnim(ANIM_LOOK_LEFT, 8, 7, 0);
						npc.doAnim(ANIM_STAND, 8, 8, 0);
						npc.doAnim(ANIM_LOOK_RIGHT, 8, 9, 0);
						pid = npc.doAnim(ANIM_STAND, 8, 10, 0);
					}
				}
				if (pid)
				{
					// A kick stuns for a while, longer against a spiked shield.
					if (kicked && (!(defense & DMG_PIERCE) || OneIn(3)))
					{
						Process *unstun;
						int ticks;
						Item shield(npc.getEquip(4));
						Boolean spiked = FALSE;
						if (shield.isValid() && shield.getType() == 845)
							spiked = TRUE;
						unstun = Kernel::findValidProcess(referent, (ProcessType)0x22a);
						if (unstun)
							unstun->fail(0);
						unstun = new UnstunNpc(referent);
						proc = Kernel::getProcess(pid);
						ticks = damage * 8;
						ticks += 22;
						if (spiked)
						{
							if (!(defense & DMG_PIERCE))
								ticks += ticks >> 1;
						}
						else if (defense & DMG_PIERCE)
							ticks >>= 1;
						wait = new Wait(ticks, referent);
						wait->then(unstun)->then(proc);
						wait->start();
					}
				}
				else
					traceGet((TraceLevel)50, "Renegade anim.\n\r");
			}
		}
		if (npc.isDead())
			npc.setAnimLock();
	}
	else
	{
		long arg = other;
		unsigned short pid;
		if (!spawnUnk(this, 1L << GOTHIT_EVENT, GOTHIT_EVENT, pid, arg))
		{
			if (isExplosive())
			{
				explode();
				return;
			}
			if (getFamily() == BREAKABLE_FAMILY)
			{
				destroy();
				return;
			}
			if (!isFixed())
			{
				int weight = getWeight();
				if (weight)
				{
					// Light things fly further, hard hits throw higher.
					int spread = 255 - weight;
					spread >>= 6;
					int lift = damage >> 3;
					if (spread == 0)
						spread = 1;
					if (lift == 0)
						lift = 1;
					int speed = spread * lift;
					hurl(rndRange(-5, 5) * speed, rndRange(-5, 5) * speed, urandom(10) * lift, 2);
				}
			}
		}
	}
}

Boolean Item::isExplosive(void)
{
	return GlobalTypes.getTypeFlag(getType()).explode;
}

Boolean Item::isFixed(void)
{
	return GlobalTypes.getTypeFlag(getType()).fixed;
}

#define EXPLOSION_RANGE	160

// Blows the item up: shows the blast, removes the item and burns
// everything loose within range. Returns the blast sprite's process.
int Item::explode(void)
{
	AreaItemFinder finder;
	if (urandom(2))
		playSFX(31, 100);
	else
		playSFX(158, 100);
	int sprite = createSprite(578, 20, 34, 34, 1, 2, getX() + 128, getY() + 128, getZ());
	unsigned x = getX();
	unsigned y = getY();
	unsigned char z = getZ();
	destroy();
	finder.init((unsigned)getX() - EXPLOSION_RANGE, (unsigned)getY() - EXPLOSION_RANGE,
		getX() + EXPLOSION_RANGE, getY() + EXPLOSION_RANGE, "$", 1);
	while (finder.found())
	{
		Item item(finder.referent);
		finder.findNext();
		if (!item.isFixed() && dist(item.getX(), item.getY(), item.getZ(), x, y, z) <= EXPLOSION_RANGE)
			item.receiveHit(0, item.getDirToCoords(x, y), rndRange(6, 12), 12);
	}
	return sprite;
}

// TRUE when the item can reach the other one (or the given point near
// it) from where it stands: their footpads, grown by range, must
// overlap, and there must be a clear line between them.
Boolean Item::canReach(Referent other, short range, const WorldPoint *otherPoint, Boolean)
{
	Item target(other);
	Box mine;
	Box theirs;
	int xs, ys, zs;
	int oxs, oys, ozs;

	while (target.getStatus() & CONTAINED)
	{
		other = target.getContainer();
		target = itemOf(other);
	}
	if (touches(other))
		return TRUE;
	GlobalTypes.typeFlags[getType()].getWorldSize(xs, ys, zs);
	GlobalTypes.typeFlags[target.getType()].getWorldSize(oxs, oys, ozs);
	mine.x2 = getX();
	mine.y2 = getY();
	mine.x1 = mine.x2 - xs - range;
	mine.y1 = mine.y2 - ys - range;
	if (otherPoint)
	{
		theirs.x2 = otherPoint->x;
		theirs.y2 = otherPoint->y;
	}
	else
	{
		theirs.x2 = target.getX();
		theirs.y2 = target.getY();
	}
	theirs.x1 = theirs.x2 - oxs - range;
	theirs.y1 = theirs.y2 - oys - range;
	if (mine.overlaps(theirs))
	{
		MotionTracker tracker;
		WorldPoint from;
		WorldPoint to;

		// First try between the two corners, at a shared height.
		from = getLoc();
		if (otherPoint)
			to = *otherPoint;
		else
			to = target.getLoc();
		if (from.z < to.z && from.z + zs > to.z)
			from.z = to.z;
		else if (from.z < to.z + ozs)
			to.z = from.z;
		if (tracker.isClear(0, from, to, 4, other, 0, referent) || tracker.targetFirst)
			return TRUE;

		// Then between the two centres.
		if (otherPoint)
			to = *otherPoint;
		else
			to = target.getLoc();
		from = getLoc();
		from.x -= xs >> 1;
		from.y -= ys >> 1;
		to.x -= oxs >> 1;
		to.y -= oys >> 1;
		if (zs > 16)
			from.z += zs - 8;
		to.z += ozs >> 1;
		if (tracker.isClear(0, from, to, 4, other, 0, referent) || tracker.targetFirst)
			return TRUE;
	}
	return FALSE;
}

Boolean Item::canReach(Referent other, short range)
{
	return canReach(other, range, 0, TRUE);
}

void Item::preload(void)
{
	shapeHandler->get(getType(), getFrame());
}

// The gap between the two footpads, along whichever axis it is widest.
int Item::getRange(Referent other)
{
	Item target(other);
	int xs, ys, zs;
	int oxs, oys, ozs;
	int gap;
	int range;
	int x1, x2, y1, y2;
	int ox1, ox2, oy1, oy2;

	GlobalTypes.typeFlags[getType()].getWorldSize(xs, ys, zs);
	GlobalTypes.typeFlags[target.getType()].getWorldSize(oxs, oys, ozs);
	x2 = getX();
	y2 = getY();
	x1 = x2 - xs;
	y1 = y2 - ys;
	ox2 = target.getX();
	oy2 = target.getY();
	ox1 = ox2 - oxs;
	oy1 = oy2 - oys;
	range = 0;
	if ((gap = x1 - ox2) > range)
		range = gap;
	if ((gap = y1 - oy2) > range)
		range = gap;
	if ((gap = ox1 - x2) > range)
		range = gap;
	if ((gap = oy1 - y2) > range)
		range = gap;
	return range;
}

// The same, to a shape of the given type standing at (x, y).
int Item::getRange(Type type, unsigned short x, unsigned short y)
{
	int xs, ys, zs;
	int oxs, oys, ozs;
	int gap;
	int range;
	int x1, x2, y1, y2;
	int ox1, ox2, oy1, oy2;

	GlobalTypes.typeFlags[getType()].getWorldSize(xs, ys, zs);
	TypeFlag &shape = GlobalTypes.typeFlags[type];
	shape.getWorldSize(oxs, oys, ozs);
	x2 = getX();
	y2 = getY();
	x1 = x2 - xs;
	y1 = y2 - ys;
	ox2 = x;
	oy2 = y;
	ox1 = ox2 - oxs;
	oy1 = oy2 - oys;
	range = 0;
	if ((gap = x1 - ox2) > range)
		range = gap;
	if ((gap = y1 - oy2) > range)
		range = gap;
	if ((gap = ox1 - x2) > range)
		range = gap;
	if ((gap = oy1 - y2) > range)
		range = gap;
	return range;
}

// Monster and teleport eggs keep two bytes in q.
int Item::getQHi(void)
{
	int family = GlobalTypes.typeFlags[ItemData::typeArray[referent]].family;
	if (family == MONSTEREGG_FAMILY || (family == TELEPORTEGG_FAMILY && getFrame() == 0))
		return (short)ItemData::qArray[referent] >> 8;
	return 0;
}

void Item::setQHi(short hi)
{
	int family = GlobalTypes.typeFlags[ItemData::typeArray[referent]].family;
	if (family == MONSTEREGG_FAMILY || (family == TELEPORTEGG_FAMILY && getFrame() == 0))
	{
		ItemData::qArray[referent] &= 0xff;
		ItemData::qArray[referent] |= hi << 8;
	}
}

int Item::getQLo(void)
{
	int family = GlobalTypes.typeFlags[ItemData::typeArray[referent]].family;
	if (family == MONSTEREGG_FAMILY || family == TELEPORTEGG_FAMILY)
		return ItemData::qArray[referent] & 0xff;
	return 0;
}

void Item::setQLo(short lo)
{
	int family = GlobalTypes.typeFlags[ItemData::typeArray[referent]].family;
	if (family == MONSTEREGG_FAMILY || family == TELEPORTEGG_FAMILY)
	{
		ItemData::qArray[referent] &= 0xff00;
		ItemData::qArray[referent] |= (unsigned)lo & 0xff;
	}
}

void Item::shoot(WorldPoint &target, short speed, short gravity)
{
	MissileTracker missile(getLoc(), target, speed, gravity);
	missile.fire(referent);
}
