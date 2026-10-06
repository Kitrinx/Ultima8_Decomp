#ifndef CDESC_H
#define CDESC_H

#include "CYTABLE.H"

// An 8-byte 286/386 segment descriptor.
class Desc
{
public:
	unsigned short limit;
	unsigned short baseLow;
	unsigned char baseMid;
	unsigned char access;
	unsigned char limitHigh;	// limit bits 16-19 in the low nibble
	unsigned char baseHigh;

	long getBase(void);
	void setBase(unsigned long base)
	{
		baseLow = base;
		baseMid = base >> 16;
		baseHigh = base >> 24;
	}
	void setLimit(unsigned long);
	void display(unsigned short);
};

#define RESERVED_SELECTORS 32

// Saves the bases of the reserved selectors and puts them back.
class HoldPointers
{
public:
	unsigned long base[RESERVED_SELECTORS];

	HoldPointers(void);
	~HoldPointers(void);
};

class LocalDescriptorTable
{
public:
	static void initialize(void);
	static Desc *getDesc(unsigned short sel) { return &table[sel >> 3]; }
	static unsigned long limit;
	static unsigned short mappedSelector;
	static Desc *table;
};

class GlobalDescriptorTable
{
public:
	static void initialize(void);
	static unsigned long limit;
	static unsigned short mappedSelector;
	static Desc *table;
};

void GetBiosSegment(void);
void SetVgaScreenDescriptor(void);
unsigned short MapSegment(unsigned short, unsigned short);
int pascal MERGEBYTEARRAY(char *, char *, short);
unsigned short getLDT(void);
void initReservedSelectors(void);
void *makePtr(unsigned long);

extern int init;
extern unsigned short TextSelector;
extern char *error;
extern char *delta;
extern char *vector;
extern int major;
extern int iterations;

#endif
