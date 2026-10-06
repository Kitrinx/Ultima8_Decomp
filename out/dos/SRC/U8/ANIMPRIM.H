#ifndef ANIMPRIM_H
#define ANIMPRIM_H

#include "..\GLIB\PROCESS.H"
#include "ANIMCACH.H"
#include "ANODE.H"
#include "MLIST.H"
#include "TYPE.H"

// The value an AnimPrimitive ends with, read back by the process it notifies.
struct AnimResult
{
	unsigned hit : 1;			// the attack hit something
	unsigned reachedTarget : 1;	// the move touched its target
	unsigned done : 1;			// the animation ran to its end
	unsigned hitTarget : 1;		// the attack hit the target
	unsigned fell : 1;			// the move would leave the NPC unsupported
	unsigned flags : 11;
	unsigned high;

	AnimResult(void) {}
	AnimResult(long value) { *(long *)this = value; }
	AnimResult &operator=(long value) { *(long *)this = value; return *this; }
	void clear(void) { *(long *)this = 0; }
};

// One animation of an NPC, played a frame per tick; moving ones also move the NPC.
class AnimPrimitive : public ProcessInterrupt
{
public:
	AnimSet anim;
	char dir;
	unsigned char frames;
	unsigned char started;
	unsigned char stopMoving;
	unsigned char collided;
	unsigned char blocked;
	unsigned char standAfter;
	unsigned char trySideways;
	unsigned char failNext;
	AnimResult result;
	unsigned f_3d;
	unsigned target;
	WorldPoint *dest;			// set when only testing a move
	unsigned char savedFirstStep;
	int savedLastAnimSet;
	char savedDir;
	unsigned char savedFlipped;
	unsigned char savedFlags2F_5;
	unsigned char savedAnimFrame;

	AnimPrimitive(unsigned short, AnimSet, char, WorldPoint *, unsigned short, unsigned short);
	virtual void process(void);
	virtual void pop(long);
	virtual void fail(long);
	virtual void interruptHandler(unsigned long, unsigned long);
	virtual WorldPoint getPoint(WorldPoint, char, AnimNode);
	virtual void failFunction(void);
	void initialize(void);
	void figureStartingPhase(void);
	void legalStore(void);
	void legalRevert(void);
	void cleanUp(void);
	void weaponCheck(AnimStreamHeader *, WorldPoint *, AnimNode *);
	void collideWithItem(unsigned short);
	unsigned char checkGravity(WorldPoint, WorldPoint, unsigned short);
};

#endif
