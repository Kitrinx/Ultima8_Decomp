#ifndef BUTTON_H
#define BUTTON_H

#include "DISPATCH.H"

class Event;
class PushButton;
class RawButton;

class RawButton : public NewGump
{
public:
	unsigned char enabled;

	inline RawButton(unsigned char, NewGump *, unsigned char);
	virtual void commandMouseLeft(Event &);
	virtual void commandMouseRight(Event &);
	virtual void commandMouseMovement(Event &);
	virtual void draw(short, short) = 0;
	virtual void select(void) = 0;
	virtual void unselect(void) = 0;
	static unsigned selecting;
};

class PushButton : public RawButton
{
public:
	PushButton(unsigned char, NewGump *);
	virtual void commandMouseLeft(Event &);
};

class CycleButton : public RawButton
{
public:
	int _frame;
	int f_37;

	CycleButton(unsigned char, NewGump *);
};

class ActivatorButton : public PushButton
{
public:
	ActivatorButton(unsigned char, NewGump *);
	virtual void commandNoEvents(Event &);
	virtual void commandMouseLeft(Event &);
	virtual void commandMouseMovement(Event &);
};

#endif
