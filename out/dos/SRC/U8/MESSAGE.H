#ifndef MESSAGE_H
#define MESSAGE_H

#include "BASECAM.H"

class Event;
class NewGump;
class NewGumpId;

class MessageGump : public PanelGump
{
public:
	char *text;
	int textHeight;
	int textWidth;

	MessageGump(NewGumpId, NewGump *, char *, unsigned char);
	virtual void commandPrivate(Event &);
	virtual void commandKeyboard(Event &);
	virtual void draw(short, short);
};

#endif
