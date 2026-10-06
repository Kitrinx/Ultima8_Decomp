#ifndef WORLD_H
#define WORLD_H

#include "..\GLIB\PROCESS.H"
#include "DISPATCH.H"
#include "HARDWARE.H"
#include "TEXTGUMP.H"
#include "ITEM.H"
#include "TYPE.H"

class Event;
class InputWindowGump;
class Item;
class TargetGump;

class InputWindowGump : public WindowGump
{
public:
	InputWindowGump(NewGumpId id, Rect r, unsigned char shown) : WindowGump(id, 0, r, 0x1e, shown, 0) {}
	unsigned char getInt(int &);
	unsigned char getString(char *);
	unsigned char getChar(char &);
};

class CheatWindow : public InputWindowGump
{
public:
	Item f_3e;
	unsigned char f_40;
	WorldPoint where;

	CheatWindow(long);
	virtual void commandPrivate(Event &);
	virtual void commandKeyboard(Event &);
	virtual void draw(short, short);
	int resetMenu(void);
	void buildMenu(void);
	void displayItemStats(void);
};

class TestScreen : public InputWindowGump
{
public:
	int language;
	HardwareChecker hardwareChecker;

	TestScreen(void);
	virtual void commandPrivate(Event &);
	virtual void draw(short, short);
};

class BogusProcess : public Process
{
public:
	virtual void process(void);
};

class World : public NewGump
{
public:
	Item f_34;
	Point f_36;
	unsigned char f_3a;
	unsigned char pad_3b[4];
	unsigned f_3f;
	unsigned f_41;
	unsigned char f_43;
	WorldPoint dragStart;
	WorldPoint dragLast;
	int f_4e;
	unsigned long savedF1d;
	TargetGump *targetGump;
	BogusProcess *bogus;
	Item *target;
	Referent f_60;
	unsigned flags;

	World(void);
	virtual void commandNoEvents(Event &);
	virtual void commandMouseLeft(Event &);
	virtual void commandMouseRight(Event &);
	virtual void commandMouseMovement(Event &);
	virtual void commandPrivate(Event &);
	virtual void commandKeyboard(Event &);
	virtual void commandMouseBoth(Event &);
	void shutdown(void);
	void saveAndExit(void);
	void prePreDragItem(Event &);
	void preDragItem(Event &);
	void startDragging(Event &);
	void postDragItem(Event &);
	void dragItem(Event &);
	int getTarget(void);
	int getTargetCoords(void);
};

void targetCoords(void);
int target(void);
void drawDiamond(unsigned short, unsigned short, unsigned char, short, short, short);
void drawShadowRect(Item);
void drawFootpadExtents(Item);

extern char *mouseBark[6];
extern char *quit[3];
extern char *toDos[3];
extern char *toPagan[3];
extern char *test[3];
extern char *testMusic[3];
extern char *testSFX[3];
extern char *testFiles[3];
extern char *view[3];
extern char *playing[3];
extern char *file[3];
extern char *passed[3];
extern char *failed[3];
extern char *pressAnyKey[3];
extern char *whatDrive[3];
extern char *invalid[3];
extern char *doesNotExist[3];
extern char *endOfFile[3];
extern char *hardwareStatus[3];
extern char *memory[3];
extern char *bytes[3];
extern char *available[3];
extern char *largestFragment[3];
extern char *hardDrive[3];
extern char *shapeCache[3];
extern char *music[3];
extern char *on[3];
extern char *off[3];
extern char *card[3];
extern char *port[3];
extern char *sfx[3];
extern unsigned char Line25[26];
extern char *TypeFlagDesc[14];
extern char *ItemStatDesc[16];
extern char InvalidString[8];
extern char *FamilyNames[16];
extern char *EquipNames[16];
extern char *AnimNames[16];
extern unsigned char searchBuffer[50];

#endif
