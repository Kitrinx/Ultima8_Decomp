#ifndef FLICPLAY_H
#define FLICPLAY_H

#include "..\GLIB\PROCESS.H"
#include "CPALETTE.H"
#include "CRECT.H"
#include "FLEX.H"

// Ends after a number of ticks.
class QuickWait : public ProcessRealTime
{
public:
	int ticks;

	QuickWait(short, short);
	virtual void process(void);
	virtual void interruptHandler(unsigned long, unsigned long);
};

class CacheFont;
class FastFlex;

// One scripted event of a flic: at a frame, do `type` with `value`.
class FlicEvent
{
public:
	int frame;
	int type;
	int value;
};

// Plays a flic from a flex file (record 0 holds its event script) on a timer.
class FlicPlayer : public ProcessRealTime
{
public:
	FlicEvent events[256];
	unsigned char pad_638;
	int textY;
	unsigned char pad_63b[2];
	Rect textRect;
	unsigned char pad_645;
	CacheFont *font;
	FastFlex *flex;
	char filename[30];
	unsigned char loop;
	int state;
	int record;
	int numRecords;
	Index index;
	char *underText;
	Palette *palette;
	char *speech;
	unsigned char f_687;
	int speed;
	PaletteData paletteData;
	char *paletteChunk;
	char *frameData;
	int value;
	unsigned char pad_69b[2];
	unsigned char shown;
	unsigned char fading;
	PaletteData *fadePalette;
	int loopCount;
	int frameCount;
	int nextEvent;

	FlicPlayer(char *, unsigned char, unsigned char);
	virtual void process(void);
	virtual ~FlicPlayer(void);
	virtual void interruptHandler(unsigned long, unsigned long);
};

void PauseGame(void);
int playIntro(unsigned char, unsigned char, unsigned char);
int playEndgame(void);

#endif
