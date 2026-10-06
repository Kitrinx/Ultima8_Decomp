#ifndef EGG_H
#define EGG_H

#include "..\GLIB\PROCESS.H"
#include "ITEM.H"

// Keeps the avatar in stasis until it runs.
class SuspendAvatar : public Process
{
public:
	unsigned char wasInStasis;

	SuspendAvatar(void);
	virtual void process(void);
};

// Moves the avatar to a teleport egg on a map, fading the screen when it fires.
class TeleporterProcess : public Process
{
public:
	unsigned destMap;
	unsigned destEgg;

	TeleporterProcess(unsigned short);
	TeleporterProcess(short, int, unsigned char);
	virtual void process(void);
	void go(unsigned char);
	static int dontFire;
};

class Egg : public Item
{
public:
	Egg(Referent r) { referent = r; }

	int getEggXRange(void);
	int getEggYRange(void);
	void setEggXRange(unsigned short);
	void setEggYRange(unsigned short);
	int getEggId(void);
	void setEggId(unsigned short);
	int hatch(void);
};

// Hatches an egg when the avatar walks into its range.
class EggHatcher : public Process
{
public:
	unsigned char hatched;

	EggHatcher(unsigned short);
	virtual void process(void);
	static unsigned char eggOutlineVisible;
};

int teleportToEgg(short, int, unsigned char);

#endif
