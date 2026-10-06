#ifndef AUXFUNCS_H
#define AUXFUNCS_H

#include "..\GLIB\PROCESS.H"

// Now and then, while the avatar stands idle, plays a look-around animation.
class HeadShaker : public ProcessRealTime
{
public:
	unsigned seconds;	// since the avatar last animated
	unsigned wait;		// seconds until the next look

	HeadShaker(void);
	virtual void process(void) {}
	virtual void postLoad(void);
	virtual void interruptHandler(unsigned long, unsigned long);
	virtual void freeMemory(void);
	void animationUsed(void);
};

extern HeadShaker *headShaker;

#endif
