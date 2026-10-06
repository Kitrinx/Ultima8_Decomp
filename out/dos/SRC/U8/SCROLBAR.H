#ifndef SCROLBAR_H
#define SCROLBAR_H

#include "BUTTON.H"
#include "CRECT.H"
#include "DISPATCH.H"

class DrawMyselfActivatorButton;
class Event;

class Thumb : public NewGump
{
public:
	long resolution;
	unsigned char vertical;
	unsigned char dragging;
	Rect thumbRect;
	long thumb;
	long scale;			// pixels per step, 16.16 fixed point

	Thumb(NewGump *, Rect, unsigned char, long);
	virtual void commandMouseLeft(Event &);
	virtual void commandMouseMovement(Event &);
	virtual void draw(short, short);
	virtual void setThumb(long);
	virtual long getThumb(void) { return thumb; }
	virtual void setResolution(int);
	virtual void incThumb(int step) { setThumb(thumb + step); }
	virtual void decThumb(int step) { setThumb(thumb - step); }
};

class ScrollBar : public NewGump
{
public:
	Thumb *pos;

	ScrollBar(unsigned char, NewGump *, int, int, short, short, unsigned char, int);
	virtual void commandPrivate(Event &);
	virtual void setPos(int);
	virtual int getPos(void);
	virtual void setRange(int);
};

class DrawMyselfActivatorButton : public ActivatorButton
{
public:
	unsigned pressed;
	unsigned char color;

	DrawMyselfActivatorButton(unsigned char id, NewGump *parent, Rect r, unsigned char color) :
		ActivatorButton(id, parent) { rect = r; this->color = color; pressed = 0; }
	virtual void select(void) { pressed = 1; refresh(); }
	virtual void unselect(void) { pressed = 0; refresh(); }
};

class RightActivatorButton : public DrawMyselfActivatorButton
{
public:
	RightActivatorButton(unsigned char id, NewGump *parent, Rect r, unsigned char color) :
		DrawMyselfActivatorButton(id, parent, r, color) {}
	virtual void draw(short, short);
};

class LeftActivatorButton : public DrawMyselfActivatorButton
{
public:
	LeftActivatorButton(unsigned char id, NewGump *parent, Rect r, unsigned char color) :
		DrawMyselfActivatorButton(id, parent, r, color) {}
	virtual void draw(short, short);
};

class UpActivatorButton : public DrawMyselfActivatorButton
{
public:
	UpActivatorButton(unsigned char id, NewGump *parent, Rect r, unsigned char color) :
		DrawMyselfActivatorButton(id, parent, r, color) {}
	virtual void draw(short, short);
};

class DownActivatorButton : public DrawMyselfActivatorButton
{
public:
	DownActivatorButton(unsigned char id, NewGump *parent, Rect r, unsigned char color) :
		DrawMyselfActivatorButton(id, parent, r, color) {}
	virtual void draw(short, short);
};

#endif
