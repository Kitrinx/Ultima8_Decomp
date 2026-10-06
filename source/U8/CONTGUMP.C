// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: CONTGUMP.C

#include <stdlib.h>
#include "..\UI\NEWGUMP.H"
#include "CRECT.H"
#include "CVPORT.H"
#include "CGLOBVP.H"
#include "FVINFO.H"
#include "CFILE.H"
#include "CAFONT.H"
#include "DISPATCH.H"
#include "ERROR.H"
#include "ITEM.H"
#include "TYPE.H"
#include "NPC.H"
#include "CONTAIN.H"
#include "SHAPHAND.H"
#include "CAMERA.H"
#include "PAL.H"
#include "MAIN.H"
#include "STATREST.H"
#include "DSFXMAN.H"
#include "U8POINT.H"
#include "U8GMPSHP.H"
#include "MENU.H"
#include "MESSAGE.H"
#include "EDITEM.H"
#include "STATGUMP.H"
#include "CONTGUMP.H"

// Inline in the shared headers when this file was built.
inline Point::Point(void) {}
inline Point::Point(short x, short y) { f_00 = x; f_02 = y; }
inline void Point::set(short x, short y) { f_00 = x; f_02 = y; }
inline Rect::Rect(void) {}
inline void Rect::set(short x1, short y1, short x2, short y2) { Point::set(x1, y1); f_04 = x2; f_06 = y2; }
inline Rect::Rect(short x1, short y1, short x2, short y2) { set(x1, y1, x2, y2); }
inline int Rect::width(void) { return f_04 - f_00 + 1; }
inline int Rect::height(void) { return f_06 - f_02 + 1; }
inline NewGumpId::NewGumpId(unsigned i) { instance = i; f_02 = ~instance; f_04 = 0; }
inline NewGumpId::NewGumpId(unsigned char type, unsigned char id) { instance = (type << 8) | id; f_02 = ~instance; f_04 = 0; }
inline unsigned char NewGumpId::getType(void) { return instance >> 8; }
inline void NewGump::moveto(short x, short y) { rect.moveto(x, y); }
inline char Event::isShift(void) { return (data >> 16) & 3; }
inline char Event::isCtrl(void) { return (data >> 16) & 4; }
inline char Event::isAlt(void) { return (data >> 16) & 8; }
inline GumpShape::GumpShape(int shape) { _frame = 0; this->shape = shape; }
inline Rect GumpShape::get_rect(void) { return drawer->get_rect(shape, _frame); }
inline void GumpShape::draw(int x, int y) { drawer->draw(x, y, shape, _frame); }
inline void GumpShape::set_frame(int frame) { _frame = frame; }
inline unsigned char GumpShape::collide(int x, int y) { return drawer->collide(x, y, shape, _frame); }
inline int CacheFont::width(char *text) { return Font::width(text); }
inline CacheFont::~CacheFont(void) {}

extern "C" Boolean Collision(void *, Point, Point);

AvatarStatusGump *theStatGump = 0;
char *cheatmsg[7] = {"Sunday Cheats", "Monday Cheats", "Tuesday Cheats", "Wednesday Cheats", "Thursday Cheats", "Friday Cheats", "Saterday Cheats"};
// The day's cheat code, checked against the one typed on the command line.
unsigned cheats1[7] = {12, 14, 7, 6, 16, 16, 12};
unsigned long cheats2[7] = {0x2666919cL, 0x22494da3L, 0x4dc36a9eL, 0xa4dda78L, 0x7d5ac187L, 0x59e6db9cL, 0xb51ac973L};
unsigned long cheats3[7] = {0xbf16fa68L, 0x5506e5eeL, 0x616801fcL, 0xfce5b7d0L, 0xfa05f2a0L, 0x76db25b8L, 0xcbbcda52L};
// Where the avatar's paperdoll shows each equipment slot.
static char equipX[7] = {24, 36, 40, 40, 40, 16, 16};
static char equipY[7] = {60, 50, 26, 63, 92, 18, 18};
char *statsText[21] = {"STR", "INT", "DEX", "ARMR", "HITS", "MANA", "WGHT", "FORCE", "INTEL", "DEXT", "ARMR", "COUPS", "MAGIE", "POIDS", "KRAFT", "INTELL.", "GESCH.", "R\232ST.", "TREFF.", "MANA", "LAST"};
unsigned char ContainerGumpBox::targetModeHack = 0;
int ContainerGump::gumpCount = 0;
unsigned char containerGameCheat[1] = {0};
// Markers around the saved container gumps.
char contgumpNotice[32] = "CONTAINER GUMPS START HERE";
char dummyPtr1[16] = "CONTAINER GUMP";
char dummyPtr2[16] = "CONTAINER RECTS";
char dummyPtr3[16] = "GUMP COUNTER";
Rect ContainerGump::rects[16];

void closeAllGumps(void)
{
	NewGump *base = Dispatcher::base[0];
	ItemRelativeGump *gump = 0;
	while (base->newGumpList.hasType(0x43, (NewGump *&)gump))
		if (gump->f_52 == 0)
		{
			Item(gump->item).use();
			gump = 0;
		}
	base->refresh();
}

void ctrlF6Key(void)
{
}

// The area of a container gump that holds its items, or the avatar's paperdoll.
ContainerGumpBox::ContainerGumpBox(NewGump *parent, Rect r, unsigned short container) :
	NewGump(NewGumpId(0x90, 0), parent, 2),
	Container(container),
	event((EventType)1, 0, 0, 0, 0, 0, 0)
{
	rect = r;
	f_1d |= 0x1c0;
	dragging = 0;
	pressed = 0;
	dragItem.referent = 0;
	backpack.set(0, 0, 15, 24);
	backpack.moveto(44, 23);
}

void ContainerGumpBox::drag(unsigned short referent)
{
	Item item(referent);
	if (isAvatar())
	{
		editorItem.offset.set(0, 0);
		avatar.freeEquip(item.referent);
	}
	else
	{
		editorItem.offset.f_00 = getRelX(event) - item.getGumpX();
		editorItem.offset.f_02 = getRelY(event) - item.getGumpY();
	}
	editorItem.startDrag(item.referent);
	send(Event(EVENT_PRIVATE, getRelX(event) + get_dx(), getRelY(event) + get_dy(), event.data, this, parent->parent, 0));
	dragging = 1;
}

void ContainerGumpBox::commandMouseMovement(Event &event)
{
	commandNoEvents(event);
}

void ContainerGumpBox::commandMouseEscape(Event &event)
{
	if (event.type == 8)
		commandNoEvents(event);
	else if (event.type == 2 && event.isRelease() && pressed)
	{
		NewGump *top = (NewGump *)parent->parent->newGumpList.tail;
		Item(pressed).look();
		f_1d &= ~0x89;
		if ((NewGump *)parent->parent->newGumpList.tail != top)
		{
			top = (NewGump *)parent->parent->newGumpList.tail;
			parent->kidnap(top, parent);
		}
	}
	pressed = 0;
}

// A held button turns into a drag once the mouse moves.
void ContainerGumpBox::commandNoEvents(Event &)
{
	if (pressed)
	{
		drag(pressed);
		f_1d &= ~0x89;
		pressed = 0;
	}
}

void ContainerGumpBox::commandMouseLeft(Event &event)
{
	Item target;
	if (avatar.inStasis && !targetModeHack)
		return;
	if (event.isSingle())
	{
		if (editorItem.none)
		{
			if (target.findTarget(getRelX(event) + get_dx(), getRelY(event) + get_dy()) && target.getContainer() == referent)
			{
				pressed = target.referent;
				this->event = event;
				f_1d |= 0x89;
			}
			else
				send(Event((EventType)2, event.x + parent->rect.f_00, event.y + parent->rect.f_02, event.data, this, parent, 0));
		}
		else if (event.isShift())
		{
			if (target.findTarget(getRelX(event) + get_dx(), getRelY(event) + get_dy()))
				editorItem.set(target.getType(), target.getFrame());
		}
		else
			dragItem.referent = 0;
	}
	else if (event.isRelease())
	{
		if (pressed)
		{
			if (targetModeHack)
			{
				send(Event(EVENT_PRIVATE, getRelX(this->event) + get_dx(), getRelY(this->event) + get_dy(), this->event.data, this, parent->parent, 0));
				f_1d &= ~0x89;
				pressed = 0;
			}
			else
			{
				NewGump *top = (NewGump *)parent->parent->newGumpList.tail;
				Item(pressed).look();
				f_1d &= ~0x89;
				pressed = 0;
				if ((NewGump *)parent->parent->newGumpList.tail != top)
				{
					top = (NewGump *)parent->parent->newGumpList.tail;
					parent->kidnap(top, parent);
				}
			}
		}
		else if (editorItem.none)
		{
			editorItem.stopDrag();
			dragging = 0;
			if (U8MousePointer::modes[U8MousePointer::sp] == 0x28)
				U8MousePointer::popMode();
		}
	}
	else if (event.isDouble())
	{
		// Double clicking the backpack on the paperdoll opens it, making one if needed.
		if (isAvatar() && backpack.contains(event.x - rect.f_00, event.y - rect.f_02))
		{
			target = avatar.getEquip(6);
			if (!target.isValid())
			{
				target.create(529, 0);
				if (!target.isValid())
					halt(__FILE__, 530);	// __LINE__ in the original file
				target.pop(getX(), getY(), getZ());
				avatar.setEquip(6, target.referent);
				target.equip();
			}
			unsigned char saved = inGameMode;
			inGameMode = 1;
			target.use();
			inGameMode = saved;
			return;
		}
		if (target.findTarget(getRelX(event) + get_dx(), getRelY(event) + get_dy()))
		{
			unsigned char saved = inGameMode;
			inGameMode = 1;
			target.use();
			inGameMode = saved;
			return;
		}
		send(Event((EventType)2, event.x + get_dx(), event.y + get_dy(), event.data, this, parent, 0));
	}
	parent->parent->refresh();
}

void ContainerGumpBox::commandMouseBoth(Event &event)
{
	commandMouseLeft(event);
}

void ContainerGumpBox::commandBroadCast(Event &event)
{
	switch (event.data)
	{
	case 0x94:
		if (event.x == referent)
			parent->refresh();
	}
}

void ContainerGumpBox::draw(short x, short y)
{
	Item item;
	item = getContents();
	if (isAvatar())
	{
		for (int slot = 5; slot >= 0; slot--)
		{
			item = avatar.getEquip(slot);
			if (item.isValid())
			{
				item.setGumpXY(equipX[slot], equipY[slot]);
				SkipDraw(GlobalVport::global_ptr, x + equipX[slot], y + equipY[slot], shapeHandler->get(item.getType(), item.getFrame() | 1));
			}
		}
		item = avatar.getEquip(7);
		if (item.isValid())
			if (Item(avatar.getEquip(6)).isValid())
			{
				if (item.getStatus() & INVISIBLE)
				{
					SkipTransformDraw(GlobalVport::global_ptr, x + backpack.f_00 + 8, y + backpack.f_02 + 16, shapeHandler->get(item.getType(), item.getFrame()), theTransformPalette->table);
					return;
				}
				SkipDraw(GlobalVport::global_ptr, x + backpack.f_00 + 8, y + backpack.f_02 + 16, shapeHandler->get(item.getType(), item.getFrame()));
				return;
			}
	}
	else
	{
		int xOffset, yOffset, width, height;
		int gumpX, gumpY;
		Rect r;
		while (item.isValid())
		{
			if (GlobalTypes.typeFlags[item.getType()].editor && inGameMode)
			{
				item = item.getNext();
				continue;
			}
			if (!item.getGumpX() && !item.getGumpY())
				item.setGumpXY(urandom(rect.width()) + 1, urandom(rect.height()) + 1);
			gumpX = item.getGumpX();
			gumpY = item.getGumpY();
			if (gumpY >= 0xc0)
				gumpY = (char)gumpY;
			if (gumpX >= 0xc0)
				gumpX = (char)gumpX;
			shapeHandler->getDim(item.getType(), item.getFrame(), width, height, xOffset, yOffset);
			r.f_00 = gumpX - xOffset + rect.f_00;
			r.f_04 = r.f_00 + width;
			r.f_02 = gumpY - yOffset + rect.f_02;
			r.f_06 = r.f_02 + height;
			if (rect.f_00 > r.f_00)
				r.moveto(rect.f_00 + 1, r.f_02);
			if (rect.f_02 > r.f_02)
				r.moveto(r.f_00, rect.f_02 + 1);
			if (rect.f_04 - width < r.f_00)
				r.moveto(rect.f_04 - width, r.f_02);
			if (rect.f_06 - height < r.f_02)
				r.moveto(r.f_00, rect.f_06 - height);
			gumpX = r.f_00 + xOffset - rect.f_00;
			gumpY = r.f_02 + yOffset - rect.f_02;
			item.setGumpXY(gumpX, gumpY);
			if (item.getStatus() & INVISIBLE)
				SkipTransformDraw(GlobalVport::global_ptr, gumpX + x, gumpY + y, shapeHandler->get(item.getType(), item.getFrame()), theTransformPalette->table);
			else
				SkipDraw(GlobalVport::global_ptr, gumpX + x, gumpY + y, shapeHandler->get(item.getType(), item.getFrame()));
			item = item.getNext();
		}
	}
}

// Puts an item dropped on the paperdoll into the slot its type wears.
Boolean ContainerGumpBox::findEquipSlot(unsigned short referent)
{
	Item item(referent);
	if (!isAvatar())
		return 0;
	if (!item.isValid())
		return 0;
	TypeFlag flags = GlobalTypes.typeFlags[item.getType()];
	if (flags.equipType == 0)
	{
		if (!avatar.setEquip(7, item.referent))
		{
			if (item.isValid())
			{
				item.push();
				item.pop();
			}
			return 0;
		}
		item.equip();
		parent->refresh();
		return 1;
	}
	for (int i = 0; i < 6; i++)
		if (i + 1 == flags.equipType)
		{
			if (!avatar.setEquip(i, item.referent))
			{
				if (!avatar.setEquip(7, item.referent))
				{
					if (item.isValid())
					{
						item.push();
						item.pop();
					}
					return 0;
				}
				item.equip();
				parent->refresh();
				return 1;
			}
			item.equip();
			parent->refresh();
			return 1;
		}
	return 0;
}

int ContainerGumpBox::findTarget(short x, short y)
{
	Referent found = 0;
	int frameAdd = 0;
	int dx = get_dx();
	int dy = get_dy();
	Item item = getContents();
	if (isAvatar())
		frameAdd = 1;
	while (item.isValid())
	{
		if (dragItem.referent != item.referent)
		{
			char *shape = shapeHandler->get(item.getType(), item.getFrame() + frameAdd);
			if (shape && Collision(shape, Point(dx + item.getGumpX(), dy + item.getGumpY()), Point(x, y)))
				found = item.referent;
		}
		item = item.getNext();
	}
	return found;
}

ContainerGump::ContainerGump(NewGump *parent, unsigned short container, int shape) :
	ItemRelativeGump((IRGumpType)0, parent, Rect(0, 0, 0, 0), container, (IRGumpMove)1, 0, 0, 0),
	Container(container),
	GumpShape(shape)
{
	containerItem = container;
	itemRect = U8GumpShapeDrawer::rects[shape - 1];
	f_1d |= 0x100;
	if (containerItem.isAvatar())
	{
		moveType = (IRGumpMove)3;
		ShapePushButton *button = new ShapePushButton(0x26, this, 0x26);
		button->rect.moveto(81, 84);
	}
	rect = get_rect();
	index = gumpCount;
	unsigned short worldX = containerItem.getX();
	unsigned short worldY = containerItem.getY();
	short screenX, screenY;
	WorldToScreenCoords(worldX, worldY, screenX, screenY);
	screenY -= containerItem.getZ();
	if (containerItem.isAvatar())
		moveto(avatar.f_3c.f_00, avatar.f_3c.f_02);
	else
		positionGump(container, screenX, screenY);
	dx = rect.f_00 - screenX;
	dy = rect.f_02 - screenY;
	dragOffset.set(dx, dy);
	dragStart.set(rect.f_00, rect.f_02);
	gumpCount++;
	Item(container).orStatus(CONTAINER_GUMP_OPEN);
	new ContainerGumpBox(this, itemRect, container);
	if (gumpCount >= 16)
	{
		unsigned char saved = inGameMode;
		inGameMode = 1;
		Item item = containerItem;
		item.use();
		inGameMode = saved;
	}
	else
	{
		refresh();
		if (containerItem.isAvatar())
		{
			avatar.f_3c.f_00 = rect.f_00;
			avatar.f_3c.f_02 = rect.f_02;
		}
	}
}

ContainerGump::~ContainerGump(void)
{
	((Container *)this)->andStatus(~CONTAINER_GUMP_OPEN);
	gumpCount--;
	if (gumpCount)
		for (int i = index; i < gumpCount; i++)
			rects[i] = rects[i + 1];
}

void ContainerGump::commandPrivate(Event &)
{
	if (!inGameMode)
		return;
	Item item(referent);
	if (item.isNpc() && !theStatGump)
		theStatGump = new AvatarStatusGump(parent, referent);
}

void ContainerGump::commandMouseBoth(Event &event)
{
	commandMouseLeft(event);
}

unsigned char ContainerGump::collide(Event &event)
{
	if (containerItem.isAvatar())
		return contains(event);
	return GumpShape::collide(getRelX(event), getRelY(event));
}

void ContainerGump::commandMouseLeft(Event &event)
{
	int code1 = 8;
	unsigned long code2 = 0x276afae3L;
	unsigned long code3 = 0x65d5d4a0L;
	Boolean cheat = 0;
	if (event.isSingle())
	{
		dragOffset.set(dx, dy);
		dragStart.set(rect.f_00, rect.f_02);
		if (!GumpShape::collide(getRelX(event), getRelY(event)))
		{
			parent->commandMouseLeft(event);
			return;
		}
	}
	rects[index] = rect;
	if (containerItem.isAvatar())
	{
		avatar.f_3c.f_00 = rect.f_00;
		avatar.f_3c.f_02 = rect.f_02;
	}
	if (event.isDouble())
	{
		unsigned char day;
		// Ctrl-Alt-Shift double click on the paperdoll checks the cheat code.
		if (containerItem.isAvatar())
		{
			if (event.isAlt() && event.isShift() && event.isCtrl())
			{
				asm {
					push ax
					push cx
					push dx
					mov ax, 0x2a00
					int 0x21
					mov day, al
					pop dx
					pop cx
					pop ax
				}
				if (cheats1[day] == cheatcheck1 && cheats2[day] == cheatcheck2 && cheats3[day] == cheatcheck3)
					cheat = 1;
				else if (cheatcheck1 == code1 && cheatcheck2 == code2 && cheatcheck3 == code3)
					cheat = 1;
				else
					cheat = 0;
				if (cheat == 0)
					return;
				avatar.canCheat = 1;
				playSFX(217);
				Dispatch(new MessageGump(NewGumpId(0), 0, "Cheats are F7!", GumpColorMap[5]));
				return;
			}
		}
		Item container = containerItem.referent;
		unsigned char saved = inGameMode;
		inGameMode = 1;
		Item item = containerItem;
		item.use();
		inGameMode = saved;
		return;
	}
	if (!(dragging && !event.isRelease()) && !(!event.isRelease()))
		if (event.isRelease())
		{
			if (U8MousePointer::modes[U8MousePointer::sp] == 0x28)
				U8MousePointer::popMode();
			dx = dragOffset.f_00 + (rect.f_00 - dragStart.f_00);
			dy = dragOffset.f_02 + (rect.f_02 - dragStart.f_02);
		}
	PanelGump::commandMouseLeft(event);
	parent->refresh();
}

void ContainerGump::draw(short x, short y)
{
	if (containerItem.isAvatar())
	{
		avatar.f_3c.f_00 = rect.f_00;
		avatar.f_3c.f_02 = rect.f_02;
	}
	GumpShape::draw(x, y);
	if (isAvatar())
	{
		char str = avatar.getStr();
		char dex = avatar.getDex();
		char intel = avatar.getInt();
		unsigned char hp = avatar.getHp();
		int mana = avatar.getMana();
		int weight = (Container(avatar.referent).getContentsWeight() + 9) / 10;
		int armor = avatar.getArmorClass();
		CacheFont labels((FontType)0);
		CacheFont numbers((FontType)12);
		CacheFont low((FontType)13);
		char lang = avatar.language;
		labels.printf(x + 120 - labels.width(statsText[lang * 7]), y + 24, statsText[lang * 7]);
		labels.printf(x + 120 - labels.width(statsText[lang * 7 + 1]), y + 33, statsText[lang * 7 + 1]);
		labels.printf(x + 120 - labels.width(statsText[lang * 7 + 2]), y + 42, statsText[lang * 7 + 2]);
		labels.printf(x + 120 - labels.width(statsText[lang * 7 + 3]), y + 51, statsText[lang * 7 + 3]);
		labels.printf(x + 120 - labels.width(statsText[lang * 7 + 4]), y + 60, statsText[lang * 7 + 4]);
		labels.printf(x + 120 - labels.width(statsText[lang * 7 + 5]), y + 69, statsText[lang * 7 + 5]);
		labels.printf(x + 120 - labels.width(statsText[lang * 7 + 6]), y + 78, statsText[lang * 7 + 6]);
		numbers.printf(x + 124, y + 24, "%3d", str);
		numbers.printf(x + 124, y + 33, "%3d", intel);
		numbers.printf(x + 124, y + 42, "%3d", dex);
		numbers.printf(x + 124, y + 51, "%3d", armor);
		if (hp > 10)
			numbers.printf(x + 124, y + 60, "%3d", hp);
		else
			low.printf(x + 124, y + 60, "%3d", hp);
		if (mana > 10)
			numbers.printf(x + 124, y + 69, "%3d", mana);
		else
			low.printf(x + 124, y + 69, "%3d", mana);
		numbers.printf(x + 124, y + 78, "%3d", weight);
	}
}

// Finds the item under a screen point in any open container gump.
unsigned char ContainerGump::findTarget(short x, short y, unsigned short &found)
{
	found = 0;
	Event event((EventType)2, x, y, 0, 0, 0, 0);
	NewGump *gump = Dispatcher::base[Dispatcher::baseSP]->findTargetedGump(event);
	if (gump)
	{
		if (gump->newGumpId.getType() == 0x90)
		{
			found = ((ContainerGumpBox *)gump)->findTarget(x, y);
			return 1;
		}
		if (gump->newGumpId.getType() == 0x43)
		{
			ItemRelativeGump *irGump = (ItemRelativeGump *)gump;
			if (irGump->f_52 == 0)
			{
				if (((ContainerGump *)gump)->GumpShape::collide(x - irGump->get_dx(), y - irGump->get_dy()))
				{
					found = irGump->item;
					return 1;
				}
			}
		}
	}
	return 0;
}

// Places a new gump near the container on screen, avoiding the other gumps.
void ContainerGump::positionGump(short referent, short x, short y)
{
	Rect avoid;
	Item item(referent);
	int left = x - 16;
	int top = y - 16;
	int width = rect.width();
	int height = rect.height();
	int maxX = 320 - width;
	int maxY = 200 - height;
	int bestX = 0;
	int bestY = 0;
	unsigned best = 0xffff;
	avoid.set(left, top, x, y);
	for (int gx = 0; gx < maxX; gx += 8)
		for (int gy = 0; gy < maxY; gy += 8)
		{
			unsigned score = abs(left - (gx + (width >> 1)));
			score += abs(top - (gy + (height >> 1)));
			rect.moveto(gx, gy);
			Rect *other = rects;
			for (int i = 0; i < gumpCount; i++, other++)
				if (rect.intersects(*other))
					score += 500;
			if (rect.intersects(avoid))
				score += 1000;
			if (score < best)
			{
				best = score;
				bestX = gx;
				bestY = gy;
			}
		}
	rect.moveto(bestX, bestY);
	rects[index] = rect;
}

void ContainerGump::load(BaseFile *file)
{
	int marker = 0;
	int ref;
	unsigned short container;
	Rect r;
	int shape, frame;
	Point offset, start;
	WorldPoint loc;
	NewGump *gump = 0;
	NewGump *base = Dispatcher::base[0];
	while (base->newGumpList.hasType(0x43, gump))
		if (((ItemRelativeGump *)gump)->f_52 == 0)
		{
			gump->killMyself();
			gump = 0;
		}
	if (!file)
		return;
	gump = 0;
	file->read(contgumpNotice, 32);
	while (marker != -1)
	{
		file->read((char *)&ref, 2);
		marker = ref;
		if (ref == -1)
			continue;
		file->read(dummyPtr1, 16);
		file->read(txtref, 4);
		file->read((char *)&container, 2);
		file->read(txtrect, 5);
		file->read((char *)&r, 8);
		file->read((char *)&offset, 4);
		file->read((char *)&start, 4);
		file->read((char *)&loc, 5);
		file->read(txtshape, 6);
		file->read((char *)&shape, 2);
		file->read(txtframe, 6);
		file->read((char *)&frame, 2);
		ContainerGump *containerGump = new ContainerGump(base, container, shape);
		containerGump->loc = loc;
		containerGump->rect = r;
		containerGump->set_frame(frame);
		containerGump->dragOffset = offset;
		containerGump->dragStart = start;
	}
	file->read(dummyPtr2, 16);
	file->read((char *)rects, sizeof(rects));
	file->read(dummyPtr3, 16);
	file->read((char *)&gumpCount, 2);
}

void ContainerGump::save(BaseFile *file)
{
	int value = -1;
	NewGump *gump = 0;
	NewGump *base = Dispatcher::base[0];
	file->write(contgumpNotice, 32);
	while (base->newGumpList.hasType(0x43, gump))
	{
		if (((ContainerGump *)gump)->f_52 != 0)
			continue;
		file->write((char *)&((ContainerGump *)gump)->f_52, 2);
		file->write(dummyPtr1, 16);
		file->write(txtref, 4);
		file->write((char *)&((ContainerGump *)gump)->referent, 2);
		file->write(txtrect, 5);
		file->write((char *)&((ContainerGump *)gump)->rect, 8);
		file->write((char *)&((ContainerGump *)gump)->dragOffset, 4);
		file->write((char *)&((ContainerGump *)gump)->dragStart, 4);
		file->write((char *)&((ContainerGump *)gump)->loc, 5);
		file->write(txtshape, 6);
		value = ((ContainerGump *)gump)->shape;
		file->write((char *)&value, 2);
		file->write(txtframe, 6);
		value = ((ContainerGump *)gump)->_frame;
		file->write((char *)&value, 2);
	}
	value = -1;
	file->write((char *)&value, 2);
	file->write(dummyPtr2, 16);
	file->write((char *)rects, sizeof(rects));
	file->write(dummyPtr3, 16);
	file->write((char *)&gumpCount, 2);
}

// Labels written between the fields of each saved gump.
char *txtref = "REF=";
char *txtrect = "RECT=";
char *txtshape = "SHAPE=";
char *txtframe = "FRAME=";
