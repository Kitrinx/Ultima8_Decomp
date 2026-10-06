#ifndef PROFILER_H
#define PROFILER_H

#include "..\GLIB\PROCESS.H"

class Profiler : public ProcessRealTime
{
public:
	Profiler(void);
	virtual void process(void);
	virtual void interruptHandler(unsigned long, unsigned long);
	void start(void);
	void stop(void);
};

#endif
