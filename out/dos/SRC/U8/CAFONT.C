// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: CAFONT.C

#include "CVPORT.H"
#include "CGLOBVP.H"
#include "FLEXHAND.H"
#include "FVINFO.H"
#include "SCRATCHM.H"
#include "FILESPEC.H"
#include "CEXIT.H"
#include "CAFONT.H"

FlexShapeHandler *CacheFont::theFontHandler = 0;
unsigned CacheFont::xSpacingArray[16] = {0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned CacheFont::ySpacingArray[16] = {0xffff, 1, 1, 0, 0, 0xffff, 0xffff, 0xffff, 0xffff, 0, 0, 0, 0, 0, 0, 0};

int CacheFont::getBaseLine(void)
{
	short width, height, xOffset, yOffset;
	char *shape = theFontHandler->get(shapeNum);
	GetShapeInfo(shape, 0, width, height, xOffset, yOffset);
	return yOffset;
}

void CacheFont::drawChar(int x, int y, unsigned char c)
{
	SkipDraw(GlobalVport::global_ptr, x, y, GetShapeAddress(theFontHandler->get(shapeNum), c));
}

int CacheFont::height(char c)
{
	short width, height, xOffset, yOffset;
	char *shape = theFontHandler->get(shapeNum);
	GetShapeInfo(shape, (unsigned char)c, width, height, xOffset, yOffset);
	return height + ySpacing;
}

int CacheFont::width(char c)
{
	short width, height, xOffset, yOffset;
	char *shape = theFontHandler->get(shapeNum);
	GetShapeInfo(shape, (unsigned char)c, width, height, xOffset, yOffset);
	return width + xSpacing;
}

void CacheFont::dim(char c, int &w, int &h)
{
	short width, height, xOffset, yOffset;
	char *shape = theFontHandler->get(shapeNum);
	GetShapeInfo(shape, (unsigned char)c, width, height, xOffset, yOffset);
	w = width + xSpacing;
	h = height + ySpacing;
}

// The outOfMemory line numbers are those of the shipped build.
CacheFont::CacheFont(FontType type)
{
	if (!theFontHandler)
	{
		theFontHandler = new FlexShapeHandler(16, (CacheNodeType)1, fileSpec(0, StaticDir, fontsFileName, 0));
		if (!theFontHandler)
			outOfMemory(__FILE__, 121);
	}
	setFont(type);
}

CacheFont::CacheFont(void)
{
	if (!theFontHandler)
	{
		theFontHandler = new FlexShapeHandler(16, (CacheNodeType)1, fileSpec(0, StaticDir, fontsFileName, 0));
		if (!theFontHandler)
			outOfMemory(__FILE__, 139);
	}
}

void CacheFont::setFont(FontType type)
{
	type < 16 ? type : (type = (FontType)0);
	shapeNum = theFontHandler->startHandle + type;
	xSpacing = xSpacingArray[type];
	ySpacing = ySpacingArray[type];
}

inline CacheFont::~CacheFont(void)
{
}

inline int CacheFont::height(char *text)
{
	return Font::height(text);
}

inline int CacheFont::width(char *text)
{
	return Font::width(text);
}

inline void CacheFont::dim(char *text, int &width, int &height)
{
	Font::dim(text, width, height);
}
