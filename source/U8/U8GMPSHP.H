#ifndef U8GMPSHP_H
#define U8GMPSHP_H

class GumpShapeDrawer;
class Rect;
class FlexShapeHandler;

class GumpShapeDrawer
{
public:
	virtual void draw(int, int, int, int) = 0;
	virtual Rect get_rect(int, int) = 0;
	virtual void *get(int) = 0;
	virtual unsigned char collide(short, short, int, short) = 0;
};

class U8GumpShapeDrawer : public GumpShapeDrawer
{
public:
	U8GumpShapeDrawer(void);
	~U8GumpShapeDrawer(void);
	virtual void draw(int, int, int, int);
	virtual Rect get_rect(int, int);
	virtual void *get(int);
	virtual unsigned char collide(short, short, int, short);
	static FlexShapeHandler *theGumpShapeHandler;
	static Rect *rects;
};

#endif
