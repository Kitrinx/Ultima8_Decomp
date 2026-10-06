// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: SKIPDRAW\WRITSKP.C

#include <mem.h>
#include "CVPORT.H"
#include "WRITSKP.H"

SkipMaker::SkipMaker(unsigned char *buffer, short w, short h, short hotX, short hotY, unsigned short flags) :
	FastSkipMaker(buffer, w, h, hotX, hotY)
{
	if (buffer)
	{
		*(short *)start = flags;
	}
	firstRow = rowOffset;
	if (h)
	{
		firstRow--;
	}
	blankRow = 0;
}

// As FastSkipMaker::skip_to, but a finished row that matches an earlier one
// is dropped and shares that row's data, and blank rows share one copy.
int SkipMaker::skip_to(short newX, short newY)
{
	int size = 0;
	unsigned i;

	if (y < newY)
	{
		if (x < width)
		{
			size += add_skip(width - x);
		}
		x = 0;
		if (y > 0)
		{
			short *row = firstRow;
			unsigned char *last = (unsigned char *)(rowOffset - 1) + rowOffset[-1];
			for (i = 0; i < y - 1; i++)
			{
				unsigned char *data = (unsigned char *)row + *row;
				if (memcmp(data, last, out - last) == 0)
				{
					out = last;
					rowOffset[-1] = data - (unsigned char *)(rowOffset - 1);
					break;
				}
				row++;
			}
		}
		while (y < newY)
		{
			if (y + 1 < height)
			{
				if (newY - y > 1)
				{
					if (blankRow == 0)
					{
						blankRow = out;
						size += add_skip(width);
						x = 0;
					}
					*rowOffset++ = blankRow - (unsigned char *)rowOffset;
				}
				else
				{
					*rowOffset++ = out - (unsigned char *)rowOffset;
				}
			}
			y++;
		}
	}
	if (height > newY)
	{
		size += add_skip(newX - x);
	}
	x = newX;
	return size;
}

// True when RLE coding the run makes it smaller.
unsigned char SkipMaker::segment_can_rle(short len, unsigned char *pixels)
{
	unsigned savedX = x;
	unsigned rleSize;
	unsigned plainSize = measure_run(len);
	rleSize = measure_rle_run(len, pixels);
	x = savedX;
	return rleSize < plainSize;
}

// Run counts are doubled; an odd count marks a repeated pixel.
int SkipMaker::add_non_rle_run(short len, unsigned char *pixels)
{
	int size = 0;
	x += len;
	while (len > 127)
	{
		*out++ = 127 * 2;
		memmove(out, pixels, 127);
		out += 127;
		pixels += 127;
		*out++ = 0;
		size += 129;
		len -= 127;
	}
	*out++ = len * 2;
	memmove(out, pixels, len);
	out += len;
	size += len + 1;
	return size;
}

unsigned SkipMaker::measure_non_rle_run(short len)
{
	unsigned size = 0;
	x += len;
	while (len > 127)
	{
		size += 129;
		len -= 127;
	}
	size += len + 1;
	return size;
}

// Four or more equal pixels become a repeat; the rest stay literal.
int SkipMaker::add_rle_run(short len, unsigned char *pixels)
{
	int i;
	int color;
	int pending = 0;
	int repeat = 0;
	int last = -1;
	unsigned char *p = pixels;
	int size = 0;
	unsigned char needSkip = 0;

	for (i = 0; i < len; i++)
	{
		color = *p++;
		pending++;
		if (color == last)
		{
			if (++repeat == 4)
			{
				if (needSkip)
				{
					*out++ = 0;
					size++;
				}
				if (pending - 4 > 0)
				{
					size += add_non_rle_run(pending - 4, pixels);
					pixels += pending - 4;
					*out++ = 0;
					size++;
				}
				while (i < len - 1 && *p == color)
				{
					p++;
					repeat++;
					i++;
				}
				pending = 0;
				pixels += repeat;
				x += repeat;
				while (repeat > 127)
				{
					*out++ = 127 * 2 + 1;
					*out++ = color;
					*out++ = 0;
					size += 3;
					repeat -= 127;
				}
				*out++ = repeat * 2 + 1;
				*out++ = color;
				size += 2;
				repeat = 0;
				needSkip = 1;
			}
		}
		else
		{
			last = color;
			repeat = 1;
		}
	}
	if (pending)
	{
		if (needSkip)
		{
			*out++ = 0;
			size++;
		}
		size += add_non_rle_run(pending, pixels);
	}
	return size;
}

unsigned SkipMaker::measure_rle_run(short len, unsigned char *pixels)
{
	int i;
	int color;
	int pending = 0;
	int repeat = 0;
	int last = -1;
	unsigned char *p = pixels;
	unsigned size = 0;
	unsigned char needSkip = 0;

	for (i = 0; i < len; i++)
	{
		color = *p++;
		pending++;
		if (color == last)
		{
			if (++repeat == 4)
			{
				if (needSkip)
				{
					size++;
				}
				if (pending - 4 > 0)
				{
					size += measure_non_rle_run(pending - 4);
					size++;
				}
				pending = 0;
				while (i < len - 1 && *p == color)
				{
					p++;
					repeat++;
					i++;
				}
				x += repeat;
				while (repeat > 127)
				{
					size += 3;
					repeat -= 127;
				}
				size += 2;
				needSkip = 1;
			}
		}
		else
		{
			last = color;
			repeat = 1;
		}
	}
	if (pending)
	{
		if (needSkip)
		{
			size++;
		}
		size += measure_non_rle_run(pending);
	}
	return size;
}

// Turns an unused RLE table back into plain counts and clears its flag.
void SkipMaker::optimize(void)
{
	unsigned char *p;
	unsigned char *lastRow = start;

	*(short *)start &= ~SKIP_RLE;
	for (unsigned i = 0; i < height; i++)
	{
		p = (unsigned char *)(firstRow + i) + firstRow[i];
		// Shared rows are converted once.
		if (p > lastRow)
		{
			lastRow = p;
			x = 0;
			while (x < width)
			{
				x += *p++;
				if (x < width)
				{
					int n = *p >> 1;
					*p++ = n;
					x += n;
					p += n;
				}
			}
		}
	}
}

unsigned EstimateWriteSkip(Vport &vport, short x1, short y1, short x2, short y2,
	short skipColor, unsigned short flags)
{
	int width = x2 - x1 + 1;
	int height = y2 - y1 + 1;
	ShapeScanner scanner(vport, x1, y1, x2, y2, skipColor);
	SkipMaker maker(0, width, height, 0, 0, flags);
	long size = height * 2 + 10L;

	while (scanner.find_segment())
	{
		size += maker.measure_skip_to(scanner.x - scanner.left, scanner.y - scanner.top);
		if (flags & SKIP_RLE)
		{
			if (maker.segment_can_rle(scanner.runLength, scanner.pos))
			{
				size += maker.measure_rle_run(scanner.runLength, scanner.pos);
			}
			else
			{
				size += maker.measure_non_rle_run(scanner.runLength);
			}
		}
		else
		{
			size += maker.measure_run(scanner.runLength);
		}
	}
	size += maker.measure_skip_to(0, height);
	if (size > 0xFFFF)
	{
		size = 0xFFFF;
	}
	return size;
}

int WriteSkip(Vport &vport, short x1, short y1, short x2, short y2, short hotX, short hotY,
	unsigned char *buffer, short skipColor, unsigned short flags)
{
	int width = x2 - x1 + 1;
	int height = y2 - y1 + 1;
	unsigned char usedRle = 0;
	ShapeScanner scanner(vport, x1, y1, x2, y2, skipColor);
	SkipMaker maker(buffer, width, height, hotX - x1, hotY - y1, flags);

	while (scanner.find_segment())
	{
		maker.skip_to(scanner.x - scanner.left, scanner.y - scanner.top);
		if (flags & SKIP_RLE)
		{
			if (maker.segment_can_rle(scanner.runLength, scanner.pos))
			{
				maker.add_rle_run(scanner.runLength, scanner.pos);
				usedRle = 1;
			}
			else
			{
				maker.add_non_rle_run(scanner.runLength, scanner.pos);
			}
		}
		else
		{
			maker.add_run(scanner.runLength, scanner.pos);
		}
	}
	maker.skip_to(0, height);
	if ((flags & SKIP_RLE) && !usedRle)
	{
		maker.optimize();
	}
	return maker.out - maker.start;
}
