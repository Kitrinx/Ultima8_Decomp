#ifndef DISPATCH_H
#define DISPATCH_H

#include "..\UI\NEWGUMP.H"
#include "CMPOINT.H"
#include "CMSELECT.H"
#include "CRECT.H"
#include "CDLIST.H"

class AccurateTimer;
class BaseFile;
class Event;
class MouseButtonStatus;
class MouseEvent;
class MouseSelectChronicler;
class MouseSelectEvent;
class NewGump;
class NewGumpId;
class Tracker;
class VGAMouseSelectEvent;
class VGAMouseSelectInspector;

// One event as kept in the record/playback file.
struct RecordedEvent
{
	int type;
	int x;
	int y;
	long data;
	unsigned buttons;
	unsigned long gameloop;
	unsigned long ticks;
};

class Dispatcher
{
public:
	Dispatcher(int);
	~Dispatcher(void);
	void uninit(void);
	void poll(void);
	void write(Event &);
	void translateMouse(VGAMouseSelectEvent &, Event *);
	void process(void);
	unsigned char read(void);
	unsigned char readingMyFuture(Event &);
	void send(Event &);
	void sendInFuture(Event &);
	void badEvent(NewGump *, Event &);
	void refresh(Rect);
	void restore(Rect);
	void setExclusive(NewGump *);
	void clrExclusive(void);
	void pushBase(NewGump *);
	void popBase(void);
	NewGump *findGump(NewGumpId *, int);
	void suspend(void);
	void resume(void);
	unsigned char getNextRecordedEvent(Event &);
	void recordEvent(Event &);
	unsigned char isMouseRightHanded(void);
	void switchHandedness(void);
	NewGump *getBase(void);
	static unsigned char active;
	static unsigned long gameloop;
	static int reader;
	static int writer;
	static int futureReader;
	static int futureWriter;
	static int baseSP;
	static int mode;
	static MouseSelectChronicler *theChronicler;
	static VGAMouseSelectInspector *theInspector;
	static Tracker *theTracker;
	static VGAMouseSelectEvent *mouseEventBuffer;
	static BaseFile *eventFile;
	static Event *eventBuffer;
	static Event *futureEventBuffer;
	static Event *event;
	static RecordedEvent *dispatcherMemory;
	static long eventCounter;
	static NewGump *exclusive;
	static NewGump *suspended;
	static NewGump **base;
};

// The base gump of a modal loop; its private events hand back the result.
class UncleErnie : public NewGump
{
public:
	long *result;

	UncleErnie(long *);
	virtual void commandPrivate(Event &);
};

// Runs a gump modally on its own event, restoring the dispatcher afterwards.
class TightDispatcher
{
public:
	NewGump *oldExclusive;
	Event *oldEvent;
	unsigned char pad_08[4];

	TightDispatcher(NewGump *, long *);
	~TightDispatcher(void);
	void run(void);
};

class VGAMouseSelectInspector : public MouseSelectInspector
{
public:
	static VGAMouseSelectEvent *get_last_event(void);
};

class MouseButtonStatus
{
public:
	char status;

	MouseButtonStatus(void);
	MouseButtonStatus(int);
};

class MouseEvent
{
public:
	unsigned _action;
	unsigned _button;
	int _x;
	unsigned _y;
	MouseButtonStatus mouseButtonStatus;
	unsigned extraStatus;

	MouseEvent(void);
	int get_y(void);
	MouseButtonStatus get_button_status(void);
	unsigned getExtraStatus(void);
	int get_button(void);
};

class MouseSelectEvent : public MouseEvent
{
public:
	unsigned _type;
	unsigned long _time;

	MouseSelectEvent(MouseSelectCommand, MouseButton, int, int);
	MouseSelectEvent(void);
	unsigned char is_valid(void);
	int get_type(void);
};

// Mouse coordinates arrive doubled horizontally; get_x halves them for 320 wide.
class VGAMouseSelectEvent : public MouseSelectEvent
{
public:
	VGAMouseSelectEvent(void);
	int get_x(void);
};

int keyWaiting(void);
long Dispatch(NewGump *);

extern Dispatcher *dispatcher;
extern AccurateTimer *atimer;
extern char *eventFileDesc;

#endif
