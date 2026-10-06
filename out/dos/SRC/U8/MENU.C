// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -vi-
// name: MENU.C

#include "..\UI\NEWGUMP.H"
#include "CRECT.H"
#include "CVPORT.H"
#include "CGLOBVP.H"
#include "FONT.H"
#include "DISPATCH.H"
#include "MENU.H"
#include "SBUTTON.H"

// Inline in the shared headers when this file was built.
inline int NewGumpId::get(void) { return instance; }
inline char MenuGump::getColor(void) { return color; }
inline char *PureMenuItemGump::getText(void) { return text; }
inline void PureMenuItemGump::setText(char *newText) { text = newText; }
inline MenuGump::MenuGump(NewGump *parent, unsigned char c) :
	NewGump(NewGumpId(0), parent, 0x10)
{
	color = c;
}
inline void MenuGump::restore(void) { NewGump::restore(); }

MenuItemGump &operator+(MenuItemGump &item, PullDownGump &pullDown)
{
	item.addPullDown(&pullDown);
	return item;
}

PullDownGump &operator+(MenuItemGump &first, MenuItemGump &second)
{
	PullDownGump *pullDown = new PullDownGump;
	pullDown->addMenuItem(&first);
	pullDown->addMenuItem(&second);
	return *pullDown;
}

PullDownGump &operator+(PullDownGump &pullDown, MenuItemGump &item)
{
	pullDown.addMenuItem(&item);
	return pullDown;
}

MenuBarGump &operator+(MenuBarGump &bar, MenuItemGump &item)
{
	bar.addMenuItem(&item);
	return bar;
}

PullDownGump &operator+(PullDownGump &pullDown, MenuLineGump &line)
{
	pullDown.addMenuItem(&line);
	return pullDown;
}

PullDownGump &operator+(MenuItemGump &item, MenuLineGump &line)
{
	PullDownGump *pullDown = new PullDownGump;
	pullDown->addMenuItem(&item);
	pullDown->addMenuItem(&line);
	return *pullDown;
}

PureMenuItemGump::PureMenuItemGump(char *itemText, int id, unsigned char enabled) :
	NewGump(NewGumpId(id), 0, 0x12)
{
	f_3c = enabled;
	f_3d = 0;
	text = itemText;
}

void PureMenuItemGump::commandPrivate(Event &event)
{
	switch (event.data)
	{
	case 1:
		event.to = parent;
		send(event);
		break;
	case 2:
		event.from->pushVisibility(3);
		((MenuGump *)event.from)->clearSelect();
		((MenuGump *)event.from)->setMovement(0);
		Rect r = event.from->rect;
		r.moveto(event.from->get_dx(), event.from->get_dy());
		restore(r);
	}
}

void PureMenuItemGump::commandMouseLeft(Event &event)
{
	if (event.isSingle())
	{
		setExclusive(this);
		commandMouseMovement(event);
		return;
	}
	if (event.isRelease())
	{
		int picked = -1;
		if (newGumpList.getHead())
			send(Event(EVENT_PRIVATE, 0, 0, 2, newGumpList.getHead(), this, 0));
		if (contains(event) && f_3d)
			picked = newGumpId.get();
		send(Event(EVENT_PRIVATE, picked, 0, 1, this, parent, 0));
		clrExclusive();
	}
}

void PureMenuItemGump::commandMouseMovement(Event &event)
{
	NewGump *pullDown = newGumpList.getHead();
	NewGump *target = 0;
	Event local;
	if (contains(event))
	{
		if (pullDown && pullDown->visibilityStack)
		{
			pullDown->popVisibility();
			((MenuGump *)pullDown)->setMovement(1);
			refresh();
		}
		if (f_3d)
			return;
		((MenuGump *)parent)->clearSelect();
		if (!f_3c)
			return;
		if (!f_3d)
		{
			f_3d = 1;
			((MenuGump *)parent)->setMovement(1);
			parent->newGumpList.moveToTail(this);
			parent->refresh();
		}
		return;
	}
	local = event;
	local.x -= rect.f_00;
	local.y -= rect.f_02;
	if (pullDown && pullDown->contains(local))
	{
		target = pullDown->newGumpList.getHead();
		if (target)
		{
			local.x -= pullDown->rect.f_00;
			local.y -= pullDown->rect.f_02;
			if (pullDown->visibilityStack)
			{
				pullDown->popVisibility();
				((MenuGump *)pullDown)->setMovement(1);
				refresh();
			}
			setExclusive(target);
			target->command(local);
			return;
		}
	}
	// Climb item and pull-down pairs up to the menu bar.
	NewGump *bar;
	for (bar = parent; bar && bar->newGumpId.getType() != 3; bar = bar->parent->parent)
		;
	if (!bar || !bar->parent)
		return;
	NewGump *menu = parent;
	for (;;)
	{
		local = event;
		local.x += parent->get_dx();
		local.y += parent->get_dy();
		local.x -= menu->parent->get_dx();
		local.y -= menu->parent->get_dy();
		unsigned char cares = menu->caresAbout(local, target, 0);
		if (cares && target != &nullGump)
		{
			NewGump *open = parent;
			if (pullDown && !pullDown->visibilityStack)
				open = pullDown;
			while (open != bar && target->parent != open)
			{
				if (!open->visibilityStack)
				{
					open->pushVisibility(3);
					((MenuGump *)open)->clearSelect();
					((MenuGump *)open)->setMovement(0);
					open->restore();
				}
				open = open->parent->parent;
			}
			setExclusive(target);
			target->command(local);
			return;
		}
		if (menu == bar)
			return;
		menu = menu->parent->parent;
	}
}

void PureMenuItemGump::addPullDown(PullDownGump *pullDown)
{
	if (newGumpList.getHead())
		return;
	pullDown->parent = this;
	pullDown->pushVisibility(3);
	pullDown->setMovement(0);
	attach(pullDown);
	if (parent && parent->newGumpId.get() == 3)
		pullDown->moveto(0, rect.height() - 1);
	else
		pullDown->moveto(rect.width() - f_38, 1);
}

MenuItemGump::MenuItemGump(char *text, int id, unsigned char enabled) :
	PureMenuItemGump(text, id, enabled)
{
	int height;
	int width;
	getDim(width, height);
	rect = Rect(0, 0, width - 1, height - 1);
	f_38 = 8;
	f_3a = 8;
}

void MenuItemGump::draw(short x, short y)
{
	if (!f_3c)
	{
		unsigned char shade = ((MenuGump *)parent)->getColor() + 2;
		DrawBox(GlobalVport::global_ptr, x, y, x + rect.width() - 1, y + rect.height() - 1, shade);
	}
	else if (f_3d)
		DrawBox(GlobalVport::global_ptr, x, y, x + rect.width() - 1, y + rect.height() - 1, GumpColorMap[8]);
	// An arrow on items that open a pull-down off a pull-down.
	if (newGumpList.getHead() && !parent->newGumpId.get())
	{
		int right = x + rect.width() - 1;
		int middle = y + (rect.height() >> 1);
		for (int i = 3; i >= 0; i--)
			DrawLine(GlobalVport::global_ptr, right - i, middle - i, right - i, middle + i, GumpColorMap[0]);
	}
	GlobalFont::global_ptr->printf(x + f_3a - 1, y + rect.height() - 1, text);
}

void MenuItemGump::getDim(int &width, int &height)
{
	GlobalFont::global_ptr->Font::dim(text, width, height);
}

void CheckMenuItemGump::commandMouseLeft(Event &event)
{
	if (!f_3c)
		return;
	if (event.isSingle())
		PureMenuItemGump::commandMouseLeft(event);
	else if (event.isRelease())
	{
		if (contains(event) && f_3d)
			*checked = !*checked;
		PureMenuItemGump::commandMouseLeft(event);
	}
	f_38 = 8;
	f_3a = 8;
}

void CheckMenuItemGump::draw(short x, short y)
{
	MenuItemGump::draw(x, y);
	if (*checked)
	{
		Point a;
		Point b;
		Point c;
		a.f_00 = x + 1;
		a.f_02 = y + (rect.height() >> 1);
		b.f_00 = a.f_00 + 2;
		b.f_02 = a.f_02 + 2;
		c.f_00 = b.f_00 + 3;
		c.f_02 = b.f_02 - 5;
		DrawLine(GlobalVport::global_ptr, a.f_00, a.f_02, b.f_00, b.f_02, GumpColorMap[0]);
		DrawLine(GlobalVport::global_ptr, b.f_00, b.f_02, c.f_00, c.f_02, GumpColorMap[0]);
	}
}

void SelectMenuItemGump::commandPrivate(Event &event)
{
	PureMenuItemGump::commandPrivate(event);
	if (event.data == 1)
	{
		PureMenuItemGump *item = (PureMenuItemGump *)((MenuGump *)newGumpList.getHead())->findMenuItem(NewGumpId(event.x));
		if (item)
			setText(item->getText());
	}
}

void MenuGump::draw(short x, short y)
{
	DrawRectangle(GlobalVport::global_ptr, x, y, x + rect.width() - 1, y + rect.height() - 1, GumpColorMap[0]);
	DrawBox(GlobalVport::global_ptr, x + 1, y + 1, x + rect.width() - 2, y + rect.height() - 2, color);
}

void MenuGump::clearSelect(void)
{
	PureMenuItemGump *item = 0;
	while (newGumpList.traverse((DoubleLink *&)item))
	{
		if (item->f_3d)
		{
			MenuGump *pullDown = (MenuGump *)item->newGumpList.getHead();
			if (pullDown && !pullDown->visibilityStack)
			{
				pullDown->pushVisibility(3);
				pullDown->clearSelect();
				pullDown->setMovement(0);
			}
			item->f_3d = 0;
			refresh();
			return;
		}
	}
}

void MenuGump::setMovement(unsigned char moving)
{
	NewGump *kid = 0;
	while (newGumpList.traverse((DoubleLink *&)kid))
	{
		if (moving)
			kid->f_1d |= 8;
		else
			kid->f_1d &= ~8;
	}
}

NewGump *MenuGump::findMenuItem(NewGumpId id)
{
	NewGump *kid = 0;
	while (newGumpList.traverse((DoubleLink *&)kid))
	{
		if (kid->newGumpId.get() == id.get())
			return kid;
		MenuGump *pullDown = (MenuGump *)kid->newGumpList.getHead();
		if (pullDown)
		{
			NewGump *found = pullDown->findMenuItem(id);
			if (found)
				return found;
		}
	}
	return 0;
}

void MenuGump::setColor(unsigned char c)
{
	NewGump *kid = 0;
	while (newGumpList.traverse((DoubleLink *&)kid))
	{
		MenuGump *pullDown = (MenuGump *)kid->newGumpList.getHead();
		if (pullDown)
			pullDown->setColor(c);
	}
	color = c;
}

PullDownGump::PullDownGump(void) :
	MenuGump(0, GumpColorMap[5])
{
	rect = Rect(0, 0, 0, 1);
	rightMargin = 8;
	leftMargin = 8;
	f_2f = 0;
}

void PullDownGump::addMenuItem(PureMenuItemGump *item)
{
	if (!item)
		return;
	int height;
	int width;
	item->getDim(width, height);
	width += rightMargin + leftMargin;
	item->parent = this;
	item->rect = Rect(0, 0, width - 1, height - 1);
	item->moveto(1, rect.height() - 1);
	rect.f_06 += height;
	if (rect.width() - 2 < item->rect.width())
	{
		int grow = item->rect.width() - (rect.width() - 2);
		rect.f_04 += grow;
		NewGump *kid = 0;
		while (newGumpList.traverse((DoubleLink *&)kid))
		{
			kid->rect.f_04 = item->rect.f_04;
			NewGump *pullDown = kid->newGumpList.getHead();
			if (pullDown)
				pullDown->moveto(kid->rect.width() - rightMargin, 1);
		}
	}
	else
		item->rect.f_04 = rect.width() - 2;
	attach(item);
	NewGump *pullDown = item->newGumpList.getHead();
	if (pullDown)
		pullDown->moveto(item->rect.width() - rightMargin, 1);
}

void PullDownGump::commandPrivate(Event &event)
{
	switch (event.data)
	{
	case 1:
		event.to = parent;
		send(event);
		send(Event(EVENT_PRIVATE, 0, 0, 2, this, parent, 0));
	}
}

MenuBarGump::MenuBarGump(unsigned char id, NewGump *parent) :
	MenuGump(parent, GumpColorMap[5])
{
	newGumpId = NewGumpId(3, id);
	rect = Rect(0, 0, 1, 1);
	rightMargin = 8;
	leftMargin = 8;
}

void MenuBarGump::addMenuItem(PureMenuItemGump *item)
{
	if (!item)
		return;
	int height;
	int width;
	GlobalFont::global_ptr->Font::dim(item->getText(), width, height);
	width += rightMargin + leftMargin;
	item->parent = this;
	item->rect = Rect(0, 0, width - 1, height - 1);
	item->moveto(rect.width() - 1, 1);
	rect.f_04 += item->rect.width();
	if (rect.height() - 2 < item->rect.height())
	{
		int grow = item->rect.height() - (rect.height() - 2);
		rect.f_06 += grow;
		NewGump *kid = 0;
		while (newGumpList.traverse((DoubleLink *&)kid))
		{
			kid->rect.f_06 = item->rect.f_06;
			NewGump *pullDown = kid->newGumpList.getHead();
			if (pullDown)
				pullDown->moveto(0, kid->rect.height() - 1);
		}
	}
	else
		item->rect.f_06 = rect.height() - 2;
	attach(item);
	NewGump *pullDown = item->newGumpList.getHead();
	if (pullDown)
		pullDown->moveto(0, item->rect.height() - 1);
}

void MenuBarGump::commandPrivate(Event &event)
{
	switch (event.data)
	{
	case 1:
		event.to = parent;
		event.from = this;
		event.data = event.x;
		send(event);
		clearSelect();
		setMovement(0);
	}
}

void MenuLineGump::draw(short x, short y)
{
	Rect r = rect;
	r.moveto(x, y);
	DrawLine(GlobalVport::global_ptr, r.f_00, r.f_02, r.f_04, r.f_06, 0);
}

void MenuLineGump::getDim(int &width, int &height)
{
	width = 0;
	height = 1;
}
