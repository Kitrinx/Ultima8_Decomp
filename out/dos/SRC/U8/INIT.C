// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r- -y
// name: INIT.C

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <dir.h>
#include <fcntl.h>
#include <phapi.h>
#include "CEXIT.H"
#include "LSYSTEM.H"
#include "SCRATCHM.H"
#include "FILESPEC.H"
#include "FEXIST.H"
#include "DESKIP.H"
#include "DSFXMAN.H"
#include "AMUSIC.H"
#include "CACHE.H"
#include "HANDLER.H"
#include "SHAPHAND.H"
#include "SAVEGAME.H"
#include "PAL.H"
#include "GAMETIME.H"
#include "UPROCESS.H"
#include "DLIST.H"
#include "ITEMCACH.H"
#include "MAPFILE.H"
#include "ITEM.H"
#include "TYPE.H"
#include "GLOB.H"
#include "NPC.H"
#include "DEBUGUMP.H"
#include "APPDEBUG.H"
#include "WPNCACHE.H"
#include "chargen.H"
#include "KERNEL.H"
#include "IRGUMP.H"
#include "U8POINT.H"
#include "U8DISPAT.H"
#include "WORLD.H"
#include "CAMERA.H"
#include "BASECAM.H"
#include "OCCLUDE.H"
#include "STATREST.H"
#include "STDINT.H"
#include "HARDWARE.H"
#include "WAIT.H"
#include "SPANKER.H"
#include "INIT.H"

#define NUM_FILES	40

#define TRACE_STARTUP	((TraceLevel)50)

// Runs one startup step, then traces its line number.
#define INIT_STEP(step, line)	step; trace(TRACE_STARTUP, "%s %d\r\n", __FILE__, line)

extern "C" int SafeOpenFile(const char *, int);
extern "C" int CloseFile(int);

int sport = 0x220;
int sirq = 7;
int sdrq = 1;
int sdma = 1;
int mport = 0x220;
int mirq = 7;
int mdrq = 1;
int mdma = 1;
char *ExeName = "u8.exe";
char *lines = "---------------------------------------------------------------------";
unsigned char doneIniting = FALSE;
unsigned char demoing = FALSE;
unsigned char musicOn;
unsigned char soundOn;
LargeGameSystem *theSystem;
unsigned _openfd[NUM_FILES];
FileHandleExtender fileHandleExtender;
unsigned char old0[4];

inline unsigned char BaseFile::is_valid(void) { return handle != -1; }
inline BaseFile::~BaseFile(void) { if (is_valid()) close(); }

FileHandleExtender::FileHandleExtender(void)
{
	int i;

	_openfd[0] = O_RDONLY | O_DEVICE;
	_openfd[1] = O_WRONLY | O_DEVICE;
	_openfd[2] = O_WRONLY | O_DEVICE;
	_openfd[3] = O_RDWR | O_DEVICE | O_BINARY;
	_openfd[4] = O_WRONLY | O_DEVICE | O_BINARY;

	for (i = 5; i < NUM_FILES; i++)
		_openfd[i] = 0xFFFF;

	// Allocate last fit while DOS grows the handle table, then first fit again.
	asm {
		mov ah, 58h
		mov al, 1
		mov bx, 2
		int 21h

		mov ah, 67h
		mov bx, NUM_FILES
		int 21h

		mov ah, 58h
		mov al, 1
		mov bx, 0
		int 21h
	}

	closed = FALSE;
}

void FileHandleExtender::uninit(void)
{
	int i;

	if (closed == FALSE)
	{
		for (i = 5; i < NUM_FILES; i++)
		{
			if (_openfd[i] != 0xFF)
			{
				_AH = 0x3E;
				_BX = _openfd[i];
				geninterrupt(0x21);
			}
		}
		closed = TRUE;
	}
}

FileHandleExtender::~FileHandleExtender(void)
{
	uninit();
}

// Opens the executable until DOS runs out of handles.
unsigned char FileHandleExtender::check(void)
{
	Boolean ok;
	int i;
	int handles[NUM_FILES - 5];

	ok = TRUE;
	for (i = 0; i < NUM_FILES - 5; i++)
	{
		handles[i] = -1;
		handles[i] = SafeOpenFile(ExeName, 0);
		if (handles[i] == -1)
		{
			trace(TRACE_ALWAYS, "FILES=%d!\r\n", i + 5);
			ok = FALSE;
			break;
		}
	}

	for (i = i - 1; i >= 0; i--)
	{
		if (handles[i] != -1)
			CloseFile(handles[i]);
	}

	return ok;
}

void pascal UNINITFILEHANDLEEXTENDER(unsigned short)
{
	fileHandleExtender.uninit();
	DosExitList(EXLST_EXIT, 0);
}

int controlBreakHandler(void)
{
	exit(1);
	return 0;
}

void initDispatcher(int mode)
{
	new U8Dispatcher(mode);
	if (!theU8Dispatcher)
		outOfMemory(__FILE__, 264);	// __LINE__
}

void uninitDispatcher(void)
{
	if (theU8Dispatcher)
	{
		theU8Dispatcher->pop(0);
		theU8Dispatcher = 0;
	}
}

void initWorldGump(void)
{
	theU8Dispatcher->pushBase(new World);
	ItemCache::load();
}

void initLanguage(Language language)
{
	char c;

	avatar.load();

	if (language != -1 && avatar.language != language)
		avatar.setLanguage(language);

	c = avatar.getLanguageChar();
	usecodeFileName[0] = c;
	introFileName[0] = c;
	creditDataFile[0] = c;
}

void initDeathLoader(void)
{
	if (!theDeathLoader)
		theDeathLoader = new DeathLoader;
}

void uninitDeathLoader(void)
{
	if (theDeathLoader)
		theDeathLoader = 0;
}

void createInitialProcesses(void)
{
	new StatRestorer;
	new HeadShaker;
	new ProcessMonitor;
}

void initSystem(CommandLineOptions options)
{
	if (!fileHandleExtender.check())
	{
		trace(TRACE_ALWAYS,
			"%s\r\n"
			"You must set FILES=%d in your CONFIG.SYS file.\r\n\r\n"
			"Vous devez configurer le param\x8Atre FILES=%d dans votre fichier CONFIG.SYS.\r\n\r\n"
			"Setzen Sie bitte in Ihrer CONFIG.SYS Datei FILES=%d.\r\n"
			"%s\r\n\r\n",
			lines, NUM_FILES, NUM_FILES, NUM_FILES, lines);
		Exit(1);
	}

	demoing = options.demo;
	traceLevel = 0xFE;
	ctrlbrk(controlBreakHandler);

	// Step line numbers are the shipped __LINE__ values.
	INIT_STEP(initKernel(), 384);

	if (FileExists(fileSpec(0, StaticDir, "u8shapes.cmp", 0)))
	{
		trace(TRACE_ALWAYS,
			"%s\r\n"
			"Decompressing the shapes file.\r\n"
			"This only happens once and takes about 10 minutes.\r\n"
			"Do not interrupt this process or you will have to re-install.\r\n\r\n"
			"D\x82" "compression du fichier formes.\r\n"
			"Cela n'arrive qu'une seule fois et prend environ 10 minutes.\r\n"
			"Ne pas interrompre le processus ou il faudra r\x82installer.\r\n\r\n"
			"Die Formendatei wird gerade entpackt.\r\n"
			"Dieser Vorgang geschieht nur einmal und dauert ungef\x84hr zehn Minuten.\r\n"
			"Wenn Sie diesen Vorgang unterbrechen, m\x81ssen Sie erneut installieren.\r\n",
			lines);
		deskip(fileSpec(0, StaticDir, "u8shapes.cmp", 0),
			fileSpec(0, StaticDir, "u8shapes.flx", 0), 0);
	}

	getSoundStuff(sport, sirq, sdma, sdrq, mport, mirq, mdma, mdrq,
		musicOn, soundOn, 0, 0);

	if (!demoing)
	{
		INIT_STEP(initCache(options.cacheSize), 433);
	}

	if (!FileExists(GamedatDir))
		mkdir(GamedatDir);

	if (!FileExists(fileSpec(0, GamedatDir, nonfixedFileDesc, 0)))
	{
		SavedGames savedGames;
		savedGames.unpack(0);
	}

	INIT_STEP(theSystem = new LargeGameSystem, 483);
	INIT_STEP(theSystem->init(), 484);
	INIT_STEP(initU8Palette(), 485);
	INIT_STEP(initGameTime(), 486);

	if (!demoing)
	{
		INIT_STEP(initScratchBuffer(), 490);
		INIT_STEP(initLanguage(options.language), 491);
		INIT_STEP(initUnk(), 492);
		INIT_STEP(initDisplayList(), 493);
		INIT_STEP(initItemCache(), 494);
		INIT_STEP(GlobalTypes.loadTypes(), 495);
		INIT_STEP(initCacheHandler(), 496);
		INIT_STEP(initShapeHandler(), 497);
		INIT_STEP(initGlobExpander(), 498);
		INIT_STEP(initDeathLoader(), 499);
		INIT_STEP(initDebugger(), 500);
		INIT_STEP(initAnimCache(), 501);
		INIT_STEP(GlobalFlag::load(GamedatDir), 502);
	}

	INIT_STEP(initDispatcher(options.recordMode), 515);

	if (!demoing)
	{
		INIT_STEP(initNpcs(), 519);
		INIT_STEP(initWorldGump(), 524);
		INIT_STEP(initU8Camera(), 525);
		INIT_STEP(initDebug(), 526);
		createInitialProcesses();

		Kernel::load(GamedatDir);
		loadGumps();

		TMMousePointer::hide();
		if (U8MousePointer::modes[U8MousePointer::sp] == 35)
		{
			U8MousePointer::popMode();
			U8MousePointer::popMode();
			U8MousePointer::popMode();
		}
	}

	if (!TheMusicProcess && musicOn)
		musicInit("music.flx", mport, mirq, mdma, mdrq);
	if (soundOn)
		soundInit(sirq, sport, sdma);

	doneIniting = TRUE;
}

void SimpleExitSystem::exit(void)
{
	if (initialized)
	{
		initialized = FALSE;

		if (theSystem)
		{
			delete theSystem;
			theSystem = 0;
		}

		uninitAnimCache();

		if (!demoing)
		{
			uninitDeathLoader();
			uninitGlobExpander();
			uninitDisplayList();
			uninitShapeHandler();
		}

		uninitU8Palette();
		if (musicOn) musicDeinit();
		if (soundOn) soundDeInit();

		if (!demoing)
		{
			uninitCache();
			uninitScratchBuffer();
		}

		uninitDispatcher();
		uninitKernel();
		Spanky::uninit();

		printf(thanks[avatar.language]);
	}
}

void load(short slot)
{
	if (soundOn)
		soundDeInit();
	if (musicOn)
		musicDeinit();

	Kernel::restart();
	Kernel::freePhantoms();
	ItemCache::itemFile->close();

	if (slot != -1)
	{
		SavedGames savedGames;
		savedGames.unpack(slot);
	}

	Kernel::load(GamedatDir);
	Npc::load();
	avatar.load();
	soundOn = avatar.soundOn;
	musicOn = avatar.musicOn;
	ItemCache::load();
	GlobalFlag::load(GamedatDir);
	ItemCache::itemFile->restart();
	loadGumps();

	if (musicOn)
		musicInit("music.flx", mport, mirq, mdma, mdrq);
	if (soundOn)
	{
		soundInit(sirq, sport, sdma);
		unsigned short pid = guardianBark(-4);
		if (pid)
		{
			Wait *wait = new Wait(30, 0);
			wait->then(Kernel::getProcess(pid));
			wait->start();
		}
	}

	Camera::init();
	theZbuffer.make();
	Camera::setCenterOn(1);
	theBaseCamera->show();
}

void save(short slot)
{
	if (((Npc *)&avatar)->isDead()) return;

	if (soundOn)
		soundDeInit();
	if (musicOn)
		musicDeinit();

	saveGumps();
	Kernel::save(GamedatDir);
	ItemCache::itemFile->commit();
	GlobalFlag::save(GamedatDir);
	ItemCache::save(GamedatDir);
	Npc::save();
	avatar.soundOn = soundOn;
	avatar.musicOn = musicOn;
	avatar.save(0);
	ItemCache::itemFile->resetChecksum();

	if (slot != -1)
	{
		SavedGames savedGames;
		savedGames.pack(slot, GamedatDir);
	}

	ItemCache::itemFile->initTemp();

	if (musicOn)
		musicInit("music.flx", mport, mirq, mdma, mdrq);
	if (soundOn)
	{
		soundInit(sirq, sport, sdma);
		unsigned short pid = guardianBark(-4);
		if (pid)
		{
			Wait *wait = new Wait(120, 0);
			wait->then(Kernel::getProcess(pid));
			wait->start();
		}
	}
}
