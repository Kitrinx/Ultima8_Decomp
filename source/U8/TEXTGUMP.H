#ifndef TEXTGUMP_H
#define TEXTGUMP_H

#include "DISPATCH.H"
#include "MENU.H"

class Event;
class NewGumpId;
class PullDownGump;
class Rect;
class TextMenuBarGump;
class TextMenuGump;
class TextMenuItemGump;
class TextMenuLineGump;
class TextPullDownGump;
class WindowFrame;
class WindowGump;

class WindowGump : public NewGump
{
public:
	int cursorX;
	int cursorY;
	char color;
	WindowFrame *frame;
	unsigned char shown;	// text mode switched on

	WindowGump(NewGumpId, NewGump *, Rect, char, unsigned char, WindowFrame *);
	virtual void draw(short, short);
	void clear(void);
	void gotoxy(short, short);
	void print(char *, ...);
	void print(char);
	void deactivate(void);
	void activate(void);
	void moveCursor(int, int);
	void setColor(char);
};

class WindowFrame : public WindowGump
{
public:
	char *frameChars;		// corners, then horizontal and vertical edges

	WindowFrame(NewGumpId, NewGump *, Rect, char *, char);
	virtual void draw(short, short);
	void setFrameChars(char *chars) { frameChars = chars; }
};

class Scroller : public WindowGump
{
public:
	int scrollX;
	int scrollY;
	int maxX;
	int maxY;

	Scroller(NewGumpId, NewGump *, Rect, char, WindowFrame *);
	int isVis(short, short);
	virtual void draw(short, short) = 0;
};

class TextMenuLineGump : public PureMenuItemGump
{
public:
	TextMenuLineGump(void);
	virtual void draw(short, short);
	virtual void getDim(int &, int &);
};

class TextMenuItemGump : public PureMenuItemGump
{
public:
	TextMenuItemGump(char *, int, unsigned char);
	virtual void draw(short, short);
	virtual void getDim(int &, int &);
};

class TextCheckMenuItemGump : public TextMenuItemGump
{
public:
	unsigned char *checked;

	TextCheckMenuItemGump(void) : TextMenuItemGump(0, 0, 0) {}
	TextCheckMenuItemGump(char *text, int id, unsigned char enabled, unsigned char *flag) : TextMenuItemGump(text, id, enabled) { checked = flag; }
	virtual void commandMouseLeft(Event &);
	virtual void draw(short, short);
	virtual void addPullDown(PullDownGump *, unsigned char);
};

class TextMenuGump : public MenuGump
{
public:
	TextMenuGump(NewGump *parent) : MenuGump(parent, GumpColorMap[5]) {}
};

class TextPullDownGump : public TextMenuGump
{
public:
	TextPullDownGump(void);
	virtual void commandPrivate(Event &);
	virtual void draw(short, short);
	virtual void addMenuItem(PureMenuItemGump *);
};

class TextMenuBarGump : public TextMenuGump
{
public:
	TextMenuBarGump(unsigned char, NewGump *);
	virtual void commandPrivate(Event &);
	virtual void draw(short, short);
	virtual void addMenuItem(PureMenuItemGump *);
};

TextMenuItemGump &operator+(TextMenuItemGump &, TextPullDownGump &);
TextPullDownGump &operator+(TextMenuItemGump &, TextMenuItemGump &);
TextPullDownGump &operator+(TextPullDownGump &, TextMenuItemGump &);
TextMenuBarGump &operator+(TextMenuBarGump &, TextMenuItemGump &);
TextPullDownGump &operator+(TextPullDownGump &, TextMenuLineGump &);
TextPullDownGump &operator+(TextMenuItemGump &, TextMenuLineGump &);

extern char *doubleFrame;
extern char *singleFrame;

#endif
