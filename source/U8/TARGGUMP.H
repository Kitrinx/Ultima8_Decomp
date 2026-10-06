#ifndef TARGGUMP_H
#define TARGGUMP_H

#include "DISPATCH.H"

class Event;
class Item;
class WorldPoint;

class TargetGump : public NewGump
{
public:
	Item *item;
	unsigned char clicks;
	unsigned char findItem;		// ask the item to find what was clicked
	unsigned char *flag;
	unsigned char toScreen;		// just report the world x and y
	unsigned short *xp;
	unsigned short *yp;
	WorldPoint *point;

	TargetGump(Item *, NewGump *);
	TargetGump(WorldPoint *, Item *, unsigned char *, NewGump *);
	TargetGump(unsigned short *, unsigned short *, NewGump *);
	virtual ~TargetGump(void);
	virtual void commandMouseLeft(Event &);
	virtual void commandKeyboard(Event &);
	void init(void);
};

extern unsigned char U8GumpColorMap[32];

#endif
