#ifndef MISSILE_H
#define MISSILE_H

class WorldPoint;

// Works out how to throw an item from one point to another.
class MissileTracker
{
public:
	int gravity;
	int range;
	int maxRange;
	unsigned speedX;
	unsigned speedY;
	int speedZ;
	unsigned char inRange;

	MissileTracker(WorldPoint, WorldPoint, short, short);
	void fire(unsigned short);
	unsigned char isPathClear(unsigned short, WorldPoint);
};

#endif
