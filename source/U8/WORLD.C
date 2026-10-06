// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: WORLD.C

#include <conio.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\UI\NEWGUMP.H"
#include "ITEM.H"
#include "NPC.H"
#include "TYPE.H"
#include "ITEMCACH.H"
#include "ITEMFIND.H"
#include "COMBIN.H"
#include "SCRIT.H"
#include "AMUSIC.H"
#include "ANIM.H"
#include "GAMETIME.H"
#include "BARK.H"
#include "BASECAM.H"
#include "CACHE.H"
#include "CAMERA.H"
#include "CEXIT.H"
#include "CGLOBVP.H"
#include "CMOUSE.H"
#include "CONTAIN.H"
#include "CONTGUMP.H"
#include "CRECT.H"
#include "CPROTMEM.H"
#include "CVPORT.H"
#include "DISPATCH.H"
#include "DLIST.H"
#include "DLITEM.H"
#include "DSFXMAN.H"
#include "EDITEM.H"
#include "EGG.H"
#include "ERROR.H"
#include "FILESPEC.H"
#include "FLEX.H"
#include "FLAG.H"
#include "HARDWARE.H"
#include "INIT.H"
#include "FEXIST.H"
#include "IRGUMP.H"
#include "KERNEL.H"
#include "MAINMENU.H"
#include "MAIN.H"
#include "MAPFILE.H"
#include "SCRATCHM.H"
#include "MENU.H"
#include "MESSAGE.H"
#include "MISSILE.H"
#include "MOTRAK.H"
#include "PAL.H"
#include "SLIDER.H"
#include "TARGGUMP.H"
#include "TEXTGUMP.H"
#include "U8POINT.H"
#include "U8YESNO.H"
#include "URANDOM.H"
#include "WAIT.H"
#include "YAMM.H"
#include "WORLD.H"

// Inline in the shared headers when this file was built.
inline Point::Point(void) {}
inline Point::Point(short x, short y) { set(x, y); }
inline void Point::set(short x, short y) { f_00 = x; f_02 = y; }
inline Rect::Rect(void) {}
inline void Rect::set(short x1, short y1, short x2, short y2) { Point::set(x1, y1); f_04 = x2; f_06 = y2; }
inline Rect::Rect(short x1, short y1, short x2, short y2) { set(x1, y1, x2, y2); }
inline NewGumpId::NewGumpId(unsigned i) { instance = i; f_02 = ~instance; f_04 = 0; }
inline unsigned char NewGumpId::getType(void) { return instance >> 8; }
inline void NewGump::popVisibility(void) { visibilityStack >>= 2; }
inline FileSpec::FileSpec(char *path, int attrib) { become(path, attrib); }
inline unsigned char BaseFile::is_valid(void) { return handle != -1; }
inline BaseFile::~BaseFile(void) { if (is_valid()) close(); }

extern unsigned char inGameMode;

// An NPC by number, for the cheat teleports.
class CheatNpc : public Item
{
public:
	CheatNpc(Referent r) { referent = r; }
	unsigned char getMap(void) { return ItemData::mapNumArray[referent]; }
};

// The mouse pointer's direction as a world direction, and its distance from the centre.
inline int mouseDir(void) { return (U8MousePointer::getDir() - 1) & 7; }
inline int mouseErection(void) { return U8MousePointer::erection; }
inline Item itemOf(Referent r) { return Item(r); }
inline void Item::pop(WorldPoint &p) { pop(p.x, p.y, p.z); }

char *mouseBark[6] = {"Right handed mouse", "Souris droiti\212re", "Maus f\201r Rechtsh\204nder", "Left handed mouse", "Souris gauch\212re", "Maus f\201r Linksh\204nder"};
char *quit[3] = {"Quit      ", "Quitter   ", "Abbrechen "};
char *toDos[3] = {"To DOS", "Retour \205 DOS", "Zur\201ck zu DOS"};
char *toPagan[3] = {"To Pagan", "Retour \205 Pagan", "Zur\201ck nach Pagan"};
char *test[3] = {"Test      ", "Tester    ", "Testen    "};
char *testMusic[3] = {"Test Music", "Tester Musique", "Musik testen"};
char *testSFX[3] = {"Test SFX", "Tester Effets Son", "Soundeffekte testen"};
char *testFiles[3] = {"Test Files", "Tester Fichiers", "Dateien testen"};
char *view[3] = {"View      ", "Voir      ", "Lesen     "};
char *playing[3] = {"Playing...", "En marche...", "Spielt..."};
char *file[3] = {"File", "Fichier", "Datei"};
char *passed[3] = {"Passed", "R\202ussi", "Erfolgreich"};
char *failed[3] = {"Failed", "Echou\202", "Nicht Erfolgreich"};
char *pressAnyKey[3] = {"Press any key...", "Appuyer sur une touche...", "Dr\201ken Sie eine beliebige Taste..."};
char *whatDrive[3] = {"What Drive", "Quel lecteur", "Welches Laufwerk"};
char *invalid[3] = {"Invalid!", "Invalide!", "Ung\201ltig!"};
char *doesNotExist[3] = {"does not exist!", "n'existe pas!", "Existiert nicht!"};
char *endOfFile[3] = {"End of File.", "Fin de Ficher.", "Ende der Datei."};
char *hardwareStatus[3] = {"Hardware Status", "Equipement", "Hardware"};
char *memory[3] = {"Memory", "M\202moire", "Speicherkapazit\204t"};
char *bytes[3] = {"bytes", "octets", "Bytes"};
char *available[3] = {"available", "disponibles", "frei"};
char *largestFragment[3] = {"largest fragment", "fragment max", "Gr\224\341tes Fragment"};
char *hardDrive[3] = {"Hard Drive", "Disque Dur", "Laufwerk"};
char *shapeCache[3] = {"Shape Cache", "Cache Formes", "Formencache"};
char *music[3] = {"Music", "Musique", "Musik"};
char *on[3] = {"On", "Oui", "An"};
char *off[3] = {"Off", "Non", "Aus"};
char *card[3] = {"Card", "Carte", "Karte"};
char *port[3] = {"Port", "Port", "Anschlu\341"};
char *sfx[3] = {"SFX", "Effets Son", "Soundeffekte"};
unsigned char Line25[26] = {196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 0};

CheatWindow::CheatWindow(long data) :
	InputWindowGump(NewGumpId(0), Rect(0, 0, 79, 49), 0)
{
	avatar.cheater = 1;
	f_3e.referent = 0;
	buildMenu();
	registerHotKeys(0xffff, 0);
	if (data)
	{
		f_40 = 1;
		makeInvisible();
		send(Event((EventType)0x20, 0, 0, data, this, this));
		playSFX(0xa3);
	}
	else
		f_40 = 0;
	if (inGameMode)
	{
		where.x = avatar.getX();
		where.y = avatar.getY();
		where.z = avatar.getZ();
	}
	else
	{
		where.x = Camera::getX();
		where.y = Camera::getY();
		where.z = Camera::getZ();
	}
}

unsigned char InputWindowGump::getInt(int &value)
{
	char *format;
	char buffer[256];
	if (strlen(gets(buffer)) == 1)
		return 0;
	buffer[strlen(buffer) - 1] = 0;
	int skip = 0;
	if (buffer[1] == 'x' || buffer[1] == 'X')
	{
		format = "%x";
		skip = 2;
	}
	else
		format = "%d";
	sscanf(buffer + skip, format, &value);
	return 1;
}

unsigned char InputWindowGump::getString(char *string)
{
	char buffer[256];
	if (strlen(gets(buffer)) == 1)
		return 0;
	buffer[strlen(buffer) - 1] = 0;
	strcpy(string, buffer);
	return 1;
}

unsigned char InputWindowGump::getChar(char &c)
{
	c = getch();
	return c != '\r';
}

int CheatWindow::resetMenu(void)
{
	NewGump *menuBar;
	NewGump *found = 0;
	PureMenuItemGump *menuItem;
	menuBar = newGumpList.hasType(3, found);
	menuItem = (PureMenuItemGump *)((MenuGump *)menuBar)->findMenuItem(NewGumpId(0x12));
	menuItem->f_3c = f_3e.isValid() && GlobalTypes.getTypeFlag(f_3e.getType()).family == QUAL_FAMILY;
}

void CheatWindow::buildMenu(void)
{
	TextMenuBarGump *menuBar;
	avatar.f_2a = avatar.isImmortal();
	menuBar = &(*new TextMenuBarGump(1, this)
		+ (*new TextMenuItemGump("Toggles", 0, 1)
			+ (*new TextCheckMenuItemGump("Hackmover         H", 2, inGameMode, &avatar.f_2b)
				+ *new TextCheckMenuItemGump("Power Avatar      P", 3, inGameMode, &avatar.f_2a)
				+ *new TextCheckMenuItemGump("Footpads          F", 4, 1, &DL_ItemNode::showFootpads)
				+ *new TextCheckMenuItemGump("Egg Bounds        E", 14, 1, &EggHatcher::eggOutlineVisible)))
		+ (*new TextMenuItemGump("Teleport", 0, 1)
			+ (*new TextMenuItemGump("To NPC", 6, 1) + *new TextMenuItemGump("To Location", 7, 1)
				+ *new TextMenuItemGump("To Map", 15, 1) + *new TextMenuItemGump("To Happy Hunting Ground", 8, 1)
				+ *new TextMenuItemGump("To Gift Shop", 9, 1)))
		+ (*new TextMenuItemGump("View/Edit", 0, 1)
			+ (*new TextMenuItemGump("NPC Stats", 10, 1) + *new TextMenuItemGump("Usecode Flags", 16, inGameMode)
				+ *new TextMenuItemGump("Time (hours)", 17, 1) + *new TextMenuItemGump("Item Stats", 11, 1)
				+ *new TextMenuItemGump("Item Quality", 18, 0)))
		+ (*new TextMenuItemGump("Listen", 0, 1)
			+ (*new TextMenuItemGump("SFX", 12, 1) + *new TextMenuItemGump("Music", 13, 1)))
		+ *new TextMenuItemGump("Quit   F7", 5, 1));
	menuBar->rect.f_04 = 79;
}

void CheatWindow::commandPrivate(Event &event)
{
	int map, npcNum;
	switch (event.x)
	{
	case 5:
		theBaseCamera->graphicVideoMode();
		killMyself();
		return;
	case 11:
		makeInvisible();
		theBaseCamera->graphicVideoMode();
		Dispatch(new TargetGump(&f_3e, 0));
		theBaseCamera->textVideoMode();
		popVisibility();
		resetMenu();
		refresh();
		break;
	case 4:
		Camera::init();
		theCamera->show();
		break;
	case 8:
		if (inGameMode)
			avatar.teleport(0x143f, 0x191f, 0x30, 0x3d);
		else
			Camera::move_to(0x143f, 0x191f, 0x30, 0x3d);
		send(Event((EventType)0x10, 5, 0, 0, this, this));
		break;
	case 9:
		if (inGameMode)
			avatar.teleport(0x2744, 0xd94, 0, 1);
		else
			Camera::move_to(0x2744, 0xd94, 0, 1);
		send(Event((EventType)0x10, 5, 0, 0, this, this));
		break;
	case 6:
		gotoxy(1, 34);
		print("Enter NPC #-->");
		if (!getInt(npcNum))
			break;
		if (npcNum > 255)
			break;
		if (CheatNpc(npcNum).getMap() == 0)
		{
			color = 0x14;
			gotoxy(1, 35);
			print("%d is at lunch.", npcNum);
			color = 0x1e;
		}
		else if (inGameMode)
			avatar.teleport(Item(npcNum).getX(), Item(npcNum).getY(), Item(npcNum).getZ(), CheatNpc(npcNum).getMap());
		else
			Camera::move_to(Item(npcNum).getX(), Item(npcNum).getY(), Item(npcNum).getZ(), CheatNpc(npcNum).getMap());
		send(Event((EventType)0x10, 5, 0, 0, this, this));
		break;
	int x, y, z, m, egg, mapNum;
	case 7:
		gotoxy(1, 34);
		print("Enter X---->");
		if (!getInt(x))
			x = where.x;
		gotoxy(1, 35);
		print("Enter Y---->");
		if (!getInt(y))
			y = where.y;
		gotoxy(1, 36);
		print("Enter Z---->");
		if (!getInt(z))
			z = where.z;
		gotoxy(1, 37);
		print("Enter Map-->");
		map = getInt(m) ? m : ItemCache::mapIn;
		gotoxy(1, 38);
		printf("Teleporting...");
		if (z < 250 && map < 255 && map > 0)
		{
			if (inGameMode)
				avatar.teleport(x, y, z, map);
			else
				Camera::move_to(x, y, z, map);
		}
		else
		{
			color = 0x14;
			gotoxy(1, 38);
			print("Not a valid destination!");
			color = 0x1e;
		}
		send(Event((EventType)0x10, 5, 0, 0, this, this));
		break;
	case 15:
		gotoxy(1, 34);
		print("Enter Map Num-->");
		if (!getInt(mapNum))
			mapNum = ItemCache::mapIn;
		map = mapNum;
		gotoxy(1, 35);
		print("Enter Dest Egg Num-->");
		getInt(egg);
		gotoxy(1, 36);
		print("Teleporting...");
		teleportToEgg(map, egg, inGameMode);
		send(Event((EventType)0x10, 5, 0, 0, this, this));
		break;
	case 16:
		{
			char buffer[80];
			FileSpec spec(fileSpec(0, UsecodeDir, "flagsym.dat", 0), 0);
			int bitNumber, numBits, value;
			gotoxy(1, 34);
			if (!spec.exists())
			{
				printf("%s doesn't exist.", (char *)spec);
				break;
			}
			printf("Flag name:");
			if (!getString(buffer))
				break;
			if (GlobalFlag::findFlag(buffer, bitNumber, numBits))
			{
				int max = 1 << (numBits - 1);
				int old = GlobalFlag::get(bitNumber, numBits);
				gotoxy(1, 35);
				printf("Old Value: (%d)   [0x%04x]", old, old);
				gotoxy(1, 36);
				printf("New Value: (0-%d) [0x0-0x%04x]:", max, max);
				getInt(value);
				gotoxy(1, 37);
				if (value >= 0 && value <= max)
				{
					GlobalFlag::set(bitNumber, numBits, value);
					printf("Done!");
				}
				else
					printf("Out of Range!");
			}
			else
			{
				gotoxy(1, 35);
				printf("This flag does not exist!");
			}
		}
		break;
	case 17:
		gotoxy(1, 34);
		print("Current time: %ld", TimeInGameHours());
		gotoxy(1, 35);
		print("Enter newtime in hours---->");
		if (getInt(x))
			SetTimeInGameHours(x);
		break;
	case 18:
		gotoxy(1, 34);
		if (f_3e.isValid())
		{
			printf("Enter New Quality--->");
			if (getInt(x))
				f_3e.setQuality(x);
		}
		break;
	default:
		refresh();
		break;
	}
	for (int i = 0; i < 4; i++)
	{
		gotoxy(1, i + 34);
		print("%45s", " ");
	}
}

void CheatWindow::commandKeyboard(Event &event)
{
	event.x = 5;
	switch (event.data)
	{
	case 'I':
	case 'i':
		event.x = 0xb;
		break;
	case 'H':
	case 'h':
		avatar.f_2b = !avatar.f_2b;
		break;
	case 'P':
	case 'p':
		if (avatar.isImmortal())
			avatar.clrImmortal();
		else
			avatar.setImmortal();
		break;
	case 'F':
	case 'f':
		DL_ItemNode::showFootpads = !DL_ItemNode::showFootpads;
		Camera::init();
		theCamera->show();
		break;
	case 'U':
	case 'u':
		Camera::drawUgliKolur = !Camera::drawUgliKolur;
		Camera::init();
		theCamera->show();
		break;
	case 0x141:
		break;
	default:
		makeInvisible();
		theBaseCamera->graphicVideoMode();
		switch (event.data)
		{
		case 0x13b: f1key(); break;
		case 0x13c: f2key(); break;
		case 0x13d: f3key(); break;
		case 0x13e: f4key(); break;
		case 0x13f: f5key(); break;
		case 0x15e: ctrlF1Key(); break;
		case 0x15f: ctrlF2Key(); break;
		case 0x160: ctrlF3Key(); break;
		case 0x161: ctrlF4Key(); break;
		case 0x162: ctrlF5Key(); break;
		case 0x165: ctrlF8Key(); break;
		case 0x154: shiftF1Key(); break;
		case 0x155: shiftF2Key(); break;
		case 0x156: shiftF3Key(); break;
		case 0x157: shiftF4Key(); break;
		case 0x158: shiftF5Key(); break;
		case 0x15b: shiftF8Key(); break;
		case 0x168: altF1Key(); break;
		case 0x169: altF2Key(); break;
		case 0x16a: altF3Key(); break;
		case 0x16b: altF4Key(); break;
		case 0x16c: altF5Key(); break;
		case 0x16f: altF8Key(); break;
		case 0x140: f6key(); break;
		case 0x163: ctrlF6Key(); break;
		case 0x144: f10key();
		default:
			if (f_40)
				break;
			popVisibility();
			refresh();
			return;
		}
		popVisibility();
		refresh();
		break;
	}
	commandPrivate(event);
}

void CheatWindow::draw(short x, short y)
{
	unsigned long total, largest;
	unsigned short selectors;
	WindowGump::draw(x, y);
	clear();
	ProtMemoryManager::getMemStatus(total, largest, selectors);
	gotoxy(1, 3);
	print("Ultima VIII %s  v%s%s  %s %s", Flavor, Version, avatar.cheater ? ".1" : "  ", DateOfBuild, TimeOfBuild);
	gotoxy(60, 3);
	print("Usecode: %c%s", avatar.getLanguageChar(), UsecodeVersion);
	gotoxy(60, 4);
	print("Speech:  %c%s", avatar.getLanguageChar(), SpeechVersion);
	gotoxy(1, 5);
	print("Memory: %lu (S%u)", total, selectors);
	color = 0x1f;
	gotoxy(1, 6);
	if (inGameMode)
		print("Avatar");
	else
		print("Camera");
	color = 0x1e;
	gotoxy(1, 7);
	print("Location: ");
	gotoxy(11, 7);
	if (mapNames)
		print("%s (%d)", ItemCache::mapIn == -1 ? (char *)0 : ((char (*)[32])mapNames)[ItemCache::mapIn], ItemCache::mapIn);
	else
		print("Map %d", ItemCache::mapIn);
	gotoxy(35, 7);
	print("(%5d,%5d,%3d) [%04x,%04x,%03x]", where.x, where.y, where.z, where.x, where.y, where.z);
	color = 0x1f;
	gotoxy(12, 9);
	print((char *)Line25);
	print(" Item ");
	print((char *)Line25);
	gotoxy(12, 32);
	print((char *)Line25 + 2);
	print(" User Input ");
	print((char *)Line25 + 2);
	color = 0x1e;
	displayItemStats();
}

char *TypeFlagDesc[14] = {"Fixed:", "Solid:", "Sea:", "Land:", "Occl:", "Bag:", "Dmgng:", "Noisy:", "Draw:", "Ignore:", "Roof:", "Trans:", "Editor:", "Explode:"};
char *ItemStatDesc[16] = {"InDlist:", "Disp:", "Virtl:", "Contd:", "Invis:", "Flipd:", "Npc:", "GlobCr:", "GumpUp:", "Equppd:", "Bounce:", "Etherl:", "Hangng:", "InFast:", "LowFric:", "IRGump:"};
char InvalidString[8] = "Invalid";
char *FamilyNames[16] = {"Generic     ", "Quality     ", "Quantity    ", "Glob Egg    ", "Unk Egg     ", "Breakable   ", "Container   ", "Monster Egg ", "Teleport Egg", "Reagent     ", InvalidString, InvalidString, InvalidString, InvalidString, InvalidString, InvalidString};
char *EquipNames[16] = {"None  ", "Shield", "Arm   ", "Head  ", "Body  ", "Legs  ", "Weapon", InvalidString, InvalidString, InvalidString, InvalidString, InvalidString, InvalidString, InvalidString, InvalidString, InvalidString};
char *AnimNames[16] = {"None   ", "Fast   ", "Random ", "Normal ", "Bubble ", "Unk    ", "On/Off ", InvalidString, InvalidString, InvalidString, InvalidString, InvalidString, InvalidString, InvalidString, InvalidString, InvalidString};

void CheatWindow::displayItemStats(void)
{
	if (f_3e.isValid())
	{
		gotoxy(1, 10);
		char *name = GlobalTypes.getTypeName(f_3e.getType());
		if (name)
			print("%s (%d) Frame:%d", GlobalTypes.getTypeName(f_3e.getType()), f_3e.getType(), f_3e.getFrame());
		else
			print("Type:%d  Frame:%d", f_3e.getType(), f_3e.getFrame());
		gotoxy(65, 10);
		print("Ref: %d", f_3e.referent);
		gotoxy(1, 11);
		print("Location: (%5d,%5d,%3d) [%04x,%04x,%03x]", f_3e.getX(), f_3e.getY(), f_3e.getZ(), f_3e.getX(), f_3e.getY(), f_3e.getZ());
		TypeFlag flag = GlobalTypes.getTypeFlag(f_3e.getType());
		color = 0x1f;
		gotoxy(1, 13);
		print("Type Flags:");
		color = 0x1e;
		int value, n, j;
		for (int i = 0; i < 5; i++)
		{
			for (j = 0; j < 3; j++)
			{
				gotoxy(j * 15 + 1, i + 14);
				n = i + j * 5;
				switch (n)
				{
				case 0: value = flag.fixed; break;
				case 1: value = flag.solid; break;
				case 2: value = flag.sea; break;
				case 3: value = flag.land; break;
				case 4: value = flag.occl; break;
				case 5: value = flag.bag; break;
				case 6: value = flag.damaging; break;
				case 7: value = flag.noisy; break;
				case 8: value = flag.draw; break;
				case 9: value = flag.ignore; break;
				case 10: value = flag.roof; break;
				case 11: value = flag.trans; break;
				case 12: value = flag.editor; break;
				case 13: value = flag.explode; break;
				}
				print("%10s%1d", TypeFlagDesc[n], value);
			}
		}
		gotoxy(1, 20);
		print("Footpad: %d, %d, %d", flag.xSize, flag.ySize, flag.zSize);
		gotoxy(30, 20);
		print("Equip to: ", EquipNames[flag.equipType]);
		gotoxy(50, 20);
		print("Weight: %d   Volume: %d", flag.weight, flag.volume);
		gotoxy(1, 21);
		print("Anim: %s  Fr:%d  Sp:%d", AnimNames[flag.animType], flag.animData, flag.animSpeed);
		gotoxy(1, 23);
		print("Family: %s", FamilyNames[flag.family]);
		gotoxy(30, 23);
		gotoxy(50, 23);
		switch (flag.family)
		{
		case QUAL_FAMILY:
			printf("(%5d)  [%04x]", f_3e.getQuality(), f_3e.getQuality());
			break;
		case QUAN_FAMILY:
		case REAGENT_FAMILY:
			printf("(%5d)  [%04x]", f_3e.getQuantity(), f_3e.getQuantity());
			break;
		}
		int status = f_3e.getStatus();
		color = 0x1f;
		gotoxy(1, 25);
		print("Item Flags:");
		color = 0x1e;
		for (i = 0; i < 4; i++)
		{
			for (j = 0; j < 4; j++)
			{
				gotoxy(j * 15 + 1, i + 26);
				int bit = i + j * 4;
				int set = (status & (1 << bit)) ? 1 : 0;
				print("%10s%1d", ItemStatDesc[bit], set);
			}
		}
	}
}

TestScreen::TestScreen(void) :
	InputWindowGump(NewGumpId(0), Rect(0, 0, 79, 49), 1),
	hardwareChecker(3000000L, 2000000L, 0, CPU_386)
{
	language = avatar.language;
	if (language == -1)
		language = 0;
	TextMenuBarGump *menuBar = &(*new TextMenuBarGump(0, this)
		+ (*new TextMenuItemGump(quit[language], 0, 1)
			+ (*new TextMenuItemGump(toDos[language], 1, 1) + *new TextMenuItemGump(toPagan[language], 7, 1)))
		+ (*new TextMenuItemGump(test[language], 0, 1)
			+ (*new TextMenuItemGump(testMusic[language], 2, musicOn) + *new TextMenuItemGump(testSFX[language], 3, soundOn)
				+ *new TextMenuItemGump(testFiles[language], 4, 1)))
		+ (*new TextMenuItemGump(view[language], 0, 1)
			+ (*new TextMenuItemGump("AUTOEXEC.BAT", 5, 1) + *new TextMenuItemGump("CONFIG.SYS", 6, 1))));
	menuBar->rect.f_04 = 79;
}

void TestScreen::commandPrivate(Event &event)
{
	int i, j;
	gotoxy(5, 16);
	switch (event.x)
	{
	case 1:
		Exit(0);
	case 7:
		theBaseCamera->graphicVideoMode();
		killMyself();
		return;
	case 2:
	case 3:
		print(playing[language]);
		if (event.x == 2)
		{
			musicPlay(0x61);
			Timer *timer = new Timer;
			timer->start();
			while (timer->seconds < 5)
				;
			musicSlowStop();
			timer->f_10 |= 4;
			timer->pop(0);
		}
		else
		{
			for (i = 0; i < 255; i++)
				playSFX(i);
		}
		gotoxy(5, 16);
		printf("          ");
		break;
	case 4:
		char files[10][80];
		for (i = 0; i < 8; i++)
		{
			gotoxy(5, i + 20);
			for (j = 0; j < 60; j++)
				print(" ");
		}
		strcpy(files[0], fileSpec(0, StaticDir, shapeFileName, 0));
		strcpy(files[1], fileSpec(0, StaticDir, fixedFileDesc, 0));
		strcpy(files[2], fileSpec(0, StaticDir, globFileName, 0));
		strcpy(files[3], fileSpec(0, StaticDir, transformPaletteFileName, 0));
		introFileName[0] = avatar.getLanguageChar();
		strcpy(files[4], fileSpec(0, StaticDir, introFileName, 0));
		strcpy(files[5], fileSpec(0, StaticDir, endgameFileName, 0));
		usecodeFileName[0] = avatar.getLanguageChar();
		strcpy(files[6], fileSpec(0, UsecodeDir, usecodeFileName, 0));
		strcpy(files[7], fileSpec(0, SoundDir, musicFileName, 0));
		strcpy(files[8], fileSpec(0, SoundDir, "sound.flx", 0));
		strcpy(files[9], fileSpec(0, GamedatDir, nonfixedFileDesc, 0));
		for (i = 0; i < 10; i++)
		{
			gotoxy(5, i + 20);
			print("%s %s: ", file[language], files[i]);
			gotoxy(40, i + 20);
			if (!FileExists(files[i]))
				print(doesNotExist[language]);
			else
			{
				FlexFile flex(files[i], ReadOnly, -1);
				long checksum;
				char ok = flex.verifyChecksum(&checksum);
				print("%s (%08lx)", ok ? passed[language] : failed[language], checksum);
			}
		}
		gotoxy(5, i + 21);
		printf(pressAnyKey[language]);
		getch();
		break;
	case 5:
	case 6:
		char drive[6];
		char path[80];
		print("%s ? [C]--->", whatDrive[language]);
		if (!getChar(drive[0]))
			drive[0] = 'C';
		gotoxy(5, 16);
		print("%20s", " ");
		strcpy(drive + 1, ":\\");
		drive[0] = toupper(drive[0]);
		if (drive[0] < 'A' || drive[0] > 'Z')
		{
			print("  %s", invalid[language]);
			break;
		}
		char autoexec = event.x == 5;
		strcpy(path, fileSpec(drive, 0, autoexec ? "AUTOEXEC.BAT" : "CONFIG.SYS", 0));
		if (!FileExists(path))
		{
			print("  %s %s!", path, doesNotExist);
			break;
		}
		BaseFile text(path, ReadWrite);
		int line = 0;
		while (!text.eof())
		{
			gotoxy(5, line++ + 16);
			if (line == 30)
			{
				gotoxy(5, line + 17);
				print(pressAnyKey[language]);
				getch();
				for (i = 0; i <= line + 1; i++)
				{
					gotoxy(5, i + 16);
					print("%70s", " ");
				}
				line = 0;
			}
			text.readString(path, 80, -1);
			path[strlen(path) - 2] = 0;
			strcpy(path + 65, "...");
			print("%-70s", path);
		}
		gotoxy(5, line + 17);
		printf("%s %s", endOfFile[language], pressAnyKey[language]);
		getch();
		break;
	}
	refresh();
}

void TestScreen::draw(short, short)
{
	int sport, sirq, sdrq, sdma, mport, mirq, mdrq, mdma;
	char sname[50];
	char mname[50];
	language = avatar.language;
	if (language == -1)
		language = 0;
	clear();
	color = 0x1f;
	gotoxy(1, 3);
	print("Ultima 8 %s   v%s%s  %s %s", Flavor, Version, avatar.cheater ? ".1" : "  ", DateOfBuild, TimeOfBuild);
	gotoxy(1, 5);
	print(hardwareStatus[language]);
	color = 0x1e;
	gotoxy(1, 7);
	print("%17s: %10ld %s %s -- %ld %s", memory[language], hardwareChecker.getTotalMemory(), bytes[language], available[language],
		hardwareChecker.getLargestMemoryFragment(), largestFragment[language]);
	gotoxy(1, 8);
	print("%17s: %10ld %s %s", hardDrive[language], hardwareChecker.getHardDriveSpace(0), bytes[language], available[language]);
	gotoxy(1, 9);
	print("%17s: %10ld %s", shapeCache[language], CacheNodePtr::totalBytes, bytes[language]);
	getSoundStuff(sport, sirq, sdma, sdrq, mport, mirq, mdma, mdrq, musicOn, soundOn, sname, mname);
	gotoxy(1, 11);
	print("%14s: %s", music[language], musicOn ? on[language] : off[language]);
	if (musicOn)
	{
		gotoxy(22, 11);
		print("%6s: %-20s  %s:0x%03x", card[language], mname, port[language], mport);
	}
	gotoxy(1, 12);
	print("%14s: %s", sfx[language], soundOn ? on[language] : off[language]);
	if (soundOn)
	{
		gotoxy(22, 12);
		print("%6s: %-20s  %s:0x%03x  IRQ:%d  DMA:%d", card[language], sname, port[language], sport, sirq, sdma);
	}
	color = 0x1f;
	gotoxy(12, 14);
	print("%s%s", Line25, Line25);
}

World::World(void) :
	NewGump(NewGumpId(0x2000), 0, 0x116)
{
	f_2e = 0;
	editorItem.referent = 0;
	editorItem.dragged.referent = 0;
	editorItem.none = 1;
	Camera::setCenterOn(1);
	f_43 = 0;
	flags = 0;
	rect = Rect(0, 0, 319, 199);
	f_3a = 0;
	f_3f = 0;
	f_41 = 0;
	registerHotKeys(0xffff, 0);
	refresh();
	target = 0;
	bogus = 0;
	U8MousePointer::pushMode((PointerModes)0, 0);
}

void World::shutdown(void)
{
	Kernel::restart();
	dispatcher->suspend();
}

void World::saveAndExit(void)
{
	musicSlowStop();
	Exit(0);
}

void World::commandPrivate(Event &event)
{
	if (flags & 0x80)
	{
		Item found;
		found.findTarget(event.x, event.y);
		U8MousePointer::popMode();
		if (target)
		{
			*target = found;
			bogus->pop(bogus->result);
			bogus = 0;
			target = 0;
		}
		flags &= ~0x80;
		ContainerGumpBox::targetModeHack = 0;
		return;
	}
	if (event.from->newGumpId.getType() == 0x90)
	{
		f_60 = editorItem.dragged.referent;
		preDragItem(event);
	}
}

void World::commandKeyboard(Event &event)
{
	if ((flags & 0x80) && event.data == 27)
	{
		U8MousePointer::popMode();
		if (target)
		{
			target->referent = 0;
			bogus->pop(bogus->result);
			bogus = 0;
			target = 0;
		}
		flags &= ~0x80;
		ContainerGumpBox::targetModeHack = 0;
		return;
	}
	if (flags)
		return;
	switch (event.data)
	{
	case 0x16:
		{
			unsigned long total, largest;
			unsigned short selectors;
			char message[256];
			ProtMemoryManager::getMemStatus(total, largest, selectors);
			sprintf(message, "Ultima 8 %s\nVersion %s%s\n%s %s\n---\n(%d,%d,%d,%d)\n", Flavor, Version, avatar.cheater ? ".1" : "  ",
				DateOfBuild, TimeOfBuild, avatar.getX(), avatar.getY(), avatar.getZ(), ItemCache::mapIn);
			Dispatch(new MessageGump(NewGumpId(0), 0, message, GumpColorMap[5]));
		}
		return;
	case 0xc:
		avatar.f_48 = 1 - avatar.f_48;
		return;
	case 6:
		Camera::skipFrames = 1 - Camera::skipFrames;
		return;
	case 0x12d:
		if (Dispatch(new U8YesNoGump(NewGumpId(0), 0, "Are you sure?")))
			saveAndExit();
		return;
	case 't':
		Timer *timer;
		timer = (Timer *)Kernel::findValidProcess(0, (ProcessType)0x201);
		if (timer->running)
			timer->stop();
		else
			timer->start();
		return;
	case 0x143:
		if (avatar.canCheat)
			Dispatch(new CheatWindow(0));
		return;
	case 'O':
	case 'o':
		if (!avatar.inStasis)
			Dispatch(new MainMenu(0, 1));
		return;
	case 'Z':
	case 'z':
		if (!avatar.inStasis && !avatar.isGumpOpen())
			avatar.use();
		return;
	case 'I':
	case 'i':
		if (!avatar.inStasis)
		{
			Item backpack(avatar.getEquip(6));
			if (!backpack.isGumpOpen())
				backpack.use();
		}
		return;
	case 'S':
	case 's':
		if (avatar.inStasis && avatar.canCheat)
		{
			avatar.inStasis = 0;
			Camera::setCenterOn(avatar.referent);
			FadeFromBlack();
		}
		return;
	case '[':
		{
			static int npcNum = 0;
			if (npcNum > 23)
				npcNum = 0;
			new Scheduler(npcNum++);
		}
		return;
	case 'C':
	case 'c':
		if (!avatar.inStasis)
		{
			if (avatar.isInCombat())
				avatar.clrInCombat();
			else
				avatar.setInCombat();
		}
		return;
	case 'B':
	case 'b':
	case 'K':
	case 'k':
	case 'R':
	case 'r':
		if (!avatar.inStasis)
		{
			RecursiveContainerItemFinder finder(avatar.referent);
			Boolean useIt = FALSE;
			while (finder.found())
			{
				switch (toupper(event.data))
				{
				case 'B':
					useIt = finder.Item::getType() == 0x216;
					break;
				case 'K':
					useIt = finder.Item::getType() == 0x4f;
					break;
				case 'R':
					useIt = finder.Item::getType() == 0x341;
					break;
				}
				if (useIt)
				{
					finder.use();
					return;
				}
				finder.findNext();
			}
		}
		return;
	case 0x141:
		U8MousePointer::pushMode((PointerModes)0x23, 1);
		save(999);
		if (musicOn)
		{
			musicVolume(90, 350);
			restoreMusic();
		}
		U8MousePointer::popMode();
		return;
	case 0x142:
		if (FileExists(fileSpec(0, SavedgamesDir, "u8save", "999")))
		{
			U8MousePointer::pushMode((PointerModes)0x23, 1);
			load(999);
			if (musicOn)
			{
				musicVolume(90, 350);
				restoreMusic();
			}
			U8MousePointer::popMode();
		}
		return;
	case 0x123:
		int hand;
		dispatcher->switchHandedness();
		hand = dispatcher->isMouseRightHanded() ? 0 : 3;
		Kernel::resetRef(avatar.referent, (ProcessType)0x200);
		avatar.bark(mouseBark[avatar.language + hand]);
		return;
	default:
		if (flags & 0x40)
		{
			flags &= ~0x40;
			U8MousePointer::popMode();
		}
		if (avatar.inStasis)
			return;
		switch (event.data)
		{
		case 27:
			if (!avatar.inStasis)
			{
				Dispatch(new MainMenu(0, 0));
				refresh();
			}
			return;
		case 8:
			{
				NewGump *gump = 0;
				while (newGumpList.hasType(0x43, gump))
				{
					if (((ItemRelativeGump *)gump)->f_52 == 0)
					{
						Item(((ItemRelativeGump *)gump)->item).use();
						gump = 0;
					}
				}
				refresh();
			}
			return;
		}
		if (avatar.canCheat)
			Dispatch(new CheatWindow(event.data));
	}
}

void World::commandMouseMovement(Event &event)
{
	if (flags & 0x80)
		return;
	if (avatar.inStasis)
		return;
	if (flags & 0x40)
		return;
	if (flags & 4)
	{
		flags &= ~4;
		preDragItem(event);
		return;
	}
	if (flags & 8)
	{
		flags &= ~8;
		if (!(event.f_0e & 2))
			f_1d &= ~9L;
		avatar.doAnim(59, 8, 10000, 0);
		avatar.doAnim(60, 8, 10000, 0);
		avatar.doAnim(avatar.isInCombat() ? 15 : 2, 8, 10000, 0);
		return;
	}
	if (flags & 1)
	{
		commandNoEvents(event);
		return;
	}
	dragItem(event);
}

void World::commandMouseLeft(Event &event)
{
	static Item clicked;
	if (flags & 0x40)
	{
		flags &= ~0x40;
		U8MousePointer::popMode();
	}
	if (avatar.inStasis && !(flags & 0x80))
		return;
	unsigned char both = (event.f_0e & 1) && (event.f_0e & 2);
	if (event.isRelease())
	{
		if (flags & 4)
		{
			clicked.look();
			f_1d &= ~9L;
			clicked.set(0);
			flags &= ~4;
			return;
		}
		if (flags & 2)
		{
			postDragItem(event);
			return;
		}
		if (flags & 0x20)
		{
			clrExclusive();
			U8MousePointer::popMode();
			flags &= ~0x20;
			return;
		}
		if (flags & 8)
		{
			flags &= ~8;
			if (!(event.f_0e & 2))
				f_1d &= ~9L;
			avatar.doAnim(59, 8, 10000, 0);
			avatar.doAnim(60, 8, 10000, 0);
			avatar.doAnim(avatar.isInCombat() ? 15 : 2, 8, 10000, 0);
			return;
		}
		if (flags & 0x100)
		{
			flags &= ~0x100;
			avatar.doAnim(60, 8, 10000, 0);
			avatar.doAnim(avatar.isInCombat() ? 15 : 2, 8, 10000, 0);
			f_1d |= 4;
		}
		return;
	}
	if (event.isDouble() && avatar.isInCombat() || event.isSingle())
	{
		if (flags & 0x80)
		{
			clicked.findTarget(event.x, event.y);
			U8MousePointer::popMode();
			if (target)
			{
				*target = clicked;
				bogus->pop(bogus->result);
				bogus = 0;
				target = 0;
			}
			flags &= ~0x80;
			ContainerGumpBox::targetModeHack = 0;
			return;
		}
		if (avatar.isInCombat())
		{
			char dir = mouseDir();
			if (event.isDouble())
			{
				if (avatar.getDir() == dir)
				{
					avatarSwing(1);
					return;
				}
				if (!avatar.isBusy())
				{
					if (!turnToFace(avatar.referent, avatar.getDir(), dir, 0))
					{
						if (avatar.getDir() != dir)
							avatar.doAnim(15, dir, 10000, 0);
						avatarSwing(1);
					}
				}
				return;
			}
			if (avatar.getDir() == dir)
			{
				flags |= 8;
				f_1d |= 9;
				return;
			}
			if (!avatar.isBusy())
				turnToFace(avatar.referent, avatar.getDir(), dir, 1);
			return;
		}
		if (both)
		{
			if (avatar.getLastAnimSet() == 0x2d)
			{
				Kernel::resetRef(avatar.referent, (ProcessType)0xf0);
				avatar.doAnim(0x2e, 8, 0, 0);
				return;
			}
			if (avatar.getLastAnimSet() == 2)
			{
				Point p(event.x, event.y);
				jumpAvatar(mouseDir(), mouseErection(), &p);
				return;
			}
			int last = avatar.getLastAnimSet();
			if (last == 1 || last == 10)
				avatar.doAnim(10, 8, 1, 0);
			return;
		}
		clicked.findTarget(event.x, event.y);
		if (clicked.isValid())
		{
			flags |= 4;
			f_1d |= 9;
			prePreDragItem(event);
		}
		return;
	}
	if (event.isDouble() && !both)
	{
		if (clicked.findTarget(event.x, event.y))
		{
			if (avatar.f_2b || clicked.isNpc() || avatar.canReach(clicked.referent, 0x80, 0, 0))
			{
				if (!(flags & 0x80) || clicked.getCapacity() > 0)
				{
					clicked.use();
					return;
				}
			}
		}
		U8MousePointer::pushMode((PointerModes)0x28, 1);
		flags |= 0x40;
	}
}

void World::commandMouseBoth(Event &event)
{
	if (flags & 0x80)
		return;
	if (flags & 0x40)
	{
		flags &= ~0x40;
		U8MousePointer::popMode();
	}
	if (avatar.inStasis)
		return;
	if (event.isSingle() && !avatar.isInCombat())
	{
		Point p(event.x, event.y);
		jumpAvatar(mouseDir(), mouseErection(), &p);
	}
	else if (event.isRelease())
	{
		commandMouseLeft(event);
		commandMouseRight(event);
	}
}

void World::commandMouseRight(Event &event)
{
	if (flags & 0x80)
		return;
	if (flags & 0x40)
	{
		flags &= ~0x40;
		U8MousePointer::popMode();
	}
	if (avatar.inStasis && !(event.isRelease() && (flags & 1)))
		return;
	unsigned char both = (event.f_0e & 1) && (event.f_0e & 2);
	if (event.isRelease())
	{
		flags &= ~1;
		f_1d &= ~9L;
		if (avatar.isInCombat() || avatar.isCombatPending())
		{
			if (avatar.isCombatPending())
			{
				avatar.doAnim(0, 8, 10000, 0);
				avatar.setInCombat();
				Npc::npcData[avatar.referent].combatPending = 0;
			}
			avatar.doAnim(15, 8, 10000, 0);
			return;
		}
		if (avatar.getLastAnimSet() == 0x2d)
		{
			avatar.doAnim(2, 8, 10000, 1);
			avatar.hurl(0, 0, 0, 2);
			return;
		}
		if (avatar.getLastAnimSet() == 1)
			avatar.doAnim(0, 8, 10000, 0);
		avatar.doAnim(2, 8, 10000, 0);
		return;
	}
	if (both && !avatar.isInCombat())
	{
		if (avatar.getLastAnimSet() == 2)
		{
			jumpAvatar(mouseDir(), mouseErection(), 0);
			return;
		}
		if (avatar.getLastAnimSet() == 1)
			avatar.doAnim(10, 8, 1, 0);
		return;
	}
	if (event.isSingle())
	{
		flags |= 1;
		f_1d |= 9;
		if (avatar.isInCombat())
		{
			commandNoEvents(event);
			return;
		}
		char dir = mouseDir();
		char erection = mouseErection();
		if (avatar.getDir() == dir)
		{
			if (erection == 1)
				carefulStepAvatar(dir, erection);
			else
				avatar.doAnim(0, dir, 0, 0);
			return;
		}
		if (!avatar.isBusy())
			turnToFace(avatar.referent, avatar.getDir(), dir, 1);
		return;
	}
	if (event.isDouble())
	{
		Item found;
		found.findTarget(event.x, event.y);
		if (found.isAvatar())
		{
			if (avatar.isInCombat())
				avatar.clrInCombat();
			else
				avatar.setInCombat();
			return;
		}
		if (avatar.isInCombat())
		{
			char dir = mouseDir();
			if (avatar.getDir() == dir)
			{
				avatarSwing(0);
				return;
			}
			if (!avatar.isBusy())
				turnToFace(avatar.referent, avatar.getDir(), dir, 1);
		}
	}
}

void World::commandNoEvents(Event &event)
{
	if (avatar.inStasis)
		return;
	if (flags & 0x40)
		return;
	if (flags & 1)
	{
		int erection = mouseErection();
		int dir = mouseDir();
		if (!avatar.isBusy())
		{
			if (avatar.isInCombat())
			{
				if (erection == 3)
				{
					Npc::npcData[avatar.referent].combatPending = 1;
					avatar.clrInCombat();
					playCombatMusic(0x6e);
					return;
				}
				char back = (dir + 4) & 7;
				if (avatar.getDir() == back)
				{
					if (!turnToFace(avatar.referent, avatar.getDir(), back, 0))
						avatar.doAnim(9, back, 0, 0);
					return;
				}
				if (!turnToFace(avatar.referent, avatar.getDir(), dir, 0))
					avatar.doAnim(8, dir, 0, 0);
				return;
			}
			unsigned char turning = turnToFace(avatar.referent, avatar.getDir(), dir, 0);
			if (!turning)
			{
				if (erection == 1)
					carefulStepAvatar(dir, erection);
				else if (erection == 2)
					avatar.doAnim(0, dir, 0, 0);
				else if (erection == 3)
				{
					int last = avatar.getLastAnimSet();
					if (last == 1 || last == 10 || last == 0)
						avatar.doAnim(1, dir, 0, 0);
					else
						avatar.doAnim(0, dir, 0, 0);
				}
			}
		}
	}
	else if (flags & 4)
	{
		flags &= ~4;
		preDragItem(event);
	}
	else if (flags & 8)
	{
		flags &= ~8;
		f_1d &= ~9L;
		if (avatar.doAnim(59, 8, 0, 0))
		{
			flags |= 0x100;
			f_1d &= ~4L;
		}
	}
}

void World::prePreDragItem(Event &event)
{
	unsigned short found = DisplayList::find_target(event.x, event.y);
	DL_ItemNode *node = (DL_ItemNode *)NODE(found);
	if (found && node->nodetype() == 2)
	{
		Point p(node->screenX, node->screenY);
		p.f_00 -= Camera::_h;
		p.f_02 -= Camera::_v;
		editorItem.offset.f_00 = event.x - p.f_00;
		editorItem.offset.f_02 = event.y - p.f_02;
		f_60 = node->referent;
	}
}

void World::preDragItem(Event &event)
{
	if (f_60)
	{
		Item item(f_60);
		Item newItem;
		newItem.set(0);
		if (item.isNpc())
		{
			Npc npc(0);
			npc.simpleCreate(item.getType(), item.getFrame());
			newItem = itemOf(npc.referent);
		}
		if (!newItem.isValid())
		{
			newItem.create(item.getType(), item.getFrame());
			newItem.setQuantity(1);
			newItem.setFrame(item.getFrame());
		}
		if (newItem.isValid())
		{
			dragWeightHack = f_60;
			newItem.orStatus(0x94);
			if (item.getContents())
				newItem.setQ(item.getQ());
			if (item.getStatus() & FLIPPED)
				newItem.orStatus(FLIPPED);
			if (item.getStatus() & CONTAINED)
			{
				newItem.pop(item.getContainer());
				newItem.setGumpXY(item.getGumpX(), item.getGumpY());
			}
			else
				newItem.pop(item.getLoc());
			Npc npc(item.referent);
			if (avatar.f_2b || !item.isNpc() || item.isNpc() && npc.isDead())
			{
				item.push();
				TypeFlag flag = GlobalTypes.typeFlags[item.getType()];
				if (avatar.f_2b || !flag.fixed && flag.weight > 0 && avatar.canReach(newItem.referent, 0x80) && !newItem.isUnder(avatar.referent))
				{
					item.pop();
					U8MousePointer::pushMode((PointerModes)0, 1);
					editorItem.startDrag(newItem.referent);
					startDragging(event);
					return;
				}
				item.pop();
			}
			newItem.setQ(0);
			newItem.destroy();
		}
	}
	flags |= 0x20;
	U8MousePointer::pushMode((PointerModes)0x28, 1);
	setExclusive(this);
}

void World::startDragging(Event &event)
{
	if (editorItem.dragged.isValid())
	{
		editorItem.dragged.grab();
		setExclusive(this);
		f_1d |= 8;
		flags |= 2;
		dragStart = editorItem.dragged.getLoc();
		dragLast = dragStart;
		event.x = MouseDevice::get_last_x() >> 1;
		event.y = MouseDevice::get_last_y();
		commandMouseMovement(event);
	}
}

void World::postDragItem(Event &)
{
	int dx, dy, dir;
	Item item, dragged;
	TypeFlag flag;
	Boolean noRoom = FALSE;
	Boolean wasContained = FALSE;
	Boolean owned = FALSE;
	WorldPoint origLoc;
	Referent origContainer;
	int gumpX, gumpY;
	Referent combineWith = 0;
	if (flags & 2)
	{
		clrExclusive();
		f_1d &= ~8L;
		flags &= ~2;
		item = Item(f_60);
		if (item.getStatus() & CONTAINED)
		{
			wasContained = TRUE;
			origContainer = item.getContainer();
			gumpX = item.getGumpX();
			gumpY = item.getGumpY();
		}
		else
			origLoc = item.getLoc();
		if (U8MousePointer::modes[U8MousePointer::sp] != 0x28)
		{
			U8MousePointer::popMode();
			dragged = editorItem.dragged;
			flag = GlobalTypes.typeFlags[item.getType()];
			if ((flag.family == QUAN_FAMILY || flag.family == REAGENT_FAMILY) && item.getQuantity() > 1)
			{
				int amount = item.getQuantity();
				if (dragged.getStatus() & CONTAINED)
				{
					Container box(dragged.getContainer());
					if (item.getContainer() != box.referent)
					{
						int room = box.getCapacity() - box.getContentsVolume();
						int fit;
						if (flag.family == QUAN_FAMILY)
							fit = room * 100 / flag.volume;
						else
							fit = room * 10 / flag.volume;
						if (fit < amount)
							amount = fit;
						if (box.isInNpc())
						{
							Npc npc(box.getContainer());
							while (!npc.isNpc())
								npc = Npc(npc.getContainer());
							room = npc.getStr() * 40 - Container(npc.referent).getContentsWeight();
							if (flag.family == QUAN_FAMILY)
								fit = room * 10 / flag.weight;
							else
								fit = room / flag.weight;
							if (fit < amount)
								amount = fit;
						}
					}
				}
				amount = Dispatch(new SliderGump(0, 0, 0, 1, amount, 1, amount));
				if (item.getQuantity() > amount && amount > 0)
				{
					combineWith = item.referent;
					item.setQuantity(item.getQuantity() - amount);
					Item newItem;
					newItem.create(item.getType(), item.getFrame());
					newItem.setQuantity(amount);
					newItem.pop(item.getLoc());
					item = newItem;
				}
			}
			if (dragged.getStatus() & CONTAINED)
			{
				item.push();
				item.popToEnd(dragged.getContainer());
				if (item.getRootContainer() == avatar.referent)
					owned = TRUE;
				item.setGumpXY(dragged.getGumpX(), dragged.getGumpY());
				if (dragged.getStatus() & 0x200)
				{
					int slot = avatar.findEquip(dragged.referent);
					if (slot < 8)
					{
						avatar.unEquip(slot);
						avatar.setEquip(slot, f_60);
					}
					dragged.andStatus(~0x200);
				}
			}
			else
				item.move(dragged.getLoc());
			editorItem.dragged = item;
			dragged.setQ(0);
			dragged.destroy();
			if (!(item.getStatus() & CONTAINED) && avatar.getRange(item.referent) > 0x80)
			{
				dx = abs(dragStart.x - dragLast.x);
				dy = abs(dragStart.y - dragLast.y);
				if (dx > dy)
					dir = dragLast.x > dragStart.x ? 2 : 6;
				else
					dir = dragLast.y > dragStart.y ? 4 : 0;
				int dex = avatar.getDex();
				if (dex < 30)
				{
					int spread = (30 - dex) * 4;
					dragLast.x += urandom(spread);
					dragLast.x -= urandom(spread);
					dragLast.y += urandom(spread);
					dragLast.y -= urandom(spread);
				}
				avatar.accumulateDexterity(1);
				avatar.doAnim(2, dir, 0, 0);
				if (!avatar.f_2b)
				{
					item.move(dragStart);
					MissileTracker path(dragStart, dragLast, f_4e, 4);
					path.fire(item.referent);
				}
				editorItem.dragged.referent = 0;
			}
			else
				editorItem.stopDrag();
			if ((flags & 0x10) && !(item.getStatus() & CONTAINED))
			{
				dragWeightHack = 0;
				Item backpack(avatar.getEquip(6));
				if (!backpack.isValid())
				{
					backpack.create(0x211, 0);
					if (!backpack.isValid())
						halt("WORLD.C", 3024);	// __LINE__
					backpack.popToEnd(avatar.referent);
					avatar.setEquip(6, backpack.referent);
					backpack.equip();
				}
				TypeFlag itemFlag = GlobalTypes.typeFlags[item.getType()];
				if (itemFlag.equipType == 0)
				{
					Container pack(avatar.getEquip(6));
					if (pack.can_hold(item.referent))
					{
						item.push();
						item.popToEnd(pack.referent);
						item.setGumpXY(0, 0);
					}
					else
						noRoom = TRUE;
				}
				else if (!Container(avatar.referent).can_hold(item.referent))
					noRoom = TRUE;
				else
				{
					for (int slot = 0; slot < 6; slot++)
					{
						if (slot + 1 == itemFlag.equipType)
						{
							if (!avatar.setEquip(slot, item.referent))
							{
								Container pack(avatar.getEquip(6));
								if (pack.can_hold(item.referent))
								{
									item.push();
									item.popToEnd(pack.referent);
									item.setGumpXY(0, 0);
								}
								else
									noRoom = TRUE;
							}
							else
								item.equip();
							break;
						}
					}
				}
				flags &= ~0x10;
				owned = TRUE;
			}
			if (noRoom)
			{
				if (wasContained)
				{
					item.move(origContainer);
					item.setGumpXY(gumpX, gumpY);
				}
				else
					item.move(origLoc);
				if (combineWith)
					((Combinable &)Item(item.referent)).combine(combineWith);
			}
			if (!(item.getStatus() & OWNED))
			{
				if (owned)
					item.orStatus(OWNED);
				AreaItemFinder finder(avatar.getX(), avatar.getY(), 640, SearchCriteria(0x23, 0x25, 0x100, 0x3c, 0x24), 0xffff);
				while (finder.found())
				{
					finder.AvatarStoleSomething(item.referent);
					finder.findNext();
				}
			}
		}
		else
		{
			U8MousePointer::popMode();
			editorItem.dragged.setQ(0);
			editorItem.dragged.destroy();
			editorItem.dragged.referent = 0;
			flags &= ~0x10;
			if (item.getStatus() & CONTAINED)
			{
				item.getContainer();
				item.setGumpXY(gumpX, gumpY);
				if (item.getContainer() == avatar.referent)
				{
					TypeFlag itemFlag = GlobalTypes.typeFlags[item.getType()];
					if (itemFlag.equipType == 0)
						halt("WORLD.C", 3144);	// __LINE__
					avatar.unEquip(itemFlag.equipType - 1);
					avatar.setEquip(itemFlag.equipType - 1, item.referent);
					item.orStatus(0x200);
					item.equip();
				}
			}
		}
		f_60 = 0;
		dragWeightHack = 0;
	}
}

void World::dragItem(Event &event)
{
	if (!(flags & 2))
		return;
	flags &= ~0x10;
	if (editorItem.dragged.isValid())
	{
		Item(f_60).push();
		Event probe((EventType)2, event.x, event.y);
		NewGump *gump = Dispatcher::base[Dispatcher::baseSP]->findTargetedGump(probe);
		avatar.unEquip(7);
		int mode;
		if (gump && gump->newGumpId.getType() == 0x90)
		{
			mode = 0x28;
			if (avatar.f_2b || avatar.canReach(((ContainerGumpBox *)gump)->referent, 0x80))
			{
				if (editorItem.move(((ContainerGumpBox *)gump)->referent, event.x - editorItem.offset.f_00 - gump->get_dx(), event.y - editorItem.offset.f_02 - gump->get_dy()))
				{
					mode = 0x22;
					((ContainerGumpBox *)gump)->findEquipSlot(editorItem.dragged.referent);
					gump->refresh();
					editorItem.dropX = event.x;
					editorItem.dropY = event.y;
				}
			}
		}
		else
		{
			Boolean flipped = (editorItem.dragged.getStatus() & FLIPPED) != 0;
			WorldPoint oldStart = dragStart;
			WorldPoint oldLast = dragLast;
			editorItem.dragged.push();
			editorItem.calculateLocation(event.x, event.y, dragLast);
			editorItem.dragged.pop();
			if (avatar.getRange(editorItem.dragged.getType(), dragLast.x, dragLast.y) > 0x80)
			{
				Item dragged = editorItem.dragged;
				int dx, dy, ax, ay, az, ix, iy, iz;
				TypeFlag avatarFlag, itemFlag;
				mode = 0x22;
				dragStart = avatar.getLoc();
				dx = abs(dragStart.x - dragLast.x);
				dy = abs(dragStart.y - dragLast.y);
				avatarFlag = GlobalTypes.typeFlags[avatar.getType()];
				avatarFlag.getWorldSize(ax, ay, az);
				itemFlag = GlobalTypes.typeFlags[dragged.getType()];
				itemFlag.getWorldSize(ix, iy, iz);
				if (dx > dy)
				{
					if (dragLast.x > dragStart.x)
						dragStart.x += ix;
					else
						dragStart.x -= ax;
					dragStart.y += (iy - ay) >> 1;
				}
				else
				{
					if (dragLast.y > dragStart.y)
						dragStart.y += iy;
					else
						dragStart.y -= ay;
					dragStart.x += (ix - ax) >> 1;
				}
				dragStart.z += az >> 1;
				editorItem.dragged.push();
				Item missile;
				MotionTracker tracker;
				if (avatar.f_2b || tracker.can_create(editorItem.dragged.getType(), dragStart, flipped))
				{
					missile.create(editorItem.dragged.getType(), 0);
					missile.orStatus(0x80);
					if (flipped)
						missile.orStatus(FLIPPED);
					missile.pop(dragStart);
					f_4e = 0x40 - editorItem.dragged.getWeightIncludingContents() + avatar.getStr();
					if (f_4e < 1)
						f_4e = 1;
					MissileTracker path(dragStart, dragLast, f_4e, 4);
					if (!avatar.f_2b && (!path.inRange || !path.isPathClear(missile.referent, dragLast)))
					{
						mode = 0x28;
						missile.destroy();
						editorItem.dragged.pop();
						dragStart = oldStart;
						dragLast = oldLast;
					}
					else
					{
						missile.destroy();
						editorItem.dragged.pop();
						editorItem.move(event.x, event.y);
					}
				}
				else
				{
					mode = 0x28;
					editorItem.dragged.pop();
					dragStart = oldStart;
					dragLast = oldLast;
				}
			}
			else
			{
				mode = 0;
				if ((avatar.f_2b || avatar.canReach(editorItem.dragged.referent, 0x80, &dragLast, 1)) && editorItem.move(event.x, event.y))
				{
					if (editorItem.f_1c == avatar.referent)
						flags |= 0x10;
					dragStart = editorItem.dragged.getLoc();
					dragLast = dragStart;
				}
				else
					mode = 0x28;
			}
			if (U8MousePointer::modes[U8MousePointer::sp] != mode)
			{
				U8MousePointer::popMode();
				U8MousePointer::pushMode((PointerModes)mode, 1);
			}
		}
		if (U8MousePointer::modes[U8MousePointer::sp] != mode)
		{
			U8MousePointer::popMode();
			U8MousePointer::pushMode((PointerModes)mode, 1);
		}
		Item(f_60).pop();
	}
}

int World::getTarget(void)
{
	if (target)
		return 0;
	flags |= 0x80;
	ContainerGumpBox::targetModeHack = 1;
	bogus = new BogusProcess;
	target = (Item *)&bogus->result;
	U8MousePointer::pushMode((PointerModes)0x22, 1);
	return bogus->pid;
}

int World::getTargetCoords(void)
{
	trace((TraceLevel)50, "Why is this here?");
	if (targetGump)
		return 0;
	savedF1d = f_1d;
	f_1d = 0x10;
	bogus = new BogusProcess;
	targetGump = new TargetGump((unsigned short *)&bogus->result, (unsigned short *)&bogus->result + 1, this);
	return bogus->pid;
}

void targetCoords(void)
{
	((World *)Dispatcher::base[Dispatcher::baseSP])->getTargetCoords();
}

int target(void)
{
	if (inGameMode)
		return ((World *)Dispatcher::base[Dispatcher::baseSP])->getTarget();
	return 0;
}

void drawDiamond(unsigned short x, unsigned short y, unsigned char z, short xd, short yd, short color)
{
	short sx, sy;
	WorldToScreenCoords(x, y, sx, sy);
	sy -= z;
	short x1 = sx + yd * 8;
	short y1 = sy - yd * 4;
	DrawLine(GlobalVport::main_screen, sx, sy, x1, y1, color);
	short x2 = x1 - xd * 8;
	short y2 = y1 - xd * 4;
	DrawLine(GlobalVport::main_screen, x1, y1, x2, y2, color);
	x1 = x2 - yd * 8;
	y1 = y2 + yd * 4;
	DrawLine(GlobalVport::main_screen, x2, y2, x1, y1, color);
	DrawLine(GlobalVport::main_screen, x1, y1, sx, sy, color);
}

void drawShadowRect(Item item)
{
	short xd, yd, zd;
	item.getFootpad(xd, yd, zd);
	drawDiamond(item.getX(), item.getY(), 0, xd, yd, 15);
}

void drawFootpadExtents(Item item)
{
	short xd, yd, zd;
	item.getFootpad(xd, yd, zd);
	drawDiamond(item.getX(), item.getY(), item.getZ(), xd, yd, 25);
	drawDiamond(item.getX(), item.getY(), item.getZ() + zd * 4, xd, yd, 25);
}

void BogusProcess::process(void)
{
}

inline void TextCheckMenuItemGump::addPullDown(PullDownGump *, unsigned char)
{
}

unsigned char searchBuffer[50];
