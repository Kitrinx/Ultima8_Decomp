#ifndef SHAPHAND_H
#define SHAPHAND_H

#include "FASTFLEX.H"
#include "HANDLER.H"

class Rect;

// A shape's frame table as it sits in the cache. A frame's offset is
// its cache offset once loaded; FRAME_ON_DISK marks a file offset.
struct FrameSlot
{
	unsigned long offset;
	unsigned size;
};

struct ShapeHeader
{
	unsigned maxWidth;
	unsigned maxHeight;
	unsigned numFrames;
};

struct ShapeIndex : public ShapeHeader
{
	FrameSlot frames[1];
};

// A loaded frame: who owns it and the table entry it replaced, then
// the frame as stored in the file.
struct FrameData
{
	unsigned shape;
	unsigned frame;
	unsigned long oldOffset;
};

struct FrameHeader
{
	unsigned compression;
	int width;
	int height;
	int xoff;
	int yoff;
};

#define FRAME_ON_DISK	0x80000000L
#define FRAME_FLAG	0x40000000L

class ShapeFlex : public FastFlex
{
public:
	ShapeFlex(void);
};

class ShapeHandler : public CacheHandler
{
public:
	ShapeHandler(void);
	virtual void cacheOut(unsigned long, unsigned);
	virtual unsigned char verify(unsigned long);
	void getDim(unsigned short, unsigned short, int &, int &, int &, int &);
	void getInfo(unsigned short, int &, int &, int &);
	void getRect(unsigned short, unsigned short, Rect &, int, int);
	char *loadFrame(unsigned short, unsigned short, unsigned long, unsigned long);
	char *get(unsigned short, unsigned short);
	char *get(unsigned short, unsigned short, unsigned short &);
	void flushType(unsigned short);
	void flushFrame(unsigned long, unsigned short);
	static unsigned startHandle;
	static ShapeFlex *shapeFile;
};

class FrameHandler : public CacheHandler
{
public:
	FrameHandler(void);
	virtual void notifyMoved(unsigned long);
	virtual void cacheOut(unsigned long, unsigned);
};

void initShapeHandler(void);
void uninitShapeHandler(void);

extern ShapeHandler *shapeHandler;
extern FrameHandler *frameHandler;

#endif
