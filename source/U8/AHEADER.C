// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: AHEADER.C

#include <mem.h>
#include "AHEADER.H"

void AnimHeader::clear(void)
{
	memset(this, 0, sizeof(AnimHeader));
}

Boolean AnimHeader::isData(void)
{
	for (int i = 0; i < 64; i++)
		if (data[i])
			return TRUE;
	return FALSE;
}
