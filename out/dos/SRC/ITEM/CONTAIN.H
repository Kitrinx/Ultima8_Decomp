#ifndef CONTAIN_H
#define CONTAIN_H

#include "ITEM.H"

class Container : public Item
{
public:
	Container(Referent r) : Item(r) {}

	Boolean holds(unsigned short);
	Boolean can_hold(Referent);
	Boolean can_hold(unsigned short, short);
	Boolean tooHeavy(short);
	int findItem(short, short);
	void removeContents(void);
	void destroyContents(void);
	int getContentsWeight(void);
	int getTotalContentsVolume(void);
	int getContentsVolume(void);
	int countItems(void);
};

extern unsigned dragWeightHack;

#endif
