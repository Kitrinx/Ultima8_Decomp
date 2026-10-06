// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: SKIPDRAW\IWRITSKP.C

#include <dos.h>
#include <mem.h>
#include "CVPORT.H"
#include "IWRITSKP.H"

ShapeScanner::ShapeScanner(Vport &vport, short x1, short y1, short x2, short y2, short color)
{
	pos = (unsigned char *)MK_FP(vport.seg, ((int *)vport.graphicYtable.index)[y1] + x1);
	lineSkip = (vport.rect.f_04 - vport.rect.f_00 + 1) - (x2 - x1 + 1);
	runLength = 0;
	x = left = x1;
	y = top = y1;
	right = x2;
	bottom = y2;
	skipColor = color;
}

// Moves past the last run to the next one; its length goes in runLength.
unsigned char ShapeScanner::find_segment(void)
{
	x += runLength;
	pos += runLength;
	runLength = 0;
	while (y <= bottom)
	{
		while (x <= right)
		{
			if (*pos != skipColor)
			{
				unsigned char *p = pos;
				int i = x;
				while (i <= right && *p++ != skipColor)
				{
					i++;
					runLength++;
				}
				return 1;
			}
			x++;
			pos++;
		}
		x = left;
		y++;
		pos += lineSkip;
	}
	return 0;
}

FastSkipMaker::FastSkipMaker(unsigned char *buffer, short w, short h, short hotX, short hotY)
{
	if (w < 0 || h < 0)
	{
		w = h = 0;
	}
	width = w;
	height = h;
	rowOffset = (short *)(start = buffer);
	out = buffer + h * 2 + 10;
	if (buffer)
	{
		*rowOffset++ = 0;
		*rowOffset++ = w;
		*rowOffset++ = h;
		*rowOffset++ = hotX;
		*rowOffset++ = hotY;
		if (h)
		{
			*rowOffset++ = out - (unsigned char *)rowOffset;
		}
	}
	x = y = 0;
}

int FastSkipMaker::add_run(short len, unsigned char *pixels)
{
	int size = 0;
	x += len;
	while (len > 255)
	{
		*out++ = 255;
		memmove(out, pixels, 255);
		out += 255;
		pixels += 255;
		*out++ = 0;
		size += 257;
		len -= 255;
	}
	*out++ = len;
	memmove(out, pixels, len);
	out += len;
	size += len + 1;
	return size;
}

unsigned FastSkipMaker::measure_run(short len)
{
	unsigned size = 0;
	x += len;
	while (len > 255)
	{
		size += 257;
		len -= 255;
	}
	size += len + 1;
	return size;
}

int FastSkipMaker::add_skip(short len)
{
	int size = 0;
	x += len;
	while (len > 255)
	{
		*out++ = 255;
		*out++ = 0;
		size += 2;
		len -= 255;
	}
	*out++ = len;
	size++;
	return size;
}

unsigned FastSkipMaker::measure_skip(short len)
{
	unsigned size = 0;
	x += len;
	while (len > 255)
	{
		size += 2;
		len -= 255;
	}
	size++;
	return size;
}

unsigned FastSkipMaker::measure_skip_to(short newX, short newY)
{
	unsigned size = 0;
	if (y < newY)
	{
		if (x < width)
		{
			size += measure_skip(width - x);
		}
		x = 0;
		while (y < newY)
		{
			y++;
			if (newY - y == 1)
			{
				size += measure_skip(width);
				x = 0;
			}
		}
	}
	if (height > newY)
	{
		size += measure_skip(newX - x);
	}
	x = newX;
	return size;
}
