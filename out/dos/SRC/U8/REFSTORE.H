#ifndef REFSTORE_H
#define REFSTORE_H

#include "..\GLIB\PROCESS.H"
#include "KERNEL.H"

class RefStorage;
class StorageData;

// Delivers one hit or got-hit event to an item as a process.
class StorageData : public Process
{
public:
	RefStorage *storage;
	unsigned other;
	short force;
	unsigned index;
	ProcessType event;
	unsigned char invalidated;

	StorageData(RefStorage *, ProcessType, unsigned short, unsigned short, short);
	virtual void process(void);
	virtual ~StorageData(void);
	void setIndex(unsigned i) { index = i; }
};

// The events of one movement, run once it is done.
class RefStorage
{
public:
	StorageData **list;
	int size;
	unsigned count;

	RefStorage(short);
	~RefStorage(void);
	void set(StorageData *);
	void invalidate(unsigned short);
	void execute(void);
};

#endif
