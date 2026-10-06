#ifndef STATREST_H
#define STATREST_H

#include "..\GLIB\PROCESS.H"
#include "RANDINT.H"
#include "URANDOM.H"

// Restores the avatar's mana and hit points over time.
class StatRestorer : public ProcessRealTime
{
public:
	unsigned ticks;

	StatRestorer(void);
	virtual void process(void);
	virtual void interruptHandler(unsigned long, unsigned long);
};

void FeedAvatar(short);

#endif
