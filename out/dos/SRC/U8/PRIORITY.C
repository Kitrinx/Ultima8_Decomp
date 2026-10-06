// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: PRIORITY.C

#include "ERROR.H"
#include "PRIORITY.H"

unsigned char PriorityClass::isValid(void)
{
	if ((valid & 0xe0) == 0xe0)
		return 1;
	if ((valid & 0x1f) && (valid & 0xe0))
		return 1;
	return 0;
}

unsigned char PriorityClass::contains(unsigned char priority)
{
	unsigned char level = valid & 0x1f;
	unsigned char tests = valid & 0xe0;

	if (priority == 0 || (priority & 0xe0))
		halt(__FILE__, 40);	// __LINE__
	if ((tests & 0x40) && priority < level)
		return 1;
	if ((tests & 0x20) && priority == level)
		return 1;
	if ((tests & 0x80) && priority > level)
		return 1;
	return 0;
}
