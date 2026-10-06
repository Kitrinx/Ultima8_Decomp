#ifndef CAFONT_H
#define CAFONT_H

#include "FONT.H"

class FlexShapeHandler;

enum FontType
{
	FontType_0
};

// A font drawn from the shapes of the fonts flex.
class CacheFont : public Font
{
public:
	unsigned shapeNum;

	CacheFont(FontType);
	CacheFont(void);
	virtual void drawChar(int, int, unsigned char);
	virtual ~CacheFont(void);
	virtual int height(char *);
	virtual int width(char *);
	virtual void dim(char *, int &, int &);
	virtual int height(char);
	virtual int width(char);
	virtual void dim(char, int &, int &);
	virtual int getBaseLine(void);
	void setFont(FontType);
	static FlexShapeHandler *theFontHandler;
	static unsigned xSpacingArray[16];
	static unsigned ySpacingArray[16];
};

#endif
