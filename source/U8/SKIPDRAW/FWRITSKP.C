// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: SKIPDRAW\FWRITSKP.C

#include "CVPORT.H"
#include "FWRITSKP.H"
#include "IWRITSKP.H"

// Skips to (newX, newY), writing the skips and row offsets on the way.
int FastSkipMaker::skip_to(short newX, short newY)
{
	int size = 0;
	if (y < newY)
	{
		if (x < width)
		{
			size += add_skip(width - x);
		}
		x = 0;
		while (y < newY)
		{
			if (y + 1 < height)
			{
				*rowOffset++ = out - (unsigned char *)rowOffset;
			}
			y++;
			if (newY - y == 1)
			{
				size += add_skip(width);
				x = 0;
			}
		}
	}
	if (height > newY)
	{
		size += add_skip(newX - x);
	}
	x = newX;
	return size;
}

// Size of the skip table FastWriteSkip would build, at most 0xFFFF.
unsigned EstimateFastWriteSkip(Vport &vport, short x1, short y1, short x2, short y2, short skipColor)
{
	int width = x2 - x1 + 1;
	int height = y2 - y1 + 1;
	ShapeScanner scanner(vport, x1, y1, x2, y2, skipColor);
	FastSkipMaker maker(0, width, height, 0, 0);
	long size = height * 2 + 10L;

	while (scanner.find_segment())
	{
		size += maker.measure_skip_to(scanner.x - scanner.left, scanner.y - scanner.top);
		size += maker.measure_run(scanner.runLength);
	}
	size += maker.measure_skip_to(0, height);
	if (size > 0xFFFF)
	{
		size = 0xFFFF;
	}
	return size;
}

// Builds the skip table of a Vport rectangle into buffer; returns its size.
int FastWriteSkip(Vport &vport, short x1, short y1, short x2, short y2,
	short hotX, short hotY, unsigned char *buffer, short skipColor)
{
	int width = x2 - x1 + 1;
	int height = y2 - y1 + 1;
	ShapeScanner scanner(vport, x1, y1, x2, y2, skipColor);
	FastSkipMaker maker(buffer, width, height, hotX - x1, hotY - y1);

	while (scanner.find_segment())
	{
		maker.skip_to(scanner.x - scanner.left, scanner.y - scanner.top);
		maker.add_run(scanner.runLength, scanner.pos);
	}
	maker.skip_to(0, height);
	return maker.out - maker.start;
}
