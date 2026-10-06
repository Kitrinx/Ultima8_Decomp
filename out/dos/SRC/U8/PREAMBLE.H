#ifndef PREAMBLE_H
#define PREAMBLE_H

#include "..\GLIB\PROCESS.H"
#include "ITEMFIND.H"

// Holds the camera process while it waits, then restores the music.
class RestoreMusicProcess : public Process
{
public:
	RestoreMusicProcess(void);
	virtual void process(void);
	virtual ~RestoreMusicProcess(void);
	virtual void fail(long);
};

// Starts a game: the intro for a new avatar, then the world.
class PreambleProcess : public Process
{
public:
	unsigned f_2a;
	AreaItemFinder startEgg;

	PreambleProcess(void);
	virtual void process(void);
};

class RefreshWorld : public Process
{
public:
	unsigned f_2a;

	RefreshWorld(void) { f_2a = 0; }
	virtual void process(void);
};

class MainMenuProcess : public Process
{
public:
	MainMenuProcess(void) {}
	virtual void process(void);
};

void preamble(void);

#endif
