#ifndef BASECAM_H
#define BASECAM_H

#include "DRAGGUMP.H"

class Event;
class Font;
class NewGump;
class NewGumpId;
class Rect;

class Panel
{
public:
	void drawup(Rect, unsigned char);
	void drawdown(Rect, unsigned char);
};

class PanelGump : public DrgNewGump
{
public:
	unsigned char color;
	Font *font;

	PanelGump(NewGumpId, NewGump *, Rect, unsigned char);
	virtual void commandMouseLeft(Event &);
	virtual void draw(short, short);
	void printf(int, int, char *, ...);
};

class BaseCamera
{
public:
	BaseCamera(Rect, Rect);
	virtual void show(void);
	virtual void show(short, short, short, short, unsigned char);
	virtual void slam(void);
	virtual void initPalette(void);
	void textVideoMode(void);
	void graphicVideoMode(void);
	void flipVideoMode(void);
	static void setNeedSlam(void);
	static unsigned char needSlam;
	static unsigned char inTextMode;
	static unsigned char okToShow;
	static unsigned char alternateFrame;
	static Rect graphics;
	static Rect text;
	static Rect screen;
};

extern BaseCamera *theBaseCamera;

#endif
