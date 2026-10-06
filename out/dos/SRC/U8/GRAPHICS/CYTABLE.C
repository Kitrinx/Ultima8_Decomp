// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: GRAPHICS\CYTABLE.C

#include "CYTABLE.H"

unsigned char GraphicYtable::verify_ytable(short step, short count)
{
	short width = step;
	unsigned n = count;
	unsigned i;

	if (index == 0)
		return 0;
	for (i = 0; i < n; i++)
		if (index[i] != index[0] + i * width)
			return 0;
	return 1;
}

unsigned char GraphicYtable::init_ytable(short step, short count, unsigned short start)
{
	unsigned long n = (unsigned)count;
	unsigned long width = (unsigned)step;
	unsigned i;

	if (index)
		free();
	if ((index = new unsigned short[n]) == 0)
		return 0;
	if (index) {
		for (i = 0; i < n; i++)
			setindex(i, start + i * width);
		return 1;
	}
	return 0;
}
