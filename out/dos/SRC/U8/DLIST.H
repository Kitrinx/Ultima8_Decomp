#ifndef DLIST_H
#define DLIST_H

#include <dos.h>
#include "CVPORT.H"

// The display list orders what is on screen so that each thing is drawn
// after everything it covers. Nodes and edges live in segments of their own
// and are passed around as offsets:
//
//   upper node --below--> edge --lower--> lower node
//   lower node --above--> edge --upper--> upper node
//
// A fixed grid of shadow square nodes covers the screen; every node is
// drawn over the squares under it.

class DisplayListNode;
class DisplayListEdge;

#define NODE(n)	((DisplayListNode *)MK_FP(DisplayListNodeSeg, n))
#define EDGE(e)	((DisplayListEdge *)MK_FP(DisplayListEdgeSeg, e))

#define NUM_NODES	1500
#define NUM_EDGES	4000

// DisplayListNode::flags
#define DLN_WAITING		0x01	// in the waiting queue
#define DLN_MARKED		0x08	// already visited by this pass
#define DLN_DRAWING		0x20	// in the drawing queue
#define DLN_SHADOW		0x40	// a shadow square
#define DLN_UNDER_TRANSLUCENT	0x80	// something translucent is drawn over it

// The screen is covered by SQUARE_COLS x SQUARE_ROWS shadow squares.
#define SQUARE_SHIFT	6
#define SQUARE_SIZE		(1 << SQUARE_SHIFT)
#define SQUARE_COLS		6
#define SQUARE_ROWS		5

class ScreenArea
{
public:
	int left;
	int top;
	int right;
	int bottom;

	ScreenArea(void) { left = 0; top = 0; right = 0; bottom = 0; }
	ScreenArea(int x, int y) { left = x; top = y; right = x; bottom = y; }
	ScreenArea(int l, int t, int r, int b) { left = l; top = t; right = r; bottom = b; }
	ScreenArea(Rect &r) { left = r.f_00; top = r.f_02; right = r.f_04; bottom = r.f_06; }
	unsigned char intersects(ScreenArea &a)
	{
		return left <= a.right && top <= a.bottom && right >= a.left && bottom >= a.top;
	}
	unsigned char contains(ScreenArea &a)
	{
		return left <= a.left && top <= a.top && right >= a.right && bottom >= a.bottom;
	}
	void intersect(ScreenArea &);
	void include(ScreenArea &);
};

class DisplayListNode
{
public:
	unsigned short markNext;	// chains the nodes marked during a pass
	unsigned short next;		// queue links
	unsigned short prev;
	unsigned short above;		// edges to the nodes drawn over this one
	unsigned short below;		// edges to the nodes this one is drawn over
	ScreenArea area;			// screen rectangle, camera included
	char flags;

	DisplayListNode(void);
	virtual int nodetype(void) { return 0; }
	virtual void draw(void) {}
	virtual int compare(unsigned short);
	virtual unsigned char is_target(short, short) { return 0; }
	virtual int getReferent(void) { return 0; }
	virtual void cleanup(void) {}
	virtual unsigned char isTranslucent(void) { return 0; }
	unsigned char waiting(void) { return flags & DLN_WAITING; }
	static void *operator new(unsigned);
	static void operator delete(void *);
	static void init(void);
	static void uninit(void);
	void link(unsigned short, unsigned short);
	void unlink(void);
	static unsigned short freelist;
	static char *buffer;
	static unsigned freecount;
};

class DisplayListQueue
{
public:
	unsigned short head;
	unsigned short tail;

	DisplayListQueue(void) { head = tail = 0; }
	void remove(unsigned short);
	unsigned short remove_head(void);
	void add_tail(unsigned short);
	void add(unsigned short n) { add_tail(n); }
};

class DisplayList
{
public:
	static void draw(unsigned short);
	static void draw(ScreenArea);
	static unsigned char connect_nodes(unsigned short, unsigned short);
	static void add_node(unsigned short);
	static void remove_node(unsigned short);
	static char sort(unsigned short, unsigned short);
	static char resort(unsigned short);
	static unsigned char resort(unsigned short, unsigned short);
	static void init(void);
	static void uninit(void);
	static void delete_node(unsigned short);
	static void delete_edge(unsigned short);
	static void writeDebugInfo(char *);
	static unsigned short find_target(short, short);
	static void mark(ScreenArea &);
	static int checkAnimatingItems(long, unsigned short *);
	static void animate(void);
	static void genesis(void);
	static void draw(void);
	static void checkNewItems(void);
	static unsigned char sort(unsigned short);
	static void stopAnimating(unsigned short r) { animatingitem[r >> 5] &= ~(1L << (r & 31)); }
	static void clearNew(unsigned short r) { newitem[r >> 5] &= ~(1L << (r & 31)); }
	static unsigned char animating;
	static long lasttime;
	static unsigned long *animatingitem;	// one bit per item
	static unsigned long *newitem;
	static DisplayListQueue _waiting_queue;
	static DisplayListQueue _drawing_queue;
	static int clip_l;
	static int clip_t;
	static int clip_r;
	static int clip_b;
	static Vport vport;
};

class DisplayListEdge
{
public:
	unsigned short upper;
	unsigned short lower;
	unsigned short nextAbove;	// links in the lower node's above list
	unsigned short prevAbove;
	unsigned short nextBelow;	// links in the upper node's below list
	unsigned short prevBelow;

	DisplayListEdge(unsigned short, unsigned short);
	static void *operator new(unsigned);
	static void operator delete(void *);
	static void init(void);
	static void uninit(void);
	void unlink(void);
	static unsigned short freelist;
	static char *buffer;
	static unsigned freecount;
};

class DL_ShadowSquareNode : public DisplayListNode
{
public:
	virtual int nodetype(void) { return 1; }
	virtual void draw(void);
	virtual int compare(unsigned short) { return 0; }
};

void initDisplayList(void);
void uninitDisplayList(void);

extern ScreenArea FullScreenArea;
extern unsigned DisplayListNodeSeg;
extern unsigned DisplayListEdgeSeg;
extern int ShadowSquareLC;
extern int ShadowSquareTC;
extern unsigned short ShadowSquare[SQUARE_ROWS][SQUARE_COLS];

#endif
