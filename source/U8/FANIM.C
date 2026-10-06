// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: FANIM.C

#include "ANIMCACH.H"
#include "WPNCACHE.H"

void initAnimCache(void)
{
	AnimCache::init();
}

void uninitAnimCache(void)
{
	AnimCache::uninit();
}
