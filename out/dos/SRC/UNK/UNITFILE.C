// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: ..\UNK\UNITFILE.C

#include "CEXIT.H"
#include "FILESPEC.H"
#include "SCRATCHM.H"
#include "FASTFLEX.H"
#include "UPROCESS.H"
#include "UNITFILE.H"

// Every unit record starts with a 12-byte header that load() skips.
#define UNIT_HEADER	12

inline Index::Index(void) { size = 0; offset = 0; }

char *UnitFile::load(char *name)
{
	long magic, size, count;

	open(fileSpec(0, UsecodeDir, name, ".uni"), (OpenMode)2);
	read((char *)&magic, 4);
	read((char *)&size, 4);
	read((char *)&count, 4);
	unsigned length = size - UNIT_HEADER;
	char *data = new char[length];
	if (data == 0)
		outOfMemory(__FILE__, 81);	// __LINE__
	read(data, length);
	return data;
}

char *UnitFile::load(unsigned unit)
{
	Index index;
	usecodeFile->getIndex(unit + 2, index);
	if (index.size <= UNIT_HEADER)
		halt(__FILE__, 99);	// __LINE__
	char *data = new char[index.size - UNIT_HEADER];
	if (data == 0)
		outOfMemory(__FILE__, 104);	// __LINE__
	usecodeFile->readRecord(unit + 2, data, index.size - UNIT_HEADER, UNIT_HEADER);
	return data;
}

int UnitFile::getSize(unsigned unit)
{
	Index index;
	usecodeFile->getIndex(unit + 2, index);
	return index.size - UNIT_HEADER;
}
