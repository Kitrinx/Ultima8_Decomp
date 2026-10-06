// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: ..\ITEM\MAPFILE.C

#include <stddef.h>
#include "CFILE.H"
#include "CEXIT.H"
#include "FILESPEC.H"
#include "SCRATCHM.H"
#include "GLOB.H"
#include "ITEMCACH.H"
#include "MAPFILE.H"

// Inline in the shared headers when this file was built.
inline Index::Index(void) { size = 0; offset = 0; }

unsigned long mapNames = 0;

int RegionFile::numItems(unsigned short region)
{
	Index index;
	getIndex(region, index);
	return index.size / sizeof(OneItem);
}

MapFile::MapFile(void) :
	StepFlexFile(::fileSpec(NULL, GamedatDir, nonfixedFileDesc, NULL), 256, 1)
{
}

void MapFile::restart(void)
{
	if (handle != 6)
		close();
	init(::fileSpec(NULL, GamedatDir, nonfixedFileDesc, NULL), (OpenMode)2, -1);
}

void MapFile::resetChecksum(void)
{
	FlexHeader header;
	getHeader(&header);
	header.checksum = getChecksum();
	setHeader(&header);
}

FixedMapFile::FixedMapFile(void) :
	FastFlex(fileSpec(NULL, StaticDir, fixedFileDesc, NULL), NULL, (OpenMode)0, 256)
{
}
