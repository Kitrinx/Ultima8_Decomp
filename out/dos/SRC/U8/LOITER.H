#ifndef LOITER_H
#define LOITER_H

#include "..\GLIB\PROCESS.H"
#include "MLIST.H"

// Wanders an npc about its spot, fidgeting now and then.
class Loiter : public ProcessInterrupt
{
public:
	int count;

	Loiter(unsigned short, short);
	virtual void process(void);
	virtual void interruptHandler(unsigned long, unsigned long);
};

#endif
