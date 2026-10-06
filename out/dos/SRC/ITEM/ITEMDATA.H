#ifndef ITEMDATA_H
#define ITEMDATA_H

#include "MAPFILE.H"

class OneItem;

#define MAX_ITEMS 9216

class ItemData
{
public:
	~ItemData(void);
	static void init(void);
	static void getItem(unsigned short, OneItem &);
	static void putItem(unsigned short, OneItem &);
	static unsigned short *xArray;
	static unsigned short *yArray;
	static unsigned char *zArray;
	static unsigned short *typeArray;
	static unsigned char *frameArray;
	static unsigned short *statusArray;
	static unsigned short *qArray;
	static unsigned char *npcNumArray;
	static unsigned char *mapNumArray;
	static unsigned short *nextArray;
};

extern ItemData itemData;

#endif
