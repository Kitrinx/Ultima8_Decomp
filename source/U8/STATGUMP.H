#ifndef STATGUMP_H
#define STATGUMP_H

#include "IRGUMP.H"
#include "SBUTTON.H"

class Event;
class NewGump;

// The avatar's health and mana bars.
class AvatarStatusGump : public ItemRelativeGump, public GumpShape
{
public:
	AvatarStatusGump(NewGump *, unsigned short);
	virtual ~AvatarStatusGump(void);
	virtual void commandMouseLeft(Event &);
	virtual void draw(short, short);
};

void closeStatusGump(void);
void openStatusGump(void);

#endif
