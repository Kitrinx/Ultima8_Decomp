#ifndef PAL_H
#define PAL_H

#include "..\GLIB\PROCESS.H"
#include "..\POLIB\INC\FLEX.H"
#include "CPALETTE.H"
#include "MLIST.H"

class BaseFile;

// Palette transforms; FadeProcess also takes PALETTE_CURRENT.
enum PaletteNum
{
	PALETTE_NORMAL,
	PALETTE_GREYSCALE,
	PALETTE_NORED,
	PALETTE_RAINSTORM,
	PALETTE_FIRESTORM,
	PALETTE_SATURATE,
	PALETTE_GBR,
	PALETTE_BRG,
	PALETTE_CURRENT,	// whatever currentPalette is, followed as it changes
	PALETTE_CUSTOM		// a colour or palette given to the fade
};

#define FADE_PROCESS	((ProcessType)0x206)

class RGB
{
public:
	char f_00;	// red, 0-63
	char f_01;	// green
	char f_02;	// blue

	RGB(void);
	RGB(char r, char g, char b) { f_00 = r; f_01 = g; f_02 = b; }
};

// Fades U8GamePalette toward a target palette, `speed` steps per tick.
// A newer fade replaces a running one unless the running one has priority.
class FadeProcess : public ProcessInterrupt
{
public:
	PaletteData *palette;
	int speed;
	int priority;
	unsigned char done;
	unsigned char started;
	unsigned paletteNum;
	unsigned char followCurrent;

	FadeProcess(RGB, short, short);
	FadeProcess(PaletteData *, short, short);
	FadeProcess(PaletteNum, short, short);
	virtual void process(void);
	virtual ~FadeProcess(void);
	virtual void load(BaseFile *);
	virtual void save(BaseFile *);
	void initPal(PaletteNum);
	unsigned char _init(void);
};

// A 256-colour palette read from a .pal file.
class FilePalette : public PaletteData
{
public:
	FilePalette(char *name) : PaletteData(0, 256) { read(name); }
	void read(char *);
};

class TransformPaletteFile : public FlexFile
{
public:
	TransformPaletteFile(void);
};

class TransformPalette
{
public:
	int count;
	FlexFile *file;
	unsigned char *table;
	unsigned char *tables;	// table + 256

	TransformPalette(void);
	~TransformPalette(void);
	void init(void);
	void uninit(void);
	void load(void);
	void save(void);
	void addPal(void);
	void removePal(short);
};

void initU8Palette(void);
void resetU8Palette(void);
void uninitU8Palette(void);
void activateCurrentPalette(void);
int FadeToBlack(void);
int FadeFromBlack(void);
int FadeToPalette(short, short);
int LightningBolt(void);
int FadeToWhite(void);
int FadeFromWhite(void);

extern unsigned currentPalette;
extern FilePalette *U8GamePalette;
extern TransformPalette *theTransformPalette;
extern PaletteData *theDefaultPalette;
extern char palFileName[];

#endif
