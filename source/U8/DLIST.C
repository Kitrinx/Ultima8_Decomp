// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: DLIST.C

#include <mem.h>
#include "ITEM.H"
#include "ITEMDATA.H"
#include "TYPE.H"
#include "GAMETIME.H"
#include "BASECAM.H"
#include "CAMERA.H"
#include "CGLOBVP.H"
#include "ERROR.H"
#include "SCRATCHM.H"
#include "SHAPHAND.H"
#include "STATREST.H"
#include "DLIST.H"
#include "DLITEM.H"

// Inline in the shared headers when this file was built.
inline GraphicYtable::GraphicYtable(void) { f_04 = 2; index = 0; }
inline Vport::Vport(void) { seg = 0; f_0f = 2; f_10 = 0; }
inline Vport::~Vport(void) { free(); }
inline Point::Point(void) {}
inline void Point::set(short x, short y) { f_00 = x; f_02 = y; }
inline Rect::Rect(void) {}
inline void Rect::set(short x1, short y1, short x2, short y2) { Point::set(x1, y1); f_04 = x2; f_06 = y2; }

class AnimItem : public Item
{
public:
	AnimItem(Referent r) { referent = r; }
};

inline Item itemOf(Referent r) { return AnimItem(r); }

#define TRACE_DLIST	((TraceLevel)50)

ScreenArea FullScreenArea(0, 0, 319, 199);
unsigned DisplayListNodeSeg = 0;
unsigned DisplayListEdgeSeg = 0;
unsigned char DisplayList::animating = 0;
long DisplayList::lasttime = 0;
unsigned long *DisplayList::animatingitem = 0;
unsigned long *DisplayList::newitem = 0;
int ShadowSquareLC = 0;
int ShadowSquareTC = 0;
unsigned short DisplayListNode::freelist = 0;
unsigned short DisplayListEdge::freelist = 0;
char *DisplayListNode::buffer = 0;
char *DisplayListEdge::buffer = 0;
unsigned DisplayListNode::freecount = 0;
unsigned DisplayListEdge::freecount = 0;
DisplayListQueue DisplayList::_waiting_queue;
DisplayListQueue DisplayList::_drawing_queue;
int DisplayList::clip_l;
int DisplayList::clip_t;
int DisplayList::clip_r;
int DisplayList::clip_b;
Vport DisplayList::vport;
unsigned short ShadowSquare[SQUARE_ROWS][SQUARE_COLS];

void ScreenArea::intersect(ScreenArea &a)
{
	if (left < a.left)
		left = a.left;
	if (top < a.top)
		top = a.top;
	if (right > a.right)
		right = a.right;
	if (bottom > a.bottom)
		bottom = a.bottom;
}

void ScreenArea::include(ScreenArea &a)
{
	if (left > a.left)
		left = a.left;
	if (top > a.top)
		top = a.top;
	if (right < a.right)
		right = a.right;
	if (bottom < a.bottom)
		bottom = a.bottom;
}

void *DisplayListNode::operator new(unsigned)
{
	DisplayListNode *node = 0;
	if (freelist)
	{
		node = NODE(freelist);
		freelist = node->next;
		freecount--;
	}
	return node;
}

void DisplayListNode::operator delete(void *p)
{
	DisplayListNode *node = (DisplayListNode *)p;
	node->next = freelist;
	freelist = FP_OFF(node);
	freecount++;
}

void DisplayListNode::init(void)
{
	if (!buffer)
		buffer = new char[NUM_NODES * sizeof(DL_ItemNode)];
	if (!buffer)
		halt(__FILE__, 152);	// __LINE__
	DisplayListNodeSeg = FP_SEG(buffer);
	unsigned short n = FP_OFF(buffer);
	freecount = NUM_NODES;
	// Offset 0 means no node, so a buffer starting there loses its first node.
	if (n == 0)
	{
		freelist = FP_OFF(buffer) + sizeof(DL_ItemNode);
		freecount--;
	}
	else
		freelist = n;
	for (int i = 0; i < NUM_NODES - 1; i++)
	{
		NODE(n)->next = n + sizeof(DL_ItemNode);
		n += sizeof(DL_ItemNode);
	}
	NODE(n)->next = 0;
}

void DisplayListNode::uninit(void)
{
	if (buffer)
	{
		delete buffer;
		buffer = 0;
		DisplayListNodeSeg = 0;
	}
}

void DisplayListQueue::remove(unsigned short n)
{
	if (head == n)
		head = NODE(n)->next;
	if (tail == n)
		tail = NODE(n)->prev;
	NODE(n)->unlink();
}

// Queues the nodes drawn over n, so they are drawn again after it.
void DisplayList::draw(unsigned short n)
{
	DisplayListNode *node = NODE(n);
	if (node->nodetype() != 1)
	{
		unsigned short e = node->above;
		while (e)
		{
			unsigned short up = EDGE(e)->upper;
			if (!(NODE(up)->flags & (DLN_WAITING | DLN_DRAWING)))
			{
				_waiting_queue.add(up);
				NODE(up)->flags |= DLN_WAITING;
			}
			e = EDGE(e)->nextAbove;
		}
	}
}

void DisplayList::draw(ScreenArea area)
{
	area.left -= Camera::_h;
	area.top -= Camera::_v;
	area.right -= Camera::_h;
	area.bottom -= Camera::_v;
	if (area.intersects(FullScreenArea))
	{
		area.intersect(FullScreenArea);
		clip_l = area.left;
		clip_t = area.top;
		clip_r = area.right;
		clip_b = area.bottom;
		vport = *GlobalVport::global_ptr;
		vport.rect.set(area.left, area.top, area.right, area.bottom);
		draw();
	}
}

// Records that upper is drawn over lower.
unsigned char DisplayList::connect_nodes(unsigned short upper, unsigned short lower)
{
	if (upper != lower)
	{
		unsigned short e = (unsigned short)new DisplayListEdge(upper, lower);
		if (e)
		{
			DisplayListNode *up = NODE(upper);
			DisplayListNode *low = NODE(lower);
			up->below = e;
			low->above = e;
			if (up->isTranslucent())
				low->flags |= DLN_UNDER_TRANSLUCENT;
		}
		else
		{
			trace(TRACE_DLIST, "dl edge alloc ");
			return 0;
		}
	}
	return 1;
}

void DisplayList::add_node(unsigned short n)
{
	if (n)
	{
		NODE(n)->flags |= DLN_WAITING;
		_waiting_queue.add(n);
	}
}

void DisplayList::remove_node(unsigned short n)
{
	DisplayListNode *node = NODE(n);
	unsigned short e, dead, low;
	for (e = node->below; e; )
	{
		low = EDGE(e)->lower;
		if (!NODE(low)->waiting())
		{
			_waiting_queue.add(low);
			NODE(low)->flags |= DLN_WAITING;
		}
		EDGE(e)->unlink();
		dead = e;
		e = EDGE(e)->nextBelow;
		delete EDGE(dead);
	}
	node->below = 0;
	for (e = node->above; e; )
	{
		EDGE(e)->unlink();
		dead = e;
		e = EDGE(e)->nextAbove;
		delete EDGE(dead);
	}
	node->above = 0;
	if (node->flags & DLN_WAITING)
	{
		node->flags &= ~DLN_WAITING;
		_waiting_queue.remove(n);
	}
}

// Connects a and b in drawing order, if they overlap.
char DisplayList::sort(unsigned short a, unsigned short b)
{
	char ok = 1;
	DisplayListNode *na = NODE(a);
	DisplayListNode *nb = NODE(b);
	if (b != a)
	{
		int order;
		if (na->nodetype() < nb->nodetype())
			order = 2;
		else if (na->nodetype() > nb->nodetype())
			order = 1;
		else
			order = na->compare(b);
		if (order == 1)
		{
			if (!connect_nodes(a, b))
				ok = 0;
		}
		else if (order == 2)
		{
			if (!connect_nodes(b, a))
				ok = 0;
		}
	}
	return ok;
}

// Sorts n again against everything in the shadow squares under it.
char DisplayList::resort(unsigned short n)
{
	char ok = 1;
	ScreenArea area;
	int x1, y1, x2, y2, x, y;
	unsigned short square, marked = 0;
	DisplayListNode *sq, *mn, *node, *on;
	unsigned short e, other;
	node = NODE(n);
	area = node->area;
	x1 = (area.left - ShadowSquareLC) >> SQUARE_SHIFT;
	y1 = (area.top - ShadowSquareTC) >> SQUARE_SHIFT;
	x2 = (area.right - ShadowSquareLC) >> SQUARE_SHIFT;
	y2 = (area.bottom - ShadowSquareTC) >> SQUARE_SHIFT;
	if (x1 < SQUARE_COLS && y1 < SQUARE_ROWS && x2 >= 0 && y2 >= 0)
	{
		// Nodes already connected to n need no sorting.
		for (e = node->above; e; e = EDGE(e)->nextAbove)
		{
			unsigned short up = EDGE(e)->upper;
			mn = NODE(up);
			mn->flags |= DLN_MARKED;
			mn->markNext = marked;
			marked = up;
		}
		for (e = node->below; e; e = EDGE(e)->nextBelow)
		{
			unsigned short low = EDGE(e)->lower;
			mn = NODE(low);
			mn->flags |= DLN_MARKED;
			mn->markNext = marked;
			marked = low;
		}
		if (x1 < 0)
			x1 = 0;
		if (y1 < 0)
			y1 = 0;
		if (x2 >= SQUARE_COLS)
			x2 = SQUARE_COLS - 1;
		if (y2 >= SQUARE_ROWS)
			y2 = SQUARE_ROWS - 1;
		for (y = y1; y <= y2; y++)
		{
			for (x = x1; x <= x2; x++)
			{
				square = ShadowSquare[y][x];
				sq = NODE(square);
				if (!(sq->flags & DLN_MARKED))
				{
					for (e = sq->above; e; e = EDGE(e)->nextAbove)
					{
						other = EDGE(e)->upper;
						on = NODE(other);
						if (!(on->flags & DLN_MARKED) && node->area.intersects(on->area))
						{
							if (!resort(n, other))
								return 0;
							on->flags |= DLN_MARKED;
							on->markNext = marked;
							marked = other;
						}
					}
					if (!resort(n, square))
						return 0;
				}
			}
		}
		while (marked)
		{
			other = marked;
			marked = NODE(other)->markNext;
			NODE(other)->flags &= ~DLN_MARKED;
		}
	}
	return ok;
}

unsigned char DisplayList::resort(unsigned short a, unsigned short b)
{
	unsigned char ok = 1;
	DisplayListNode *na = NODE(a);
	DisplayListNode *nb = NODE(b);
	if (b != a)
	{
		int order;
		if (na->nodetype() < nb->nodetype())
			order = 2;
		else if (na->nodetype() > nb->nodetype())
			order = 1;
		else
			order = na->compare(b);
		if (order == 1)
		{
			if (!connect_nodes(a, b))
				ok = 0;
		}
		else if (order == 2)
		{
			if (!connect_nodes(b, a))
				ok = 0;
		}
	}
	return ok;
}

void DisplayList::init(void)
{
	int x, y;
	DisplayListNode::init();
	DisplayListEdge::init();
	animating = 0;
	clip_l = 0;
	clip_t = 0;
	clip_r = 319;
	clip_b = 199;
	for (x = 0; x < SQUARE_COLS; x++)
	{
		for (y = 0; y < SQUARE_ROWS; y++)
		{
			unsigned short square = (unsigned short)new DL_ShadowSquareNode;
			if (square)
			{
				NODE(square)->flags |= DLN_SHADOW;
				ShadowSquare[y][x] = square;
				int left = x * SQUARE_SIZE;
				int top = y * SQUARE_SIZE;
				NODE(square)->area = ScreenArea(left, top, left + SQUARE_SIZE - 1, top + SQUARE_SIZE - 1);
				add_node(square);
			}
			else
			{
				trace(TRACE_DLIST, "dl init ");
				ShadowSquare[y][x] = 0;
			}
		}
	}
	if (!animatingitem)
		animatingitem = (unsigned long *)new char[0x480];
	memset(animatingitem, 0, 0x480);
	if (!newitem)
		newitem = (unsigned long *)new char[0x480];
	memset(newitem, 0, 0x480);
}

void DisplayList::uninit(void)
{
	DisplayListNode::uninit();
	DisplayListEdge::uninit();
	if (animatingitem)
	{
		delete animatingitem;
		animatingitem = 0;
	}
	if (newitem)
	{
		delete newitem;
		newitem = 0;
	}
}

void DisplayList::delete_node(unsigned short n)
{
	remove_node(n);
	delete NODE(n);
}

void DisplayList::delete_edge(unsigned short e)
{
	EDGE(e)->unlink();
	delete EDGE(e);
}

void DisplayList::writeDebugInfo(char *)
{
}

// The topmost node that claims screen point (x, y), or 0.
unsigned short DisplayList::find_target(short x, short y)
{
	ScreenArea area;
	unsigned short target = 0;
	unsigned short square;
	x += Camera::_h;
	y += Camera::_v;
	area = ScreenArea(x, y);
	square = ShadowSquare[(y - ShadowSquareTC) >> SQUARE_SHIFT][(x - ShadowSquareLC) >> SQUARE_SHIFT];
	DisplayListNode *sq = NODE(square);
	DisplayListNode *n;
	DisplayListEdge *edge, *edge2;
	unsigned short e;
	for (e = sq->above; e; e = edge->nextAbove)
	{
		edge = EDGE(e);
		n = NODE(edge->upper);
		if (n->is_target(x, y))
		{
			target = edge->upper;
			n->flags |= DLN_MARKED;
		}
	}
	unsigned short first = e = sq->above;
	if (target)
	{
		// Of the marked nodes, take one with no marked node over it.
		for (e = first; e; e = edge->nextAbove)
		{
			edge = EDGE(e);
			target = edge->upper;
			n = NODE(target);
			if (n->flags & DLN_MARKED)
			{
				for (e = n->above; e; e = edge2->nextAbove)
				{
					edge2 = EDGE(e);
					if (NODE(edge2->upper)->flags & DLN_MARKED)
					{
						target = 0;
						break;
					}
				}
				if (target)
					break;
			}
		}
		for (e = first; e; e = edge->nextAbove)
		{
			edge = EDGE(e);
			NODE(edge->upper)->flags &= ~DLN_MARKED;
		}
	}
	return target;
}

DisplayListNode::DisplayListNode(void)
{
	next = prev = 0;
	above = below = 0;
	flags = 0;
}

// Queues every node that overlaps area to be drawn again.
void DisplayList::mark(ScreenArea &area)
{
	int x1, x2, y2, x, y;
	DisplayListNode *node;
	x1 = (area.left - ShadowSquareLC) >> SQUARE_SHIFT;
	int y1 = (area.top - ShadowSquareTC) >> SQUARE_SHIFT;
	x2 = (area.right - ShadowSquareLC) >> SQUARE_SHIFT;
	y2 = (area.bottom - ShadowSquareTC) >> SQUARE_SHIFT;
	if (x1 < 0)
		x1 = 0;
	if (y1 < 0)
		y1 = 0;
	if (x2 >= SQUARE_COLS)
		x2 = SQUARE_COLS - 1;
	if (y2 >= SQUARE_ROWS)
		y2 = SQUARE_ROWS - 1;
	for (y = y1; y <= y2; y++)
	{
		for (x = x1; x <= x2; x++)
		{
			unsigned short e = ShadowSquare[y][x];
			for (e = NODE(e)->above; e; e = EDGE(e)->nextAbove)
			{
				unsigned short up = EDGE(e)->upper;
				node = NODE(up);
				if (!node->waiting() && node->area.intersects(area))
				{
					_waiting_queue.add(up);
					node->flags |= DLN_WAITING;
				}
				unsigned r = node->getReferent();
				newitem[r >> 5] |= 1L << (r & 31);
			}
		}
	}
}

// Fills list with the animating items on screen that are due a new frame
// at time now, and returns how many. Items gone from view stop animating.
int DisplayList::checkAnimatingItems(long now, unsigned short *list)
{
	Referent base;
	unsigned long mask, word;
	unsigned long *bits;
	int w, b, count;
	Item item;
	TypeFlag type;
	Referent r;
	count = 0;
	base = 0;
	bits = animatingitem;
	for (w = 0; w < 0x120; w++)
	{
		if (*bits)
		{
			word = *bits;
			for (r = base, b = 0, mask = 1; b < 32; r++, b++, mask <<= 1)
			{
				item = itemOf(r);
				if (word & mask)
				{
					if (ItemData::statusArray[item.referent] & 1)
					{
						int x, y, z, sx, sy, left, top, right, bottom, xd, yd, zd;
						type = GlobalTypes.typeFlags[ItemData::typeArray[item.referent]];
						x = ItemData::xArray[item.referent] >> 2;
						y = ItemData::yArray[item.referent] >> 2;
						z = ItemData::zArray[item.referent];
						sx = x - y;
						sy = (x >> 1) + (y >> 1) - z;
						if (ItemData::statusArray[item.referent] & 0x20)
							footpad(yd, xd, zd, type);
						else
							footpad(xd, yd, zd, type);
						left = sx - xd - Camera::_h;
						top = sy - (xd >> 1) - (yd >> 1) - zd - Camera::_v;
						right = sx + yd - Camera::_h;
						bottom = sy - Camera::_v;
						if (left < 320 && top < 200 && right >= 0 && bottom >= 0)
						{
							int speed = type.animSpeed;
							if (speed < 2 || lasttime / speed != now / speed)
								list[count++] = r;
						}
						else
							*bits &= ~mask;
					}
					else
						*bits &= ~mask;
				}
			}
		}
		base += 32;
		bits++;
	}
	return count;
}

void DisplayList::animate(void)
{
	if (theAnimation->f_4d)
		return;
	int maxWidth, maxHeight, frames, i, count;
	TypeFlag type;
	Item item;
	unsigned short *list;
	long now;
	int frame;
	DL_ItemNode::delayed_draw();
	list = (unsigned short *)theScratchMem->checkOut(0x4800);
	if (!list)
		outOfMemory(__FILE__, 1154);	// __LINE__
	now = theAnimation->f_3c;
	checkNewItems();
	count = checkAnimatingItems(now, list);
	lasttime = now;
	DL_ItemNode::delayed_draw();
	for (i = 0; i < count; i++)
	{
		item = itemOf(list[i]);
		type = GlobalTypes.typeFlags[ItemData::typeArray[item.referent]];
		shapeHandler->getInfo(ItemData::typeArray[item.referent], maxWidth, maxHeight, frames);
		switch (type.animType)
		{
		case 2:
			// Random frames.
			if (urandom(2))
			{
				item.setFrame(urandom(frames));
				BaseCamera::needSlam = 1;
			}
			break;
		case 1:
		case 3:
			frame = item.getFrame();
			if (type.animData == 0 || (type.animData == 1 && urandom(2)))
			{
				if (++frame >= frames)
					frame = 0;
				item.setFrame(frame);
			}
			else
			{
				// Loop within a group of data frames.
				frame = (frame + 1) % type.animData + (frame - frame % type.animData);
				if (frame >= frames)
					frame -= frame % type.animData;
				item.setFrame(frame);
			}
			BaseCamera::needSlam = 1;
			break;
		case 4:
			frame = item.getFrame();
			if (frame == 0 && urandom(type.animData + 2))
				break;
			if (++frame >= frames)
				frame = 0;
			item.setFrame(frame);
			BaseCamera::needSlam = 1;
			break;
		case 5:
			item.anim();
			break;
		case 6:
			frame = item.getFrame();
			if (type.animData == 0 || (type.animData == 1 && urandom(2)))
			{
				if (frame)
				{
					if (++frame >= frames)
						frame = 1;
					item.setFrame(frame);
					BaseCamera::needSlam = 1;
				}
			}
			else if (frame % type.animData)
			{
				frame = (frame + 1) % type.animData + (frame - frame % type.animData);
				if (frame % type.animData == 0)
					frame++;
				if (frame >= frames)
					frame -= frame % type.animData;
				item.setFrame(frame);
				BaseCamera::needSlam = 1;
			}
			break;
		}
	}
	theScratchMem->checkIn((char *)list);
}

// Empties the display list down to its shadow squares.
void DisplayList::genesis(void)
{
	int x, y;
	DisplayListNode *node;
	animating = 0;
	for (y = 0; y < SQUARE_ROWS; y++)
	{
		for (x = 0; x < SQUARE_COLS; x++)
		{
			unsigned short e = ShadowSquare[y][x];
			for (e = NODE(e)->above; e; )
			{
				unsigned short up = EDGE(e)->upper;
				node = NODE(up);
				node->cleanup();
				e = EDGE(e)->nextAbove;
				delete_node(up);
			}
		}
	}
	memset(animatingitem, 0, 0x480);
	memset(newitem, 0, 0x480);
}

int DisplayListNode::compare(unsigned short)
{
	return 0;
}

void DisplayListNode::link(unsigned short p, unsigned short n)
{
	prev = p;
	next = n;
	if (p)
		NODE(p)->next = FP_OFF(this);
	if (n)
		NODE(n)->prev = FP_OFF(this);
}

void DisplayListNode::unlink(void)
{
	if (prev)
		NODE(prev)->next = next;
	if (next)
		NODE(next)->prev = prev;
}

void *DisplayListEdge::operator new(unsigned)
{
	DisplayListEdge *edge = 0;
	if (freelist)
	{
		edge = EDGE(freelist);
		freelist = edge->nextAbove;
		freecount--;
	}
	return edge;
}

void DisplayListEdge::operator delete(void *p)
{
	DisplayListEdge *edge = (DisplayListEdge *)p;
	edge->nextAbove = freelist;
	freelist = FP_OFF(edge);
	freecount++;
}

void DisplayListEdge::init(void)
{
	if (!buffer)
		buffer = new char[NUM_EDGES * sizeof(DisplayListEdge)];
	if (!buffer)
		halt(__FILE__, 1380);	// __LINE__
	DisplayListEdgeSeg = FP_SEG(buffer);
	unsigned short e = FP_OFF(buffer);
	freecount = NUM_EDGES;
	if (e == 0)
	{
		freelist = FP_OFF(buffer) + sizeof(DisplayListEdge);
		freecount--;
	}
	else
		freelist = e;
	for (int i = 0; i < NUM_EDGES - 1; i++)
	{
		EDGE(e)->nextAbove = e + sizeof(DisplayListEdge);
		e += sizeof(DisplayListEdge);
	}
	EDGE(e)->nextAbove = 0;
}

void DisplayListEdge::uninit(void)
{
	if (buffer)
	{
		delete buffer;
		buffer = 0;
		DisplayListEdgeSeg = 0;
	}
}

DisplayListEdge::DisplayListEdge(unsigned short up, unsigned short low)
{
	upper = up;
	lower = low;
	nextAbove = NODE(low)->above;
	if (nextAbove)
		EDGE(nextAbove)->prevAbove = FP_OFF(this);
	prevAbove = 0;
	nextBelow = NODE(up)->below;
	if (nextBelow)
		EDGE(nextBelow)->prevBelow = FP_OFF(this);
	prevBelow = 0;
}

void DisplayListEdge::unlink(void)
{
	if (prevBelow)
		EDGE(prevBelow)->nextBelow = nextBelow;
	else
		NODE(upper)->below = nextBelow;
	if (nextBelow)
		EDGE(nextBelow)->prevBelow = prevBelow;
	if (prevAbove)
		EDGE(prevAbove)->nextAbove = nextAbove;
	else
		NODE(lower)->above = nextAbove;
	if (nextAbove)
		EDGE(nextAbove)->prevAbove = prevAbove;
}

void initDisplayList(void)
{
	DisplayList::init();
}

void uninitDisplayList(void)
{
	DisplayList::uninit();
}
