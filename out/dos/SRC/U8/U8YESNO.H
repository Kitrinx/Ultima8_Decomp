#ifndef U8YESNO_H
#define U8YESNO_H

#include "BASECAM.H"
#include "SBUTTON.H"

class Event;
class NewGump;
class NewGumpId;

class U8YesNoGump : public PanelGump, public GumpShape
{
public:
	unsigned char drawn;

	U8YesNoGump(NewGumpId, NewGump *, char *);
	virtual void commandMouseLeft(Event &) {}
	virtual void commandPrivate(Event &);
	virtual void commandKeyboard(Event &);
	virtual void draw(short, short);
};

extern char *mouseBark[6];

#endif
