// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: CAMERA.C

#include <dos.h>
#include <stdlib.h>
#include "ITEM.H"
#include "ITEMCACH.H"
#include "ITEMDATA.H"
#include "NPC.H"
#include "TYPE.H"
#include "GAMETIME.H"
#include "CAMERA.H"
#include "CFILE.H"
#include "CGLOBVP.H"
#include "CTMPOINT.H"
#include "CVPORT.H"
#include "DISPATCH.H"
#include "DLITEM.H"
#include "ERROR.H"
#include "IRGUMP.H"
#include "MOTRAK.H"
#include "MVSCRN.H"
#include "OCCLUDE.H"
#include "PAL.H"
#include "STATREST.H"

// Inline in the shared headers when this file was built.
inline GraphicYtable::GraphicYtable(void) { f_04 = 2; index = 0; }
inline Vport::Vport(void) { seg = 0; f_0f = 2; f_10 = 0; }
inline Vport::~Vport(void) { free(); }
inline int Vport::getSeg(void) { return seg; }
inline GraphicYtable *Vport::getYTable(void) { return &graphicYtable; }
inline int GraphicYtable::getindex(short y) { return index[y]; }
inline void Vport::slam(Vport *from) { SlamScreen(getSeg(), MK_FP(from->getSeg(), from->getYTable()->getindex(0))); }
inline Point::Point(void) {}
inline void Point::set(short x, short y) { f_00 = x; f_02 = y; }
inline Rect::Rect(void) {}
inline void Rect::set(short x1, short y1, short x2, short y2) { Point::set(x1, y1); f_04 = x2; f_06 = y2; }
inline Rect::Rect(short x1, short y1, short x2, short y2) { set(x1, y1, x2, y2); }

extern "C" int PutPixel(Vport *, int, int, unsigned char);

extern unsigned char inGameMode;

class CameraItem : public Item
{
public:
	CameraItem(Referent r) { referent = r; }
};

inline Item itemOf(Referent r) { return CameraItem(r); }

// The world corner of region (rx, ry).
inline void regionOrigin(int rx, int ry, unsigned &x, unsigned &y)
{
	x = rx << 9;
	y = ry << 9;
}

#define PROCESS_CAMERA	((ProcessType)0x219)
#define TRACE_CAMERA	((TraceLevel)50)
#define ROOF_NONE		250

unsigned char hideEditorItems = 0;
int Camera::_h = 0;
int Camera::_v = 0;
unsigned char Camera::_moved = 0;
unsigned char Camera::_drawn = 0;
unsigned char Camera::lastShowFootpads = 0;
int Camera::_roof = ROOF_NONE;
unsigned Camera::roof_item_x = 0xffff;
unsigned Camera::roof_item_y = 0xffff;
unsigned char Camera::roof_item_z = 255;
unsigned short Camera::centeredOn = 0;
unsigned char Camera::needAnim = 0;
unsigned Camera::globBounds = 0;
unsigned Camera::globBoundsX = 0;
unsigned Camera::globBoundsY = 0;
char Camera::quakeMagnitude = 0;
char Camera::quakeX = 0;
char Camera::quakeY = 0;
char Camera::quakeDX = 0;
char Camera::quakeDY = 0;
unsigned char Camera::displayListCount = 0;
unsigned char Camera::disableDrawBit = 0;
unsigned char Camera::drawUgliKolur = 0;
unsigned char Camera::screenInverted = 0;
unsigned char Camera::skipFrames = 0;
Camera *theCamera = 0;
CameraProcess *theCameraProcess = 0;
ScreenArea Camera::_validarea;
unsigned Camera::_x;
unsigned Camera::_y;
unsigned char Camera::_z;
unsigned char Camera::catchingAvatar;

void Camera::initPalette(void)
{
	activateCurrentPalette();
}

CameraProcess::CameraProcess(void)
{
	setProcessType(PROCESS_CAMERA);
	Kernel::setIdString(pid, "Camera");
}

void CameraProcess::postLoad(void)
{
	theCameraProcess = this;
	new FadeProcess((PaletteNum)currentPalette, 0x7fff, 0);
	DisplayList::genesis();
	Camera::init();
	theZbuffer.make();
}

void CameraProcess::fail(long result)
{
	theCameraProcess = 0;
	Process::fail(result);
}

void CameraProcess::process(void)
{
	theCamera->show();
	if (Camera::displayListCount)
	{
		trace(TRACE_CAMERA, 1, 3, "Nodes: %04d", DisplayListNode::freecount);
		trace(TRACE_CAMERA, 1, 4, "Edges: %04d", DisplayListEdge::freecount);
	}
}

void CameraProcess::load(BaseFile *file)
{
	unsigned char inverted;
	Process::load(file);
	file->read((char *)&Camera::_h, 2);
	file->read((char *)&Camera::_v, 2);
	file->read((char *)&Camera::_validarea, 8);
	file->read((char *)&Camera::_x, 2);
	file->read((char *)&Camera::_y, 2);
	file->read((char *)&Camera::_z, 1);
	file->read((char *)&Camera::_roof, 2);
	file->read((char *)&Camera::roof_item_x, 2);
	file->read((char *)&Camera::roof_item_y, 2);
	file->read((char *)&Camera::roof_item_z, 1);
	file->read((char *)&Camera::centeredOn, 2);
	file->read((char *)&Camera::catchingAvatar, 1);
	file->read((char *)&Camera::needAnim, 1);
	file->read((char *)&Camera::quakeMagnitude, 1);
	file->read((char *)&Camera::quakeX, 1);
	file->read((char *)&Camera::quakeY, 1);
	file->read((char *)&Camera::quakeDX, 1);
	file->read((char *)&Camera::quakeDY, 1);
	file->read((char *)&Camera::skipFrames, 1);
	file->read((char *)&inverted, 1);
	Camera::setScreenInverted(inverted);
	file->read((char *)&currentPalette, 2);
}

void CameraProcess::save(BaseFile *file)
{
	Process::save(file);
	file->write((char *)&Camera::_h, 2);
	file->write((char *)&Camera::_v, 2);
	file->write((char *)&Camera::_validarea, 8);
	file->write((char *)&Camera::_x, 2);
	file->write((char *)&Camera::_y, 2);
	file->write((char *)&Camera::_z, 1);
	file->write((char *)&Camera::_roof, 2);
	file->write((char *)&Camera::roof_item_x, 2);
	file->write((char *)&Camera::roof_item_y, 2);
	file->write((char *)&Camera::roof_item_z, 1);
	file->write((char *)&Camera::centeredOn, 2);
	file->write((char *)&Camera::catchingAvatar, 1);
	file->write((char *)&Camera::needAnim, 1);
	file->write((char *)&Camera::quakeMagnitude, 1);
	file->write((char *)&Camera::quakeX, 1);
	file->write((char *)&Camera::quakeY, 1);
	file->write((char *)&Camera::quakeDX, 1);
	file->write((char *)&Camera::quakeDY, 1);
	file->write((char *)&Camera::skipFrames, 1);
	file->write((char *)&Camera::screenInverted, 1);
	file->write((char *)&currentPalette, 2);
}

void initU8Camera(void)
{
	theCameraProcess = new CameraProcess;
	theCameraProcess->setDaemon();
	Kernel::setIdString(theCameraProcess->pid, "Camera");
}

void ScreenToWorldCoords(short sx, short sy, unsigned short &x, unsigned short &y)
{
	int h = sx + Camera::_h;
	int v = sy + Camera::_v;
	x = h * 2 + v * 4;
	y = v * 4 - h * 2;
}

// As above, but onto the top of the item under the point, if any. *above is
// set when the point is past the item's top.
void ScreenToWorldCoords(short sx, short sy, unsigned short &x, unsigned short &y, unsigned char &z, FastItem *found, unsigned char *above)
{
	ScreenToWorldCoords(sx, sy, x, y);
	unsigned short target = DisplayList::find_target(sx, sy);
	DisplayListNode *node = NODE(target);
	if (target && node->nodetype() == 2)
	{
		CameraItem item(((DL_ItemNode *)node)->referent);
		if (item.isValid())
		{
			int xd, yd, zd;
			TypeFlag type = GlobalTypes.typeFlags[ItemData::typeArray[item.referent]];
			type.getWorldSize(xd, yd, zd);
			z = ItemData::zArray[item.referent] + zd;
			if (z > ROOF_NONE)
				z = ROOF_NONE;
			x += z * 4;
			y += z * 4;
			if (found)
				*(Item *)found = item;
			if (ItemData::xArray[item.referent] <= x || ItemData::yArray[item.referent] <= y)
			{
				int d;
				if ((int)(x - ItemData::xArray[item.referent]) > (int)(y - ItemData::yArray[item.referent]))
					d = x - ItemData::xArray[item.referent];
				else
					d = y - ItemData::yArray[item.referent];
				if (z >= (d >> 2))
				{
					x -= d;
					y -= d;
					z -= d >> 2;
				}
				if (above)
					*above = 0;
			}
			else if (above)
				*above = 1;
			return;
		}
		z = 0;
		return;
	}
	z = 0;
}

void WorldToScreenCoords(unsigned short x, unsigned short y, short &sx, short &sy)
{
	sx = ((int)(x - y) >> 2) - Camera::_h;
	sy = ((x + y) >> 3) - Camera::_v;
}

// Puts every item of the loaded regions that shows on screen into the display list.
void Camera::add_items_to_list(void)
{
	ScreenArea grid, view, region;
	Item item;
	int xd, yd, zd;
	int top, left;
	int x, y, sy, sx;
	int rh, rv;
	unsigned wx, wy;
	DL_ItemNode::delayed_pop();
	left = _h & ~(SQUARE_SIZE - 1);
	top = _v & ~(SQUARE_SIZE - 1);
	view = ScreenArea(left, top, left + SQUARE_COLS * SQUARE_SIZE - 1, top + SQUARE_ROWS * SQUARE_SIZE - 1);
	left = ShadowSquareLC;
	top = ShadowSquareTC;
	grid = ScreenArea(left, top, left + SQUARE_COLS * SQUARE_SIZE - 1, top + SQUARE_ROWS * SQUARE_SIZE - 1);
	for (int sum = ItemCache::regionsIn.minSum; sum <= ItemCache::regionsIn.maxSum; sum++)
	{
		for (int diff = ItemCache::regionsIn.minDiff; diff <= ItemCache::regionsIn.maxDiff; diff++)
		{
			// Only even sums and differences give whole regions.
			if ((sum + diff) & 1)
				continue;
			unsigned rx = (unsigned)(sum + diff) >> 1;
			unsigned ry = (unsigned)(sum - diff) >> 1;
			if (!ItemCache::regionsIn.isNeeded(rx, ry))
				continue;
			if (_moved)
			{
				// A region already on screen needs nothing new.
				regionOrigin(rx, ry, wx, wy);
				rh = (wx >> 2) - (wy >> 2);
				rv = (wx >> 3) + (wy >> 3);
				region = ScreenArea(rh - 240, rv - 250, rh + 239, rv + 127);
				if (!region.intersects(grid))
					continue;
				if (view.contains(region))
					continue;
			}
			if (!ItemCache::regionsIn.isNeeded(rx, ry))
				continue;
			unsigned char hideEditor = inGameMode || hideEditorItems;
			for (Referent r = ItemCache::regionStartHash[rx][ry]; r; r = ItemData::nextArray[item.referent])
			{
				item = itemOf(r);
				TypeFlag *type = &GlobalTypes.typeFlags[ItemData::typeArray[item.referent]];
				if (!(ItemData::statusArray[item.referent] & IN_DISPLAY_LIST) &&
					((!disableDrawBit && type->draw) || ItemData::zArray[item.referent] < _roof) &&
					!theZbuffer.isHidden(item.referent) &&
					((!type->editor && hideEditor) || !hideEditor) &&
					(!(ItemData::statusArray[item.referent] & INVISIBLE) || item.isAvatar() || !inGameMode))
				{
					x = ItemData::xArray[item.referent] >> 2;
					y = ItemData::yArray[item.referent] >> 2;
					sx = x - y;
					sy = (x >> 1) + (y >> 1) - ItemData::zArray[item.referent];
					footpad(xd, yd, zd, *type);
					ScreenArea area(sx - xd, sy - (xd >> 1) - (yd >> 1) - zd, sx + yd, sy);
					if (grid.intersects(area))
					{
						DL_ItemNode *node = new DL_ItemNode;
						unsigned short n = FP_OFF(node);
						if (n)
						{
							node->set(r);
							if (node->area.intersects(grid))
							{
								item.orStatus(IN_DISPLAY_LIST);
								DisplayList::add_node(n);
								if (!DisplayList::sort(n))
								{
									item.andStatus(~IN_DISPLAY_LIST);
									DisplayList::remove_node(n);
									delete node;
								}
							}
							else
								delete node;
						}
						else
							trace(TRACE_CAMERA, "item node alloc");
					}
				}
			}
		}
	}
}

// Moves the shadow square grid so its corner is at (left, top).
void Camera::renumber_shadow_squares(short left, short top)
{
	int row, x, y, col;
	for (row = 0; row < SQUARE_ROWS; row++)
	{
		for (col = 0; col < SQUARE_COLS; col++)
		{
			x = left + col * SQUARE_SIZE;
			y = top + row * SQUARE_SIZE;
			NODE(ShadowSquare[row][col])->area = ScreenArea(x, y, x + SQUARE_SIZE - 1, y + SQUARE_SIZE - 1);
		}
	}
	ShadowSquareLC = left;
	ShadowSquareTC = top;
}

// Centres the camera on world point (x, y, z). The shadow square grid
// scrolls in whole squares; nodes on squares that scroll off are dropped and
// the rest sorted again.
void Camera::move_to(unsigned short x, unsigned short y, unsigned char z, short force)
{
	int col, row, h, v, r, dx, dy, newCol, newRow, oldCol, oldRow;
	DisplayListNode *np;
	int c, left, top, right, bottom;
	unsigned short e, up, edge;
	unsigned short saved[SQUARE_ROWS][SQUARE_COLS];
	_x = x;
	_y = y;
	_z = z;
	ScreenArea newArea, oldArea;
	ItemCache::goTo(x, y, force);
	if (inGameMode && centeredOn == 1 && roof_item_moved())
		find_new_roof(avatar);
	h = (x >> 2) - (y >> 2) - 160;
	v = (x >> 3) + (y >> 3) - z - 100;
	if (!_moved)
	{
		DisplayList::genesis();
		renumber_shadow_squares(h & ~(SQUARE_SIZE - 1), v & ~(SQUARE_SIZE - 1));
		add_items_to_list();
	}
	else
	{
		dx = dy = 0;
		c = ShadowSquareLC;
		r = ShadowSquareTC;
		if (h < c)
			dx = ((h & ~(SQUARE_SIZE - 1)) - c) >> SQUARE_SHIFT;
		if (v < r)
			dy = ((v & ~(SQUARE_SIZE - 1)) - r) >> SQUARE_SHIFT;
		if (h + 319 >= c + SQUARE_COLS * SQUARE_SIZE)
			dx = (((h + 319) & ~(SQUARE_SIZE - 1)) - (c + (SQUARE_COLS - 1) * SQUARE_SIZE)) >> SQUARE_SHIFT;
		if (v + 199 >= r + SQUARE_ROWS * SQUARE_SIZE)
			dy = (((v + 199) & ~(SQUARE_SIZE - 1)) - (r + (SQUARE_ROWS - 1) * SQUARE_SIZE)) >> SQUARE_SHIFT;
		if (dx == 0 && dy == 0)
			goto moved;
		newCol = (ShadowSquareLC >> SQUARE_SHIFT) + dx;
		newRow = (ShadowSquareTC >> SQUARE_SHIFT) + dy;
		oldArea = ScreenArea(c, r, c + SQUARE_COLS * SQUARE_SIZE - 1, r + SQUARE_ROWS * SQUARE_SIZE - 1);
		newArea = ScreenArea(newCol << SQUARE_SHIFT, newRow << SQUARE_SHIFT,
			(newCol << SQUARE_SHIFT) + SQUARE_COLS * SQUARE_SIZE - 1, (newRow << SQUARE_SHIFT) + SQUARE_ROWS * SQUARE_SIZE - 1);
		unsigned char overlap = oldArea.intersects(newArea);
		oldCol = ShadowSquareLC >> SQUARE_SHIFT;
		oldRow = ShadowSquareTC >> SQUARE_SHIFT;
		// Drop what stood on squares that leave the grid.
		for (row = 0; row < SQUARE_ROWS; row++)
		{
			for (col = 0; col < SQUARE_COLS; col++)
			{
				saved[row][col] = ShadowSquare[row][col];
				if (oldCol + col < newCol || oldCol + col >= newCol + SQUARE_COLS ||
					oldRow + row < newRow || oldRow + row >= newRow + SQUARE_ROWS)
				{
					for (e = NODE(ShadowSquare[row][col])->above; e; )
					{
						edge = e;
						up = EDGE(e)->upper;
						np = NODE(up);
						e = EDGE(e)->nextAbove;
						if (!np->area.intersects(newArea) || !overlap)
						{
							DisplayList::remove_node(up);
							if (np->nodetype() == 2)
							{
								((DL_ItemNode *)np)->flag_out_of_list();
								delete np;
							}
						}
						else
							DisplayList::delete_edge(edge);
					}
				}
			}
		}
		// Rotate the squares so they keep their screen order.
		if (dy >= 0)
			r = dy % SQUARE_ROWS;
		else
			r = SQUARE_ROWS - 1 - (-dy - 1) % SQUARE_ROWS;
		for (row = 0; row < SQUARE_ROWS; row++)
		{
			if (dx >= 0)
				c = dx % SQUARE_COLS;
			else
				c = SQUARE_COLS - 1 - (-dx - 1) % SQUARE_COLS;
			for (col = 0; col < SQUARE_COLS; col++)
			{
				ShadowSquare[row][col] = saved[r][c];
				if (++c == SQUARE_COLS)
					c = 0;
			}
			if (++r == SQUARE_ROWS)
				r = 0;
		}
		renumber_shadow_squares(newCol << SQUARE_SHIFT, newRow << SQUARE_SHIFT);
		// Sort again what reaches past the grid.
		for (row = 0; row < SQUARE_ROWS; row++)
		{
			for (col = 0; col < SQUARE_COLS; col++)
			{
				for (e = NODE(ShadowSquare[row][col])->above; e; )
				{
					up = EDGE(e)->upper;
					np = NODE(up);
					e = EDGE(e)->nextAbove;
					if (!(np->flags & 0x10) && np->area.intersects(newArea) && !oldArea.contains(np->area))
					{
						if (DisplayList::resort(up))
							np->flags |= 0x10;
						else
						{
							if (np->nodetype() == 2)
							{
								Item item(((DL_ItemNode *)np)->referent);
								item.andStatus(~IN_DISPLAY_LIST);
							}
							DisplayList::remove_node(up);
							delete np;
						}
					}
				}
			}
		}
		for (row = 0; row < SQUARE_ROWS; row++)
		{
			for (col = 0; col < SQUARE_COLS; col++)
			{
				for (e = NODE(ShadowSquare[row][col])->above; e; )
				{
					up = EDGE(e)->upper;
					np = NODE(up);
					e = EDGE(e)->nextAbove;
					if (np->area.intersects(newArea) && !oldArea.contains(np->area))
						np->flags &= ~0x10;
				}
			}
		}
		add_items_to_list();
	}
moved:
	unsigned char scrolled = 0;
	if (_drawn)
	{
		left = _h - h;
		top = _v - v;
		if (left >= -319 && left <= 319 && top >= -199 && top <= 199)
		{
			if (left || top)
			{
				ItemRelativeGump::notifyMoved(0, (MovedState)0);
				scrolled = 1;
				DL_ItemNode::delayed_draw();
				DL_ItemNode::delayed_gump();
				MoveScreen(GlobalVport::global_ptr, left, -top);
				right = _validarea.right + left;
				bottom = _validarea.bottom + top;
				left += _validarea.left;
				top += _validarea.top;
				if (left < 0)
					left = 0;
				if (top < 0)
					top = 0;
				if (right < 0)
					right = 0;
				if (bottom < 0)
					bottom = 0;
				if (left > 319)
					left = 319;
				if (top > 199)
					top = 199;
				if (right > 319)
					right = 319;
				if (bottom > 199)
					bottom = 199;
				_validarea = ScreenArea(left, top, right, bottom);
			}
		}
		else
			_drawn = 0;
	}
	_h = h;
	_v = v;
	_moved = 1;
	if (scrolled)
		ItemRelativeGump::notifyMoved(0, (MovedState)1);
}

void SwapInt(int *a, int *b)
{
	int t = *b;
	*b = *a;
	*a = t;
}

// A Bresenham line, clipped point by point to the screen.
void Line(Vport *vport, int x1, int y1, int x2, int y2, unsigned char color)
{
	int d, incDiagonal, incStraight, xstep, ystep, x, y, dx, dy;
	if (abs(x2 - x1) < abs(y2 - y1))
	{
		if (y1 > y2)
		{
			SwapInt(&x1, &x2);
			SwapInt(&y1, &y2);
		}
		xstep = x2 > x1 ? 1 : -1;
		dy = y2 - y1;
		dx = abs(x2 - x1);
		d = dx * 2 - dy;
		incDiagonal = (dx - dy) * 2;
		incStraight = dx * 2;
		x = x1;
		y = y1;
		if (x > 0 && x < 320 && y > 0 && y < 200)
			PutPixel(vport, x, y, color);
		for (y = y1 + 1; y <= y2; y++)
		{
			if (d >= 0)
			{
				x += xstep;
				d += incDiagonal;
			}
			else
				d += incStraight;
			if (x > 0 && x < 320 && y > 0 && y < 200)
				PutPixel(vport, x, y, color);
		}
	}
	else
	{
		if (x1 > x2)
		{
			SwapInt(&x1, &x2);
			SwapInt(&y1, &y2);
		}
		ystep = y2 > y1 ? 1 : -1;
		dx = x2 - x1;
		dy = abs(y2 - y1);
		d = dy * 2 - dx;
		incDiagonal = (dy - dx) * 2;
		incStraight = dy * 2;
		x = x1;
		y = y1;
		if (x >= 0 && x < 320 && y >= 0 && y < 200)
			PutPixel(vport, x, y, color);
		for (x = x1 + 1; x <= x2; x++)
		{
			if (d >= 0)
			{
				y += ystep;
				d += incDiagonal;
			}
			else
				d += incStraight;
			if (x > 0 && x < 320 && y > 0 && y < 200)
				PutPixel(vport, x, y, color);
		}
	}
}

void Camera::show(short left, short top, short right, short bottom, unsigned char full)
{
	if (!okToShow)
		return;
	DL_ItemNode::delayed_pop();
	DL_ItemNode::delayed_draw();
	if (!full)
	{
		ScreenArea area(left + _h, top + _v, right + _h, bottom + _v);
		DisplayList::mark(area);
		DisplayList::clip_l = left;
		DisplayList::clip_t = top;
		DisplayList::clip_r = right;
		DisplayList::clip_b = bottom;
		DisplayList::vport = *GlobalVport::global_ptr;
		DisplayList::vport.rect.set(left, top, right, bottom);
		DisplayList::draw();
	}
	if (globBounds)
		drawBounds(GlobalVport::global_ptr);
	dispatcher->refresh(Rect(left, top, right, bottom));
}

ScrollProcessPlusPlus::ScrollProcessPlusPlus(unsigned short toX, unsigned short toY, unsigned char toZ, int maxStep)
{
	Camera::setCenterOn(0);
	maxSpeed = maxStep << 2;
	speed = 1;
	if (maxSpeed <= 0 || maxSpeed > 0x400)
		maxSpeed = 0x40;
	x = Camera::getX();
	y = Camera::getY();
	z = Camera::getZ();
	destX = toX;
	destY = toY;
	destZ = toZ;
	dx = destX - x;
	dy = destY - y;
	dz = destZ - z;
	if (dx > 0)
		sx = 1;
	else if (dx == 0)
		sx = 0;
	else
		sx = -1;
	if (dy > 0)
		sy = 1;
	else if (dy == 0)
		sy = 0;
	else
		sy = -1;
	if (dz > 0)
		sz = 1;
	else if (dz == 0)
		sz = 0;
	else
		sz = -1;
	dx = abs(dx);
	dy = abs(dy);
	dz = abs(dz);
	if (dx > dy)
		steps = dx;
	else
		steps = dy;
	if (dz > steps)
		steps = dz;
	errX = 0;
	errY = 0;
	errZ = 0;
	step = 0;
	Camera::setCenterOn(0);
}

void ScrollProcessPlusPlus::process(void)
{
	if (steps - step < speed)
	{
		speed >>= 1;
		if (speed == 0)
			speed++;
	}
	else if (speed < maxSpeed)
	{
		speed <<= 1;
		if (speed > maxSpeed)
			speed = maxSpeed;
	}
	for (int i = 0; i < speed; i++)
	{
		if (++step > steps)
		{
			pop(0);
			break;
		}
		errX += dx;
		errY += dy;
		errZ += dz;
		if (errX > steps)
		{
			errX -= steps;
			x += sx;
		}
		if (errY > steps)
		{
			errY -= steps;
			y += sy;
		}
		if (errZ > steps)
		{
			errZ -= steps;
			z += sz;
		}
	}
	dispatcher->restore(Rect(0, 0, 199, 319));
	Camera::move_to(x, y, z, 0);
	dispatcher->refresh(Rect(0, 0, 199, 319));
	BaseCamera::needSlam = 1;
}

ScrollProcess::ScrollProcess(unsigned short toX, unsigned short toY, unsigned char toZ, int stepsPerTick)
{
	WorldToScreenCoords(Camera::getX(), Camera::getY(), x, y);
	WorldToScreenCoords(toX, toY, destX, destY);
	destY += Camera::getZ() - toZ;
	dx = abs(destX - x);
	dy = abs(destY - y);
	speed = stepsPerTick;
	if (destX >= x && destY <= y)
	{
		if (dx < dy)
			octant = 3;
		else
			octant = 1;
	}
	else if (destX <= x && destY <= y)
	{
		if (dx < dy)
			octant = 2;
		else
			octant = 4;
	}
	else if (destX <= x && destY >= y)
	{
		if (dx < dy)
			octant = 6;
		else
			octant = 5;
	}
	else if (destX >= x && destY >= y)
	{
		if (dx < dy)
			octant = 7;
		else
			octant = 8;
	}
	if (dx < dy)
	{
		error = dx * 2 - dy;
		incStraight = dx * 2;
		incDiagonal = (dx - dy) * 2;
	}
	else
	{
		error = dy * 2 - dx;
		incStraight = dy * 2;
		incDiagonal = (dy - dx) * 2;
	}
	Camera::setCenterOn(0);
}

void ScrollProcess::process(void)
{
	int done = 0;
	short startX = x;
	short startY = y;
	unsigned short wx, wy;
	short sx, sy;
	for (int i = 0; i < speed; i++)
	{
		if (octant == 1 && x < destX)
		{
			if (error <= 0)
				error += incStraight;
			else
			{
				error += incDiagonal;
				y--;
			}
			x++;
		}
		else if (octant == 2 && y > destY)
		{
			if (error <= 0)
				error += incStraight;
			else
			{
				error += incDiagonal;
				x--;
			}
			y--;
		}
		else if (octant == 3 && y > destY)
		{
			if (error <= 0)
				error += incStraight;
			else
			{
				error += incDiagonal;
				x++;
			}
			y--;
		}
		else if (octant == 4 && x > destX)
		{
			if (error <= 0)
				error += incStraight;
			else
			{
				error += incDiagonal;
				y--;
			}
			x--;
		}
		else if (octant == 5 && x > destX)
		{
			if (error <= 0)
				error += incStraight;
			else
			{
				error += incDiagonal;
				y++;
			}
			x--;
		}
		else if (octant == 6 && y < destY)
		{
			if (error <= 0)
				error += incStraight;
			else
			{
				error += incDiagonal;
				x--;
			}
			y++;
		}
		else if (octant == 7 && y < destY)
		{
			if (error <= 0)
				error += incStraight;
			else
			{
				error += incDiagonal;
				x++;
			}
			y++;
		}
		else if (octant == 8 && x < destX)
		{
			if (error <= 0)
				error += incStraight;
			else
			{
				error += incDiagonal;
				y++;
			}
			x++;
		}
		else
			done = 1;
	}
	WorldToScreenCoords(Camera::getX(), Camera::getY(), sx, sy);
	ScreenToWorldCoords(sx + (x - startX), sy + (y - startY), wx, wy);
	Camera::move_to(wx, wy, Camera::getZ(), 0);
	BaseCamera::needSlam = 1;
	if (done)
		pop(0);
}

int Camera::scrollTo(unsigned short x, unsigned short y, unsigned char z, int speed)
{
	if (getX() >= x - 1 && getX() <= x + 1 && getY() >= y - 1 && getY() <= y + 1 &&
		getZ() >= z - 1 && getZ() <= z + 1)
		return 0;
	ScrollProcessPlusPlus *scroll = new ScrollProcessPlusPlus(x, y, z, speed);
	return scroll->pid;
}

// Outlines the bounds of the current glob's area.
void Camera::drawBounds(Vport *vport)
{
	unsigned short x = globBoundsX;
	unsigned short y = globBoundsY;
	short sx, sy;
	WorldToScreenCoords(x, y, sx, sy);
	sy -= 2;
	Line(vport, sx - 128, sy + 65, sx - 1, sy + 129, 14);
	Line(vport, sx, sy, sx + 127, sy + 64, 14);
	Line(vport, sx - 1, sy, sx - 128, sy + 64, 14);
	Line(vport, sx + 127, sy + 65, sx, sy + 129, 14);
}

// Follows the item the camera is centred on.
void Camera::position(void)
{
	if (centeredOn)
	{
		int xd, yd, zd;
		TypeFlag *type = &GlobalTypes.typeFlags[avatar.getType()];
		type->getFootpad(xd, yd, zd);
		Item item(centeredOn);
		move_to(item.getX() - (xd >> 1), item.getY() - (yd >> 1), item.getZ() + (zd >> 1), 0);
	}
	else if (!_moved)
		move_to(_x, _y, _z, 0);
}

void Camera::show(void)
{
	position();
	if (DL_ItemNode::showFootpads != lastShowFootpads)
	{
		_drawn = 0;
		lastShowFootpads = DL_ItemNode::showFootpads;
	}
	if (inGameMode && centeredOn == 1 && roof_item_moved())
	{
		find_new_roof(Item(centeredOn));
		move_to(_x, _y, _z, 0);
	}
	theAnimation->lockTime();
	if (needAnim)
	{
		needAnim = 0;
		DL_ItemNode::delayed_pop();
		DisplayList::animate();
	}
	if (_drawn)
	{
		// Draw only what scrolled into view.
		if (_validarea.left > 0)
		{
			if (drawUgliKolur)
				DrawBox(GlobalVport::global_ptr, 0, 0, _validarea.left - 1, 199, 0x58);
			show(0, 0, _validarea.left - 1, 199, 0);
			needSlam = 1;
		}
		if (_validarea.top > 0)
		{
			if (drawUgliKolur)
				DrawBox(GlobalVport::global_ptr, _validarea.left, 0, _validarea.right, _validarea.top - 1, 0x58);
			show(_validarea.left, 0, _validarea.right, _validarea.top - 1, 0);
			needSlam = 1;
		}
		if (_validarea.right < 319)
		{
			if (drawUgliKolur)
				DrawBox(GlobalVport::global_ptr, _validarea.right + 1, 0, 319, 199, 0x58);
			show(_validarea.right + 1, 0, 319, 199, 0);
			needSlam = 1;
		}
		if (_validarea.bottom < 199)
		{
			if (drawUgliKolur)
				DrawBox(GlobalVport::global_ptr, _validarea.left, _validarea.bottom + 1, _validarea.right, 199, 0x58);
			show(_validarea.left, _validarea.bottom + 1, _validarea.right, 199, 0);
			needSlam = 1;
		}
	}
	else
	{
		if (drawUgliKolur)
			GlobalVport::global_ptr->clear(0x58);
		show(0, 0, 319, 199, 0);
		needSlam = 1;
	}
	theAnimation->unlockTime();
	slam();
	_validarea = ScreenArea(0, 0, 319, 199);
	_drawn = 1;
}

void Camera::slam(void)
{
	alternateFrame = !alternateFrame;
	if (needSlam && skipFrames && alternateFrame)
		return;
	if (!needSlam && quakeMagnitude == 0)
		return;
	DL_ItemNode::delayed_pop();
	DL_ItemNode::delayed_draw();
	DL_ItemNode::delayed_gump();
	if (MousePointer::visible)
		TMMousePointer::before_slam(GlobalVport::global_ptr);
	if (quakeMagnitude)
		slamQuake();
	else if (screenInverted)
		GlobalVport::main_screen->copy_from(GlobalVport::global_ptr);
	else
		GlobalVport::main_screen->slam(GlobalVport::global_ptr);
	if (MousePointer::visible)
		TMMousePointer::after_slam(GlobalVport::global_ptr);
	needSlam = 0;
}

void Camera::startQuake(short magnitude)
{
	quakeMagnitude = magnitude;
}

void Camera::stopQuake(void)
{
	quakeMagnitude = 0;
	quakeX = quakeY = quakeDX = quakeDY = 0;
	needSlam = 1;
}

// Shows the picture shifted by the quake offset, and moves the offset at random.
void Camera::slamQuake(void)
{
	Vport screen, buffer;
	unsigned short *index;
	int i;
	screen = *GlobalVport::global_ptr;
	buffer = *GlobalVport::main_screen;
	if (quakeY < 0)
	{
		DrawBox(GlobalVport::main_screen, 0, quakeY + 200, 319, 199, 0);
		screen.init_ytable(0);
		index = screen.graphicYtable.index;
		for (i = 0; i < quakeY + 200; i++)
			index[i] = index[i - quakeY];
		screen.rect.f_06 += quakeY;
	}
	else
	{
		DrawBox(GlobalVport::main_screen, 0, 0, 319, quakeY - 1, 0);
		buffer.init_ytable(0);
		index = buffer.graphicYtable.index;
		for (i = 0; i < 200 - quakeY; i++)
			index[i] = index[i + quakeY];
		if (screenInverted)
		{
			for (i = 0; i < 100; i++)
			{
				unsigned short t = index[i];
				index[i] = index[199 - i];
				index[199 - i] = t;
			}
		}
		screen.rect.f_06 -= quakeY;
	}
	if (quakeX < 0)
	{
		DrawBox(GlobalVport::main_screen, quakeX + 320, 0, 319, 199, 0);
		screen.rect.f_00 -= quakeX;
	}
	else
	{
		DrawBox(GlobalVport::main_screen, 0, 0, quakeX - 1, 199, 0);
		screen.rect.f_04 -= quakeX;
		buffer.rect.f_00 += quakeX;
	}
	buffer.copy_from(&screen);
	quakeX < 0 ? (char)(quakeDX += -quakeX >> 1) : (char)(quakeDX -= quakeX >> 1);
	quakeY < 0 ? (char)(quakeDY += -quakeY >> 1) : (char)(quakeDY -= quakeY >> 1);
	urandom(2) ? quakeDX++ : quakeDX--;
	urandom(2) ? quakeDY++ : quakeDY--;
	if (quakeDX > quakeMagnitude)
		quakeDX = quakeMagnitude;
	if (quakeDX < -quakeMagnitude)
		quakeDX = -quakeMagnitude;
	if (quakeDY > quakeMagnitude)
		quakeDY = quakeMagnitude;
	if (quakeDY < -quakeMagnitude)
		quakeDY = -quakeMagnitude;
	quakeX += quakeDX;
	quakeY += quakeDY;
}

// Turns the picture upside down over 100 frames.
void Camera::invertScreen(unsigned char inverted)
{
	if (screenInverted == inverted)
		return;
	int frame;
	screenInverted = 1;
	unsigned short *index = GlobalVport::main_screen->graphicYtable.index;
	for (frame = 0; frame < 100; frame++)
	{
		int i;
		unsigned short t;
		for (i = 0; i < 199; i += 2)
		{
			t = index[i];
			index[i] = index[i + 1];
			index[i + 1] = t;
		}
		for (i = 1; i < 198; i += 2)
		{
			t = index[i];
			index[i] = index[i + 1];
			index[i + 1] = t;
		}
		needSlam = 1;
		theCamera->slam();
	}
	screenInverted = inverted;
}

void Camera::setScreenInverted(unsigned char inverted)
{
	if (screenInverted != inverted)
	{
		screenInverted = 1;
		unsigned short *index = GlobalVport::main_screen->graphicYtable.index;
		for (int i = 0; i < 100; i++)
		{
			unsigned short t = index[i];
			index[i] = index[199 - i];
			index[199 - i] = t;
		}
		screenInverted = inverted;
	}
}

void Camera::setCenterOn(unsigned short r)
{
	centeredOn = r;
}

void Camera::move_rel(int dx, int dy, int dz)
{
	move_to(_x + dx, _y + dy, _z + dz, 0);
}

void Camera::set_roof(short z)
{
	if (z >= 0 && z <= ROOF_NONE && z != _roof)
	{
		_roof = z;
		theZbuffer.make();
		_drawn = 0;
		_moved = 0;
	}
}

// The roof is the lowest thing above the item; nothing above means no roof.
void Camera::find_new_roof(Item item)
{
	unsigned char z;
	int xd, yd, zd;
	roof_item_x = item.getX();
	roof_item_y = item.getY();
	z = item.getZ();
	roof_item_z = z;
	TypeFlag *type = &GlobalTypes.typeFlags[item.getType()];
	type->getWorldSize(xd, yd, zd);
	int roof = ROOF_NONE;
	MotionTracker tracker;
	WorldPoint top = item.getLoc();
	top.z = ROOF_NONE;
	if (!tracker.isClear(item.getType(), item.getLoc(), top, 4, 0, 8, 0) && tracker.hitItem)
		roof = Item(tracker.hitItem).getZ();
	set_roof(roof);
}

unsigned char Camera::roof_item_moved(void)
{
	return Item(centeredOn).getX() != roof_item_x || Item(centeredOn).getY() != roof_item_y ||
		Item(centeredOn).getZ() != roof_item_z;
}

void Camera::init(void)
{
	_moved = 0;
	_drawn = 0;
	DL_ItemNode::delayed_pop();
}

unsigned Camera::getX(void)
{
	return _x;
}

unsigned Camera::getY(void)
{
	return _y;
}

unsigned char Camera::getZ(void)
{
	return _z;
}

int Camera::roof(void)
{
	return _roof;
}
