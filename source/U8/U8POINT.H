#ifndef U8POINT_H
#define U8POINT_H

#include "CTMPOINT.H"

class BaseFile;

enum PointerModes
{
	PointerModes_0
};

class U8MousePointer : public TMMousePointer
{
public:
	U8MousePointer(short, short);
	virtual int getFrame(int, int);
	static int calcErection(short, short);
	static void pushMode(PointerModes, unsigned char);
	static void popMode(void);
	static int getDir(void);
	static void save(BaseFile *);
	static void load(BaseFile *);
	static int centerX;
	static int centerY;
	static int shape;
	static int maxShapes;
	static int modes[5];
	static int sp;
	static int erection;
};

extern U8MousePointer *theU8Mouse;

#endif
