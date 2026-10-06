#ifndef COLBUF_H
#define COLBUF_H

#include "ITEM.H"
#include "ITEMDATA.H"

class TypeFlag;

// Collision flags. COLLIDE_ANY marks a shape that takes up space at all;
// scanArea drops it and returns -1 for an area off the map.
#define COLLIDE_SOLID		0x0001
#define COLLIDE_SEA			0x0002
#define COLLIDE_LAND		0x0004
#define COLLIDE_ROOF		0x0008
#define COLLIDE_DAMAGING	0x0020
#define COLLIDE_ANY			0x8000

// An item handle that can be pointed at another item.
class CollisionItem : public Item
{
public:
	CollisionItem(void) {}
	CollisionItem(Referent r) { referent = r; }
	CollisionItem &operator=(Referent r) { referent = r; return *this; }
	unsigned short getType(void) { return ItemData::typeArray[referent]; }
	unsigned short getStatus(void) { return ItemData::statusArray[referent]; }
};

class CollisionBuffer
{
public:
	static int volumeFlags(TypeFlag &);
	static int surfaceFlags(TypeFlag &);
	static int get(unsigned short, unsigned short, unsigned char);
	static int getCell(short, short, short);
	static int scanAreaCell(short, short, short, short, short, short);
	static int scanArea(unsigned short, unsigned short, unsigned short, unsigned short, unsigned char, unsigned char);
	static Boolean isSupported(Referent);
};

#endif
