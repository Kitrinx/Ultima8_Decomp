#ifndef AHEADER_H
#define AHEADER_H

#include "ITEM.H"
#include "ANIMCACH.H"

// Offsets of a shape's animations in anim.dat, one per AnimSet.
class AnimHeader
{
public:
	unsigned long data[64];

	AnimHeader(void) { clear(); }
	void clear(void);
	Boolean hasAnim(AnimSet anim) { return data[anim] != 0; }
	void setAnim(AnimSet anim, long offset) { data[anim] = offset; }
	void clearAnim(AnimSet anim) { data[anim] = 0; }
	Boolean isData(void);
};

#endif
