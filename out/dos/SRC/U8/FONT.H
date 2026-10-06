#ifndef FONT_H
#define FONT_H

class Font;
class Rect;

class Font
{
public:
	int cursorX;
	int cursorY;
	int xSpacing;
	int ySpacing;

	Font(void);
	virtual void drawChar(int, int, unsigned char) = 0;
	virtual ~Font(void);
	virtual int height(char *);
	virtual int width(char *);
	virtual void dim(char *, int &, int &);
	virtual int height(char) = 0;
	virtual int width(char) = 0;
	virtual void dim(char, int &, int &) = 0;
	virtual int getBaseLine(void) = 0;
	void print(unsigned char);
	void print(char *);
	int print(int, char *);
	void printf(int, int, char, char *, ...);
	int printf(int, int, char *, ...);
	void printf(int, int, int, char *, ...);
	void printf(char *, ...);
	void getRect(Rect &, int, int, char *);
	void gotoXY(int, int);
	void setXSpacing(int);
	void setYSpacing(int);
};

class GlobalFont
{
public:
	static void setMainFont(Font *);
	static Font *main_font;
	static Font *global_ptr;
};

// Makes a font current for the life of a scope.
class CurrentFont
{
public:
	Font *oldFont;

	CurrentFont(Font *);
	~CurrentFont(void);
};

extern "C" void *getRomFontPtr(void);

#endif
