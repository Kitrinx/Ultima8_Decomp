// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: EDITEM.C

#include "..\UI\NEWGUMP.H"
#include "CRECT.H"
#include "DISPATCH.H"
#include "ITEM.H"
#include "TYPE.H"
#include "NPC.H"
#include "COMBIN.H"
#include "CONTAIN.H"
#include "ITEMDATA.H"
#include "MOTRAK.H"
#include "CAMERA.H"
#include "EDITEM.H"

// Inline in the shared headers when this file was built.
inline Point::Point(void) {}
inline Point::Point(short x, short y) { f_00 = x; f_02 = y; }
inline void Point::set(short x, short y) { f_00 = x; f_02 = y; }
inline void Rect::set(short x1, short y1, short x2, short y2) { Point::set(x1, y1); f_04 = x2; f_06 = y2; }
inline Rect::Rect(short x1, short y1, short x2, short y2) { set(x1, y1, x2, y2); }
inline void Item::pop(WorldPoint &p) { pop(p.x, p.y, p.z); }

EditorItem theEditorItem;

EditorItem::EditorItem(void)
{
	referent = 0;
	dragged.referent = 0;
	type = 0;
	frame = 0;
	none = 0;
	align = 0;
	isDefault = 0;
	raise = 0;
	f_0b = 0;
	offset = Point(0, 0);
}

void EditorItem::set(Type t, Frame f)
{
	NewGump *base = Dispatcher::base[Dispatcher::baseSP];
	type = t;
	frame = f;
	if (type == 0)
	{
		none = 1;
		return;
	}
	dispatcher->send(Event((EventType)0x40, type, frame, 0x88, base, base, 0));
	update();
}

// Checks that the item can be made at its location.
void EditorItem::update(void)
{
	if (isValid())
		destroy();
	if (none)
	{
		if (isValid())
			referent = 0;
		return;
	}
	else
	{
		WorldPoint p(locX, locY, locZ);
		if (raise)
		{
			p.x += locZ * 4;
			p.y += locZ * 4;
		}
		if (align)
		{
			p.x |= 0x1f;
			p.y |= 0x1f;
		}
		if (!legal_create(type, frame, p))
			referent = 0;
	}
}

void EditorItem::dragFromContainer(void)
{
	if (dragged.getStatus() & CONTAINED)
	{
		NewGump *base = Dispatcher::base[Dispatcher::baseSP];
		Item container = dragged.getContainer();
		if (dragged.getStatus() & 0x200)
			dragged.unequip();
		if (container.getStatus() & CONTAINER_GUMP_OPEN)
			dispatcher->send(Event((EventType)0x40, container.referent, 0, 0x94, base, base, 0));
	}
}

// Drops the dragged item in the world near x, y, searching outwards for a free spot.
char EditorItem::move(short x, short y)
{
	Boolean moved = 0;
	WorldPoint p;
	f_1c = 0;
	if (none)
	{
		if (dragged.isValid())
		{
			MotionTracker tracker;
			dragFromContainer();
			dragged.push();
			calculateLocation(x, y, p);
			char tries = 10;
			Rect r(p.x, p.y, p.x, p.y);
			while (!moved && tries)
			{
				for (p.x = r.f_00; p.x <= r.f_04; p.x += 4)
				{
					p.y = r.f_02;
					if (tracker.can_create(dragged.getType(), p, 0))
					{
						moved = 1;
						break;
					}
					p.y = r.f_06;
					if (tracker.can_create(dragged.getType(), p, 0))
					{
						moved = 1;
						break;
					}
				}
				for (p.y = r.f_02 + 4; p.y <= r.f_06 - 4 && !moved; p.y += 4)
				{
					p.x = r.f_00;
					if (tracker.can_create(dragged.getType(), p, 0))
					{
						moved = 1;
						break;
					}
					p.x = r.f_04;
					if (tracker.can_create(dragged.getType(), p, 0))
					{
						moved = 1;
						break;
					}
				}
				if (!moved)
				{
					tries--;
					r.f_00 -= 4;
					r.f_02 -= 4;
					r.f_04 += 4;
					r.f_06 += 4;
				}
			}
			if (!moved)
			{
				WorldPoint q = p;
				for (int i = 0; p.z - i > p.z && i < 24; i++)
				{
					q.z = p.z - i;
					if (tracker.can_create(dragged.getType(), q, 0))
					{
						p = q;
						moved = 1;
						break;
					}
				}
			}
			if (moved)
			{
				dragged.pop(p.x, p.y, p.z);
				moved = 1;
				dropX = x;
				dropY = y;
			}
			else
				dragged.pop();
		}
	}
	else
	{
		if (isValid())
			destroy();
		ScreenToWorldCoords(x, y, (unsigned short &)p.x, (unsigned short &)p.y, p.z, 0, 0);
		create(type, frame);
		pop(p.x, p.y, p.z);
		moved = 1;
	}
	return moved;
}

// Drops the item into a container gump at x, y.
char EditorItem::move(unsigned short container, short x, short y)
{
	Boolean moved = 0;
	dropX = dropY = -1;
	if (none)
	{
		if (dragged.isValid())
		{
			avatar.unEquip(7);
			if (Container(container).can_hold(dragged.referent))
			{
				if (!(dragged.getStatus() & CONTAINED) || dragged.getContainer() != container)
					dragFromContainer();
				dragged.push();
				dragged.popToEnd(container);
				dragged.setGumpXY(x, y);
				moved = 1;
			}
		}
	}
	else
	{
		if (isValid())
			destroy();
		create(type, frame);
		if (Container(container).can_hold(referent))
		{
			if (!(getStatus() & CONTAINED) || getContainer() != container)
			{
				NewGump *base = Dispatcher::base[Dispatcher::baseSP];
				Item old = getContainer();
				if (old.getStatus() & CONTAINER_GUMP_OPEN)
					dispatcher->send(Event((EventType)0x40, old.referent, 0, 0x94, base, base, 0));
			}
			popToEnd(container);
			setGumpXY(x, y);
			moved = 1;
		}
	}
	return moved;
}

void EditorItem::click(void)
{
	setDefault();
	if (isDefault)
		orStatus(0x1000);
	else
		fall();
	referent = 0;
}

void EditorItem::setDefault(void)
{
	NewGump *base = Dispatcher::base[Dispatcher::baseSP];
	int family = getFamily();
	if (family == 6 || family == 0)
		return;
	if (family == 2 || family == 9)
		setQ(1);
	else
		setQ(0);
	if (family == 4 || family == 7 || family == 8)
		return;
	dispatcher->send(Event((EventType)0x40, 0, 0, 0x96, base, base, 0));
}

void EditorItem::startDrag(unsigned short item)
{
	dropX = dropY = -1;
	dragged.referent = 0;
	avatar.unEquip(7);
	if (item)
	{
		Referent container = Item(item).getContainer();
		if (container && container < 0x100)
			Item(item).unequip();
		dragged = item;
	}
}

void EditorItem::stopDrag(void)
{
	if (dragged.isValid())
	{
		int family;
		TypeFlag flags = GlobalTypes.typeFlags[dragged.getType()];
		if ((family = flags.family) == 2 || family == 9)
		{
			if (dropX >= 0 && dropY >= 0)
			{
				dragged.push();
				Item target;
				if (target.findTarget(dropX, dropY))
				{
					if (target.getType() == dragged.getType())
					{
						if (target.getStatus() & CONTAINED)
						{
							dragged.pop(target.getContainer());
							dragged.setGumpXY(target.getGumpX(), target.getGumpY());
						}
						else
							dragged.pop(target.getLoc());
						((Combinable &)Item(dragged.referent)).combine(target.referent);
					}
					else
						dragged.pop();
				}
				else
					dragged.pop();
			}
		}
		Referent container = dragged.getContainer();
		if (container && container < 0x100)
		{
			Item weapon = avatar.getEquip(7);
			if (Item(avatar.getEquip(6)).isValid() && weapon.isValid())
			{
				avatar.unEquip(7);
				weapon.push();
				weapon.popToEnd(avatar.getEquip(6));
				weapon.setGumpXY(0, 0);
			}
			NewGump *base = Dispatcher::base[Dispatcher::baseSP];
			if (Item(container).getStatus() & CONTAINER_GUMP_OPEN)
				dispatcher->send(Event((EventType)0x40, container, 0, 0x94, base, base, 0));
		}
		if (!(dragged.getStatus() & CONTAINED))
			dragged.fall();
		dragged.referent = 0;
	}
}

void EditorItem::calculateLocation(short x, short y, WorldPoint &p)
{
	FastItem hit(0);
	unsigned char found;
	ScreenToWorldCoords(x, y, (unsigned short &)p.x, (unsigned short &)p.y, p.z, &hit, &found);
	f_1c = hit.referent;
	ScreenToWorldCoords(x - offset.f_00, y - offset.f_02, (unsigned short &)p.x, (unsigned short &)p.y);
	p.x += p.z * 4;
	p.y += p.z * 4;
	if (hit.isValid())
	{
		TypeFlag flags = GlobalTypes.typeFlags[dragged.getType()];
		if (!found)
		{
			ScreenToWorldCoords(x - offset.f_00, y - offset.f_02, (unsigned short &)p.x, (unsigned short &)p.y, p.z, &hit, &found);
			if (found == 0 || ItemData::xArray[hit.referent] > p.x || p.y < ItemData::yArray[hit.referent])
			{
				int sizeX, sizeY, sizeZ;
				flags.getWorldSize(sizeX, sizeY, sizeZ);
				int size = ItemData::xArray[hit.referent] == p.x ? sizeY : sizeX;
				p.x += size;
				p.y += size;
				p.z += size >> 2;
			}
		}
	}
}
