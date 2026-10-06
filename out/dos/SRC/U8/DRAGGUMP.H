#ifndef DRAGGUMP_H
#define DRAGGUMP_H

#include "DISPATCH.H"

class Event;
class NewGumpId;
class Shape;

// A gump the mouse can drag around the screen.
class DrgNewGump : public NewGump
{
public:
	Shape *dragShape;	// the gump's image while it is dragged
	unsigned dragging;
	int lastX;
	int lastY;

	DrgNewGump(NewGumpId, NewGump *);
	virtual ~DrgNewGump(void);
	virtual void commandMouseLeft(Event &);
	virtual void commandMouseMovement(Event &);
	virtual void draw(int, int) {}
};

#endif
