#ifndef DLITEM_H
#define DLITEM_H

#include "DLIST.H"
#include "TYPE.H"

// An item's node in the display list. Positions and sizes are world units / 4.
class DL_ItemNode : public DisplayListNode
{
public:
	unsigned short referent;	// the item
	int screenX;	// x - y
	int screenY;	// (x + y) / 2 - z
	int x;
	int y;
	int z;
	int xd;		// footprint, x and y swapped for a flipped item
	int yd;
	int zd;

	virtual int nodetype(void) { return 2; }
	virtual void draw(void);
	virtual int compare(unsigned short);
	virtual unsigned char is_target(short, short);
	virtual int getReferent(void);
	virtual void cleanup(void) { flag_out_of_list(); }
	virtual unsigned char isTranslucent(void);
	void set(unsigned short r) { referent = r; update(); }
	void update(void);
	void flag_out_of_list(void);
	static unsigned short find(unsigned short, short, short, short, short);
	static unsigned short find(unsigned short);
	static void before_push(unsigned short);
	static void after_pop(unsigned short);
	static void delayed_pop(void);
	static void delayed_draw(void);
	static void include_area(short, short, short, short);
	static void delayed_gump(void);
	static unsigned char showFootpads;
	static unsigned char footpadTargetMode;
	static unsigned char ignoreMode;
	static unsigned short _popref;	// item waiting to be put back in the list
	static unsigned char gumpflag;
	static unsigned char drawflag;
	static ScreenArea gumprect;		// screen to refresh through the dispatcher
	static ScreenArea drawrect;		// screen to redraw from the display list
};

char isGameRunning(void);

inline void footpad(int &x, int &y, int &z, TypeFlag &type)
{
	type.getFootpad(x, y, z);
}


#endif
