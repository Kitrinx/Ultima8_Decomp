// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: STATGUMP.C

#include "..\UI\NEWGUMP.H"
#include "CRECT.H"
#include "CVPORT.H"
#include "CGLOBVP.H"
#include "DISPATCH.H"
#include "ITEM.H"
#include "NPC.H"
#include "CAMERA.H"
#include "CEXIT.H"
#include "U8GMPSHP.H"
#include "CONTGUMP.H"
#include "STATGUMP.H"

// Inline in the shared headers when this file was built.
inline Point::Point(void) {}
inline void Point::set(short x, short y) { f_00 = x; f_02 = y; }
inline void Rect::set(short x1, short y1, short x2, short y2) { Point::set(x1, y1); f_04 = x2; f_06 = y2; }
inline Rect::Rect(short x1, short y1, short x2, short y2) { set(x1, y1, x2, y2); }
inline void NewGump::moveto(short x, short y) { rect.moveto(x, y); }
inline GumpShape::GumpShape(int shape) { _frame = 0; this->shape = shape; }
inline Rect GumpShape::get_rect(void) { return drawer->get_rect(shape, _frame); }
inline void GumpShape::draw(int x, int y) { drawer->draw(x, y, shape, _frame); }

void closeStatusGump(void)
{
	if (theStatGump)
	{
		theStatGump->killMyself();
		theStatGump = 0;
	}
}

void openStatusGump(void)
{
	if (!theStatGump)
	{
		new AvatarStatusGump(Dispatcher::base[0], avatar.referent);
		if (!theStatGump)
			outOfMemory(__FILE__, 29);	// __LINE__ in the original file
	}
}

// Where a drag started: the gump's offset from the avatar and its screen position.
static Point dragOffset;
static Point dragStart;

AvatarStatusGump::AvatarStatusGump(NewGump *owner, unsigned short referent) :
	ItemRelativeGump((IRGumpType)5, owner, Rect(0, 0, 0, 0), referent, (IRGumpMove)3, 0, 0, 0),
	GumpShape(33)
{
	f_1d |= 2;
	rect = get_rect();
	moveto(avatar.f_40.f_00, avatar.f_40.f_02);
	if (!parent->rect.contains(rect.f_00, rect.f_02))
		rect.moveto(4, 175);
	Item item(referent);
	unsigned short x = item.getX();
	unsigned short y = item.getY();
	short screenX, screenY;
	WorldToScreenCoords(x, y, screenX, screenY);
	dx = rect.f_00 - screenX;
	dy = rect.f_02 - screenY;
	dragOffset.set(dx, dy);
	dragStart.set(rect.f_00, rect.f_02);
	refresh();
	theStatGump = this;
}

AvatarStatusGump::~AvatarStatusGump(void)
{
	theStatGump = 0;
}

void AvatarStatusGump::commandMouseLeft(Event &event)
{
	if (event.isSingle())
	{
		dragOffset.set(dx, dy);
		dragStart.set(rect.f_00, rect.f_02);
	}
	if (event.isDouble())
	{
		killMyself();
		return;
	}
	dx = dragOffset.f_00 + (rect.f_00 - dragStart.f_00);
	dy = dragOffset.f_02 + (rect.f_02 - dragStart.f_02);
	PanelGump::commandMouseLeft(event);
	avatar.f_40.f_00 = rect.f_00;
	avatar.f_40.f_02 = rect.f_02;
}

// Health and mana bars, three pixels wide, filled from the bottom.
void AvatarStatusGump::draw(short x, short y)
{
	GumpShape::draw(x, y);
	Npc npc(item);
	long len = npc.getHp() * 14 / (npc.getStr() * 2);
	len--;
	if (len >= 14)
		len = 13;
	if (len >= 0)
	{
		DrawLine(GlobalVport::global_ptr, x + 6, rect.f_06 - rect.f_02 + y - 6, x + 6, y + (rect.f_06 - rect.f_02 - 6 - len), 41);
		DrawLine(GlobalVport::global_ptr, x + 7, rect.f_06 - rect.f_02 + y - 6, x + 7, y + (rect.f_06 - rect.f_02 - 6 - len), 39);
		DrawLine(GlobalVport::global_ptr, x + 8, rect.f_06 - rect.f_02 + y - 6, x + 8, y + (rect.f_06 - rect.f_02 - 6 - len), 37);
	}
	len = npc.getMana() * 14 / (npc.getInt() * 2);
	len--;
	if (len >= 14)
		len = 13;
	if (len >= 0)
	{
		DrawLine(GlobalVport::global_ptr, x + 13, rect.f_06 - rect.f_02 + y - 6, x + 13, y + (rect.f_06 - rect.f_02 - 6 - len), 138);
		DrawLine(GlobalVport::global_ptr, x + 14, rect.f_06 - rect.f_02 + y - 6, x + 14, y + (rect.f_06 - rect.f_02 - 6 - len), 139);
		DrawLine(GlobalVport::global_ptr, x + 15, rect.f_06 - rect.f_02 + y - 6, x + 15, y + (rect.f_06 - rect.f_02 - 6 - len), 141);
	}
}
