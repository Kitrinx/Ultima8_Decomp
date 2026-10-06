#ifndef ASTREAM_H
#define ASTREAM_H

class AnimNode;

// The 4-byte header of one animation in anim.dat.
class AnimStreamHeader
{
public:
	unsigned frames : 8;
	unsigned twoStep : 1;
	unsigned attack : 1;
	unsigned looping : 1;
	unsigned unstoppable : 1;
	unsigned looping2 : 1;
	unsigned flag20 : 1;
	unsigned flag40 : 1;
	unsigned hanging : 1;
	unsigned repeat : 4;
	unsigned unk4 : 6;
	unsigned unk10 : 5;
	unsigned destroyActor : 1;

	AnimStreamHeader(void);
	unsigned char getFrames(void) { return frames; }
};

// One animation: its header and its frames for all 8 directions.
class AnimStream
{
public:
	AnimNode *nodes;
	AnimStreamHeader header;

	AnimStream(void) { nodes = 0; }
	~AnimStream(void) { free(); }
	void allocate(void);
	void setAnimNode(char, unsigned char, AnimNode *);
	void free(void);
};

#endif
