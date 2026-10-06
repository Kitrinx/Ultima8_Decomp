#ifndef EDITEM_H
#define EDITEM_H

#include "ITEM.H"
#include "CRECT.H"

class WorldPoint;

// The item the map editor places or drags with the mouse.
class EditorItem : public Item
{
public:
	Item dragged;
	Type type;
	Frame frame;
	unsigned char none;		// no type chosen: the mouse drags existing items
	unsigned char align;	// snap to the 32-unit grid
	unsigned char raise;	// lift by the item's z
	unsigned long f_0b;
	unsigned locX;
	unsigned locY;
	unsigned char locZ;
	Point offset;
	int dropX;
	int dropY;
	Referent f_1c;
	unsigned char isDefault;

	EditorItem(void);
	void set(Type, Frame);
	void update(void);
	void dragFromContainer(void);
	char move(short, short);
	char move(unsigned short, short, short);
	void click(void);
	void setDefault(void);
	void startDrag(unsigned short);
	void stopDrag(void);
	void calculateLocation(short, short, WorldPoint &);
};

#endif
