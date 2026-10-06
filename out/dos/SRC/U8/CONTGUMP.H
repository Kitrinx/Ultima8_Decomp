#ifndef CONTGUMP_H
#define CONTGUMP_H

#include "..\UI\NEWGUMP.H"
#include "DISPATCH.H"
#include "CRECT.H"
#include "CONTAIN.H"
#include "SBUTTON.H"
#include "IRGUMP.H"

class AvatarStatusGump;
class BaseFile;
class EditorItem;

class ContainerGumpBox : public NewGump, public Container
{
public:
	unsigned char dragging;
	Item dragItem;
	Rect backpack;			// where the paperdoll shows the backpack
	Referent pressed;		// item under a held button, dragged if the mouse moves
	Event event;			// the press that started it

	ContainerGumpBox(NewGump *, Rect, unsigned short);
	virtual void commandNoEvents(Event &);
	virtual void commandMouseLeft(Event &);
	virtual void commandMouseMovement(Event &);
	virtual void commandBroadCast(Event &);
	virtual void commandMouseEscape(Event &);
	virtual void commandMouseBoth(Event &);
	virtual void draw(short, short);
	void drag(unsigned short);
	Boolean findEquipSlot(unsigned short);
	int findTarget(short, short);
	static unsigned char targetModeHack;
};

class ContainerGump : public ItemRelativeGump, public Container, public GumpShape
{
public:
	Point dragOffset;
	Point dragStart;
	unsigned char pad_64[2];
	int index;				// this gump's entry in rects
	Rect itemRect;
	Item containerItem;

	ContainerGump(NewGump *, unsigned short, int);
	virtual ~ContainerGump(void);
	virtual void commandMouseLeft(Event &);
	virtual void commandPrivate(Event &);
	virtual void commandMouseBoth(Event &);
	virtual void draw(short, short);
	virtual unsigned char collide(Event &);
	static unsigned char findTarget(short, short, unsigned short &);
	void positionGump(short, short, short);
	static void load(BaseFile *);
	static void save(BaseFile *);
	static int gumpCount;
	static Rect rects[16];	// the open gumps' screen rectangles
};

void closeAllGumps(void);
void ctrlF6Key(void);

extern AvatarStatusGump *theStatGump;
extern char *cheatmsg[7];
extern unsigned cheats1[7];
extern unsigned long cheats2[7];
extern unsigned long cheats3[7];
extern char *statsText[21];
extern unsigned char containerGameCheat[1];
extern char contgumpNotice[32];
extern char dummyPtr1[16];
extern char dummyPtr2[16];
extern char dummyPtr3[16];
extern char *txtref;
extern char *txtrect;
extern char *txtshape;
extern char *txtframe;
// The editor item lives in raw storage; its constructor never runs.
extern unsigned char theEditorItem[32];
#define editorItem (*(EditorItem *)theEditorItem)

#endif
