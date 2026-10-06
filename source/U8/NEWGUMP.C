// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -vi-
// name: NEWGUMP.C

#include <dos.h>
#include <mem.h>
#include <stdarg.h>
#include "..\UI\NEWGUMP.H"
#include "BASECAM.H"
#include "CDLIST.H"
#include "CRECT.H"
#include "DISPATCH.H"
#include "ERROR.H"

#define TRACE_MEMORY	((TraceLevel)50)

inline DoubleLinkedList::DoubleLinkedList(void) { head = 0; tail = 0; }
inline DoubleLink::DoubleLink(DoubleLink *next, DoubleLink *prev) { this->next = next; this->prev = prev; }
inline int NewGump::visibility(void) { return visibilityStack & 3; }
inline void NewGump::detach(NewGump *kid) { newGumpList.detach(kid); }
inline unsigned char NewGumpId::check(void) { return !(instance ^ ~f_02); }

NewGump nullGump(0, 0, 0);
NewGump *NewGump::prevMouseMoveGump = 0;
unsigned char NewGump::drew = 0;
int NewGump::gumpCount = 0;
int NewGump::maxGumpCount = 0;

Event::Event(EventType t, int ex, int ey, long d, NewGump *f, NewGump *dest, void *e)
{
	type = t;
	x = ex;
	y = ey;
	data = d;
	to = dest;
	from = f;
	extra = e;
}

unsigned char Event::isSingle(void)
{
	return (int)data == 1;
}

unsigned char Event::isDouble(void)
{
	return (int)data == 2;
}

unsigned char Event::isRelease(void)
{
	return (int)data == 3;
}

NewGumpList::NewGumpList(void)
{
}

NewGumpList::~NewGumpList(void)
{
	kill();
}

void NewGumpList::attach(NewGump *gump)
{
	addToTail(gump);
}

void NewGumpList::moveToTail(NewGump *gump)
{
	NewGump *kid = 0;
	while (traverse((DoubleLink *&)kid))
	{
		if (kid == gump)
		{
			DoubleLinkedList::moveToTail(kid);
			return;
		}
	}
}

unsigned char NewGumpList::detach(NewGump *gump)
{
	NewGump *kid = 0;
	while (traverse((DoubleLink *&)kid))
	{
		if (kid == gump)
		{
			destroy(kid);
			return 1;
		}
	}
	return 0;
}

unsigned char NewGumpList::remove(NewGump *gump)
{
	NewGump *kid = 0;
	while (traverse((DoubleLink *&)kid))
	{
		if (kid == gump)
		{
			DoubleLinkedList::remove(kid);
			return 1;
		}
	}
	return 0;
}

NewGump *NewGumpList::operator[](int index)
{
	int i = -1;
	NewGump *kid = 0;
	while (traverse((DoubleLink *&)kid) && i != index)
		i++;
	return kid;
}

NewGump *NewGumpList::hasInstance(unsigned char instance)
{
	NewGump *kid = 0;
	while (traverse((DoubleLink *&)kid))
	{
		if (kid->newGumpId.getInstance() == instance)
			break;
	}
	return kid;
}

NewGump *NewGumpList::hasType(unsigned char type, NewGump *&kid)
{
	while (traverse((DoubleLink *&)kid))
	{
		if (kid->newGumpId.getType() == type)
			break;
	}
	return kid;
}

NewGump *NewGumpList::hasGump(unsigned char type, unsigned char instance)
{
	NewGump *kid = 0;
	while (traverse((DoubleLink *&)kid))
	{
		if (kid->newGumpId.getInstance() == instance && kid->newGumpId.getType() == type)
			break;
	}
	return kid;
}

NewGump::NewGump(NewGumpId id, NewGump *parent, long flags) :
	DoubleLink(0, 0), newGumpId(0)
{
	newGumpId = id;
	this->parent = parent;
	f_1d = flags;
	visibilityStack = 0;
	f_2d = 0;
	f_2e = 1;
	f_2f = 1;
	hotKeys = 0;
	if (this->parent)
		this->parent->attach(this);
}

NewGump::~NewGump(void)
{
	if (this == prevMouseMoveGump)
		prevMouseMoveGump = 0;
	if (hotKeys)
	{
		delete hotKeys;
		hotKeys = 0;
	}
}

void *NewGump::operator new(unsigned size)
{
	void *p = ::operator new(size);
	if (!p)
	{
		traceGet(TRACE_MEMORY, "\rOut of Memory! [%u]", size);
		halt(__FILE__, 474);	// __LINE__
	}
	if (++gumpCount > maxGumpCount)
		maxGumpCount++;
	return p;
}

void NewGump::operator delete(void *p)
{
	// Gumps built on the stack are only counted.
	if (_SS != FP_SEG(p))
		::operator delete(p);
	gumpCount--;
}

unsigned char NewGump::isInvisible(void)
{
	if (visibility() == 1 || visibility() == 3)
		return 1;
	return 0;
}

void NewGump::makeInvisible(void)
{
	if (visibility() != 1 && visibility() != 3)
		pushVisibility(3);
}

void NewGump::refresh(short dx, short dy, Rect clip)
{
	NewGump *kid = 0;
	Rect r = rect;
	r.moverel(dx, dy);
	if (BaseCamera::inTextMode)
		r.f_02--;
	if (!clip.intersects(r))
		return;
	if (!(visibilityStack & 1))
	{
		drew = 1;
		draw(rect.f_00 + dx, rect.f_02 + dy);
		if (drew)
			BaseCamera::setNeedSlam();
	}
	if (!(visibilityStack & 2))
	{
		while (newGumpList.traverse((DoubleLink *&)kid))
			kid->refresh(rect.f_00 + dx, rect.f_02 + dy, r);
	}
}

void NewGump::refresh(void)
{
	int dx = parent ? parent->get_dx() : 0;
	int dy = parent ? parent->get_dy() : 0;
	Rect r;
	r = rect;
	r.moverel(dx, dy);
	refresh(dx, dy, r);
	if (!BaseCamera::inTextMode)
		BaseCamera::setNeedSlam();
}

NewGump *NewGump::findTargetedGump(Event event)
{
	unsigned char inside = contains(event);
	if (!inside || (visibilityStack & 1))
		return 0;
	if (!f_2d && !(visibilityStack & 2))
	{
		NewGump *kid = 0;
		NewGump *target = 0;
		event.x -= rect.f_00;
		event.y -= rect.f_02;
		while (newGumpList.traverseBackward((DoubleLink *&)kid))
		{
			target = kid->findTargetedGump(event);
			if (target)
				return target;
		}
	}
	return this;
}

unsigned char NewGump::caresAbout(Event &event, NewGump *&target, int mode)
{
	unsigned char cares;
	unsigned char hit = collide(event) || (event.type & 0xe0);
	target = 0;
	if (!(mode & 1) && !hit)
		return 0;
	if (!f_2d && !(visibilityStack & 2))
	{
		NewGump *kid = 0;
		event.x -= rect.f_00;
		event.y -= rect.f_02;
		while (newGumpList.traverseBackward((DoubleLink *&)kid))
		{
			kid->check(__FILE__, 790);	// __LINE__
			cares = kid->caresAbout(event, target, mode);
			if (cares && target != &nullGump)
			{
				if (kid->parent)
					kid->parent->newGumpList.moveToTail(kid);
				return cares;
			}
		}
		event.x += rect.f_00;
		event.y += rect.f_02;
	}
	if ((visibilityStack & 1) && !(mode & 4))
		return 0;
	if ((event.type & 0x20) && !searchHotKeys(event))
		return 0;
	if ((f_1d & event.type) || (mode & 2))
		target = this;
	else
		target = &nullGump;
	return 1;
}

unsigned char NewGump::contains(Event &event)
{
	return rect.contains(event.x, event.y);
}

void NewGump::pushVisibility(int visibility)
{
	if (!visibilityStack)
		visibilityStack = visibility;
	else
	{
		visibilityStack <<= 2;
		visibilityStack |= visibility;
	}
}

unsigned char NewGump::kidnap(NewGump *kid, NewGump *from)
{
	if (from->newGumpList.remove(kid))
	{
		newGumpList.attach(kid);
		kid->parent = this;
		return 1;
	}
	return 0;
}

int NewGump::get_dx(void)
{
	if (!parent)
		return rect.f_00;
	return rect.f_00 + parent->get_dx();
}

int NewGump::get_dy(void)
{
	if (!parent)
		return rect.f_02;
	return rect.f_02 + parent->get_dy();
}

int NewGump::getRelX(Event &event)
{
	return event.x - rect.f_00;
}

int NewGump::getRelY(Event &event)
{
	return event.y - rect.f_02;
}

void NewGump::command(Event &event)
{
	if (event.type == 2 || event.type == 4 || event.type == 0x100 || event.type == 8)
	{
		if (prevMouseMoveGump && this != prevMouseMoveGump && (prevMouseMoveGump->f_1d & 0x80))
		{
			Event escape = event;
			escape.from = this;
			escape.to = prevMouseMoveGump;
			prevMouseMoveGump->commandMouseEscape(escape);
		}
		prevMouseMoveGump = this;
	}
	switch (event.type)
	{
	case 1:
		commandNoEvents(event);
		return;
	case 2:
		commandMouseLeft(event);
		return;
	case 4:
		commandMouseRight(event);
		return;
	case 0x100:
		commandMouseBoth(event);
		return;
	case 8:
		commandMouseMovement(event);
		return;
	case 0x40:
	{
		NewGump *kid = 0;
		while (newGumpList.traverse((DoubleLink *&)kid))
			kid->command(event);
		if (f_1d & 0x40)
			commandBroadCast(event);
		return;
	}
	case EVENT_PRIVATE:
		commandPrivate(event);
		return;
	case 0x20:
		commandKeyboard(event);
		return;
	default:
		dispatcher->badEvent(this, event);
	}
}

// Adds a zero-terminated list of keys to the ones this gump answers.
void NewGump::registerHotKeys(...)
{
	int total;
	int oldCount = 0;
	int newCount = 0;
	// The keys follow this on the stack: saved BP, return address, this.
	va_list args = (va_list)MK_FP(_SS, _BP + 10);
	while (((int *)args)[newCount])
		newCount++;
	if (newCount == 0)
	{
		if (hotKeys)
		{
			delete hotKeys;
			hotKeys = 0;
		}
		f_1d &= ~0x20;
		return;
	}
	if (hotKeys)
	{
		int *key = hotKeys;
		while (*key++)
			oldCount++;
	}
	total = newCount + oldCount + 1;
	int *keys = new int[total];
	if (!keys)
		outOfMemory(__FILE__, 1169);	// __LINE__
	if (hotKeys)
	{
		memcpy(keys, hotKeys, oldCount * sizeof(int));
		delete hotKeys;
	}
	memcpy(keys + oldCount, args, (newCount + 1) * sizeof(int));
	hotKeys = keys;
	f_1d |= 0x20;
}

// A key list starting with -1 takes every key.
unsigned char NewGump::searchHotKeys(Event &event)
{
	if (!hotKeys)
		return 0;
	int *key = hotKeys;
	if (*key == -1)
		return 1;
	while (*key)
	{
		if (*key++ == event.data)
			return 1;
	}
	return 0;
}

void NewGump::restore(Rect r)
{
	dispatcher->restore(r);
}

void NewGump::restore(void)
{
	Rect r = rect;
	r.moveto(get_dx(), get_dy());
	dispatcher->restore(r);
}

void NewGump::setExclusive(NewGump *gump)
{
	if (!gump)
		gump = this;
	dispatcher->setExclusive(gump);
}

void NewGump::clrExclusive(void)
{
	dispatcher->clrExclusive();
}

void NewGump::send(Event &event)
{
	dispatcher->send(event);
}

void NewGump::sendInFuture(Event &event)
{
	dispatcher->sendInFuture(event);
}

void NewGump::badEvent(NewGump *gump, Event &event)
{
	dispatcher->badEvent(gump, event);
}

void NewGump::killMyself(void)
{
	if (!isInvisible())
	{
		makeInvisible();
		restore();
	}
	parent->detach(this);
}

void NewGump::check(char *file, int line)
{
	if (!newGumpId.check())
		halt("Bad NewGumpId [%04X] at %s, %d", newGumpId.get(), file, line);
}

unsigned char NewGump::brattsApprove(Event &event)
{
	NewGump *kid = 0;
	while (newGumpList.traverse((DoubleLink *&)kid))
	{
		if (kid->commandAsk(event))
			return 1;
	}
	return 0;
}
