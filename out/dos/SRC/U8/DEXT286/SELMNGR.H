#ifndef SELMNGR_H
#define SELMNGR_H

// Hands out LDT selectors from a ring of descriptors free at startup.
class SelectorManager
{
public:
	static void init(void);
	static void uninit(void);
	static unsigned short allocateSelector(void);
	static void deallocateSelector(unsigned short);
	static void setupDescriptor(unsigned short, unsigned long);
	static void setupDescriptor(unsigned short, unsigned long, unsigned long);
	static void remapBase(unsigned long, unsigned long);
	static unsigned short selectorStart;
	static unsigned short selectorCount;
	static unsigned short currentSelectorCount;
	static unsigned short globalSel;
};

#endif
