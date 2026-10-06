// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: DEXT286\SELMNGR.C

#include "CDESC.H"
#include "SPANKER.H"
#include "SELMNGR.H"

unsigned short SelectorManager::selectorStart = 0;
unsigned short SelectorManager::selectorCount = 0;
unsigned short SelectorManager::currentSelectorCount = 0;
unsigned short SelectorManager::globalSel = 0;

// Free descriptors form a ring: limit holds the previous entry, baseLow the next.
void SelectorManager::init(void)
{
	unsigned short unused = 0;
	unsigned short entries = (LocalDescriptorTable::limit + 1) >> 3;
	unsigned short i;
	unsigned short prev;
	Desc *desc;
	unsigned short last;

	for (i = 0; i < entries; i++) {
		if (LocalDescriptorTable::table[i].access == 0)
			unused++;
		if (unused > 200) {
			selectorStart = i;
			last = i;
			prev = i;
			for (; i < entries; i++) {
				if (LocalDescriptorTable::table[i].access == 0) {
					selectorCount++;
					last = i;
					desc = &LocalDescriptorTable::table[i];
					desc->access = 0x80;
					desc->limit = prev;
					LocalDescriptorTable::table[prev].baseLow = i;
					prev = i;
				}
			}
			LocalDescriptorTable::table[selectorStart].limit = last;
			LocalDescriptorTable::table[last].baseLow = selectorStart;
		}
	}
	currentSelectorCount = selectorCount;
	globalSel = allocateSelector();
	setupDescriptor(globalSel, 0, 0xFFFFFFFFL);
}

void SelectorManager::uninit(void)
{
	unsigned char done = 0;
	unsigned short i = selectorStart;
	unsigned short next;

	while (!done) {
		next = LocalDescriptorTable::table[i].baseLow;
		if (next == selectorStart)
			done = 1;
		LocalDescriptorTable::table[i].access = 0;
		i = next;
	}
}

unsigned short SelectorManager::allocateSelector(void)
{
	Desc *head = &LocalDescriptorTable::table[selectorStart];
	unsigned short i = head->baseLow;

	if (i == selectorStart)
		return 0;
	else {
		Desc *desc = &LocalDescriptorTable::table[i];
		Desc *prev = &LocalDescriptorTable::table[desc->limit];
		Desc *after = &LocalDescriptorTable::table[desc->baseLow];

		prev->baseLow = desc->baseLow;
		after->limit = desc->limit;
		desc->access = 0xF3;
		currentSelectorCount--;
		return (i << 3) | 7;
	}
}

void SelectorManager::deallocateSelector(unsigned short sel)
{
	unsigned short i = sel >> 3;
	Desc *desc = &LocalDescriptorTable::table[i];
	Desc *head = &LocalDescriptorTable::table[selectorStart];
	Desc *next = &LocalDescriptorTable::table[head->baseLow];

	desc->access = 0x80;
	desc->limit = selectorStart;
	desc->baseLow = head->baseLow;
	head->baseLow = i;
	next->limit = i;
	currentSelectorCount++;
}

void SelectorManager::setupDescriptor(unsigned short sel, unsigned long base)
{
	Desc *desc = LocalDescriptorTable::getDesc(sel);

	desc->setBase(base);
}

void SelectorManager::setupDescriptor(unsigned short sel, unsigned long base, unsigned long size)
{
	Desc *desc = LocalDescriptorTable::getDesc(sel);
	unsigned long pages;

	desc->setBase(base);
	if (size > 0x100000L) {
		pages = size >> 12;
		if ((size & 0xFFF) && size <= 0xFFFFEFFFL)
			pages++;
		desc->setLimit(pages);
		desc->limitHigh |= 0x80;
	} else {
		desc->setLimit(size - 1);
		desc->limitHigh &= 0x7F;
	}
}

// Moves every selector based at oldBase to newBase.
void SelectorManager::remapBase(unsigned long oldBase, unsigned long newBase)
{
	Desc *desc;
	unsigned short end = selectorStart + selectorCount;
	unsigned short i;

	for (i = selectorStart; i < end; i++) {
		desc = &LocalDescriptorTable::table[i];
		if (desc->access == 0xF3 && desc->getBase() == oldBase) {
			asm pushf
			asm cli
			if (oldBase == Spanky::originalIdtAddress)
				Spanky::setOriginalIdtAddress(newBase);
			desc->setBase(newBase);
			asm popf
		}
	}
}
