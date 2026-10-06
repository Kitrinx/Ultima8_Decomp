#ifndef CSHAPE_H
#define CSHAPE_H

#include "CMEMHNDP.H"

class GraphicPoint;
class GraphicPrimitive;
class Rect;
class Vport;

class GraphicPrimitive
{
public:
	virtual void draw(void) = 0;
	virtual void hide(void) = 0;
	virtual void move(short, short) = 0;
};

class GraphicPoint : public GraphicPrimitive
{
public:
	unsigned f_02;
	unsigned f_04;

	virtual void draw(void);
	virtual void hide(void);
	virtual void move(short, short);
	virtual void draw(short, short);
	void set_position(short, short);
	void move_position(short, short);
};

class Shape : public GraphicPoint
{
public:
	MemHandle shape;
	MemHandle background;	// screen under the shape while it is drawn
	unsigned f_18;			// frame
	unsigned savedX;
	unsigned savedY;

	Shape(void);
	Shape(MemHandle &);
	Shape(char *, char, unsigned char);
	Shape(MemHandle &, short, short);
	Shape(MemHandle &, short, short, short);
	Shape(const char *, short, short, short);
	~Shape(void);
	virtual void draw(void);
	virtual void hide(void);
	virtual void move(short, short);
	virtual void draw(short, short);
	virtual void move(short, short, short);
	virtual void move_absolute(short, short);
	virtual void restore(short, short);
	virtual void clear_back(void);
	void free(void);
	void reset(void);
	void set_shape(MemHandle &);
	void set_shape(char *, char, unsigned char);
	void set_frame(short);
	Rect get_rect(short);
	void init_background(void);
	void draw(short);
};

extern "C" void SaveRect(Vport *, void *, Rect *);
extern "C" void RestoreRect(Vport *, void *, Rect *);

#endif
