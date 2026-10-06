// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: DEXT286\CDESC.C

#include <stdio.h>
#include <dos.h>
#include <phapi.h>
#include "CEXIT.H"
#include "CDESC.H"

static int nextReserved = 0;
static Desc *ldt;
static unsigned short reserved[RESERVED_SELECTORS];

unsigned short getLDT(void)
{
	asm sldt ax
	return _AX;
}

void initReservedSelectors(void)
{
	int i;

	GlobalDescriptorTable::initialize();
	LocalDescriptorTable::initialize();
	ldt = LocalDescriptorTable::table;
	for (i = 0; i < RESERVED_SELECTORS; i++)
		if (DosMapLinSeg(0L, 0xFFFFL, &reserved[i]))
			Fatal("%s %d", __FILE__, 45);	// __LINE__
}

// Points the next reserved selector at a linear address.
void *makePtr(unsigned long linear)
{
	unsigned short sel;

	if (linear < 0x400)
		Fatal("%s %d", __FILE__, 53);	// __LINE__
	asm {
		push es
		mov bx, offset reserved
		mov si, nextReserved
		shl si, 1
		mov si, [bx+si]
		mov sel, si
		and si, 0FFF8h
		les bx, ldt
		add bx, si
		mov ax, word ptr linear
		mov es:[bx+2], ax
		mov ax, word ptr linear+2
		mov es:[bx+4], al
		mov es:[bx+7], ah
		pop es
	}
	nextReserved++;
	nextReserved &= RESERVED_SELECTORS - 1;
	return MK_FP(sel, 0);
}

HoldPointers::HoldPointers(void)
{
	int i;

	for (i = 0; i < RESERVED_SELECTORS; i++)
		base[i] = LocalDescriptorTable::getDesc(reserved[i])->getBase();
}

HoldPointers::~HoldPointers(void)
{
	int i;
	unsigned long b;

	for (i = 0; i < RESERVED_SELECTORS; i++) {
		b = base[i];
		LocalDescriptorTable::getDesc(reserved[i])->baseLow = b;
		LocalDescriptorTable::getDesc(reserved[i])->baseMid = b >> 16;
		LocalDescriptorTable::getDesc(reserved[i])->baseHigh = b >> 24;
	}
}

unsigned long GlobalDescriptorTable::limit;
unsigned short GlobalDescriptorTable::mappedSelector;
Desc *GlobalDescriptorTable::table;
unsigned long LocalDescriptorTable::limit;
unsigned short LocalDescriptorTable::mappedSelector;
Desc *LocalDescriptorTable::table;

void LocalDescriptorTable::initialize(void)
{
	unsigned short sel;
	Desc *desc;
	unsigned long base;

	sel = getLDT();
	desc = &GlobalDescriptorTable::table[sel >> 3];
	base = desc->getBase();
	limit = desc->limit;
	if (DosMapLinSeg(base, limit + 1, &mappedSelector))
		Fatal("%s %d", __FILE__, 118);	// __LINE__
	table = (Desc *)MK_FP(mappedSelector, 0);
}

void GlobalDescriptorTable::initialize(void)
{
	unsigned short gdtr[3];
	unsigned short *p;
	unsigned long base;

	p = gdtr;
	asm {
		les bx, p
		sgdt es:[bx]
	}
	base = ((unsigned long)gdtr[2] << 16) + gdtr[1];
	if (DosMapLinSeg(base, gdtr[0] + 1, &mappedSelector))
		Fatal("%s %d", __FILE__, 141);	// __LINE__
	table = (Desc *)MK_FP(mappedSelector, 0);
	limit = gdtr[0];
}

long Desc::getBase(void)
{
	_DH = baseHigh;
	_DL = baseMid;
	_AX = baseLow;
}

void Desc::setLimit(unsigned long l)
{
	limit = l;
	limitHigh &= 0xF0;
	limitHigh |= l >> 16;
}

void Desc::display(unsigned short sel)
{
	printf("[%04X] base=%02X%02X%04X limit=%04X acc=%02X ", sel, baseHigh, baseMid, baseLow, limit, access);
	if (!access)
		printf("INVALID ");
	printf("\n");
}
