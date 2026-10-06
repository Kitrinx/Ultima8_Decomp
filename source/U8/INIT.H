#ifndef INIT_H
#define INIT_H

class CommandLineOptions;
class LargeGameSystem;

enum Language
{
	Language_0,
	ENGLISH = 0,
	FRENCH = 1,
	GERMAN = 2
};

class CommandLineOptions
{
public:
	int recordMode;		// 1 record, 2 playback
	int test;
	Language language;	// -1 keeps the saved one
	int demo;
	int f_08;
	long cacheSize;

	CommandLineOptions(void)
	{
		recordMode = 0;
		test = 0;
		language = (Language)-1;
		demo = 0;
		f_08 = 0;
		cacheSize = 0x800000L;
	}
};

// Raises the DOS file handle limit and widens the runtime's handle table to match.
class FileHandleExtender
{
public:
	unsigned char closed;

	FileHandleExtender(void);
	~FileHandleExtender(void);
	void uninit(void);
	unsigned char check(void);
};

class SimpleExitSystem
{
public:
	static void exit(void);
	static unsigned char initialized;
};

void pascal UNINITFILEHANDLEEXTENDER(unsigned short);
int controlBreakHandler(void);
void initDispatcher(int);
void uninitDispatcher(void);
void initWorldGump(void);
void initLanguage(Language);
void initDeathLoader(void);
void uninitDeathLoader(void);
void createInitialProcesses(void);
void initSystem(CommandLineOptions);
void load(short);
void save(short);

extern int sport;
extern int sirq;
extern int sdrq;
extern int sdma;
extern int mport;
extern int mirq;
extern int mdrq;
extern int mdma;
extern char *ExeName;
extern char *lines;
extern unsigned char doneIniting;
extern unsigned char demoing;
extern unsigned char musicOn;
extern unsigned char soundOn;
extern LargeGameSystem *theSystem;
extern unsigned _openfd[40];
extern FileHandleExtender fileHandleExtender;
extern unsigned char old0[4];

#endif
