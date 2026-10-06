#ifndef WAIT_H
#define WAIT_H

#include "..\GLIB\PROCESS.H"

// Pops after a number of game loops.
class RoundTimer : public Process
{
public:
	unsigned count;

	RoundTimer(unsigned short, unsigned short);
	virtual void process(void) { if (--count == 0) pop(0); }
};

// Pops after a number of 30 Hz ticks.
class Wait : public ProcessRealTime
{
public:
	unsigned count;
	unsigned ticks;

	Wait(unsigned short, unsigned short);
	virtual void process(void);
	virtual void interruptHandler(unsigned long, unsigned long);
	virtual void restart(void);
	virtual void timerDone(void) { pop(0); }
	void start(void);
};

// A Wait that fails instead of popping.
class FailWait : public Wait
{
public:
	FailWait(unsigned short t, unsigned short r) : Wait(t, r) {}
	virtual void interruptHandler(unsigned long, unsigned long);
};

// Counts seconds and tenths while running.
class Timer : public ProcessRealTime
{
public:
	unsigned char running;
	unsigned seconds;
	unsigned tenths;

	Timer(void);
	virtual void process(void) {}
	virtual void interruptHandler(unsigned long, unsigned long);
	void stop(void);
	void start(void);
};

class AccurateTimer : public ProcessRealTime
{
public:
	unsigned char timing;
	unsigned long ticks;

	AccurateTimer(int);
	virtual void process(void) {}
	virtual void interruptHandler(unsigned long, unsigned long);
	void stop(void);
	void start(void);
	unsigned char isTiming(void);
	unsigned getTicks(void);
};

#endif
