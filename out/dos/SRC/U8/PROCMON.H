#ifndef PROCMON_H
#define PROCMON_H

#include "..\GLIB\PROCESS.H"

// Counts frames and prints process, memory and timing figures once a frame.
class ProcessMonitor : public ProcessRealTime
{
public:
	unsigned frames;
	unsigned fps;
	unsigned ticks;

	ProcessMonitor(void);
	virtual void process(void);
	virtual void interruptHandler(unsigned long, unsigned long);
	void turnOn(void);
	void turnOff(void);
};

#endif
