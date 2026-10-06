// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: ASTREAM.C

#include <alloc.h>
#include <mem.h>
#include "ERROR.H"
#include "ANODE.H"
#include "ASTREAM.H"

AnimStreamHeader::AnimStreamHeader(void)
{
	memset(this, 0, sizeof(AnimStreamHeader));
}

void AnimStream::allocate(void)
{
	free();
	if (header.getFrames() == 0)
		halt(__FILE__, 26);	// __LINE__
	nodes = (AnimNode *)farmalloc(8 * sizeof(AnimNode) * header.frames);
	if (!nodes)
		halt(__FILE__, 31);	// __LINE__
}

void AnimStream::setAnimNode(char dir, unsigned char frame, AnimNode *node)
{
	if (frame >= header.frames)
		halt(__FILE__, 37);	// __LINE__
	(nodes + header.frames * dir)[frame] = *node;
}

void AnimStream::free(void)
{
	if (nodes)
	{
		delete nodes;
		nodes = 0;
	}
}
