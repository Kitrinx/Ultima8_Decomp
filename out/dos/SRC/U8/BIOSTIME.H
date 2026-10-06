#ifndef BIOSTIME_H
#define BIOSTIME_H

#include <phapi.h>
#include "..\GLIB\PROCESS.H"

class BiosTimer : public ProcessRealTime
{
public:
	unsigned long realVector;
	PIHANDLER protVector;

	BiosTimer(void);
	virtual void process(void);
	virtual void interruptHandler(unsigned long, unsigned long);
	void setTimerVector(unsigned long, PIHANDLER);
};

#endif
