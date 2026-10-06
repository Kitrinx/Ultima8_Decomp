// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: ANODE.C

#include <mem.h>
#include "ANODE.H"

void AnimNode::initialize(void)
{
	memset(this, 0, sizeof(AnimNode));
}

void AnimNode::operator=(AnimNode &node)
{
	memcpy(this, &node, sizeof(AnimNode));
}
