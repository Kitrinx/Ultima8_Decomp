#ifndef MAINMENU_H
#define MAINMENU_H

#include "DISPATCH.H"
#include "CRECT.H"
#include "SBUTTON.H"

class Event;

// The main menu: intro, load, save, options, credits, quit, quotes and end game.
class MainMenu : public NewGump, public GumpShape
{
public:
	int saving;
	unsigned char endGameSeen;
	unsigned char quotesSeen;
	unsigned char resume;		// go back to the game when closed
	int mode;					// 0 menu, 1 and 2 save pages, 3 options
	unsigned f_3f;
	int slot;
	int highlight;
	int named;					// the avatar has a name
	char saveNames[12][104];	// four lines of 26 per save

	MainMenu(NewGump *, unsigned char);
	virtual ~MainMenu(void);
	virtual void commandMouseLeft(Event &);
	virtual void commandMouseRight(Event &);
	virtual void commandMouseMovement(Event &);
	virtual void commandPrivate(Event &);
	virtual void commandKeyboard(Event &);
	virtual void draw(short, short);
	void setOptions(void);
	void doMenuCommand(short);
	static GumpShape menuOptions[8];
	static GumpShape pagan;
	static GumpShape pagebutton;
	static GumpShape nums;
	static GumpShape entry;
	static Rect slots[6];
	static Rect options[8];
};

class EnterName : public NewGump
{
public:
	int x;
	int y;
	int len;
	char *text;

	EnterName(NewGump *, char *);
	virtual void commandKeyboard(Event &);
	virtual void draw(short, short);
};

long DispatchMainMenu(void);

extern char *HeaderFileName;
extern char *optionNames[27];
extern char *langs[3];
extern char *beginningText[3];
extern char *enterNameText[3];

#endif
