#ifndef U8SBAR_H
#define U8SBAR_H

#include "DISPATCH.H"
#include "SBUTTON.H"
#include "SCROLBAR.H"

class Event;
class Rect;

class U8RightActivatorButton : public DrawMyselfActivatorButton, public GumpShape
{
public:
	U8RightActivatorButton(unsigned char id, NewGump *parent, Rect r, unsigned char color) :
		DrawMyselfActivatorButton(id, parent, r, color), GumpShape(44) { rect = get_rect(); }
	virtual void draw(short, short);
};

class U8LeftActivatorButton : public DrawMyselfActivatorButton, public GumpShape
{
public:
	U8LeftActivatorButton(unsigned char id, NewGump *parent, Rect r, unsigned char color) :
		DrawMyselfActivatorButton(id, parent, r, color), GumpShape(43) { rect = get_rect(); }
	virtual void draw(short, short);
};

// Thumb positions run 0..resolution; scale is pixels per step in 16.16 fixed point.
class U8Thumb : public NewGump, public GumpShape
{
public:
	long resolution;
	unsigned char dragging;
	Rect thumbRect;
	long thumb;
	long scale;

	U8Thumb(NewGump *, Rect, long);
	virtual void commandMouseLeft(Event &);
	virtual void commandMouseMovement(Event &);
	virtual void draw(short, short);
	virtual void setThumb(long);
	virtual long getThumb(void) { return thumb; }
	virtual void setResolution(int);
	virtual void incThumb(int step) { setThumb(thumb + step); }
	virtual void decThumb(int step) { setThumb(thumb - step); }
};

class U8ScrollBar : public NewGump
{
public:
	U8Thumb *pos;

	U8ScrollBar(unsigned char, NewGump *, int, int, short, short);
	virtual void commandPrivate(Event &);
	virtual void setPos(int);
	virtual int getPos(void);
	virtual void setRange(int);
};

#endif
