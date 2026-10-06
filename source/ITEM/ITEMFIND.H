#ifndef ITEMFIND_H
#define ITEMFIND_H

#include "ITEM.H"
#include "ITEMDATA.H"

// SurfaceItemFinder::what
#define SURFACE_ON		1	// items standing on the surface
#define SURFACE_UNDER	2	// items the surface stands on

class AreaItemFinder;
class ContainerItemFinder;
class ItemFinder;
class RegionItemFinder;

class ItemFinder : public Item
{
public:
	Referent first;				// head of the list being searched
	unsigned char criteria[32];	// a SearchCriteria program

	ItemFinder(void) {}
	ItemFinder(unsigned short, char *, unsigned short);
	Boolean found(void) { return referent != 0; }
	unsigned short getType(void) { return ItemData::typeArray[referent]; }
	virtual void findNext(void);
	void restartSearch(void);
	unsigned char meetsSearchCriteria(void);
};

class RegionItemFinder : public ItemFinder
{
public:
	RegionItemFinder(void) {}
	RegionItemFinder(unsigned short, unsigned short, char *, unsigned short);
	void gotoRegion(unsigned short, unsigned short);
};

class AreaItemFinder : public RegionItemFinder
{
public:
	unsigned left;
	unsigned top;
	unsigned right;
	unsigned bottom;
	unsigned curX;		// corner of the region being searched
	unsigned curY;

	AreaItemFinder(void) {}
	AreaItemFinder(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2) { init(x1, y1, x2, y2, "$", 1); }
	AreaItemFinder(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, char *s, unsigned short q) { init(x1, y1, x2, y2, s, q); }
	// The square of the given radius around x, y.
	AreaItemFinder(int x, int y, unsigned short range, char *s, unsigned short q) { init(x - range, y - range, x + range, y + range, s, q); }
	virtual void findNext(void);
	virtual Boolean inArea(void);
	void init(unsigned short, unsigned short, unsigned short, unsigned short, char *, unsigned short);
	void init(short, short, char *, unsigned short);
	void gotoNextList(void);
};

class RecursiveAreaItemFinder : public AreaItemFinder
{
public:
	RecursiveAreaItemFinder(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, char *s, unsigned short q) { init(x1, y1, x2, y2, s, q); }
	RecursiveAreaItemFinder(int x, int y, int range, char *s, unsigned short q) { init(x - range, y - range, x + range, y + range, s, q); }
	virtual void findNext(void);
	virtual Boolean inArea(void);
};

class ContainerItemFinder : public ItemFinder
{
public:
	ContainerItemFinder(void) {}
	ContainerItemFinder(Referent container, char *s, unsigned short q) { init(container, s, q); }
	void init(unsigned short, char *, unsigned short);
};

class RecursiveContainerItemFinder : public ContainerItemFinder
{
public:
	Referent container;

	RecursiveContainerItemFinder(Referent container) { init(container, "$", 1); }
	RecursiveContainerItemFinder(Referent container, char *s, unsigned short q) { init(container, s, q); }
	virtual void findNext(void);
	void init(unsigned short, char *, unsigned short);
};

class SurfaceItemFinder : public AreaItemFinder
{
public:
	unsigned x;
	unsigned y;
	unsigned char z;
	Referent surface;
	unsigned char what;		// SURFACE_ON, SURFACE_UNDER
	unsigned char where;	// which test the found item passed

	SurfaceItemFinder(Referent r, char *s, unsigned short q, unsigned char what) { init(r, s, q, what); }
	virtual Boolean inArea(void);
	void init(unsigned short, char *, unsigned short, unsigned char);
};

#endif
