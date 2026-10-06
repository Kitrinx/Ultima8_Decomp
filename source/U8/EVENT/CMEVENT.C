// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: EVENT\CMEVENT.C

#include <mem.h>
#include "CMEVENT.H"
#include "CMPOINT.H"
#include "DISPATCH.H"

// Inline in the shared headers when this file was built.
inline MouseButtonStatus::MouseButtonStatus(void) {}
inline MouseButtonStatus::MouseButtonStatus(int s) { status = s; }
inline MouseEvent::MouseEvent(void) { _action = MOUSE_MOVED; }

// Driver event bits: movement, left down/up, right down/up, middle down/up.
#define ALL_MOUSE_EVENTS	0x7E

char *MouseEventChronicler::head = 0;
char *MouseEventChronicler::tail = 0;
char *MouseEventChronicler::read = 0;
char *MouseEventChronicler::write = 0;
int MouseEventChronicler::record_size;
MouseEvent MouseEventInspector::last_event;

MouseEventChronicler::MouseEventChronicler(MouseEvent *buffer, int count) :
	MouseHandler(ALL_MOUSE_EVENTS)
{
	record_size = sizeof(MouseEvent);
	head = (char *)buffer;
	tail = head + record_size * count;
	read = write = (char *)buffer;
}

MouseEventChronicler::MouseEventChronicler(void *buffer, int size, int count) :
	MouseHandler(ALL_MOUSE_EVENTS)
{
	record_size = size;
	head = (char *)buffer;
	tail = head + record_size * count;
	read = write = (char *)buffer;
}

// A full buffer drops its oldest event.
void MouseEventChronicler::inc_write(void)
{
	write += record_size;
	if (write >= tail)
		write = head;
	if (write == read)
		inc_read();
}

void MouseEventChronicler::inc_read(void)
{
	read += record_size;
	if (read >= tail)
		read = head;
}

void *MouseEventChronicler::get_last_write(void)
{
	if (write == head)
		return tail - record_size;
	return write - record_size;
}

void *MouseEventChronicler::get_next_read(void *event)
{
	char *next = (char *)event + record_size;

	if (next >= tail)
		return head;
	return next;
}

// Whether event lies in the unread part of the ring.
unsigned char MouseEventChronicler::is_valid(void *event)
{
	if (read == write)
		return 0;
	if (read < write)
		return (char *)event >= read && (char *)event < write;
	return (char *)event >= read || (char *)event < write;
}

void MouseEventChronicler::write_event(MouseEvent *event)
{
	memcpy(write, event, record_size);
	inc_write();
}

unsigned char MouseEventChronicler::read_event(void *event)
{
	if (read != write)
	{
		memcpy(event, read, record_size);
		inc_read();
		return 1;
	}
	return 0;
}

// Mouse driver callback: mask holds the int 33h event bits.
void MouseEventChronicler::handle(int mask, int buttons, int x, int y)
{
	MouseEvent event;

	event._x = x;
	event._y = y;
	event.mouseButtonStatus = MouseButtonStatus(buttons);
	event._button = NO_BUTTON;
	if (mask & 0x06)
		event._button = LEFT_BUTTON;
	if (mask & 0x18)
	{
		if (event._button)
			event._button = BOTH_BUTTONS;
		else
			event._button = RIGHT_BUTTON;
	}
	else
		event._button = NO_BUTTON;
	if (mask & 0x2A)
		event._action = MOUSE_DOWN;
	else if (mask & 0x54)
		event._action = MOUSE_UP;
	else
		event._action = MOUSE_MOVED;
	write_event(&event);
}

MouseEvent *MouseEventInspector::poll(void)
{
	last_event._action = MOUSE_MOVED;
	if (!MouseEventChronicler::read_event(&last_event))
		check_was_moved(&last_event);
	return &last_event;
}

MouseEvent &MouseEventInspector::poll(MouseEvent &event)
{
	poll();
	event = last_event;
	return event;
}

unsigned char MouseEventInspector::check_was_moved(MouseEvent *event)
{
	if (MouseDevice::get_last_x() != event->_x || MouseDevice::get_last_y() != event->_y)
	{
		event->_action = MOUSE_POLL_MOVED;
		event->_x = MouseDevice::get_last_x();
		event->_y = MouseDevice::get_last_y();
		return 1;
	}
	event->_action = MOUSE_MOVED;
	return 0;
}
