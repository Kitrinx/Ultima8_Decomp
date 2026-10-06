// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: ..\SOUND\SONARC.C

#include <dos.h>
#include <mem.h>
#include "CDESC.H"
#include "CEXIT.H"
#include "DSFXMAN.H"
#include "SONARC.H"

// Moves a selector's base forward by offset bytes.
void incPtr(char *ptr, int offset)
{
	Desc *desc = &LocalDescriptorTable::table[FP_SEG(ptr) >> 3];
	unsigned long base = desc->getBase() + offset;
	desc->baseLow = base;
	desc->baseMid = base >> 16;
	desc->baseHigh = base >> 24;
}

// Decompresses Sonarc blocks from src to dest; returns the bytes written.
long decompress(char *src, long srcLength, char *dest, int toUnsigned)
{
	unsigned long srcBase = LocalDescriptorTable::table[FP_SEG(src) >> 3].getBase();
	unsigned long destBase = LocalDescriptorTable::table[FP_SEG(dest) >> 3].getBase();
	long total = 0;

	while (srcLength != 0)
	{
		int packed = ((int *)src)[0];
		int length = ((int *)src)[1];
		int headerLength = src[7];

		memset(decompressionBuffer, 0, headerLength);
		if (DCMP8(src, (char *)decompressionBuffer))
		{
			halt(__FILE__, 155);	// __LINE__
		}
		if (toUnsigned)
		{
			char *p = (char *)decompressionBuffer + headerLength;
			for (int i = 0; i < length; i++, p++)
			{
				*p += 0x80;
			}
		}
		memcpy(dest, (char *)decompressionBuffer + headerLength, length);
		total += length;
		incPtr(dest, length);
		incPtr(src, packed);
		srcLength -= packed;
	}
	LocalDescriptorTable::table[FP_SEG(src) >> 3].setBase(srcBase);
	LocalDescriptorTable::table[FP_SEG(dest) >> 3].setBase(destBase);
	return total;
}
