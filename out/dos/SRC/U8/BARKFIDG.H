#ifndef BARKFIDG_H
#define BARKFIDG_H

#include "..\GLIB\PROCESS.H"
#include "ITEM.H"

// Keeps a barking NPC playing its talk animation while its bark is shown.
class BarkFidget : public ProcessInterrupt
{
public:
	unsigned char f_2e;

	BarkFidget(unsigned short, unsigned char);
	virtual void process(void);
	virtual void interruptHandler(unsigned long, unsigned long);
	Boolean hasBark(void);
};

#endif
