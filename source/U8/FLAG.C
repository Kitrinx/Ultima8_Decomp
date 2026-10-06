// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: FLAG.C

#include <io.h>
#include <mem.h>
#include <string.h>
#include "CFILE.H"
#include "FEXIST.H"
#include "FILESPEC.H"
#include "ERROR.H"
#include "SCRATCHM.H"
#include "FLAG.H"

// Inline in the shared headers when this file was built.
inline unsigned char BaseFile::is_valid(void) { return handle != -1; }
inline BaseFile::~BaseFile(void) { if (is_valid()) close(); }
inline FileSpec::FileSpec(char *path, int attrib) { become(path, attrib); }

char *flagFileName = "flag.dat";
char *GlobalFlag::flagSym = 0;
int GlobalFlag::flagSymSize = 0;
char *GlobalFlag::bits;

int GlobalFlag::get(int index, int size)
{
	long word = *(long *)(bits + (index >> 3));
	long mask = ((1 << size) - 1) << (index & 7);
	long value = (word & mask) >> (index & 7);
	return value;
}

void GlobalFlag::set(int index, int size, int value)
{
	long *word = (long *)(bits + (index >> 3));
	long mask = ~(((1 << size) - 1) << (index & 7));
	*word &= mask;
	*word |= value << (index & 7);
}

void GlobalFlag::destroy(void)
{
	if (bits)
	{
		delete bits;
		bits = 0;
	}
	if (flagSym)
	{
		delete flagSym;
		flagSym = 0;
	}
}

void GlobalFlag::load(char *dir)
{
	char *name;

	if (!bits)
	{
		bits = new char[0x400];
		if (!bits)
			outOfMemory(__FILE__, 50);	// __LINE__
	}
	memset(bits, 0, 0x400);
	name = fileSpec(0, dir, flagFileName, 0);
	if (FileExists(name))
	{
		BaseFile file(name, ReadWrite);
		file.read(bits, file.size());
	}
	FileSpec spec(fileSpec(0, UsecodeDir, "flagsym.dat", 0), 0);
	if (spec.exists())
	{
		BaseFile file(spec, ReadWrite);
		flagSymSize = file.size();
		if (!flagSym)
		{
			flagSym = new char[flagSymSize];
			if (!flagSym)
				return;
			file.read(flagSym, flagSymSize);
		}
	}
}

void GlobalFlag::save(char *dir)
{
	char *name = fileSpec(0, dir, flagFileName, 0);
	unlink(name);
	BaseFile file;
	file.forceOpen(name, ReadWrite);
	file.write(bits, 0x400);
}

// flagsym.dat holds, per flag, its name and two words: bit index and width.
char GlobalFlag::findFlag(char *name, int &index, int &size)
{
	char found = 0;

	if (flagSym)
	{
		char *p = flagSym;
		for (unsigned pos = 0; pos < flagSymSize; )
		{
			int len = strlen(p) + 1;
			char *sym = p;
			p += len;
			if (!strcmp(sym, name))
			{
				index = ((int *)p)[0];
				size = ((int *)p)[1];
				found = 1;
				break;
			}
			p += 4;
			pos += len + 4;
		}
	}
	return found;
}
