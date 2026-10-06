#ifndef MAPFILE_H
#define MAPFILE_H

#include "FASTFLEX.H"
#include "STEPFLEX.H"

class RegionFile : public FlexFile
{
public:
	int numItems(unsigned short);
};

// Movable map items, one record per map; the working copy lives in GAMEDAT.
class MapFile : public StepFlexFile
{
public:
	MapFile(void);
	void restart(void);
	void resetChecksum(void);
};

// Fixed map items, one record per map, read from STATIC.
class FixedMapFile : public FastFlex
{
public:
	FixedMapFile(void);
};

extern unsigned long mapNames;

#endif
