// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: GRAPHICS\CVPORT.C

#include <dos.h>
#include <mem.h>
#include "CRECT.H"
#include "CVPORT.H"

inline void Point::set(short x, short y) { f_00 = x; f_02 = y; }
inline void Rect::set(short x1, short y1, short x2, short y2) { Point::set(x1, y1); f_04 = x2; f_06 = y2; }
inline int Rect::width(void) { return f_04 - f_00 + 1; }
inline int Rect::height(void) { return f_06 - f_02 + 1; }
inline Rect *Vport::getRect(void) { return &rect; }
inline void GraphicYtable::setType(unsigned char type) { f_04 = type; }

unsigned VgaScreenDescriptor;
unsigned char SegmentInterval[2];

unsigned char Vport::alloc(int keepYtable)
{
	char *mem = 0;
	unsigned w = rect.width();
	unsigned h = rect.height();
	unsigned long size = h * w + 2;

	if ((mem = new char[size]) != 0)
	{
		if (mem)
			seg = FP_SEG(mem);
		if (graphicYtable.index && keepYtable)
		{
			if (f_10 & 1)
				delete mem;
		}
		else if (!init_ytable(FP_OFF(mem)))
		{
			delete mem;
			mem = 0;
		}
		if (mem)
		{
			seg = FP_SEG(mem);
			f_10 |= 1;
		}
	}
	return mem != 0;
}

void Vport::map_to_screen(unsigned char color)
{
	f_0f = 2;
	rect.set(0, 0, 319, 199);
	seg = VgaScreenDescriptor;
	if (init_ytable(0) == 0)
		Fatal("Failed creating Ytable!");
	clear(color);
}

unsigned char Vport::init_ytable(unsigned short start)
{
	unsigned short *oldIndex = graphicYtable.index;
	Desc *desc = LocalDescriptorTable::getDesc(seg);
	long base = desc->getBase();

	if (base & 3)
		start += 2;
	if (graphicYtable.index && !(unsigned char)(f_10 & 2))
		graphicYtable.index = 0;
	graphicYtable.setType(f_0f);
	unsigned char ok = graphicYtable.init_ytable(rect.width(), rect.height(), start);
	if (ok)
		f_10 |= 2;
	else
		graphicYtable.index = oldIndex;
	return ok;
}

void Vport::free(void)
{
	if (seg)
	{
		if (f_10 & 1)
			;
		(f_10 & 2) && graphicYtable.index ? (delete graphicYtable.index, graphicYtable.index = 0) : 0;
		if (f_10 & 1)
			delete MK_FP(seg, 0);
		f_10 = 0;
	}
}

Vport &Vport::operator=(Vport &v)
{
	free();
	memcpy(this, &v, sizeof(Vport));
	f_10 = 0;
	return *this;
}

void Vport::copy_to(Vport *dst)
{
	CopyViewport(dst, this);
}

void Vport::copy_from(Vport *src)
{
	CopyViewport(this, src);
}

void Vport::clear(unsigned char color)
{
	ClearViewport(this, color);
}

void Vport::fill(Rect &r, unsigned char color)
{
	DrawBox(this, r.f_00, r.f_02, r.f_04, r.f_06, color);
}

void Vport::fill(short x1, short y1, short x2, short y2, unsigned char color)
{
	DrawBox(this, x1, y1, x2, y2, color);
}

// Copies r (all of this viewport if r is empty) to dst at x, y, clipped to dst.
void Vport::copy_to(Vport *dst, short x, short y, Rect r)
{
	if (r.width() <= 0 && r.height() <= 0)
		r = rect;
	else
		r.clip(rect);

	Rect dr = *dst->getRect();
	int rows = r.height();
	char *src = (char *)MK_FP(seg, *graphicYtable.index) + r.f_02 * rect.width() + r.f_00;
	char *to = (char *)MK_FP(dst->seg, *dst->graphicYtable.index) + y * dr.width() + x;
	int srcStride = rect.width();
	int dstStride = dr.width();
	int cols = r.width();
	int skip;
	int i;

	skip = dr.f_02 - y;
	if (skip > 0)
	{
		rows -= skip;
		src += rect.width() * skip;
		to += dr.width() * skip;
	}
	if (y + r.height() >= dr.f_06)
		rows -= r.f_06 - r.f_02 + y - dr.f_06;
	skip = dr.f_00 - x;
	if (skip > 0)
	{
		cols -= skip;
		src += skip;
		to += skip;
	}
	if (x + r.width() >= dr.f_04)
		cols -= r.f_04 - r.f_00 + x - dr.f_04;
	if (rows > 0 && cols > 0)
	{
		for (i = 0; i < rows; i++, src += srcStride, to += dstStride)
			memcpy(to, src, cols);
	}
}

void VideoRestore::exit_code(void)
{
	activate();
}
