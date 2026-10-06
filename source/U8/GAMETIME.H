#ifndef GAMETIME_H
#define GAMETIME_H

#include "..\GLIB\PROCESS.H"

// Counts game loops; starts the npc schedules when the game hour changes.
class GameTime : public Process
{
public:
	unsigned long loops;
	int pause;		// loops to skip counting, -1 to stop
	unsigned char hourChanged;

	GameTime(void);
	virtual void process(void);
	virtual void postLoad(void);
	unsigned char isPaused(void) { return pause != 0; }
};

// The 30 Hz game clock. A game hour is 27000 ticks.
class Animation : public ProcessRealTime
{
public:
	unsigned long f_38;	// animation frames
	long f_3c;		// frame count as of the last unlock
	long f_40;		// ticks
	long f_44;		// game hours
	long f_48;		// tick count to jump to, -1 for none
	unsigned char f_4c;	// frame count locked
	unsigned char f_4d;	// animation stopped

	Animation(void);
	virtual void process(void) {}
	virtual void postLoad(void);
	virtual void interruptHandler(unsigned long, unsigned long);
	void lockTime(void);
	void unlockTime(void);
	void setTicks(long t) { f_48 = t; }
	long getHours(void) { return f_44; }
};

void initGameTime(void);
long TimeInGameLoops(void);
long TimeInSeconds(void);
long TimeInMinutes(void);
long TimeInGameHours(void);
void SetTimeInSeconds(long);
void SetTimeInMinutes(long);
void SetTimeInGameHours(short);

extern GameTime *theGameTime;
extern Animation *theAnimation;

#endif
