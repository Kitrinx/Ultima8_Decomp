#ifndef CTMPOINT_H
#define CTMPOINT_H

#include "CMPOINT.H"
#include "CRECT.H"

class MemHandle;
class Shape;
class Vport;

// A mouse pointer drawn in software as a shape on a viewport.
class TMMousePointer : public MousePointer
{
public:
	TMMousePointer(void);
	~TMMousePointer(void);
	virtual void handle(int, int, int, int);
	virtual int getFrame(int, int);
	static void map_to(Vport &, unsigned char);
	static void set_shape(MemHandle &);
	static void show(void);
	static void hide(void);
	static void draw(int, int);
	static void undraw(void);
	static void draw(Vport &);
	static void undraw(Vport &);
	static void before_slam(Vport *);
	static void after_slam(Vport *);
	static Vport *mapped_vport;
	static Shape *pointer;
	static Point dimensions;
};

extern TMMousePointer *theMouse;

#endif
