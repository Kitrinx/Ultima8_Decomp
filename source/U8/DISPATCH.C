// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -vi-
// name: DISPATCH.C

#include "..\UI\NEWGUMP.H"
#include "ERROR.H"
#include "CFILE.H"
#include "CMSELECT.H"
#include "CTRACKER.H"
#include "CTMPOINT.H"
#include "CRECT.H"
#include "CVPORT.H"
#include "CGLOBVP.H"
#include "BASECAM.H"
#include "MENU.H"
#include "DISPATCH.H"
#include "WAIT.H"

// Inline in the shared headers when this file was built.
inline MouseButtonStatus::MouseButtonStatus(void) {}
inline MouseEvent::MouseEvent(void) { _action = MOUSE_MOVED; }
inline int MouseEvent::get_y(void) { return _y; }
inline MouseButtonStatus MouseEvent::get_button_status(void) { return mouseButtonStatus; }
inline unsigned MouseEvent::getExtraStatus(void) { return extraStatus; }
inline int MouseEvent::get_button(void) { return _button; }
inline MouseSelectEvent::MouseSelectEvent(void) { _type = NO_SELECT; }
inline unsigned char MouseSelectEvent::is_valid(void) { return _type != NO_SELECT; }
inline int MouseSelectEvent::get_type(void) { return _type; }
inline VGAMouseSelectEvent::VGAMouseSelectEvent(void) {}
inline int VGAMouseSelectEvent::get_x(void) { return _x >> 1; }
inline MouseSelectInspector::MouseSelectInspector(int t) { threshold = t; }
inline VGAMouseSelectEvent *VGAMouseSelectInspector::get_last_event(void) { return (VGAMouseSelectEvent *)&MouseSelectInspector::last_event; }
inline unsigned char BaseFile::is_valid(void) { return handle != -1; }
inline BaseFile::~BaseFile(void) { if (is_valid()) close(); }
inline unsigned char AccurateTimer::isTiming(void) { return timing; }
inline unsigned AccurateTimer::getTicks(void) { return ticks; }
inline NewGumpId::NewGumpId(unsigned i) { instance = i; f_02 = ~instance; f_04 = 0; }
inline int NewGumpId::operator==(NewGumpId id) { return instance == id.instance; }
inline unsigned char NewGumpId::getInstance(void) { return instance & 0xff; }
inline void NewGump::attach(NewGump *gump) { newGumpList.attach(gump); }
inline NewGump *Dispatcher::getBase(void) { return base[baseSP]; }
inline UncleErnie::UncleErnie(long *r) : NewGump(NewGumpId(0), 0, EVENT_PRIVATE)
{
	result = r;
	rect.set(0, 0, 319, 199);
}

// Dispatcher::mode
#define NOT_RECORDING	0
#define RECORDING	1
#define PLAYING_BACK	2

#define MOUSE_EVENTS	10
#define EVENT_BUFFER_SIZE	25
#define BASE_STACK_SIZE	5
#define RECORD_BUFFER_SIZE	3275	// 65500 bytes of RecordedEvent

#define TRACE_MEMORY	((TraceLevel)50)

Dispatcher *dispatcher = 0;
unsigned char Dispatcher::active = 0;
unsigned long Dispatcher::gameloop = 0;
int Dispatcher::reader = 0;
int Dispatcher::writer = 0;
int Dispatcher::futureReader = 0;
int Dispatcher::futureWriter = 0;
int Dispatcher::baseSP = -1;
int Dispatcher::mode = NOT_RECORDING;
MouseSelectChronicler *Dispatcher::theChronicler = 0;
VGAMouseSelectInspector *Dispatcher::theInspector = 0;
Tracker *Dispatcher::theTracker = 0;
VGAMouseSelectEvent *Dispatcher::mouseEventBuffer = 0;
BaseFile *Dispatcher::eventFile = 0;
Event *Dispatcher::eventBuffer = 0;
Event *Dispatcher::futureEventBuffer = 0;
Event *Dispatcher::event = 0;
RecordedEvent *Dispatcher::dispatcherMemory = 0;
long Dispatcher::eventCounter = 0;
NewGump *Dispatcher::exclusive = 0;
NewGump *Dispatcher::suspended = 0;
NewGump **Dispatcher::base = 0;
AccurateTimer *atimer = 0;
char *eventFileDesc = "events.dat";

Dispatcher::Dispatcher(int recordMode)
{
	if (!active)
	{
		active = 1;
		mouseEventBuffer = new VGAMouseSelectEvent[MOUSE_EVENTS];
		if (!mouseEventBuffer)
			outOfMemory(__FILE__, 94);	// __LINE__
		theChronicler = new MouseSelectChronicler(mouseEventBuffer, MOUSE_EVENTS);
		theInspector = new VGAMouseSelectInspector;
		theTracker = new Tracker;
		event = new Event;
		eventBuffer = new Event[EVENT_BUFFER_SIZE];
		futureEventBuffer = new Event[EVENT_BUFFER_SIZE];
		base = new NewGump *[BASE_STACK_SIZE];
		for (int i = 0; i < BASE_STACK_SIZE; i++)
			base[i] = 0;
		if (!theChronicler || !theInspector || !theTracker || !event || !mouseEventBuffer ||
			!eventBuffer || !futureEventBuffer || !base)
			outOfMemory(__FILE__, 121);	// __LINE__
		mode = recordMode;
		if (mode)
		{
			dispatcherMemory = new RecordedEvent[RECORD_BUFFER_SIZE];
			eventFile = new BaseFile;
			atimer = new AccurateTimer(500);
			if (!dispatcherMemory || !eventFile)
			{
				traceGet(TRACE_MEMORY, "No room for Dispatcher Record/Playback.");
				mode = NOT_RECORDING;
			}
			if (mode == RECORDING)
				eventFile->create(eventFileDesc, ReadWrite);
			else
				eventFile->open(eventFileDesc, ReadWrite);
		}
	}
	if (dispatcher)
		halt(__FILE__, 153);	// __LINE__
	dispatcher = this;
}

Dispatcher::~Dispatcher(void)
{
	uninit();
}

void Dispatcher::uninit(void)
{
	if (active)
	{
		while (baseSP >= 0)
			popBase();
		if (suspended)
			delete suspended;
		delete base;
		delete eventBuffer;
		delete futureEventBuffer;
		base = 0;
		eventBuffer = 0;
		futureEventBuffer = 0;
		if (dispatcherMemory)
		{
			eventFile->write((char *)dispatcherMemory, eventCounter * sizeof(RecordedEvent));
			eventFile->flush();
			delete dispatcherMemory;
		}
		if (eventFile)
			delete eventFile;
		delete theTracker;
		delete theInspector;
		delete theChronicler;
		delete mouseEventBuffer;
		active = 0;
	}
}

// Nonzero when a key is waiting in the BIOS buffer.
int keyWaiting(void)
{
	asm mov ah, 1
	asm int 16h
	asm jnz done
	asm xor ax, ax
done:
}

void Dispatcher::poll(void)
{
	unsigned char sent = 0;
	long key;

	if (mode == PLAYING_BACK)
	{
		Event e;
		if (getNextRecordedEvent(e))
		{
			if (e.type == 8)
			{
				TMMousePointer::undraw();
				TMMousePointer::draw(e.x, e.y);
			}
			write(e);
			sent = 1;
		}
	}
	else if (MouseSelectInspector::poll()->is_valid())
	{
		Event e;
		translateMouse(*VGAMouseSelectInspector::get_last_event(), &e);
		e.from = 0;
		write(e);
		sent = 1;
	}
	if (keyWaiting())
	{
		key = 0;
		char scan = 0;
		char ascii = 0;
		asm xor ah, ah
		asm int 16h
		scan = _AH;
		ascii = _AL;
		if (ascii)
			key = ascii;
		else
			key = scan + 0x100;
		if (mode == PLAYING_BACK)
		{
			if (key == 27 && !exclusive)
				mode = NOT_RECORDING;
		}
		else
		{
			write(Event((EventType)0x20, 0, 0, key, 0, 0, 0));
			sent = 1;
		}
	}
	if (!sent)
	{
		Event e;
		translateMouse(*VGAMouseSelectInspector::get_last_event(), &e);
		e.type = EVENT_NONE;
		e.from = 0;
		write(e);
	}
}

void Dispatcher::write(Event &e)
{
	eventBuffer[writer] = e;
	// Overflow is tested but not handled.
	if ((writer + 1) % EVENT_BUFFER_SIZE == reader)
		;
	writer++;
	if (writer == EVENT_BUFFER_SIZE)
		writer = 0;
}

void Dispatcher::translateMouse(VGAMouseSelectEvent &m, Event *e)
{
	e->x = m.get_x();
	e->y = m.get_y();
	if (BaseCamera::inTextMode)
	{
		e->x >>= 2;
		e->y >>= 3;
	}
	e->f_0e = m.get_button_status().status;
	e->data = (unsigned long)m.getExtraStatus() << 16;
	if (m.get_type() == SELECT_MOVE)
	{
		e->type = 8;
		e->data |= m.get_button();
		return;
	}
	e->data |= m.get_type();
	switch (m.get_button())
	{
	case LEFT_BUTTON:
		e->type = 2;
		break;
	case RIGHT_BUTTON:
		e->type = 4;
		break;
	case BOTH_BUTTONS:
		e->type = 0x100;
		break;
	default:
		e->type = EVENT_NONE;
	}
}

void Dispatcher::process(void)
{
	Event e;
	NewGump *target;

	if (atimer && !atimer->isTiming() && mode)
		atimer->start();
	if (!getBase())
		return;
	while (readingMyFuture(e))
		write(e);
	gameloop++;
	getBase()->check(__FILE__, 475);	// __LINE__
	do
	{
		poll();
		while (read())
		{
			if (mode == RECORDING)
				recordEvent(*event);
			if (exclusive && event->type != EVENT_PRIVATE)
			{
				if (exclusive->f_1d & event->type)
				{
					target = exclusive;
					if (target->parent)
					{
						event->x -= target->parent->get_dx();
						event->y -= target->parent->get_dy();
					}
				}
				else
				{
					gameloop++;
					continue;
				}
			}
			else if (event->to)
				target = event->to;
			else if (!getBase()->caresAbout(*event, target, 0))
				continue;
			target->check(__FILE__, 518);	// __LINE__
			target->command(*event);
			if (exclusive)
			{
				BaseCamera::alternateFrame = 1;
				theBaseCamera->slam();
				gameloop++;
			}
		}
	} while (exclusive);
}

unsigned char Dispatcher::read(void)
{
	if (reader == writer)
		return 0;
	*event = eventBuffer[reader];
	reader++;
	if (reader == EVENT_BUFFER_SIZE)
		reader = 0;
	return 1;
}

unsigned char Dispatcher::readingMyFuture(Event &e)
{
	if (futureReader == futureWriter)
		return 0;
	e = futureEventBuffer[reader];	// reader, not futureReader
	futureReader++;
	if (futureReader == EVENT_BUFFER_SIZE)
		futureReader = 0;
	return 1;
}

void Dispatcher::send(Event &e)
{
	if (!e.from || !e.to)
		badEvent(e.from, e);
	write(e);
}

void Dispatcher::sendInFuture(Event &e)
{
	if (!e.from || !e.to)
		badEvent(e.from, e);
	futureEventBuffer[futureWriter] = e;
	// Overflow is tested but not handled.
	if ((futureWriter + 1) % EVENT_BUFFER_SIZE == futureReader)
		;
	futureWriter++;
	if (futureWriter == EVENT_BUFFER_SIZE)
		futureWriter = 0;
}

void Dispatcher::badEvent(NewGump *, Event &)
{
	halt(__FILE__, 722);	// __LINE__
}

void Dispatcher::refresh(Rect r)
{
	for (int i = 0; i <= baseSP; i++)
		if (base[i])
			base[i]->refresh(0, 0, r);
}

void Dispatcher::restore(Rect r)
{
	if (r.f_00 < 0)
		r.f_00 = 0;
	if (r.f_02 < 0)
		r.f_02 = 0;
	if (BaseCamera::inTextMode)
		r.clip(Rect(0, 0, 79, 49));
	else
	{
		r.clip(*GlobalVport::global_ptr->getRect());
		DrawBox(GlobalVport::global_ptr, r.f_00, r.f_02, r.f_04, r.f_06, GumpColorMap[10]);
	}
	theBaseCamera->show(r.f_00, r.f_02, r.f_04, r.f_06, BaseCamera::inTextMode);
}

void Dispatcher::setExclusive(NewGump *gump)
{
	exclusive = gump;
}

void Dispatcher::clrExclusive(void)
{
	exclusive = 0;
}

void Dispatcher::pushBase(NewGump *gump)
{
	if (baseSP < BASE_STACK_SIZE - 1)
		base[++baseSP] = gump;
	else
		halt(__FILE__, 857);	// __LINE__
}

void Dispatcher::popBase(void)
{
	if (baseSP >= 0)
	{
		delete base[baseSP];
		base[baseSP] = 0;
		baseSP--;
	}
	else
		halt(__FILE__, 874);	// __LINE__
}

// Finds the gump whose id path starts at a base gump: id[0] names the
// base, id[1..depth] each next child down.
NewGump *Dispatcher::findGump(NewGumpId *id, int depth)
{
	for (int i = 0; i <= baseSP; i++)
	{
		if (base[i]->newGumpId == *id)
		{
			NewGump *found = base[i];
			for (int j = 1; j <= depth; j++)
			{
				found = found->newGumpList.hasGump(id[j].getType(), id[j].getInstance());
				if (!found)
					return 0;
			}
			return found;
		}
	}
	return 0;
}

void Dispatcher::suspend(void)
{
	NewGump *old = suspended;

	if (base[baseSP])
	{
		suspended = base[baseSP];
		baseSP--;
	}
	if (old)
	{
		pushBase(old);
		send(Event(EVENT_PRIVATE, 0, 0, 1, old, old, 0));
	}
}

void Dispatcher::resume(void)
{
	pushBase(suspended);
	suspended = 0;
}

unsigned char Dispatcher::getNextRecordedEvent(Event &e)
{
	static long loaded = 0;
	static long fileSize = 0;
	static unsigned char opened = 0;
	unsigned long when;
	unsigned long ticks;

	if (opened == 0)
	{
		opened++;
		fileSize = eventFile->size();
	}
	if (eventCounter >= loaded)
	{
		loaded = (fileSize - eventFile->tell()) / sizeof(RecordedEvent);
		if (loaded == 0)
		{
			mode = NOT_RECORDING;
			traceGet(TRACE_MEMORY, "End of script! (press any key)");
			return 0;
		}
		if (loaded > RECORD_BUFFER_SIZE)
			loaded = RECORD_BUFFER_SIZE;
		eventFile->read((char *)dispatcherMemory, loaded * sizeof(RecordedEvent));
		eventCounter = 0;
	}
	if ((when = dispatcherMemory[eventCounter].gameloop) == gameloop)
	{
		e.type = dispatcherMemory[eventCounter].type;
		e.x = dispatcherMemory[eventCounter].x;
		e.y = dispatcherMemory[eventCounter].y;
		e.data = dispatcherMemory[eventCounter].data;
		e.f_0e = dispatcherMemory[eventCounter].buttons;
		ticks = dispatcherMemory[eventCounter].ticks;
		while (atimer->getTicks() < ticks)
			;
		eventCounter++;
		return 1;
	}
	if (when < gameloop)
		halt(__FILE__, 1029);	// __LINE__
	return 0;
}

void Dispatcher::recordEvent(Event &e)
{
	if (!e.from)
	{
		if (eventCounter >= RECORD_BUFFER_SIZE)
		{
			eventFile->write((char *)dispatcherMemory, eventCounter * sizeof(RecordedEvent));
			eventFile->flush();
			eventCounter = 0;
		}
		dispatcherMemory[eventCounter].type = e.type;
		dispatcherMemory[eventCounter].x = e.x;
		dispatcherMemory[eventCounter].y = e.y;
		dispatcherMemory[eventCounter].data = e.data;
		dispatcherMemory[eventCounter].buttons = e.f_0e;
		dispatcherMemory[eventCounter].gameloop = gameloop;
		dispatcherMemory[eventCounter].ticks = atimer->getTicks();
		if (e.type == EVENT_NONE)
			dispatcherMemory[eventCounter].data = 0;
		eventCounter++;
	}
}

unsigned char Dispatcher::isMouseRightHanded(void)
{
	return MouseSelectChronicler::handedness == 0;
}

void Dispatcher::switchHandedness(void)
{
	MouseSelectChronicler::handedness = !MouseSelectChronicler::handedness;
}

void UncleErnie::commandPrivate(Event &e)
{
	*result = e.data;
}

TightDispatcher::TightDispatcher(NewGump *gump, long *result)
{
	if (!gump)
		return;
	oldExclusive = Dispatcher::exclusive;
	Dispatcher::exclusive = 0;
	oldEvent = Dispatcher::event;
	Dispatcher::event = new Event;
	if (!Dispatcher::event)
		outOfMemory(__FILE__, 1181);	// __LINE__
	UncleErnie *ernie = new UncleErnie(result);
	dispatcher->pushBase(ernie);
	gump->parent = ernie;
	ernie->attach(gump);
	gump->refresh();
}

TightDispatcher::~TightDispatcher(void)
{
	dispatcher->popBase();
	Dispatcher::exclusive = oldExclusive;
	delete Dispatcher::event;
	Dispatcher::event = oldEvent;
}

// Runs until the modal gump closes, leaving no children on the base.
void TightDispatcher::run(void)
{
	unsigned char going = 1;

	while (going)
	{
		dispatcher->process();
		BaseCamera::alternateFrame = 1;
		theBaseCamera->slam();
		NewGump *kid = 0;
		going = dispatcher->getBase()->newGumpList.traverse((DoubleLink *&)kid);
	}
}

long Dispatch(NewGump *gump)
{
	long result;

	if (!gump)
		return 0;
	if (gump->parent)
		halt(__FILE__, 1255);	// __LINE__
	TightDispatcher tight(gump, &result);
	tight.run();
	return result;
}
