// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: ..\XCACHE\HANDLER.C

#include <mem.h>
#include "CEXIT.H"
#include "HANDLER.H"

unsigned long *CacheHandler::handles = 0;
int CacheHandler::numHandles;

CacheHandler::~CacheHandler(void)
{
	CacheNodePtr::detach(type);
}

void CacheHandler::init(void)
{
	handles = new unsigned long[MAX_HANDLES];
	if (handles == 0)
		outOfMemory(__FILE__, 43);	// __LINE__
	memset(handles, 0, MAX_HANDLES * sizeof(unsigned long));
	numHandles = 0;
}

// Hands out the next run of count handles; returns its first.
int CacheHandler::reserve(int count)
{
	if (count + numHandles < MAX_HANDLES)
	{
		int first = numHandles;
		numHandles += count;
		return first;
	}
	halt(__FILE__, 56);	// __LINE__
}

void initCacheHandler(void)
{
	CacheHandler::init();
}
