// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: SYSTEM\LSYSTEM.C

#include "AIL.H"
#include "CDESC.H"
#include "CICRTC.H"
#include "KTIMER.H"
#include "INIT.H"
#include "LSYSTEM.H"

// Inline in the shared headers when this file was built.
inline GraphicYtable::GraphicYtable(void) { f_04 = 2; index = 0; }
inline Vport::Vport(void) { seg = 0; f_0f = 2; f_10 = 0; }
inline Vport::~Vport(void) { free(); }
inline Point::Point(void) {}
inline Rect::Rect(void) {}
inline void Point::set(short x, short y) { f_00 = x; f_02 = y; }
inline void Rect::set(short x1, short y1, short x2, short y2) { Point::set(x1, y1); f_04 = x2; f_06 = y2; }

// A blank message for the sound drivers to show on shutdown.
static char signoff[2] = "";
unsigned char RawSystem::active = 0;
Vport Vscreen;

void LargeGameSystem::init(void)
{
	if (!active)
	{
		initMemory();
		initRemember();
		initCrtc();
		initVideoMode();
		initMainScreen();
		initVirtualScreen();
		initMouse();
		initTimer();
		active = 1;
	}
}

void LargeGameSystem::initMouse(void)
{
}

void LargeGameSystem::remove(void)
{
	free();
}

void LargeGameSystem::initTimer(void)
{
	AIL_startup();
	ailTimer = new AilTimer;
}

void LargeGameSystem::initMemory(void)
{
}

void LargeGameSystem::initRemember(void)
{
}

// Saves the text mode for exit, then switches to 320x200x256.
void LargeGameSystem::initVideoMode(void)
{
	VideoMode mode;

	videoRestore.grab();
	mode.mode = 0x13;
	mode.activate();
}

void LargeGameSystem::initCrtc(void)
{
	GetBiosSegment();
	InitializeCrtc();
}

void LargeGameSystem::initMainScreen(void)
{
	SetVgaScreenDescriptor();
	mainScreen.create_main_screen();
}

// An off-screen buffer the size of the screen, made current.
void LargeGameSystem::initVirtualScreen(void)
{
	Vscreen.rect.set(0, 0, 319, 199);
	Vscreen.alloc(0);
	Vscreen.clear(0);
	GlobalVport::set(&Vscreen);
}

void LargeGameSystem::free(void)
{
	if (active)
	{
		active = 0;
		AIL_shutdown(signoff);
	}
}

void LargeGameSystem::exit_code(void)
{
	free();
}
