#ifndef SYSTIMER_H
#define SYSTIMER_H

#include <phapi.h>
#include "..\GLIB\PROCESS.H"

class BaseFile;

class SystemTimer : public ProcessRealTime
{
public:
	unsigned long ticks;

	SystemTimer(void);
	virtual void process(void);
	virtual void freeMemory(void);
	virtual void load(BaseFile *);
	virtual void interruptHandler(unsigned long, unsigned long);
	void wait(unsigned long);
};

extern SystemTimer *systemTimer;

#endif
