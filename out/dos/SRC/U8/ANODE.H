#ifndef ANODE_H
#define ANODE_H

// One frame of an animation in one direction.
class AnimNode
{
public:
	unsigned frame : 11;
	unsigned special : 1;		// run the type's special animation code
	unsigned flagsHi : 4;
	int deltaZ : 8;
	unsigned sfx : 8;
	int deltaDir : 9;			// distance moved, in steps of 4 world units
	unsigned onGround : 1;
	unsigned attackRange : 3;
	unsigned flipped : 1;
	unsigned callUsecode : 1;
	unsigned flag80 : 1;

	void initialize(void);
	void operator=(AnimNode &);
};

#endif
