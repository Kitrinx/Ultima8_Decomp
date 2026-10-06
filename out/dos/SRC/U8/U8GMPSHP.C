// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: U8GMPSHP.C

#include "CRECT.H"
#include "CGLOBVP.H"
#include "FVINFO.H"
#include "FLEXHAND.H"
#include "CFILE.H"
#include "FILESPEC.H"
#include "SCRATCHM.H"
#include "CEXIT.H"
#include "U8GMPSHP.H"

// Inline in the shared headers when this file was built.
inline Point::Point(void) {}
inline Point::Point(short x, short y) { f_00 = x; f_02 = y; }
inline Rect::Rect(void) {}
inline unsigned char BaseFile::is_valid(void) { return handle != -1; }
inline BaseFile::~BaseFile(void) { if (is_valid()) close(); }

extern "C" int Collision(void *, Point, Point);

FlexShapeHandler *U8GumpShapeDrawer::theGumpShapeHandler = 0;
Rect *U8GumpShapeDrawer::rects;

U8GumpShapeDrawer::U8GumpShapeDrawer(void)
{
	theGumpShapeHandler = new FlexShapeHandler(75, (CacheNodeType)3, fileSpec(0, StaticDir, gumpsFileName, 0));
	if (rects == 0)
	{
		rects = new Rect[75];
		if (rects == 0)
			outOfMemory(__FILE__, 77);	// __LINE__ in the original file
		BaseFile file(fileSpec(0, StaticDir, "gumpage.dat", 0), (OpenMode)0);
		file.read((char *)rects, 75 * sizeof(Rect), -1);
		file.close();
	}
	if (theGumpShapeHandler == 0)
		outOfMemory(__FILE__, 86);	// __LINE__ in the original file
}

void U8GumpShapeDrawer::draw(int x, int y, int shape, int frame)
{
	SkipDraw(GlobalVport::global_ptr, x, y, GetShapeAddress(get(shape), frame));
}

unsigned char U8GumpShapeDrawer::collide(short x, short y, int shape, short frame)
{
	return Collision(GetShapeAddress(get(shape), frame), Point(0, 0), Point(x, y));
}

Rect U8GumpShapeDrawer::get_rect(int shape, int frame)
{
	Rect rect;
	short xOffset, yOffset;
	rect.f_00 = 1;
	rect.f_02 = 1;
	GetShapeInfo((char *)get(shape), frame, (short &)rect.f_04, (short &)rect.f_06, xOffset, yOffset);
	rect.moveto(0, 0);
	return rect;
}
