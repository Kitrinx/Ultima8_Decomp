// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: U8POINT.C

#include "CRECT.H"
#include "CFILE.H"
#include "CMEMHNDP.H"
#include "FILESPEC.H"
#include "FVINFO.H"
#include "SCRATCHM.H"
#include "CSHAPE.H"
#include "U8POINT.H"

// Inline in the shared headers when this file was built.
inline Point::Point(void) {}
inline void Point::set(short x, short y) { f_00 = x; f_02 = y; }
inline void Rect::set(short x1, short y1, short x2, short y2) { Point::set(x1, y1); f_04 = x2; f_06 = y2; }
inline Rect::Rect(short x1, short y1, short x2, short y2) { set(x1, y1, x2, y2); }
inline int Rect::width(void) { return f_04 - f_00 + 1; }
inline int Rect::height(void) { return f_06 - f_02 + 1; }
inline unsigned char BaseFile::is_valid(void) { return handle != -1; }
inline BaseFile::~BaseFile(void) { if (is_valid()) close(); }
inline MemHandle::MemHandle(unsigned long size, unsigned char type) { alloc(size, type); }
inline MemHandle::~MemHandle(void) { free(0); }

U8MousePointer *theU8Mouse = 0;
int U8MousePointer::centerX;
int U8MousePointer::centerY;
int U8MousePointer::shape;
int U8MousePointer::maxShapes;
int U8MousePointer::modes[5];
int U8MousePointer::sp;
int U8MousePointer::erection;

U8MousePointer::U8MousePointer(short x, short y)
{
	theU8Mouse = this;
	centerX = x;
	centerY = y;
	BaseFile file(fileSpec(0, StaticDir, "u8mouse.shp", 0), (OpenMode)2);
	int size = file.size();
	MemHandle mem(size, 2);
	set_shape(mem);
	mem.f_04 = 0;
	file.read((char *)mem.f_00, size, -1);
	maxShapes = GetNumShapes((void *)mem.f_00);
	for (int i = 0; i < 5; i++)
		modes[i] = 41;
	sp = -1;
	hide();
}

// How far the pointer is from the avatar: 1 near, 2 middle, 3 far.
int U8MousePointer::calcErection(short x, short y)
{
	static Rect close(-30, -25, 30, 25);
	static Rect screen(10, 10, 310, 190);
	static int halfWidth = close.width() >> 1;
	static int halfHeight = close.height() >> 1;
	close.moveto(centerX - halfWidth, centerY - halfHeight);
	if (!screen.contains(x, y))
		erection = 3;
	else if (!close.contains(x, y))
		erection = 2;
	else
		erection = 1;
	return erection;
}

// Frames 0-7 point in the eight directions, plus 8 for each step of erection.
int U8MousePointer::getFrame(int x, int y)
{
	int mode;
	switch (mode = modes[sp])
	{
	case 0:
	case 25:
		{
			int dx = x - centerX;
			int dy = y - centerY;
			calcErection(x, y);
			if (dx == 0)
				shape = dy < 0 ? 0 : 4;
			else
			{
				int slope = dy * 8 / dx;
				if (slope < -2 && slope > -12)
					shape = dx < 0 ? 5 : 1;
				else if (slope <= -12 || slope >= 12)
					shape = dy <= 0 ? 0 : 4;
				else if (slope < 12 && slope > 2)
					shape = dy <= 0 ? 7 : 3;
				else if (slope <= 2 && slope >= -2)
					shape = dx < 0 ? 6 : 2;
				else
					shape = 0;
			}
			shape += (char)mode;
			if (mode == 0)
				shape += (erection - 1) << 3;
		}
		break;
	default:
		shape = (char)mode;
	}
	if (shape < maxShapes)
		return shape;
	return maxShapes - 1;
}

void U8MousePointer::pushMode(PointerModes mode, unsigned char update)
{
	unsigned char hidden = !visible;
	if (sp < 4)
	{
		if (update && !hidden)
			hide();
		modes[sp + 1] = mode;
		sp++;
		int frame = theU8Mouse->getFrame(old_x, old_y);
		pointer->f_18 = frame;
		if (update && !hidden)
			show();
	}
}

void U8MousePointer::popMode(void)
{
	unsigned char hidden = !visible;
	if (sp > 0)
	{
		if (!hidden)
			hide();
		sp--;
		int frame = theU8Mouse->getFrame(old_x, old_y);
		pointer->f_18 = frame;
		if (!hidden)
			show();
	}
}

int U8MousePointer::getDir(void)
{
	if (modes[sp] == 0)
		return shape & 7;
	if (modes[sp] == 25)
		return (shape - 25) & 7;
	return 0;
}

void U8MousePointer::save(BaseFile *file)
{
	pushCat((CatState)0);
	file->write((char *)modes, sizeof(modes));
	file->write((char *)&sp, sizeof(sp));
	popCat();
}

void U8MousePointer::load(BaseFile *file)
{
	pushCat((CatState)0);
	file->read((char *)modes, sizeof(modes));
	file->read((char *)&sp, sizeof(sp));
	popCat();
}
