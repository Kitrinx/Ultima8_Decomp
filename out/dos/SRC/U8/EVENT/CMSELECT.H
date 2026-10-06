#ifndef CMSELECT_H
#define CMSELECT_H

#include "CMEVENT.H"
#include "CMOUSE.H"

class MouseSelectEvent;

class MouseSelectChronicler : public MouseEventChronicler
{
public:
	MouseSelectChronicler(MouseSelectEvent *, int);
	MouseSelectChronicler(void *, int, int);
	virtual void handle(int, int, int, int);
	static void check_button_swap(void *);
	static unsigned char read_event(void *, int);
	static unsigned char handedness;
};

class MouseSelectInspector : public MouseDevice
{
public:
	MouseSelectInspector(int = 12);
	static MouseSelectEvent *poll(void);
	static MouseSelectEvent &poll(MouseSelectEvent &);
	static unsigned char check_was_moved(MouseSelectEvent *);
	static int threshold;
	static MouseSelectEvent last_event;
};

#endif
