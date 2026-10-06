// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: ..\ITEM\ITEMDATA.C

#include <stddef.h>
#include "CFILE.H"
#include "CEXIT.H"
#include "FILESPEC.H"
#include "SCRATCHM.H"
#include "GLOB.H"
#include "ITEMCACH.H"
#include "ITEMDATA.H"

// Inline in the shared headers when this file was built.
inline Index::Index(void) { size = 0; offset = 0; }

ItemData itemData;

unsigned short *ItemData::xArray;
unsigned short *ItemData::yArray;
unsigned char *ItemData::zArray;
unsigned short *ItemData::typeArray;
unsigned char *ItemData::frameArray;
unsigned short *ItemData::statusArray;
unsigned short *ItemData::qArray;
unsigned char *ItemData::npcNumArray;
unsigned char *ItemData::mapNumArray;
unsigned short *ItemData::nextArray;

void ItemData::init(void)
{
	xArray = new unsigned short[MAX_ITEMS];
	yArray = new unsigned short[MAX_ITEMS];
	zArray = new unsigned char[MAX_ITEMS];
	typeArray = new unsigned short[MAX_ITEMS];
	frameArray = new unsigned char[MAX_ITEMS];
	statusArray = new unsigned short[MAX_ITEMS];
	qArray = new unsigned short[MAX_ITEMS];
	npcNumArray = new unsigned char[MAX_ITEMS];
	mapNumArray = new unsigned char[MAX_ITEMS];
	nextArray = new unsigned short[MAX_ITEMS];
	if (!xArray || !yArray || !zArray || !typeArray || !frameArray ||
		!qArray || !npcNumArray || !mapNumArray || !nextArray)
		outOfMemory(__FILE__, 80);	// __LINE__ in the original file
}

ItemData::~ItemData(void)
{
	if (xArray)
	{
		delete xArray;
		xArray = 0;
	}
	if (yArray)
	{
		delete yArray;
		yArray = 0;
	}
	if (zArray)
	{
		delete zArray;
		zArray = 0;
	}
	if (typeArray)
	{
		delete typeArray;
		typeArray = 0;
	}
	if (frameArray)
	{
		delete frameArray;
		frameArray = 0;
	}
	if (statusArray)
	{
		delete statusArray;
		statusArray = 0;
	}
	if (qArray)
	{
		delete qArray;
		qArray = 0;
	}
	if (npcNumArray)
	{
		delete npcNumArray;
		npcNumArray = 0;
	}
	if (mapNumArray)
	{
		delete mapNumArray;
		mapNumArray = 0;
	}
	if (nextArray)
	{
		delete nextArray;
		nextArray = 0;
	}
}

void ItemData::getItem(unsigned short referent, OneItem &item)
{
	item.x = xArray[referent];
	item.y = yArray[referent];
	item.z = zArray[referent];
	item.type = typeArray[referent];
	item.frame = frameArray[referent];
	item.flags = statusArray[referent];
	item.quality = qArray[referent];
	item.npcNum = npcNumArray[referent];
	item.mapNum = mapNumArray[referent];
	item.next_object = nextArray[referent];
}

void ItemData::putItem(unsigned short referent, OneItem &item)
{
	xArray[referent] = item.x;
	yArray[referent] = item.y;
	zArray[referent] = item.z;
	typeArray[referent] = item.type;
	frameArray[referent] = item.frame;
	statusArray[referent] = item.flags;
	qArray[referent] = item.quality;
	npcNumArray[referent] = item.npcNum;
	mapNumArray[referent] = item.mapNum;
	nextArray[referent] = item.next_object;
}

