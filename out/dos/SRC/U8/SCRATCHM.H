#ifndef SCRATCHM_H
#define SCRATCHM_H

class ScratchMem
{
public:
	char *buffer;
	unsigned char locked;

	ScratchMem(void) { buffer = 0; locked = 0; }
	void init(void);
	void uninit(void);
	char *checkOut(unsigned short);
	void checkIn(char *);
};

void initScratchBuffer(void);
void uninitScratchBuffer(void);
void trace(char *, ...);

extern ScratchMem *theScratchMem;
extern char StaticDir[9];
extern char GamedatDir[9];
extern char SavedgamesDir[9];
extern char SoundDir[9];
extern char EditorDir[9];
extern char UsecodeDir[80];
extern char *shapeFileName;
extern char *typeNamesFileName;
extern char *fixedFileDesc;
extern char *nonfixedFileDesc;
extern char *globFileName;
extern char *endgameFileName;
extern char *transformPaletteFileName;
extern char *musicFileName;
extern char *gumpsFileName;
extern char *fontsFileName;
extern char *quoteDataFile;
extern char *usecodeFileName;
extern char *introFileName;
extern char *speechFileName;
extern char *creditDataFile;
extern char *copyrightStatement;
extern char *endGameFlagName;
extern char *quotesFlagName;
extern char *flagExtension;
extern char *thanks[15];
extern unsigned char signToDirTable[9];
extern char x8add[9];
extern char y8add[10];

#endif
