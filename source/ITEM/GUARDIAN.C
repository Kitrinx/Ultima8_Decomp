// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: ..\ITEM\GUARDIAN.C

#include <string.h>
#include <mem.h>
#include "CACHE.H"
#include "ITEM.H"
#include "ITEMCACH.H"
#include "GLOB.H"
#include "SHAPHAND.H"
#include "CFILE.H"
#include "FEXIST.H"
#include "FILESPEC.H"
#include "CEXIT.H"
#include "SCRATCHM.H"
#include "STATREST.H"
#include "UPROCESS.H"
#include "TYPE.H"


unsigned char WorldPoint::operator==(WorldPoint &p)
{
	if (x == p.x && y == p.y && z == p.z)
		return TRUE;
	return FALSE;
}

#define GUARDIAN_BARK_EVENT	0x15

// Runs the Avatar's guardian bark usecode one time in three.
int guardianBark(int num)
{
	unsigned short pid;
	if (urandom(3) > 0)
		return 0;
	Item avatar(1);
	long arg = num;
	if (spawnUnk(&avatar, 1L << GUARDIAN_BARK_EVENT, GUARDIAN_BARK_EVENT, pid, arg))
		return pid;
	return 0;
}
