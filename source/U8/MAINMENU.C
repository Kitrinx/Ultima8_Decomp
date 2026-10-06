// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: MAINMENU.C

#include <stdio.h>
#include <string.h>
#include <io.h>
#include "..\UI\NEWGUMP.H"
#include "CRECT.H"
#include "DISPATCH.H"
#include "CAFONT.H"
#include "CFILE.H"
#include "FILESPEC.H"
#include "FEXIST.H"
#include "SCRATCHM.H"
#include "CEXIT.H"
#include "ITEM.H"
#include "NPC.H"
#include "CAMERA.H"
#include "CTMPOINT.H"
#include "U8POINT.H"
#include "U8GMPSHP.H"
#include "U8DISPAT.H"
#include "GAMETIME.H"
#include "AMUSIC.H"
#include "DSFXMAN.H"
#include "PAL.H"
#include "PAGAN.H"
#include "FLICPLAY.H"
#include "INIT.H"
#include "SLIDER.H"
#include "KTEXTED.H"
#include "U8YESNO.H"
#include "PREAMBLE.H"
#include "MAINMENU.H"

// Inline in the shared headers when this file was built.
inline Point::Point(void) {}
inline void Point::set(short x, short y) { f_00 = x; f_02 = y; }
inline Rect::Rect(void) {}
inline void Rect::set(short x1, short y1, short x2, short y2) { Point::set(x1, y1); f_04 = x2; f_06 = y2; }
inline Rect::Rect(short x1, short y1, short x2, short y2) { set(x1, y1, x2, y2); }
inline NewGumpId::NewGumpId(unsigned i) { instance = i; f_02 = ~instance; f_04 = 0; }
inline GumpShape::GumpShape(int shape) { _frame = 0; this->shape = shape; }
inline Rect GumpShape::get_rect(void) { return drawer->get_rect(shape, _frame); }
inline void GumpShape::draw(int x, int y) { drawer->draw(x, y, shape, _frame); }
inline void GumpShape::set_frame(int frame) { _frame = frame; }
inline int CacheFont::width(char *text) { return Font::width(text); }
inline CacheFont::~CacheFont(void) {}
inline unsigned char BaseFile::is_valid(void) { return handle != -1; }
inline BaseFile::~BaseFile(void) { if (is_valid()) close(); }

char *HeaderFileName = "sgheader";
char *optionNames[27] = {"Music:", "Sound FX:", "Avatar Steps:", "Animations:", "Text Speed:", "Frame Skipping:", "Speed Limiting:", 0, 0, "Musique:", "Effets Son:", "Pas de l'Avatar:", "Animations:", "Vitesse Dialogues:", "Limiter Animations:", "Limiter Vitesse:", 0, 0, "Musik:", "Soundeffekte:", "Avatarschritte:", "Animationen:", "Sprachtempo:", "Graphiken begrenzt:", "Geschw. begrenzt:", 0, 0};
char *langs[3] = {"(English)", "(Fran\207ais)", "(Deutsch)"};
GumpShape MainMenu::menuOptions[8] = {37, 37, 37, 37, 37, 37, 37, 37};
GumpShape MainMenu::pagan(32);
GumpShape MainMenu::pagebutton(34);
GumpShape MainMenu::nums(36);
GumpShape MainMenu::entry(46);
Rect MainMenu::options[8];
Rect MainMenu::slots[6] = {Rect(46, 12, 150, 52), Rect(46, 52, 150, 92), Rect(46, 92, 150, 132), Rect(165, 12, 269, 52), Rect(165, 52, 269, 92), Rect(165, 92, 269, 132)};
char *beginningText[3] = {"The Beginning...", "Au Debut...", "Der Anfang..."};

long DispatchMainMenu(void)
{
	theU8Dispatcher->flags &= ~PROC_SUSPENDED;
	return Dispatch(new MainMenu(0, 0));
}

void MainMenuProcess::process(void)
{
	pop(0);
	Dispatch(new MainMenu(0, 0));
}

MainMenu::MainMenu(NewGump *parent, unsigned char showOptions) :
	NewGump(NewGumpId(0), parent, 0),
	GumpShape(35)
{
	char *name;
	char number[4];
	theGameTime->pause = -1;
	highlight = -1;
	resume = 0;
	rect = get_rect();
	rect.moveto(10, 10);
	f_1d |= 0xe;
	registerHotKeys('1', '2', '3', '4', '5', '6', '7', '8', 0x141, 0x142, 27, 0);
	setOptions();
	for (int i = 0; i < 12; i++)
		for (int j = 0; j < 104; j++)
			saveNames[i][j] = 0;
	name = fileSpec(0, SavedgamesDir, HeaderFileName, "dat");
	BaseFile file;
	if (!FileExists(name))
	{
		file.forceOpen(name, (OpenMode)0);
		file.close();
	}
	file.open(name, (OpenMode)0);
	for (i = 0; i < 12; i++)
	{
		file.read(saveNames[i], 104, -1);
		sprintf(number, "%03d", i);
		if (!FileExists(fileSpec(0, SavedgamesDir, "u8save", number)))
			memset(saveNames[i], 0, 104);
	}
	file.close();
	name = fileSpec(0, StaticDir, quotesFlagName, flagExtension);
	if (FileExists(name))
		quotesSeen = 1;
	else
		quotesSeen = 0;
	name = fileSpec(0, StaticDir, endGameFlagName, flagExtension);
	if (FileExists(name))
		endGameSeen = 1;
	else
		endGameSeen = 0;
	if (!showOptions)
		mode = 0;
	else
		mode = 3;
	U8MousePointer::pushMode((PointerModes)0x24, 1);
	refresh();
	named = 1;
	char *avatarName = avatar.name;
	if (!*avatarName)
	{
		send(Event(EVENT_PRIVATE, 0, 0, -1, this, this, 0));
		named = 0;
	}
	else if (musicOn)
	{
		musicVolume(0, 1000);
		musicStop();
	}
}

MainMenu::~MainMenu(void)
{
	U8MousePointer::popMode();
	if (named && resume && musicOn)
	{
		musicVolume(90, 350);
		restoreMusic();
	}
	theGameTime->pause = 0;
}

// Lays out the menu entries down the right of the gump.
void MainMenu::setOptions(void)
{
	int y = 28;
	int x = 176;
	Rect r;
	for (int i = 0; i < 8; i++)
	{
		menuOptions[i].shape = 37;
		menuOptions[i].set_frame(i * 2 + avatar.language * 16);
		r = menuOptions[i].get_rect();
		r.moveto(x, y);
		y += 14;
		options[i] = r;
	}
}

void MainMenu::commandKeyboard(Event &event)
{
	switch (event.data)
	{
	case 0x141:		// F7: quick save
		U8MousePointer::pushMode((PointerModes)0x23, 1);
		save(999);
		U8MousePointer::popMode();
		return;
	case 0x142:		// F8: quick load
		if (FileExists(fileSpec(0, SavedgamesDir, "u8save", "999")))
		{
			U8MousePointer::pushMode((PointerModes)0x23, 1);
			load(999);
			U8MousePointer::popMode();
			if (!avatar.isDead())
			{
				resume = 1;
				killMyself();
				return;
			}
		}
		break;
	case 27:
		if (mode == 1 || mode == 2)
			U8MousePointer::popMode();
		if (mode)
		{
			mode = 0;
			refresh();
			return;
		}
		if (!avatar.isDead())
		{
			resume = 1;
			killMyself();
			return;
		}
		break;
	default:
		if (event.data >= '1' && event.data <= '8' && mode == 0)
			doMenuCommand((event.data & 0xf) - 1);
	}
}

void MainMenu::commandMouseLeft(Event &event)
{
	if (event.isSingle())
	{
		if (mode == 1 || mode == 2)
		{
			if (Rect(33, 3, 57, 19).contains(event.x - rect.f_00, event.y - rect.f_02))
			{
				if (mode == 1)
				{
					U8MousePointer::popMode();
					mode = 0;
				}
				else
					mode = 1;
			}
			else if (Rect(244, 3, 269, 19).contains(event.x - rect.f_00, event.y - rect.f_02))
			{
				if (mode == 1)
					mode = 2;
			}
			else
			{
				for (int i = 0; i < 6; i++)
				{
					if (slots[i].contains(event.x - rect.f_00, event.y - rect.f_02))
					{
						int page = 0;
						if (mode == 2)
							page = 6;
						i += page;
						if (!saving)
						{
							Boolean used = 0;
							for (int line = 0; line < 4; line++)
								if (saveNames[i][line * 26])
									used = 1;
							if (used || i == 0)
							{
								U8MousePointer::pushMode((PointerModes)0x23, 1);
								if (!i)
								{
									U8MousePointer::popMode();
									resume = 1;
									killMyself();
								}
								load(i);
								if (!i)
								{
									currentPalette = 0;
									U8MousePointer::popMode();
									U8MousePointer::popMode();
									U8MousePointer::popMode();
									U8MousePointer::popMode();
									U8MousePointer::popMode();
									preamble();
									return;
								}
								parent->restore();
								U8MousePointer::popMode();
							}
							else
							{
								U8MousePointer::popMode();
								mode = 0;
								refresh();
								return;
							}
						}
						else
						{
							if (i && !avatar.isDead())
							{
								slot = i;
								i %= 6;
								new KillerTextEditGump(0, this, saveNames[slot], Rect(slots[i].f_00, slots[i].f_02 - 3, slots[i].f_04, slots[i].f_06 - 3));
								return;
							}
							mode = 0;
							U8MousePointer::popMode();
							refresh();
							return;
						}
						U8MousePointer::popMode();
						resume = 1;
						killMyself();
						return;
					}
				}
			}
		}
		else if (mode == 3)
		{
			if (event.y >= 95 && event.y < 108)
				avatar.f_39 = Dispatch(new SliderGump(0, 0, 0, 0, 9, 1, avatar.f_39));
			else if (event.y >= 80 && event.y < 93)
				theAnimation->f_4d = 1 - theAnimation->f_4d;
			else if (event.y >= 65 && event.y < 78)
			{
				if (soundOn)
					avatar.f_36 = !avatar.f_36;
			}
			else if (event.y >= 35 && event.y < 48)
			{
				if (mcard.type)
				{
					U8MousePointer::pushMode((PointerModes)0x23, 1);
					if (musicOn)
						musicDeinit();
					else
						musicInit("music.flx", mport, mirq, mdma, mdrq);
					musicOn = !musicOn;
					U8MousePointer::popMode();
				}
			}
			else if (event.y >= 50 && event.y < 63)
			{
				if (scard.type)
				{
					if (soundOn)
						soundDeInit();
					else
						soundInit(sirq, sport, sdma);
					soundOn = !soundOn;
				}
			}
			else if (event.y >= 110 && event.y < 124)
				Camera::skipFrames = 1 - Camera::skipFrames;
			else if (event.y >= 125 && event.y < 139)
				avatar.f_48 = 1 - avatar.f_48;
			else
				mode = 0;
		}
		else
		{
			doMenuCommand(highlight);
			return;
		}
		refresh();
	}
}

void MainMenu::commandMouseRight(Event &event)
{
	if (event.isSingle() && event.y >= 95 && event.y < 108 && avatar.f_39 < 9)
		avatar.f_39++;
	refresh();
}

void MainMenu::doMenuCommand(short command)
{
	switch (command)
	{
	case 0:		// intro
		musicVolume(90, 350);
		playIntro(0, 1, 1);
		killMyself();
		return;
	case 1:		// load
		U8MousePointer::pushMode((PointerModes)0x27, 1);
		saving = 0;
	case 2:		// save
		if (command == 2)
		{
			U8MousePointer::pushMode((PointerModes)0x26, 1);
			saving = 1;
		}
		if (avatar.f_3b && command == 2)
		{
			U8MousePointer::popMode();
			return;
		}
		mode = 1;
		refresh();
		return;
	case 3:		// options
		mode = 3;
		refresh();
		return;
	case 4:		// credits
		musicVolume(90, 350);
		runCredits(1);
		killMyself();
		return;
	case 5:		// quit
		if (Dispatch(new U8YesNoGump(NewGumpId(0), 0, "Are you sure?")))
		{
			Exit(0);
			return;
		}
		refresh();
		return;
	case 6:		// quotes
		if (endGameSeen || quotesSeen)
		{
			musicVolume(90, 350);
			runQuotes();
			killMyself();
			return;
		}
		break;
	case 7:		// end game
		if (!endGameSeen)
			return;
		musicVolume(90, 350);
		playEndgame();
		killMyself();
	}
}

void MainMenu::commandPrivate(Event &event)
{
	if (event.data == -1)
	{
		Dispatch(new EnterName(0, avatar.name));
		resume = 1;
		killMyself();
	}
	if (event.data == 32)
	{
		char *name = fileSpec(0, SavedgamesDir, HeaderFileName, "dat");
		unlink(name);
		BaseFile file;
		file.forceOpen(name, (OpenMode)2);
		for (int i = 0; i < 12; i++)
			file.write(saveNames[i], 104, -1);
		file.close();
		U8MousePointer::pushMode((PointerModes)0x23, 1);
		if (!IDidNotTypeInAnything)
			save(slot);
		U8MousePointer::popMode();
		mode = 0;
		U8MousePointer::popMode();
		refresh();
	}
	else if (event.data == 31)
		refresh();
}

void MainMenu::draw(short x, short y)
{
	GumpShape::draw(x, y);
	int count = 6;
	if (quotesSeen)
		count = 7;
	if (endGameSeen)
		count = 8;
	if (mode == 1 || mode == 2)
	{
		CacheFont font((FontType)4);
		int start, page;
		if (mode == 1)
		{
			font.printf(slots[0].f_00 + x, slots[0].f_02 + y + 21, beginningText[avatar.language]);
			start = 1;
			page = 0;
		}
		else
		{
			start = 0;
			page = 6;
		}
		for (int i = start; i < 6; i++)
		{
			font.printf(slots[i].f_00 + x, slots[i].f_02 + 24, saveNames[i + page]);
			font.printf(slots[i].f_00 + x, slots[i].f_02 + 30, saveNames[i + page] + 26);
			font.printf(slots[i].f_00 + x, slots[i].f_02 + 36, saveNames[i + page] + 52);
			font.printf(slots[i].f_00 + x, slots[i].f_02 + 42, saveNames[i + page] + 78);
		}
		entry.set_frame(avatar.language);
		entry.draw(x + 46, y + 10);
		entry.draw(x + 46, y + 50);
		entry.draw(x + 46, y + 92);
		entry.draw(x + 165, y + 10);
		entry.draw(x + 165, y + 50);
		entry.draw(x + 165, y + 92);
		Rect r = entry.get_rect();
		for (i = 0; i < 6; i++)
		{
			nums.set_frame(mode * 6 + i - 6);
			nums.draw(r.f_04 - r.f_00 + slots[i].f_00 + x + 3, i % 3 * 41 + y + 10);
		}
		pagebutton._frame = 0;
		pagebutton.draw(x + 33, y + 3);
		if (mode == 1)
		{
			pagebutton._frame = 1;
			pagebutton.draw(x + 244, y + 3);
		}
		return;
	}
	if (mode == 3)
	{
		char lang = avatar.language;
		CacheFont font((FontType)9);
		font.printf(x + 40, y + 35, optionNames[lang * 9]);
		font.printf(x + 40, y + 50, optionNames[lang * 9 + 1]);
		font.printf(x + 40, y + 65, optionNames[lang * 9 + 2]);
		font.printf(x + 40, y + 80, optionNames[lang * 9 + 3]);
		font.printf(x + 40, y + 95, optionNames[lang * 9 + 4]);
		font.printf(x + 40, y + 110, optionNames[lang * 9 + 5]);
		font.printf(x + 40, y + 125, optionNames[lang * 9 + 6]);
		// '@' and '\376' are the font's ticked and empty boxes.
		if (!theAnimation->f_4d)
			font.printf(x + 130, y + 80, "@");
		else
			font.printf(x + 130, y + 80, "\376");
		if (musicOn)
			font.printf(x + 130, y + 35, "@");
		else
			font.printf(x + 130, y + 35, "\376");
		if (soundOn)
			font.printf(x + 130, y + 50, "@");
		else
			font.printf(x + 130, y + 50, "\376");
		if (avatar.f_36 && soundOn)
			font.printf(x + 130, y + 65, "@");
		else
			font.printf(x + 130, y + 65, "\376");
		font.printf(x + 130, y + 95, "(%d)", avatar.f_39);
		if (Camera::skipFrames)
			font.printf(x + 130, y + 110, "@");
		else
			font.printf(x + 130, y + 110, "\376");
		if (avatar.f_48)
			font.printf(x + 130, y + 125, "@");
		else
			font.printf(x + 130, y + 125, "\376");
		return;
	}
	if (mode == 0)
	{
		pagan.draw(x + 43, y + 10);
		if (named)
		{
			for (int i = 0; i < count; i++)
			{
				if (highlight == i)
				{
					menuOptions[i].set_frame(menuOptions[i]._frame | 1);
					menuOptions[i].draw(options[i].f_00, options[i].f_02);
				}
				else
				{
					menuOptions[i].set_frame(menuOptions[i]._frame & 0xfe);
					menuOptions[i].draw(options[i].f_00, options[i].f_02);
				}
			}
			CacheFont font((FontType)6);
			int width = font.width(avatar.name);
			font.printf(x + ((100 - width) >> 1) + 40, y + 126, avatar.name);
		}
	}
}

void MainMenu::commandMouseMovement(Event &event)
{
	if (mode == 0)
	{
		for (int i = 0; i < 8; i++)
			if (options[i].contains(event.x, event.y))
			{
				highlight = i;
				refresh();
				return;
			}
		if (highlight != -1)
		{
			highlight = -1;
			refresh();
		}
	}
}

char *enterNameText[3] = {"Give thy name:", "Marque ton nom:", "Namen eingeben:"};

EnterName::EnterName(NewGump *parent, char *) :
	NewGump(NewGumpId(0), parent, 0)
{
	rect.set(0, 0, 9, 70);
	registerHotKeys(0xffff, 0);
	f_1d |= 0x20;
	x = 170;
	y = 40;
	rect.moveto(x, y);
	text = avatar.name;
	text[0] = '-';
	text[1] = 0;
	len = 0;
	TMMousePointer::hide();
	refresh();
}

void EnterName::draw(short x, short y)
{
	CacheFont font((FontType)6);
	font.printf(x, y - 12, enterNameText[avatar.language]);
	font.printf(x, y, text);
}

// The name is typed with a '-' cursor at its end.
void EnterName::commandKeyboard(Event &event)
{
	switch (event.data)
	{
	case 13:
	case 27:
		if (len)
		{
			text[len] = 0;
			TMMousePointer::show();
			killMyself();
			return;
		}
		break;
	case 8:
		if (len)
		{
			len--;
			text[len + 1] = 0;
			text[len] = '-';
		}
		restore();
		return;
	default:
		if (event.data >= ' ' && event.data <= '~' && len < 16)
		{
			text[len++] = event.data;
			text[len] = '-';
			text[len + 1] = 0;
			restore();
		}
	}
}
