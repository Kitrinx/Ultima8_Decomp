#ifndef SBUTTON_H
#define SBUTTON_H

#include "BUTTON.H"

class GumpShape;
class GumpShapeDrawer;
class NewGump;
class Rect;

class GumpShape
{
public:
	unsigned _frame;
	unsigned shape;

	GumpShape(int);
	Rect get_rect(void);
	void draw(int, int);
	void *get(void);
	void set_frame(int);
	unsigned char collide(int, int);
	static void setDrawer(GumpShapeDrawer *d) { drawer = d; }
	static GumpShapeDrawer *drawer;
};

class ShapePushButton : public PushButton, public GumpShape
{
public:
	ShapePushButton(unsigned char, NewGump *, int);
	virtual void draw(short, short);
	virtual void select(void);
	virtual void unselect(void);
	int set_frame(int);
};

class ShapeCycleButton : public CycleButton, public GumpShape
{
public:
	ShapeCycleButton(unsigned char, NewGump *, int);
	virtual void draw(short, short);
	virtual void select(void);
	virtual void unselect(void);
	int set_frame(int);
};

class ShapeActivatorButton : public ActivatorButton, public GumpShape
{
public:
	ShapeActivatorButton(unsigned char, NewGump *, int);
	virtual void draw(short, short);
	virtual void select(void);
	virtual void unselect(void);
	int set_frame(int);
};

#endif
