#ifndef IRGUMP_H
#define IRGUMP_H

#include "BASECAM.H"
#include "KERNEL.H"
#include "WAIT.H"
#include "ITEM.H"
#include "TYPE.H"

class Event;
class NewGump;
class Rect;

enum IRGumpType
{
	IRGumpType_0
};

// What the gump follows: 0 and 1 its item, 2 the avatar, 3 the camera.
// The name is a guess that reproduces the constructor's hashed symbol.
enum IRGumpMove
{
	IRGumpMove_0
};

enum MovedState
{
	MovedState_0
};

class GumpNotifier : public Wait
{
public:
	NewGumpId path[10];		// ids from the base gump down to the target
	unsigned depth;
	unsigned char f_70;
	unsigned char active;

	GumpNotifier(unsigned short, NewGump *, unsigned short, ProcessType, unsigned char);
	virtual ~GumpNotifier(void);
	virtual void restart(void);
	virtual void timerDone(void);
};

class ItemRelativeGump : public PanelGump
{
public:
	short dx;			// offset from the followed point on screen
	short dy;
	WorldPoint loc;
	IRGumpMove moveType;
	unsigned f_4e;		// pid of the GumpNotifier closing the gump
	unsigned delay;
	unsigned f_52;		// IRGumpType
	Referent item;

	ItemRelativeGump(IRGumpType, NewGump *, Rect, unsigned short, IRGumpMove, short, short, unsigned short);
	virtual ~ItemRelativeGump(void);
	virtual void commandMouseRight(Event &);
	virtual void commandMouseBoth(Event &);
	virtual void killMyself(void);
	virtual void move(MovedState);
	static void notifyMoved(unsigned short, MovedState);
	void sort(void);
	static void notifyDestroyed(unsigned short);
};

void loadGumps(void);
void saveGumps(void);

extern char *GumpSaveFile;

#endif
