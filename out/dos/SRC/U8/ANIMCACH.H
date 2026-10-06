#ifndef ANIMCACH_H
#define ANIMCACH_H

#include "ASTREAM.H"

class AnimNode;

// Animation numbers within a shape's anim.dat entry.
enum AnimSet
{
	ANIM_WALK = 0,
	ANIM_RUN = 1,
	ANIM_STAND = 2,
	ANIM_JUMP_UP = 3,
	ANIM_STAND_UP = 4,
	ANIM_DRAW_WEAPON = 5,
	ANIM_SHEATH_WEAPON = 6,
	ANIM_ATTACK = 7,
	ANIM_ADVANCE = 8,
	ANIM_RETREAT = 9,
	ANIM_RUNNING_LEAP = 10,
	ANIM_SHAKE_HEAD = 11,
	ANIM_STEP = 12,
	ANIM_STUMBLE_BACK = 13,
	ANIM_DIE = 14,
	ANIM_COMBAT_STAND = 15,
	ANIM_LAND = 16,
	ANIM_JUMP = 17,
	ANIM_AIRWALK_JUMP = 18,
	ANIM_CLIMB16 = 19,
	ANIM_CLIMB72 = 26,
	ANIM_CAST1 = 27,
	ANIM_CAST5 = 31,
	ANIM_LOOK_LEFT = 32,
	ANIM_LOOK_RIGHT = 33,
	ANIM_KNEEL_DOWN = 34,
	ANIM_KNEEL = 35,
	ANIM_EDGE_BALANCE = 42,
	ANIM_FALL_BACK = 44,
	ANIM_HANG = 45,
	ANIM_CLIMB_UP = 46,
	ANIM_FIDGET1 = 47,
	ANIM_FIDGET2 = 48,
	ANIM_KNEEL2 = 49,
	ANIM_KNEEL_UP = 50,
	ANIM_SIT_DOWN = 51,
	ANIM_SIT_UP = 52,
	ANIM_TALK = 53,
	ANIM_WORK = 55,
	ANIM_DROWN = 56,
	ANIM_BURN = 57,
	ANIM_KICK = 58,
	ANIM_BLOCK_START = 59,
	ANIM_BLOCK_END = 60
};

class AnimFile;

// The animations of one cached shape, indexed by AnimSet.
class AnimCacheHeader
{
public:
	AnimStream *streams[64];

	void clear(void);
	unsigned char hasStream(int anim) { return streams[anim] != 0; }
	void setStream(int anim, AnimStream *stream) { streams[anim] = stream; }
};

// Keeps the animations of the last few shapes used; slot 0 holds shape 1, the Avatar.
class AnimCache
{
public:
	static void init(void);
	static void uninit(void);
	static void free(short);
	static int getSlot(unsigned short);
	static int load(unsigned short);
	static AnimCacheHeader *getHeader(unsigned short);
	static void getNode(unsigned short, AnimSet, char, short, AnimNode *);
	static AnimStreamHeader getAnimStreamHeader(unsigned short, AnimSet);
	static void flush(void);
	static void openFile(void);
	static void closeFile(void);
	static unsigned char isAnimInExistence(unsigned short);
	static unsigned char isAnimInExistence(unsigned short, AnimSet);
	static char marker;
	static AnimCacheHeader *animHeader;
	static AnimFile file;
	static int typeArray[10];
};

#endif
