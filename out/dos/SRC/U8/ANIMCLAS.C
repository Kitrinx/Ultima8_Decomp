// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: ANIMCLAS.C

#include "NPC.H"
#include "SCRATCHM.H"
#include "ANIMCACH.H"
#include "ANODE.H"
#include "ASTREAM.H"
#include "ANIMCLAS.H"

StandingLeap::StandingLeap(unsigned short npc, char d, WorldPoint *point) :
	AnimPrimitive(npc, ANIM_JUMP, d, 0, 0, 0)
{
	totalDist = 0;
	zDiff = 0;
	jumpFrames = 0;
	if (npc == 1)
	{
		start = avatar.getLoc();
		jumpFrames = AnimCache::getAnimStreamHeader(1, ANIM_JUMP).frames;
		if (point)
		{
			target = *point;
			zDiff = target.z - avatar.getZ();
		}
		AnimNode node;
		for (int i = 0; i < jumpFrames; i++)
		{
			AnimCache::getNode(1, ANIM_JUMP, avatar.getDir(), i, &node);
			totalDist += node.deltaDir * 4;
		}
	}
}

// Spreads the move from start to target over the frames, and the height
// difference over the frames after the fifth.
WorldPoint StandingLeap::getPoint(WorldPoint point, char d, AnimNode node)
{
	int step = node.deltaDir * 4;
	int dx;
	int dy;
	char dz;

	if (!totalDist)
	{
		dx = x8add[d] * step;
		dy = y8add[d] * step;
	}
	else
	{
		long x = ((long)target.x - start.x) * step / totalDist;
		long y = ((long)target.y - start.y) * step / totalDist;
		int frame = Npc::npcData[avatar.referent].animFrame;
		long z = node.deltaZ;
		if (frame > 5)
			z += (long)zDiff / (jumpFrames - 5);
		dx = x;
		dy = y;
		dz = z;
	}
	WorldPoint result(point.x + dx, point.y + dy, point.z + dz);
	return result;
}
