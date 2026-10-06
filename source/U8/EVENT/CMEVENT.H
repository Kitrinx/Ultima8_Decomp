#ifndef CMEVENT_H
#define CMEVENT_H

#include "CMOUSE.H"

class MouseEvent;

// MouseEvent actions.
#define MOUSE_MOVED		0
#define MOUSE_DOWN		1
#define MOUSE_UP		2
#define MOUSE_POLL_MOVED	3	// a poll found the pointer somewhere new

// Records mouse driver events into a ring buffer of fixed-size records,
// filled by the interrupt and drained by the inspectors.
class MouseEventChronicler : public MouseHandler
{
public:
	MouseEventChronicler(MouseEvent *, int);
	MouseEventChronicler(void *, int, int);
	virtual void handle(int, int, int, int);
	static void inc_write(void);
	static void inc_read(void);
	static void *get_last_write(void);
	static void *get_next_read(void *);
	static unsigned char is_valid(void *);
	static void write_event(MouseEvent *);
	static unsigned char read_event(void *);
	static char *head;
	static char *tail;
	static char *read;
	static char *write;
	static int record_size;
};

class MouseEventInspector
{
public:
	static MouseEvent *poll(void);
	static MouseEvent &poll(MouseEvent &);
	static unsigned char check_was_moved(MouseEvent *);
	static MouseEvent last_event;
};

#endif
