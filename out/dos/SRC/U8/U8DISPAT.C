// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: U8DISPAT.C

#include "ERROR.H"
#include "CFILE.H"
#include "CAFONT.H"
#include "CSHAPE.H"
#include "FLEXHAND.H"
#include "BASECAM.H"
#include "CAMERA.H"
#include "U8POINT.H"
#include "WORLD.H"
#include "U8GMPSHP.H"
#include "SBUTTON.H"
#include "MENU.H"
#include "TARGGUMP.H"
#include "U8DISPAT.H"

// Inline in the shared headers when this file was built.
inline void nameProcess(unsigned pid, char *name) { ((char **)Kernel::idStringList)[pid] = name; }
inline Point::Point(void) {}
inline void Point::set(short x, short y) { f_00 = x; f_02 = y; }
inline void Rect::set(short x1, short y1, short x2, short y2) { Point::set(x1, y1); f_04 = x2; f_06 = y2; }
inline Rect::Rect(short x1, short y1, short x2, short y2) { set(x1, y1, x2, y2); }
inline BaseCamera::BaseCamera(Rect graphicsRect, Rect textRect)
{
	theBaseCamera = this;
	graphics = graphicsRect;
	text = textRect;
	screen = graphicsRect;
}
inline Camera::Camera(void) :
	BaseCamera(Rect(0, 0, 319, 199), Rect(0, 0, 79, 49))
{
	theCamera = this;
}
inline TMMousePointer::~TMMousePointer(void) { if (pointer) delete pointer; }
inline U8GumpShapeDrawer::~U8GumpShapeDrawer(void) { if (rects) delete rects; }

unsigned char U8GumpColorMap[32] = {0, 149, 24, 22, 115, 20, 38, 255, 15, 51, 88, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
U8Dispatcher *theU8Dispatcher = 0;

U8Dispatcher::U8Dispatcher(int size) :
	Dispatcher(size)
{
	theU8Dispatcher = this;
	GumpColorMap = (char *)U8GumpColorMap;
	nameProcess(pid, "Dispatcher");
	setProcessType((ProcessType)0x244);
	setDaemon();
	font = new CacheFont((FontType)1);
	drawer = new U8GumpShapeDrawer;
	theCamera = new Camera;
	new U8MousePointer(160, 114);
	if (!font || !drawer || !theCamera || !theU8Mouse)
		outOfMemory(__FILE__, 89);	// __LINE__
	GlobalFont::setMainFont(font);
	GumpShape::setDrawer(drawer);
}

void U8Dispatcher::process(void)
{
	Dispatcher::process();
}

void U8Dispatcher::uninit(void)
{
	if (theU8Mouse)
	{
		delete theU8Mouse;
		theU8Mouse = 0;
	}
	if (drawer)
	{
		delete drawer;
		drawer = 0;
	}
	if (font)
	{
		delete font;
		font = 0;
	}
	if (theCamera)
	{
		delete theCamera;
		theCamera = 0;
	}
	Dispatcher::uninit();
}

void U8Dispatcher::freeMemory(void)
{
	Process::freeMemory();
	theU8Dispatcher = 0;
}

void U8Dispatcher::postLoad(void)
{
	theU8Dispatcher = this;
	World *world = (World *)Dispatcher::base[0];
	world->flags = 0;
}

void U8Dispatcher::load(BaseFile *file)
{
	Process::load(file);
	U8MousePointer::load(file);
}

void U8Dispatcher::save(BaseFile *file)
{
	Process::save(file);
	U8MousePointer::save(file);
}

inline void *U8GumpShapeDrawer::get(int shape)
{
	return theGumpShapeHandler->get(theGumpShapeHandler->startHandle + shape);
}
