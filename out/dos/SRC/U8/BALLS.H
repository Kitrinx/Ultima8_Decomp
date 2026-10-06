#ifndef BALLS_H
#define BALLS_H

#include "..\GLIB\PROCESS.H"
#include "MLIST.H"

// Ball frame; frames 1 and 4 explode on contact.
enum BallType
{
	BallType_0
};

// Bit 1: the ball moves in z; bit 2: it trails child balls.
enum BallVariety
{
	BallVariety_0
};

class Explode : public Process
{
public:
	char count;

	Explode(unsigned short);
	virtual void process(void);
};

class Ball : public ProcessInterrupt
{
public:
	char dx;
	char dy;
	char dz;
	unsigned short target;
	BallVariety variety;
	unsigned short *children;
	char maxSpeed;
	BallType type;
	int numChildren;

	Ball(BallVariety, unsigned short, char);
	virtual void process(void);
	unsigned char init(BallType, unsigned short, unsigned short, unsigned short);
	int cantMove(void);
	void generateChild(void);
	void reflect(void);
	void addSpeed(void);
	void explode(void);
};

void TonysBalls(short, short, unsigned short, unsigned short, unsigned short);

#endif
