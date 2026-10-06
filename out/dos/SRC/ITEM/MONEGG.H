#ifndef MONEGG_H
#define MONEGG_H

#include "ITEM.H"

// An egg that hatches monsters. Its fields live in the item:
// quality = probability index << 11 | monster shape, mapArray = monster id << 3 | activity,
// npcArray low 3 bits = number of monsters.
class MonsterEgg : public Item
{
public:
	void hatch(void);
	int getMonId(void);
	void setMonId(short);
	int getActivity(void);
	void setActivity(short);
	int getShapeType(void);
	void setShapeType(short);
	int getNumMonsters(void);
	void setNumMonsters(short);
	int getProb(void);
	void setProb(short);
};

extern unsigned char chances[8];

#endif
