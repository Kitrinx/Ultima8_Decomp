#ifndef SLIDER_H
#define SLIDER_H

#include "BASECAM.H"
#include "SBUTTON.H"

class Event;
class NewGump;

class SliderGump : public PanelGump, public GumpShape
{
public:
	int value;
	int min;
	int max;
	int delta;

	SliderGump(NewGump *, short, short, int, int, int, int);
	virtual void commandMouseLeft(Event &) {}
	virtual void commandPrivate(Event &);
	virtual void commandKeyboard(Event &);
	virtual void draw(short, short);
};

long getSliderInput(unsigned short, int, int, int);

#endif
