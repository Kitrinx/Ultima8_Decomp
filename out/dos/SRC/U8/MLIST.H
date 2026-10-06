#ifndef MLIST_H
#define MLIST_H

class BaseFile;
class PidEntry;

// A list word: a pid with flag bits above it, or an operator (0x400 set).
class PidEntry
{
public:
	unsigned value;

	PidEntry(void) {}
	PidEntry(unsigned v) { value = v; }
	unsigned getPid(void) { return value & 0x3ff; }
	unsigned char isOperator(void) { return (value & 0x400) ? 1 : 0; }
	unsigned char isPid(void) { return !isOperator(); }
	unsigned char isPassiveInt(void) { return (value & 0x800) ? 1 : 0; }
	unsigned char isActiveInt(void) { return (value & 0x1000) ? 1 : 0; }
	unsigned char isPassiveDep(void) { return (value & 0x2000) ? 1 : 0; }
	unsigned char isActiveDep(void) { return (value & 0x4000) ? 1 : 0; }
	unsigned char isUsed(void) { return isPassiveInt() || isActiveInt() || isPassiveDep() || isActiveDep(); }
};

class List
{
public:
	unsigned count;
	unsigned linearType;
	unsigned *data;

	List(void) { count = 0; data = 0; linearType = 0; }
	~List(void);
	virtual void load(BaseFile *);
	virtual void save(BaseFile *);
	int constructList(unsigned short, ...);
	void clear(void);
	int addToList(unsigned short);
	int appendToList(unsigned short);
	int removeFromList(unsigned short);
	unsigned getMember(unsigned short);
	void setMember(unsigned short, unsigned short);
	int isInList(unsigned short);
	PidEntry isPidInList(unsigned short);
	PidEntry isPidInList(unsigned short, unsigned short &);
	void setListSize(unsigned short);
	unsigned char isAllocated(void) { return data != 0; }
	void setLinearType(unsigned type) { linearType = type; }
};

#endif
