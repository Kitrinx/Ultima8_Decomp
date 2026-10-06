#ifndef MENU_H
#define MENU_H

#include "DISPATCH.H"

class Event;
class MenuBarGump;
class MenuGump;
class MenuItemGump;
class MenuLineGump;
class NewGumpId;
class PullDownGump;
class PureMenuItemGump;

// One entry of a menu bar or pull-down. A pull-down attached to an item is its only kid.
class PureMenuItemGump : public NewGump
{
public:
	char *text;
	unsigned f_38;			// right margin
	unsigned f_3a;			// left margin
	unsigned char f_3c;		// enabled
	unsigned char f_3d;		// selected

	PureMenuItemGump(char *, int, unsigned char);
	virtual void commandMouseLeft(Event &);
	virtual void commandMouseMovement(Event &);
	virtual void commandPrivate(Event &);
	virtual void draw(short, short) = 0;
	virtual void getDim(int &, int &) = 0;
	virtual void addPullDown(PullDownGump *);
	char *getText(void);
	void setText(char *);
};

class MenuItemGump : public PureMenuItemGump
{
public:
	MenuItemGump(char *, int, unsigned char);
	virtual void draw(short, short);
	virtual void getDim(int &, int &);
};

class CheckMenuItemGump : public MenuItemGump
{
public:
	unsigned char *checked;

	virtual void commandMouseLeft(Event &);
	virtual void draw(short, short);
};

// Takes the text of the item picked from its pull-down.
class SelectMenuItemGump : public MenuItemGump
{
public:
	virtual void commandPrivate(Event &);
};

class MenuLineGump : public PureMenuItemGump
{
public:
	virtual void draw(short, short);
	virtual void getDim(int &, int &);
};

class MenuGump : public NewGump
{
public:
	int rightMargin;
	int leftMargin;
	unsigned char color;

	MenuGump(NewGump *, unsigned char);
	virtual void commandPrivate(Event &) = 0;
	virtual void draw(short, short);
	virtual void clearSelect(void);
	virtual void setMovement(unsigned char);
	virtual void addMenuItem(PureMenuItemGump *) = 0;
	virtual void restore(void);
	NewGump *findMenuItem(NewGumpId);
	void setColor(unsigned char);
	char getColor(void);
};

class PullDownGump : public MenuGump
{
public:
	PullDownGump(void);
	virtual void commandPrivate(Event &);
	virtual void addMenuItem(PureMenuItemGump *);
};

class MenuBarGump : public MenuGump
{
public:
	MenuBarGump(unsigned char, NewGump *);
	virtual void commandPrivate(Event &);
	virtual void addMenuItem(PureMenuItemGump *);
};

MenuItemGump &operator+(MenuItemGump &, PullDownGump &);
PullDownGump &operator+(MenuItemGump &, MenuItemGump &);
PullDownGump &operator+(PullDownGump &, MenuItemGump &);
MenuBarGump &operator+(MenuBarGump &, MenuItemGump &);
PullDownGump &operator+(PullDownGump &, MenuLineGump &);
PullDownGump &operator+(MenuItemGump &, MenuLineGump &);

extern unsigned char defaultGumpColorMap[32];
extern unsigned char *GumpColorMap;

#endif
