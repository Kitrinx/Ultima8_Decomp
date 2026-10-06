#ifndef ANIMCLAS_H
#define ANIMCLAS_H

#include "ANIMPRIM.H"

// The Avatar's jump, aimed so its frames carry it from start to target.
class StandingLeap : public AnimPrimitive
{
public:
	int totalDist;		// ground covered by all the frames together
	int zDiff;
	int jumpFrames;
	WorldPoint target;
	WorldPoint start;

	StandingLeap(unsigned short, char, WorldPoint *);
	virtual WorldPoint getPoint(WorldPoint, char, AnimNode);
};

#endif
