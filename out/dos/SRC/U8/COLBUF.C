// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r- -y
// name: COLBUF.C

#include "ITEM.H"
#include "ITEMCACH.H"
#include "ITEMFIND.H"
#include "TYPE.H"
#include "COLBUF.H"

// A shape's size in world units: 32 per x/y cell, 8 per z step.
// Flat shapes that are not fixed still count as one unit tall.
inline void footprint(TypeFlag &type, int &xd, int &yd, int &zd)
{
	xd = type.xSize * 32;
	yd = type.ySize * 32;
	zd = type.zSize * 8;
	if (zd == 0 && !type.fixed)
		zd = 1;
}

int CollisionBuffer::volumeFlags(TypeFlag &type)
{
	int flags = COLLIDE_ANY;
	if (type.solid) flags |= COLLIDE_SOLID;
	if (type.damaging) flags |= COLLIDE_DAMAGING;
	if (type.roof) flags |= COLLIDE_ROOF;
	return flags;
}

int CollisionBuffer::surfaceFlags(TypeFlag &type)
{
	int flags = 0;
	if (type.sea) flags |= COLLIDE_SEA;
	if (type.land) flags |= COLLIDE_LAND;
	return flags;
}

int CollisionBuffer::get(unsigned short x, unsigned short y, unsigned char z)
{
	// World coordinates to cells.

	x >>= 5;
	y >>= 5;
	z >>= 3;

	return scanAreaCell(x, y, x, y, z, z);
}

int CollisionBuffer::getCell(short x, short y, short z)
{
	return scanAreaCell(x, y, x, y, z, z);
}

int CollisionBuffer::scanAreaCell(short x1, short y1, short x2, short y2, short z1, short z2)
{
	if (x1 < 0 || y1 < 0 || x2 < 0 || y2 < 0)
		return -1;

	unsigned short worldX1 = x1 << 5;
	unsigned short worldY1 = y1 << 5;
	unsigned short worldX2 = (x2 << 5) + 31;
	unsigned short worldY2 = (y2 << 5) + 31;
	unsigned char worldZ1 = z1 << 3;
	unsigned char worldZ2 = z2 << 3 + 7;

	return scanArea(worldX1, worldY1, worldX2, worldY2, worldZ1, worldZ2);
}

int CollisionBuffer::scanArea(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, unsigned char z1, unsigned char z2)
{
	int flags = 0;

	if ((short)x1 < 0 || (short)y1 < 0 || (short)x2 < 0 || (short)y2 < 0)
		return -1;

	if (z1 == 0 && z2 == 255)
		// The whole height of the map.
		return 0;

	int xd, yd, zd;

	CollisionItem item;
	TypeFlag type;
	AreaItemFinder finder(x1, y1, x2 + 256, y2 + 256);
	while (finder.found())
	{
		item = finder.referent;
		if (ItemData::xArray[item.referent] >= x1 && ItemData::yArray[item.referent] >= y1 && ItemData::zArray[item.referent] <= z2)
		{
			type = GlobalTypes.typeFlags[ItemData::typeArray[item.referent]];
			footprint(type, xd, yd, zd);
			if (ItemData::xArray[item.referent] - xd + 1 <= x2 && ItemData::yArray[item.referent] - yd + 1 <= y2 && ItemData::zArray[item.referent] + zd - 1 >= z1)
			{
				flags |= volumeFlags(type) & ~COLLIDE_ANY;
			}
		}
		finder.findNext();
	}
	return flags;
}

Boolean CollisionBuffer::isSupported(Referent referent)
{
	CollisionItem item(referent);
	unsigned char z = ItemData::zArray[item.referent];
	if (z == 0 || ItemData::statusArray[item.referent] & 8)
		// On the ground, or inside a container.
		return TRUE;

	int supportFlags = 0;
	unsigned short x = ItemData::xArray[item.referent];
	unsigned short y = ItemData::yArray[item.referent];
	TypeFlag type = GlobalTypes.typeFlags[ItemData::typeArray[item.referent]];
	int needFlags = volumeFlags(type);
	int xd, yd, zd;
	footprint(type, xd, yd, zd);
	unsigned short left = x - xd + 1;
	unsigned short top = y - yd + 1;
	AreaItemFinder finder(left, top, x + 256, y + 256);
	while (finder.found())
	{
		item = finder.referent;
		if (item.referent != referent && ItemData::xArray[item.referent] >= left && ItemData::yArray[item.referent] >= top && ItemData::zArray[item.referent] <= z)
		{
			type = GlobalTypes.typeFlags[ItemData::typeArray[item.referent]];
			footprint(type, xd, yd, zd);
			if (ItemData::xArray[item.referent] - xd + 1 <= x && ItemData::yArray[item.referent] - yd + 1 <= y && ItemData::zArray[item.referent] + zd == z)
			{
				supportFlags |= volumeFlags(type) & ~COLLIDE_ANY;
			}
		}
		finder.findNext();
	}
	return (supportFlags & needFlags) != 0;
}
