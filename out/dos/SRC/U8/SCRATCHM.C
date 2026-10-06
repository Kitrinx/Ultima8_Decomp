// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: SCRATCHM.C

#include <alloc.h>
#include "ERROR.H"
#include "SCRATCHM.H"

#define TRACE_MEMORY	((TraceLevel)50)

ScratchMem *theScratchMem = 0;

void ScratchMem::init(void)
{
	if (!buffer)
	{
		buffer = (char *)farmalloc(0x10000L);
		locked = 0;
	}
}

void ScratchMem::uninit(void)
{
	if (buffer)
	{
		farfree(buffer);
		buffer = 0;
	}
}

// Lends out the 64K scratch buffer, or a new block if it is already lent.
char *ScratchMem::checkOut(unsigned short size)
{
	char *p = 0;

	if (locked)
	{
		trace(TRACE_MEMORY, "Scratch already locked! ");
		p = new char[size];
	}
	else
	{
		locked = 1;
		p = buffer;
	}
	return p;
}

void ScratchMem::checkIn(char *p)
{
	if (buffer == p)
		locked = 0;
	else
		delete p;
}

void initScratchBuffer(void)
{
	if (!theScratchMem)
	{
		theScratchMem = new ScratchMem;
		theScratchMem->init();
	}
}

void uninitScratchBuffer(void)
{
	if (theScratchMem)
	{
		theScratchMem->uninit();
		delete theScratchMem;
		theScratchMem = 0;
	}
}

void trace(char *, ...)
{
}
