// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: ..\ITEM\CONTAIN.C

#include "CRECT.H"
#include "ITEM.H"
#include "NPC.H"
#include "TYPE.H"
#include "ITEMDATA.H"
#include "SHAPHAND.H"
#include "CONTAIN.H"

inline Point::Point(void) {}
inline Point::Point(short x, short y) { f_00 = x; f_02 = y; }

extern "C" char Collision(void *, Point, Point);

// The item being dragged, whose weight a container must not count twice.
unsigned dragWeightHack = 0;

// A handle that reads the item tables directly.
class ContentItem : public Item
{
public:
	ContentItem(Referent r) { referent = r; }
	Referent next(void) { return ItemData::nextArray[referent]; }
	TypeFlag &typeFlag(void) { return GlobalTypes.typeFlags[ItemData::typeArray[referent]]; }
};

// A handle made from an Item.
class HeldItem : public Item
{
public:
	HeldItem(Item item) { referent = item.referent; }
};

// True when item is somewhere inside this container.
Boolean Container::holds(Referent item)
{
	HeldItem i(item);

	if (ItemData::statusArray[i.referent] & CONTAINED)
	{
		while ((ItemData::statusArray[i.referent] & CONTAINED) && i.getContainer() != referent)
			i = HeldItem(i.getContainer());
		return i.getContainer() == referent;
	}
	return FALSE;
}

Boolean Container::can_hold(Referent item)
{
	if (Item(item).isNpc())
		return FALSE;
	if (holds(item))
		return TRUE;
	if (referent == item || Container(item).holds(referent))
		return FALSE;
	if (dragWeightHack && (referent == dragWeightHack || Container(dragWeightHack).holds(referent)))
		return FALSE;
	int equipType;
	int weight;

	if (isNpc())
	{
		int unused;

		weight = Container(item).getContentsWeight();
		weight += Item(item).getWeight();
		Npc npc(referent);
		int limit = npc.getStr() * 40 - getContentsWeight();
		if (weight > limit)
			return FALSE;
		TypeFlag flags = ContentItem(item).typeFlag();
		if ((equipType = flags.equipType) != 0 && !Item(avatar.getEquip(equipType - 1)).isValid())
			return TRUE;
		Container backpack(avatar.getEquip(6));
		if (backpack.isValid())
			return backpack.can_hold(item);
		return FALSE;
	}
	else
	{
		int unused;

		weight = Container(item).getContentsWeight();
		weight += Item(item).getWeight();
		if (tooHeavy(weight))
			return FALSE;
		int volume = Container(item).getTotalContentsVolume();
		int room = getCapacity() - getContentsVolume();
		if (room >= volume)
		{
			if (GlobalTypes.typeFlags[getType()].volume == 0 && (getStatus() & CONTAINED))
				return Container(getContainer()).can_hold(item);
			return TRUE;
		}
		return FALSE;
	}
}

// Room for quantity items of a type, counting quantity and reagent piles by their units.
Boolean Container::can_hold(unsigned short type, short quantity)
{
	int volume;
	TypeFlag flags = GlobalTypes.typeFlags[type];

	if (flags.family == QUAN_FAMILY)
	{
		if (tooHeavy((flags.weight * quantity + 9) / 10))
			return FALSE;
		volume = (flags.volume * quantity + 99) / 100;
	}
	else if (flags.family == REAGENT_FAMILY)
	{
		if (tooHeavy(flags.weight * quantity))
			return FALSE;
		volume = (flags.volume * quantity + 9) / 10;
	}
	else
	{
		if (tooHeavy(flags.weight * 10))
			return FALSE;
		if (flags.family == CONTAINER_FAMILY && flags.volume == 0)
			volume = 1;
		else
			volume = flags.volume;
	}
	if (getCapacity() - getContentsVolume() >= volume)
	{
		if (GlobalTypes.typeFlags[getType()].volume == 0 && (getStatus() & CONTAINED))
			return Container(getContainer()).can_hold(type, quantity);
		return TRUE;
	}
	return FALSE;
}

// True when weight more would be too much for the npc carrying this container.
Boolean Container::tooHeavy(short weight)
{
	Container top(referent);

	while (top.isValid() && (top.getStatus() & CONTAINED))
		top = Container(top.getContainer());
	if (top.isValid() && top.isNpc())
	{
		Npc npc(top.referent);
		int limit = npc.getStr() * 40 - top.getContentsWeight();
		if (weight > limit)
			return TRUE;
	}
	return FALSE;
}

// The item whose shape covers gump position x, y, or 0.
int Container::findItem(short x, short y)
{
	Point pos, click;
	HeldItem i(getContents());

	while (i.isValid())
	{
		pos = Point(i.getGumpX(), i.getGumpY());
		click = Point(x, y);
		if (Collision(shapeHandler->get(ItemData::typeArray[i.referent], i.getFrame()), pos, click))
			break;
		i = HeldItem(ItemData::nextArray[i.referent]);
	}
	return i.referent;
}

// Empties the container into its own container, or onto the map where it is.
void Container::removeContents(void)
{
	if (getStatus() & CONTAINED)
	{
		while (getContents())
		{
			Item item(getContents());
			item.move((unsigned short)getContainer());
			item.setGumpXY(getGumpX(), getGumpY());
		}
	}
	else
		while (getContents())
			Item(getContents()).move(getLoc());
}

void Container::destroyContents(void)
{
	while (getContents())
		Item(getContents()).destroy();
}

int Container::getContentsWeight(void)
{
	int weight = 0;
	int item;

	if (referent == dragWeightHack)
		return 0;
	if (getType() == 79)
		return 0;
	for (item = getContents(); item; item = ContentItem(item).next())
	{
		weight += Container(item).getContentsWeight();
		weight += Item(item).getWeight();
	}
	return weight;
}

int Container::getTotalContentsVolume(void)
{
	int volume = 0;
	int item;

	for (item = getContents(); item; item = ContentItem(item).next())
		volume += Container(item).getTotalContentsVolume();
	if (getVolume() > volume)
		volume = getVolume();
	return volume;
}

int Container::getContentsVolume(void)
{
	int volume = 0;
	int item;

	for (item = getContents(); item; item = ContentItem(item).next())
		volume += Container(item).getTotalContentsVolume();
	return volume;
}

int Container::countItems(void)
{
	int count = 0;
	int item;

	for (item = getContents(); item; item = ContentItem(item).next())
		count += Container(item).countItems() + 1;
	return count;
}
