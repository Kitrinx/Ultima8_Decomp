#ifndef PATHFIND_H
#define PATHFIND_H

#include "..\GLIB\PROCESS.H"
#include "TYPE.H"

// Walks an NPC toward a point or an item one step at a time, trying the
// direct direction first and sweeping around obstacles.
class PathFinder : public ProcessInterrupt
{
public:
	WorldPoint dest;
	char dir;
	char lastDir;
	char tryDir;
	unsigned char sweeping;
	unsigned char sweepLeft;
	unsigned char targetSolid;
	int bestDist;
	int closeEnough;
	unsigned maxSteps;
	unsigned steps;
	unsigned char testOnly;	// plan the path without animating
	unsigned char arrived;
	unsigned target;

	PathFinder(unsigned short, unsigned short, WorldPoint, unsigned short, unsigned char);
	PathFinder(unsigned short, unsigned short, unsigned short, unsigned char);
	virtual void process(void);
	virtual void pop(long);
	virtual void fail(long);
	void end(void);
	char getLegalDir(WorldPoint *);
};

char canGetThere(unsigned short, unsigned short, unsigned short);
char canGetThere(unsigned short, unsigned short, WorldPoint, unsigned short);

#endif
