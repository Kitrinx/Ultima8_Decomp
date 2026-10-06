#ifndef UPROCESS_H
#define UPROCESS_H

#include "..\GLIB\PROCESS.H"
#include "INTER.H"

class BaseFile;
class FastFlex;
class Item;

// A usecode process: runs one event routine of a unit (usecode class).
class UnkProcess : public Process
{
public:
	unsigned unit;
	Routine routine;
	unsigned long *resultDest;	// receives the result when the routine returns
	unsigned thisOffset;		// stack offset of the pushed this pointer
	unsigned char thisSize;
	unsigned char finished;
	unsigned char running;
	char *killed;			// set while running when the process is freed

	UnkProcess(unsigned long *, void *, unsigned short, unsigned short, unsigned short, unsigned char, char *, int);
	virtual void process(void);
	virtual void freeMemory(void);
	virtual void load(BaseFile *);
	virtual void save(BaseFile *);
};

// Record 1 of the usecode file: one entry per unit.
class Overload
{
public:
	unsigned long events;	// bit n set: the unit handles event n
	char name[9];
};

// A unit's code, loaded on demand and cached.
class UnitIndex
{
public:
	unsigned unit;
	char *code;
	unsigned useCount;
	unsigned size;
};

#define MAX_UNITS	30

class RoutineIndex
{
public:
	RoutineIndex(void);
	~RoutineIndex(void);
	static UnitIndex *findUnit(unsigned short, UnitIndex *&);
	static int getNumFree(void);
	static char *getCode(unsigned short, unsigned short, unsigned char, long);
	static void markUnused(unsigned short);
	static unsigned char cacheOut(int);
	static Overload *overloadTable;
	static long unkMemoryUsed;
	static long maxUnkMemory;
	static UnitIndex unitIndex[MAX_UNITS];
};

unsigned char mangleName(char *, char *);
void enumMangle(char *, const char *);
void initCFunctions(void);
void initUnk(void);
void reloadUnk(void);
unsigned char spawnUnk(Item *, unsigned long, unsigned short, unsigned short &, unsigned long);
void resetRef(unsigned short, unsigned short);
void setRef(unsigned short, unsigned short, unsigned short);

extern FastFlex *usecodeFile;
extern unsigned char failedInit;
extern unsigned char unkInitted;
extern void **cTable;
extern unsigned numCFunctions;

#endif
