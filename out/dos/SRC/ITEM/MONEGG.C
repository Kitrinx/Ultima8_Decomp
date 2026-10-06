// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: ..\ITEM\MONEGG.C

#include "NPC.H"
#include "TYPE.H"
#include "MOTRAK.H"
#include "STATREST.H"
#include "MONEGG.H"

// Hatch chance in percent, by probability index.
unsigned char chances[8] = {5, 10, 20, 35, 50, 65, 80, 100};

void MonsterEgg::hatch(void)
{
	Npc npc(0);
	int count = getNumMonsters();
	int shape = getShapeType();
	int x = getX();
	int y = getY();
	int z = getZ();
	int nx, ny;
	int prob = getProb();
	int i, j, k;

	if (urandom(100) >= chances[prob])
		return;
	if (count == 0)
		return;
	if (npc.create(shape, 0))
	{
		npc.pop(x, y, z);
		npc.cSetActivity(getActivity());
		npc.setStatus(npc.getStatus() | 0x80);
	}
	// Place the rest on the first free spot of a 4x4 grid around the egg.
	for (i = 1; i < count; i++)
		for (j = 0; j < 4; j++)
		{
			nx = x + j * 32 * 2 - 128;
			for (k = 0; k < 4; k++)
			{
				ny = y + k * 32 * 2 - 128;
				if (canExistAt(shape, nx, ny, z, 1, 0, 0))
				{
					if (npc.create(shape, 0))
					{
						npc.pop(nx, ny, z);
						npc.cSetActivity(getActivity());
						npc.setStatus(npc.getStatus() | 0x80);
						k = 4;
						j = 4;
					}
					else
						return;
				}
			}
		}
}

int MonsterEgg::getMonId(void)
{
	return getMapArray() >> 3;
}

void MonsterEgg::setMonId(short id)
{
	int v = getMapArray() & 7;
	v |= id << 3;
	setMapArray(v);
}

int MonsterEgg::getActivity(void)
{
	return getMapArray() & 7;
}

void MonsterEgg::setActivity(short activity)
{
	int v = getMapArray() & 0xf8;
	v |= activity;
	setMapArray(v);
}

int MonsterEgg::getShapeType(void)
{
	return getQ() & 0x7ff;
}

void MonsterEgg::setShapeType(short shape)
{
	int v = getQ() & 0xf800;
	v |= shape & 0x7ff;
	setQ(v);
}

int MonsterEgg::getNumMonsters(void)
{
	return getNpcArray() & 7;
}

void MonsterEgg::setNumMonsters(short count)
{
	int v = getNpcArray() & 0xf8;
	v |= count;
	setNpcArray(v);
}

int MonsterEgg::getProb(void)
{
	int prob = getQ() >> 11;
	return prob;
}

void MonsterEgg::setProb(short prob)
{
	int v = getQ() & 0x7ff;
	v |= prob << 11;
	setQ(v);
}
