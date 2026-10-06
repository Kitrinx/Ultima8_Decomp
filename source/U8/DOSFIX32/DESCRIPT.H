#ifndef DESCRIPT_H
#define DESCRIPT_H

// A selector: table index, table bit (4: local) and privilege level.
class FullSelector
{
public:
	unsigned short sel;

	int isGlobal(void) { return !(sel & 4); }
	unsigned short index(void) { return sel >> 3; }
};

struct SegmentFields
{
	unsigned limitLow : 16;
	unsigned baseLow : 16;
	unsigned baseMid : 8;
	unsigned type : 4;
	unsigned system : 1;
	unsigned dpl : 2;
	unsigned present : 1;
	unsigned limitHigh : 4;
	unsigned available : 1;
	unsigned reserved : 1;
	unsigned big : 1;
	unsigned granular : 1;
	unsigned baseHigh : 8;
};

struct GateFields
{
	unsigned short offsetLow;
	unsigned short selector;
	unsigned count : 5;
	unsigned reserved : 3;
	unsigned type : 4;
	unsigned system : 1;
	unsigned dpl : 2;
	unsigned present : 1;
	unsigned short offsetHigh;
};

// An 8-byte segment or gate descriptor.
class Descriptor
{
public:
	union {
		SegmentFields seg;
		GateFields gate;
	};

	Descriptor(void);
	unsigned char operator==(Descriptor &);
	unsigned char operator!=(Descriptor &d) { return !(*this == d); }
	void setBase(unsigned long);
	void setLimit(unsigned long);
	unsigned long getBase(void);
	unsigned long getLimit(void);
	void setOffset(unsigned long);
	unsigned long getOffset(void);
	// A present, ring 3 gate into code segment sel.
	void setGate(unsigned short sel, unsigned gateType)
	{
		gate.selector = sel;
		gate.count = 0;
		gate.reserved = 0;
		gate.type = gateType;
		gate.system = 0;
		gate.dpl = 3;
		gate.present = 1;
	}
};

// What sgdt and sidt store.
struct TableRegister
{
	unsigned short limit;
	unsigned long base;
};

class DescMngr
{
public:
	static void init(void);
	static Descriptor getDescriptor(FullSelector);
	static void setDescriptor(FullSelector, Descriptor);
	static Descriptor getGlobalDescriptor(unsigned short);
	static Descriptor getLocalDescriptor(unsigned short);
	static Descriptor getIntDescriptor(unsigned short);
	static void setGlobalDescriptor(unsigned short, Descriptor &);
	static void setLocalDescriptor(unsigned short, Descriptor &);
	static void setIntDescriptor(unsigned short, Descriptor &);
	static unsigned char dosCreateDSAlias(FullSelector, FullSelector &);
	static Descriptor *globalDescriptorTable;
	static Descriptor *localDescriptorTable;
	static Descriptor *interruptDescriptorTable;
	static TableRegister gdtr;
	static TableRegister idtr;
	static unsigned short ldtr;
	static TableRegister simLdtr;
	static unsigned short gdtSel;
	static unsigned short ldtSel;
	static unsigned short idtSel;
};

extern "C" void getRegisters(TableRegister *gdtr, unsigned short *ldtr, TableRegister *idtr);

#endif
