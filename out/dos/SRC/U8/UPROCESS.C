// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: UPROCESS.C

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <dos.h>
#include "CFILE.H"
#include "FILESPEC.H"
#include "FEXIST.H"
#include "FASTFLEX.H"
#include "UNITFILE.H"
#include "ERROR.H"
#include "SCRATCHM.H"
#include "SYSTEM.H"
#include "ITEM.H"
#include "TYPE.H"
#include "NPC.H"
#include "YAMM.H"
#include "KERNEL.H"
#include "UPROCESS.H"

// Inline in the shared headers when this file was built.
inline unsigned char BaseFile::is_valid(void) { return handle != -1; }
inline BaseFile::~BaseFile(void) { if (is_valid()) close(); }
inline Index::Index(void) { size = 0; offset = 0; }

void main(int, char **);

Overload *RoutineIndex::overloadTable = 0;
long RoutineIndex::unkMemoryUsed = 0;
long RoutineIndex::maxUnkMemory = 0x19000L;
FastFlex *usecodeFile = 0;
unsigned char failedInit = 0;
unsigned char unkInitted = 0;
UnitIndex RoutineIndex::unitIndex[MAX_UNITS];
void **cTable;
unsigned numCFunctions;

// Turns a C prototype from the link map ("name(int, char far*)") into the
// name the usecode file uses for it.  Returns 0 if the line has no name.
unsigned char mangleName(char *mangled, char *name)
{
	char *paren;
	char *space;
	char *comma;
	char *close;
	char *end;
	unsigned char last;

	paren = strstr(name, "(");
	if (paren == 0)
	{
		char *cr = strstr(name, "\r");
		if (cr)
		{
			*cr = 0;
			strcpy(mangled, name);
			return 1;
		}
		return 0;
	}
	*paren = 0;
	strcpy(mangled, name);
	name = paren + 1;
	if (*name == ')')
		return 1;
	name = paren + 1;
	last = 0;
	while (!last)
	{
		while (!*name || *name == ' ')
			name++;
		space = strstr(name, " ");
		comma = strstr(name, ",");
		close = strstr(name, ")");
		if (space == 0)
			space = (char *)-1L;
		if (comma == 0)
			comma = (char *)-1L;
		if (close == 0)
			close = (char *)-1L;
		if (space < comma && space < close)
			end = space;
		else if (comma < space && comma < close)
			end = comma;
		else
		{
			if (close == (char *)-1L)
				return 1;
			end = close;
			last++;
		}
		if ((comma != 0 && comma != (char *)-1L) || (close != 0 && close != (char *)-1L))
		{
			if (comma == 0 || comma == (char *)-1L)
				comma = close;
			*comma = 0;
			if (!strcmp(name, "char far*"))
			{
				name += 10;
				strcat(mangled, "@s");
				continue;
			}
			*comma = ',';
		}
		if (end)
			*end = 0;
		if (!strcmp(name, "unsigned"))
		{
			name += 8;
			continue;
		}
		if (!strcmp(name, "int"))
		{
			name += 3;
			strcat(mangled, "@i");
			continue;
		}
		if (!strcmp(name, "short"))
		{
			name += 5;
			strcat(mangled, "@i");
			continue;
		}
		if (!strcmp(name, "char"))
		{
			name += 4;
			strcat(mangled, "@i");
			continue;
		}
		if (!strcmp(name, "long"))
		{
			name += 4;
			strcat(mangled, "@d");
			continue;
		}
		if (!strstr(name, "*") && !strstr(name, "&"))
		{
			strcat(mangled, "@$");
			strcat(mangled, name);
			strcat(mangled, "$");
		}
		name += strlen(name) + 1;
	}
	return 1;
}

// Replaces each "$type$" in a mangled name with "i".
void enumMangle(char *out, const char *in)
{
	int i = 0;
	int j = 0;

	while (in[i])
	{
		if (in[i] != '$')
		{
			out[j] = in[i];
			i++;
			j++;
		}
		else
		{
			out[j] = 'i';
			j++;
			i++;
			while (in[i++] != '$')
				;
		}
	}
	out[j] = 0;
}

// Fixed-size records of unkcsym.dat, one C function name each.
#define SYMBOL_SIZE	74

// Builds cTable, the address of every C function usecode can call.  The
// addresses come from the link map when it is newer than the cached table
// (unkcoff.dat), whose last entry is the address of main.
void initCFunctions(void)
{
	unsigned mainSeg;
	unsigned char useCache;
	int delta;
	unsigned i;
	FileSpec coffSpec(0, UsecodeDir, "unkcoff", ".dat");
	FileSpec symSpec(0, UsecodeDir, "unkcsym", ".dat");
	FileSpec mapSpec(0, 0, "u8", ".map");

	useCache = 0;
	if (mapSpec.exists() && symSpec.exists())
	{
		if (mapSpec.isYounger(symSpec) || symSpec.isYounger(coffSpec))
		{
			FILE *map;
			unsigned long size;
			char *symbols;
			char line[200];
			unsigned j;

			map = fopen(mapSpec, "rb");
			if (map == 0)
				unableToOpenFile(__FILE__, 259, mapSpec);	// __LINE__
			BaseFile symFile(symSpec, ReadWrite);
			BaseFile coffFile;
			coffFile.forceOpen(coffSpec, ReadWrite);
			size = symFile.size();
			numCFunctions = size / SYMBOL_SIZE;
			cTable = new void *[numCFunctions];
			if (cTable == 0)
				outOfMemory(__FILE__, 271);	// __LINE__
			memset(cTable, 0, numCFunctions * 4);
			symbols = new char[size];
			if (symbols == 0)
				outOfMemory(__FILE__, 278);	// __LINE__
			symFile.read(symbols, size);
			do
				fgets(line, 200, map);
			while (!strstr(line, "Publics by Name"));
			// Map lines read " 0001:0234       _name".
			do
			{
				char *name;
				unsigned char found;
				unsigned n;
				char *sym;
				char mangled[256];

				fgets(line, 200, map);
				name = line + 17;
				found = mangleName(WorkString.string, name);
				enumMangle(mangled, WorkString.string);
				if (!strcmp(name, "_main"))
				{
					line[5] = 0;
					line[22] = 0;
					mainSeg = strtol(line + 1, 0, 16);
				}
				if (found)
				{
					for (n = 0, sym = symbols; n < numCFunctions; n++, sym += SYMBOL_SIZE)
					{
						if (!strcmp(sym, WorkString.string) || !strcmp(sym, mangled))
						{
							unsigned seg;
							unsigned off;

							line[5] = 0;
							line[10] = 0;
							seg = strtol(line + 1, 0, 16);
							off = strtol(line + 6, 0, 16);
							cTable[n] = MK_FP(seg, off);
						}
					}
				}
			}
			while (!strstr(line, "Publics by Value"));
			for (j = 0; j < numCFunctions; j++)
			{
				if (cTable[j] == 0)
				{
					char key;

					trace(TRACE_ALWAYS, "\r\nUnresolved c symbol %s", &symbols[j * SYMBOL_SIZE]);
					printf("\r\nContinue anyway? ");
					key = getch();
					printf("\r\n");
					if (key != 'y' && key != 'Y')
					{
						failedInit = 1;
						break;
					}
				}
			}
			delete symbols;
			if (failedInit)
			{
				traceGet(TRACE_ALWAYS, "\r\nNo Usecode! (press any key)\r\n\r\n");
				return;
			}
			coffFile.write((char *)cTable, numCFunctions * 4);
		}
		else
			useCache = 1;
	}
	else
		useCache = 1;
	if (useCache)
	{
		BaseFile coffFile(coffSpec, ReadWrite);
		unsigned size = coffFile.size();

		numCFunctions = size / 4;
		cTable = new void *[numCFunctions];
		if (cTable == 0)
			outOfMemory(__FILE__, 366);	// __LINE__
		coffFile.read((char *)cTable, size);
		mainSeg = FP_SEG(cTable[numCFunctions - 1]);
	}
	// The map gives real-mode segments; relocate them to selectors.
	delta = (FP_SEG(main) >> 3) - mainSeg;
	for (i = 0; i < numCFunctions; i++)
	{
		unsigned seg = (FP_SEG(cTable[i]) + delta) << 3;
		unsigned sel = seg | 7;

		cTable[i] = MK_FP(sel, FP_OFF(cTable[i]));
	}
}

// Opens the usecode file for the avatar's language.
void initUnk(void)
{
	initYamm();
	if (!*UsecodeDir)
		return;
	usecodeFileName[0] = avatar.getLanguageChar();
	if (!FileExists(fileSpec(0, UsecodeDir, usecodeFileName, 0)))
		return;
	usecodeFile = new FastFlex(fileSpec(0, UsecodeDir, usecodeFileName, 0), 0, ReadOnly, -1);
	Index index;
	usecodeFile->getIndex(1, index);
	RoutineIndex::overloadTable = (Overload *)new char[index.size];
	if (RoutineIndex::overloadTable == 0)
		outOfMemory(__FILE__, 408);	// __LINE__
	usecodeFile->readRecord(1, RoutineIndex::overloadTable);
	initCFunctions();
	unkInitted = 1;
}

void reloadUnk(void)
{
}

// Starts the item's usecode for an event, if its unit handles that event.
unsigned char spawnUnk(Item *item, unsigned long event, unsigned short number, unsigned short &pid, unsigned long arg)
{
	UnkProcess *process;
	unsigned unit;

	if (failedInit)
		return 0;
	// Permanent NPCs (status bit 0x80 clear) and unk eggs have units of their own.
	if (item->isNpc() && !(Boolean)((item->getStatus() & 0x80) ? 1 : 0))
		unit = item->referent + 1024;
	else if (item->getFamily() == UNKEGG_FAMILY)
		unit = item->getUnkEggType() + 1151;
	else
		unit = item->getType();
	if (RoutineIndex::overloadTable && (RoutineIndex::overloadTable[unit].events & event))
	{
		process = new UnkProcess(0, item, 2, unit, number, 0, (char *)&arg, 4);
		if (process->isPhantom())
			pid = 0;
		else
			pid = process->pid;
		return 1;
	}
	return 0;
}

UnkProcess::UnkProcess(unsigned long *dest, void *thisPtr, unsigned short size, unsigned short unitNumber, unsigned short entry, unsigned char isOffset, char *args, int argSize) :
	routine((char *)thisPtr, size, RoutineIndex::getCode(unitNumber, entry, isOffset, -1), this)
{
	Kernel::setIdString(pid, RoutineIndex::overloadTable[unitNumber].name);
	finished = 0;
	thisSize = size;
	thisOffset = 0;
	unit = unitNumber;
	resultDest = dest;
	// The stack holds the this data (copied by Data), the arguments, then a
	// pointer back to the this data.
	if (args)
	{
		char *thisSp = routine.sp;
		unsigned length = argSize;

		routine.sp -= length;
		memcpy(routine.sp, args, argSize);
		if (size)
		{
			char *p = thisSp;

			*(long *)(routine.sp -= 4) = (long)p;
			thisOffset = routine.sp - routine.buffer;
		}
	}
	killed = new char;
	if (killed == 0)
		outOfMemory(__FILE__, 565);	// __LINE__
	*killed = 0;
	process();
}

void UnkProcess::process(void)
{
	int status;

	running = 1;
	status = routine.interpret(result);
	if (status == 1)
	{
		if (resultDest)
			*resultDest = result;
		finished = 1;
		running = 0;
		pop(result);
		return;
	}
	if (status != 4)
		running = 0;
}

void UnkProcess::freeMemory(void)
{
	Process::freeMemory();
	RoutineIndex::markUnused(unit);
	if (!finished)
		Yamm::clean(pid);
	if (running)
	{
		if (killed)
		{
			*killed = 1;
			return;
		}
	}
	else
		delete killed;
}

void UnkProcess::save(BaseFile *file)
{
	unsigned offset;

	Process::save(file);
	file->write((char *)&thisOffset, 2);
	file->write((char *)&thisSize, 1);
	if (killed)
		file->write(killed, 1);
	offset = (unsigned)routine.ip;
	file->write((char *)&offset, 2);
}

void UnkProcess::load(BaseFile *file)
{
	int spOffset;
	unsigned offset;

	Process::load(file);
	spOffset = routine.sp - routine.buffer;
	file->read((char *)&thisOffset, 2);
	file->read((char *)&thisSize, 1);
	if (killed)
	{
		killed = new char;
		file->read(killed, 1);
	}
	resultDest = 0;
	if (thisOffset)
		*(char **)&routine.buffer[thisOffset] = &routine.buffer[200 - thisSize];
	routine.sp = &routine.buffer[spOffset];
	routine.top = &routine.buffer[200];
	file->read((char *)&offset, 2);
	routine.ip = RoutineIndex::getCode(unit, offset, 1, -1);
	routine.process = this;
}

RoutineIndex::RoutineIndex(void)
{
	memset(unitIndex, 0, sizeof(unitIndex));
}

// Returns the unit's cache entry, or 0 with slot set to a free entry.
UnitIndex *RoutineIndex::findUnit(unsigned short unit, UnitIndex *&slot)
{
	unsigned i;
	int unused = -1;
	UnitIndex *index = unitIndex;

	for (i = 0; i < MAX_UNITS; i++, index++)
	{
		if (index->unit == unit)
			return index;
		else if (index->unit == 0)
		{
			if (slot == 0)
				slot = index;
		}
		else if (index->useCount == 0)
		{
			if (unused == -1)
				unused = i;
		}
	}
	if (slot == 0)
	{
		if (unused == -1)
			halt(__FILE__, 748);	// __LINE__
		cacheOut(unused);
		slot = &unitIndex[unused];
	}
	return 0;
}

int RoutineIndex::getNumFree(void)
{
	int count = 0;
	UnitIndex *index = unitIndex;
	unsigned i;

	for (i = 0; i < MAX_UNITS; i++, index++)
		if (index->unit == 0 || index->useCount == 0)
			count++;
	return count;
}

RoutineIndex::~RoutineIndex(void)
{
	int i;

	for (i = 0; i < MAX_UNITS; i++)
		if (unitIndex[i].code)
			delete unitIndex[i].code;
}

// The code of a unit's routine: entry is an offset into the unit when
// isOffset is set, else an event number.
char *RoutineIndex::getCode(unsigned short unit, unsigned short entry, unsigned char isOffset, long offset)
{
	UnitIndex *slot = 0;
	UnitIndex *index;
	char *code;

	index = findUnit(unit, slot);
	if (index == 0)
	{
		UnitFile file;
		unsigned size = file.getSize(unit);

		while (unkMemoryUsed + size > maxUnkMemory && cacheOut(-1))
			;
		slot->unit = unit;
		slot->size = size;
		slot->code = file.load(unit);
		index = slot;
		unkMemoryUsed += size;
	}
	index->useCount++;
	code = index->code;
	if (isOffset)
		code += entry;
	else
		code += ((long *)index->code)[entry];

	if (offset != -1)
		code += (unsigned)offset;
	return code;
}

void RoutineIndex::markUnused(unsigned short unit)
{
	UnitIndex *slot;
	UnitIndex *index;

	index = findUnit(unit, slot);
	if (index == 0)
		halt(__FILE__, 829);	// __LINE__
	index->useCount--;
}

// Frees a unit's cached code; -1 picks the last unused one.
unsigned char RoutineIndex::cacheOut(int which)
{
	int i;

	if (which == -1)
	{
		for (i = MAX_UNITS - 1; i >= 0; i--)
			if (unitIndex[i].useCount <= 0 && unitIndex[i].code)
				break;
		if (i < 0)
			return 0;
	}
	else
	{
		if (unitIndex[which].useCount)
			return 0;
		i = which;
	}
	delete unitIndex[i].code;
	unitIndex[i].code = 0;
	unkMemoryUsed -= unitIndex[i].size;
	unitIndex[i].size = 0;
	unitIndex[i].useCount = 0;
	unitIndex[i].unit = 0;
	return 1;
}

void resetRef(unsigned short ref, unsigned short type)
{
	Kernel::resetRef(ref, (ProcessType)type);
}

void setRef(unsigned short pid, unsigned short type, unsigned short ref)
{
	Process *process;

	process = Kernel::findValidProcess(pid, (ProcessType)type);
	if (process)
		process->setRef(ref);
}
