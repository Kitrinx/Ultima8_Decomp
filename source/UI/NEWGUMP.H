#ifndef NEWGUMP_H
#define NEWGUMP_H

#include "CDLIST.H"
#include "CRECT.H"

class NewGump;

enum EventType
{
	EventType_0,
	EVENT_NONE,
	EVENT_PRIVATE = 0x10
};

class NewGumpList : public DoubleLinkedList
{
public:
	NewGumpList(void);
	virtual ~NewGumpList(void);
	NewGump *hasType(unsigned char);
	NewGump *getHead(void);
	void attach(NewGump *);
	void moveToTail(NewGump *);
	unsigned char detach(NewGump *);
	unsigned char remove(NewGump *);
	NewGump *operator[](int);
	NewGump *hasInstance(unsigned char);
	NewGump *hasType(unsigned char, NewGump *&);
	NewGump *hasGump(unsigned char, unsigned char);
};

class Event
{
public:
	int type;
	int x;
	int y;
	long data;
	void *extra;
	unsigned f_0e;
	NewGump *to;
	NewGump *from;

	Event(EventType = EVENT_NONE, int = 0, int = 0, long = 0, NewGump * = 0, NewGump * = 0, void * = 0);
	unsigned char isSingle(void);
	unsigned char isDouble(void);
	unsigned char isRelease(void);
	char isShift(void);
	char isCtrl(void);
	char isAlt(void);
};

// Type in the high byte, instance in the low; f_02 holds the complement as a check.
class NewGumpId
{
public:
	unsigned instance;
	unsigned f_02;
	unsigned char f_04;

	inline NewGumpId(unsigned char, unsigned char);
	NewGumpId(unsigned = 0);
	unsigned char getType(void);
	int operator==(NewGumpId);
	unsigned char getInstance(void);
	int get(void);
	unsigned char check(void);
};

class NewGump : public DoubleLink
{
public:
	NewGumpId newGumpId;
	NewGumpList newGumpList;
	NewGump *parent;
	unsigned long f_1d;
	Rect rect;
	long visibilityStack;	// two bits per level, top in the low bits
	unsigned char f_2d;
	unsigned char f_2e;
	unsigned char f_2f;
	int *hotKeys;

	NewGump(NewGumpId, NewGump *, long);
	virtual ~NewGump(void);
	virtual void commandNoEvents(Event &) {}
	virtual void commandMouseLeft(Event &) {}
	virtual void commandMouseRight(Event &) {}
	virtual void commandMouseMovement(Event &) {}
	virtual void commandPrivate(Event &) {}
	virtual void commandKeyboard(Event &);
	virtual void commandBroadCast(Event &) {}
	virtual void commandMouseEscape(Event &) {}
	virtual void commandMouseBoth(Event &) {}
	virtual void commandJoystick(Event &) {}
	virtual unsigned char commandAsk(Event &) { return 0; }
	virtual void draw(short, short) { drew = 0; }
	virtual void command(Event &);
	virtual unsigned char collide(Event &event) { return contains(event); }
	virtual void killMyself(void);
	void moveto(short, short);
	void popVisibility(void);
	void attach(NewGump *);
	static void *operator new(unsigned);
	static void operator delete(void *);
	unsigned char isInvisible(void);
	void makeInvisible(void);
	void refresh(short, short, Rect);
	void refresh(void);
	NewGump *findTargetedGump(Event);
	unsigned char caresAbout(Event &, NewGump *&, int);
	unsigned char contains(Event &);
	void pushVisibility(int);
	unsigned char kidnap(NewGump *, NewGump *);
	int get_dx(void);
	int get_dy(void);
	int getRelX(Event &);
	int getRelY(Event &);
	void registerHotKeys(...);
	unsigned char searchHotKeys(Event &);
	void restore(Rect);
	void restore(void);
	void setExclusive(NewGump *);
	void clrExclusive(void);
	void send(Event &);
	void sendInFuture(Event &);
	void badEvent(NewGump *, Event &);
	void check(char *, int);
	unsigned char brattsApprove(Event &);
	int visibility(void);
	void detach(NewGump *);
	static NewGump *prevMouseMoveGump;
	static unsigned char drew;
	static int gumpCount;
	static int maxGumpCount;
};

// Stands in for "no gump" where a null pointer means "not decided yet".
extern NewGump nullGump;

#endif
