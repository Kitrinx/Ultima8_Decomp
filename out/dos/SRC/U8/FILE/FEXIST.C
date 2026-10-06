// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: FILE\FEXIST.C

#include <dir.h>
#include "FEXIST.H"

unsigned char FileExists(const char *name)
{
	struct ffblk ff;
	return !findfirst(name, &ff, 0);
}
