// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: GRAPHICS\CPALETTE.C

#include "CPALETTE.H"
#include "CDESC.H"
#include "CMEMHNDP.H"
#include "PAL.H"

inline int min(int a, int b) { return a < b ? a : b; }

void PaletteData::init(unsigned char first, short n)
{
	start = first;
	count = n;
	colors = new RGB[n];
}

void PaletteData::init(RGB *rgb)
{
	RGB *from = rgb;
	RGB *to = colors;
	int i;

	for (i = 0; i < count; i++, from++, to++)
	{
		to->f_00 = from->f_00;
		to->f_01 = from->f_01;
		to->f_02 = from->f_02;
	}
}

// The data holds the first index (byte), a word count, then the RGB triples.
void PaletteData::init(MemHandle &h)
{
	if (colors)
	{
		delete colors;
		colors = 0;
	}
	char *p = (char *)h.f_00;
	start = *p;
	p += 2;
	count = *(short *)p;
	p += 2;
	colors = new RGB[count];
	if (colors)
		init((RGB *)p);
	h.free(0);
}

void PaletteData::init(unsigned char first, short n, RGB *rgb)
{
	init(first, n);
	init(rgb);
}

void PaletteData::grab(void)
{
	if (colors)
		GETCOLORBANKRGB(start, count, colors);
}

void PaletteData::activate(unsigned char first, short n, int steps)
{
	if (colors)
	{
		if (start > first)
			first = start;
		if (start + count < first + n)
			n = start + count - first;
		RGB *rgb = colors + (first - start);
		if (steps)
		{
			int chunk = n / steps;
			unsigned char at = first;
			int left = n;
			if (n % steps)
				chunk++;
			while (chunk > 0)
			{
				SETCOLORBANKRGB(at, chunk, rgb);
				at += chunk;
				rgb += chunk;
				chunk = min(left -= chunk, chunk);
			}
		}
		else
			SETCOLORBANKRGB(first, n, rgb);
	}
}

void PaletteData::grab(unsigned char first, short n)
{
	if (colors)
	{
		if (start > first)
			first = start;
		if (start + count < first + n)
			n = start + count - first;
		GETCOLORBANKRGB(first, n, colors + (first - start));
	}
}

void PaletteData::clear(RGB &color, unsigned char first, short n)
{
	if (colors)
	{
		if (start > first)
			first = start;
		if (start + count < first + n)
			n = start + count - first;
		RGB *rgb = colors;
		colors = colors + (first - start);
		int i;
		for (i = 0; i < n; i++, rgb++)
		{
			rgb->f_00 = color.f_00;
			rgb->f_01 = color.f_01;
			rgb->f_02 = color.f_02;
		}
	}
}

void PaletteData::copy_from(PaletteData &from, unsigned char first, short n)
{
	if (colors && from.colors)
	{
		if (start > first)
			first = start;
		if (start + count < first + n)
			n = start + count - first;
		RGB *src = from.colors;
		RGB *dst = colors + from.start;
		int i;
		for (i = 0; i < from.count; i++, src++, dst++)
		{
			dst->f_00 = src->f_00;
			dst->f_01 = src->f_01;
			dst->f_02 = src->f_02;
		}
	}
}

void PaletteData::set(RGB &color, unsigned char index)
{
	if (start < index && start + count > index)
		SETCOLORRGB(index, &color);
}

unsigned char PaletteData::fade_step(PaletteData &target)
{
	RGB *to = colors + target.start;
	RGB *from = target.colors;
	return MERGEBYTEARRAY((char *)to, (char *)from, target.count * 3);
}

PaletteData::~PaletteData(void)
{
	if (colors)
		delete colors;
}

void Palette::init(MemHandle &h)
{
	PaletteData::init(h);
	activate(start, count, 0);
}

void Palette::fade_to_color(RGB &color, int steps, unsigned char first, short n)
{
	RGB *rgb = new RGB[n];
	int i;
	for (i = 0; i < n; i++)
	{
		rgb[i].f_00 = color.f_00;
		rgb[i].f_02 = color.f_02;
		rgb[i].f_01 = color.f_01;
	}
	PaletteData target(first, n, rgb);
	while (fade_step(target))
		activate(steps);
}

void Palette::fade_from_color(RGB &color, int steps, unsigned char first, short n)
{
	PaletteData target(first, n);
	target.copy_from(*this);
	clear(color, first, n);
	while (fade_step(target))
		activate(steps);
}
