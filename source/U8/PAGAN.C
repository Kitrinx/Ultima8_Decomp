// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: PAGAN.C

#include <conio.h>
#include <dos.h>
#include <string.h>
#include "ERROR.H"
#include "PRIORITY.H"
#include "CAFONT.H"
#include "PAL.H"
#include "CVPORT.H"
#include "CCURRVP.H"
#include "CGLOBVP.H"
#include "CTMPOINT.H"
#include "AMUSIC.H"
#include "GAMETIME.H"
#include "PREAMBLE.H"
#include "U8DISPAT.H"
#include "CAMERA.H"
#include "U8DIR.H"
#include "FILESPEC.H"
#include "SCRATCHM.H"
#include "INIT.H"
#include "WAIT.H"
#include "CLLIST.H"
#include "MSGPROC.H"
#include "PAGAN.H"

// Inline in the shared headers when this file was built.
inline Point::Point(void) {}
inline void Point::set(short x, short y) { f_00 = x; f_02 = y; }
inline void Rect::set(short x1, short y1, short x2, short y2) { Point::set(x1, y1); f_04 = x2; f_06 = y2; }
inline Rect::Rect(void) {}
inline GraphicYtable::GraphicYtable(void) { f_04 = 2; index = 0; }
inline Vport::Vport(void) { seg = 0; f_0f = 2; f_10 = 0; }
inline Vport::~Vport(void) { free(); }
inline unsigned char BaseFile::is_valid(void) { return handle != -1; }
inline BaseFile::~BaseFile(void) { if (is_valid()) close(); }
inline LinkedList::LinkedList(void) { head = tail = 0; }
inline LinkedList::~LinkedList(void) { kill(); }
inline unsigned char Msg::scrolling(void) { return !allShown; }

// Kills every process of a type running at the current priority.
inline void killAtPriority(ProcessType type)
{
	PriorityClass samePriority(Kernel::currentPriority | 0x20);
	Kernel::killProcess(0, type, samePriority);
}

Msg *PaganProcess::lnk;

void MakeSemi::process(void)
{
	BaseFile file;
	file.forceOpen(fileSpec(0, StaticDir, quotesFlagName, flagExtension), (OpenMode)2);
	file.write(copyrightStatement, strlen(copyrightStatement) + 1);
	file.close();
	pop(0);
}

void RefreshWorld::process(void)
{
	if (f_2a == 0) {
		(new FadeProcess((PaletteNum)currentPalette, 0x7fff, 1))->then(this);
		theAnimation->intFlags |= 2;
		GlobalVport::main_screen->clear(0);
		if (theCameraProcess) {
			theCameraProcess->flags &= ~PROC_SUSPENDED;
			theCameraProcess->setDaemon();
		}
		Camera::init();
		((BaseCamera *)theCamera)->show();
		TMMousePointer::show();
		f_2a = 1;
	} else {
		if (kbhit() && getch() == 0)
			getch();
		theU8Dispatcher->flags &= ~PROC_SUSPENDED;
		theU8Dispatcher->setDaemon();
		pop(0);
	}
}

CoolKeyboard::CoolKeyboard(void)
{
	Kernel::setIdString(pid, "CoolKeyboard");
	if (kbhit() && getch() == 0)
		getch();
}

void CoolKeyboard::process(void)
{
	if (kbhit() && getch() == 27)
		killAtPriority((ProcessType)6);
}

PaganProcess::PaganProcess(char *filename, unsigned char credits, short font, short musicNum)
{
	theCameraProcess->flags &= ~PROC_SUSPENDED;
	Kernel::setIdString(pid, "PaganScroller");
	setProcessType((ProcessType)0x241);
	music = musicNum;
	Msg::credits = credits;
	Msg::fonttype = font;
	BaseFile file(filename, (OpenMode)0);
	int size = file.size();
	text = new char[size];
	if (text == 0)
		outOfMemory(__FILE__, 149);	// __LINE__
	file.read(text, file.size());
	file.close();

	// Decrypt the text.
	unsigned char key = 0;
	char seed[3];
	seed[0] = 0xff;
	seed[1] = 0xe0;
	seed[2] = 0xaa;
	for (int i = 0; i < size; i++) {
		key += i;
		text[i] ^= key;
		key ^= seed[0];
		seed[0] += seed[1];
		seed[1] *= seed[2];
		seed[2] ^= seed[0];
		seed[2]++;
	}

	messages = new LinkedList;
	background = new char[64000];
	if (background == 0)
		outOfMemory(__FILE__, 186);	// __LINE__
	offset = 0;
	next = text;
	line = next;
	state = -1;
	current = 0;
	f_54 = 1;
	setHertz(20);
	intFlags |= 2;
	connectToRealTime(1);
}

PaganProcess::~PaganProcess(void)
{
	delete messages;
	delete text;
	delete background;
	if (musicOn)
		musicSlowStop();
}

void PaganProcess::process(void)
{
}

void PaganProcess::interruptHandler(unsigned long type, unsigned long)
{
	Rect window;

	if (type == 2) {
		if (state == -1) {
			musicPlay(music);
			GlobalVport::global_ptr->clear(0);
			GlobalVport::main_screen->clear(0);
			memcpy(background, MK_FP(GlobalVport::main_screen->seg, *GlobalVport::main_screen->graphicYtable.index), 64000);
			new FadeProcess((PaletteNum)currentPalette, 0x7fff, 1);
			state = 1;
		}
		if (state == 0 || state == 2) {
			unsigned top = 45 * 320;
			if (*next == '+' && !current->scrolling())
				top = 0;
			memcpy((char *)MK_FP(GlobalVport::global_ptr->seg, *GlobalVport::global_ptr->graphicYtable.index) + top,
				background + top, 64000 - top);
			if (state == 0) {
				if (!current->scrolling()) {
					state = 1;
					if (setupText()) {
						state = 2;
						current->scroll = 1;
					}
				}
			} else {
				current->scroll = 1;
				killAtPriority((ProcessType)6);
				MakeSemi *semi = new MakeSemi;
				Wait *wait = new Wait(150, 0);
				semi->then(wait);
				wait->start();
				return;
			}
		}
		if (state == 1) {
			state = 0;
			current = new Msg(this, line);
			messages->addToTail(current);
		}
		lnk = 0;
		window.set(0, 45, 319, 199);
		Vport port;
		port = *GlobalVport::global_ptr;
		port.rect = window;
		{
			CurrentVport current(&port);
			while (messages->traverse((Link *&)lnk))
				lnk->run();
		}
		BaseCamera::needSlam = 1;
	}
}

// Moves on to the next message; true at the end marker.
unsigned char PaganProcess::setupText(void)
{
	line = next;
	if (text[offset] == '@')
		return 1;
	while (text[offset++] != 0)
		;
	next = text + offset;
	return 0;
}

void PaganProcess::kill(Msg *msg)
{
	messages->destroy(msg, 0);
	lnk = 0;
}

void runCredits(unsigned char fromMenu)
{
	TMMousePointer::hide();
	theAnimation->intFlags &= ~2;
	Kernel::setCurrentPriority(Kernel::currentPriority + 1);
	if (fromMenu) {
		new MainMenuProcess;
		Kernel::setCurrentPriority(Kernel::currentPriority + 1);
	}
	(new FadeProcess(RGB(0, 0, 0), 0x7fff, 1))->then(new RefreshWorld);
	Kernel::setCurrentPriority(Kernel::currentPriority + 1);
	theU8Dispatcher->flags |= PROC_SUSPENDED;
	theU8Dispatcher->flags &= ~PROC_DAEMON;
	new CoolKeyboard;
	languageSensitiveFile(StaticDir, creditDataFile);
	new PaganProcess(fileSpec(0, StaticDir, creditDataFile, 0), 1, 1, 0x33);
	Kernel::setCurrentPriority(Kernel::currentPriority + 1);
	new FadeProcess(RGB(0, 0, 0), 0x7fff, 1);
}

void runQuotes(void)
{
	TMMousePointer::hide();
	theAnimation->intFlags &= ~2;
	Kernel::setCurrentPriority(Kernel::currentPriority + 1);
	new MainMenuProcess;
	Kernel::setCurrentPriority(Kernel::currentPriority + 1);
	(new FadeProcess(RGB(0, 0, 0), 0x7fff, 1))->then(new RefreshWorld);
	Kernel::setCurrentPriority(Kernel::currentPriority + 1);
	theU8Dispatcher->flags |= PROC_SUSPENDED;
	theU8Dispatcher->flags &= ~PROC_DAEMON;
	new CoolKeyboard;
	new PaganProcess(fileSpec(0, StaticDir, quoteDataFile, 0), 0, 1, 0x71);
	Kernel::setCurrentPriority(Kernel::currentPriority + 1);
	new FadeProcess(RGB(0, 0, 0), 0x7fff, 1);
}
