// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: FLICPLAY.C

#include <string.h>
#include <alloc.h>
#include "ERROR.H"
#include "FERROR.H"
#include "PRIORITY.H"
#include "CAFONT.H"
#include "FASTFLEX.H"
#include "PAL.H"
#include "CMEMHNDP.H"
#include "CVPORT.H"
#include "CSHAPE.H"
#include "CCURRVP.H"
#include "CGLOBVP.H"
#include "FVINFO.H"
#include "CTMPOINT.H"
#include "AMUSIC.H"
#include "SNDMGR.H"
#include "DSFXMAN.H"
#include "GAMETIME.H"
#include "PREAMBLE.H"
#include "PAGAN.H"
#include "U8DISPAT.H"
#include "U8DIR.H"
#include "FILESPEC.H"
#include "SCRATCHM.H"
#include "INIT.H"
#include "FLICPLAY.H"

// Inline in the shared headers when this file was built.
inline void nameProcess(unsigned pid, char *name) { ((char **)Kernel::idStringList)[pid] = name; }
inline Point::Point(void) {}
inline Rect::Rect(void) {}
inline void Point::set(short x, short y) { f_00 = x; f_02 = y; }
inline void Rect::set(short x1, short y1, short x2, short y2) { Point::set(x1, y1); f_04 = x2; f_06 = y2; }
inline int Rect::width(void) { return f_04 - f_00 + 1; }
inline int Rect::height(void) { return f_06 - f_02 + 1; }
inline Index::Index(void) { size = 0; offset = 0; }
inline unsigned char BaseFile::is_valid(void) { return handle != -1; }
inline BaseFile::~BaseFile(void) { if (is_valid()) close(); }
inline MemHandle::MemHandle(char *p) { f_00 = (unsigned long)p; f_04 = 1; f_05 = 0; }
inline MemHandle::~MemHandle(void) { free(0); }

// Kills every process of a type running at the current priority.
inline void killAtPriority(ProcessType type)
{
	PriorityClass samePriority(Kernel::currentPriority | 0x20);
	Kernel::killProcess(0, type, samePriority);
}

// Flic event types.
#define FLIC_MUSIC		3
#define FLIC_STOP_MUSIC		4
#define FLIC_SFX		5
#define FLIC_STOP_SFX		6
#define FLIC_SPEED		7
#define FLIC_FADE_OUT		8
#define FLIC_FADE_IN		9
#define FLIC_LOOP		11
#define FLIC_WAIT		12
#define FLIC_SUBTITLE		14
#define FLIC_FLASH		15
#define FLIC_UNFLASH		16
#define FLIC_CLEAR_SUBTITLE	18

void PauseGame(void)
{
	Kernel::setCurrentPriority(Kernel::currentPriority + 1);
	theU8Dispatcher->flags |= PROC_SUSPENDED;
	theU8Dispatcher->flags &= ~PROC_DAEMON;
	TMMousePointer::hide();
}

QuickWait::QuickWait(short count, short hertz)
{
	nameProcess(pid, "QuickWait");
	ticks = count;
	setHertz(hertz);
	intFlags |= 2;
	connectToRealTime(1);
}

void QuickWait::interruptHandler(unsigned long type, unsigned long)
{
	if (type == 2)
	{
		if (ticks == 0)
			pop(0);
		ticks--;
	}
}

FlicPlayer::FlicPlayer(char *name, unsigned char repeat, unsigned char flag)
{
	nameProcess(pid, "FlicPlayer");
	setProcessType((ProcessType)0x240);
	theAnimation->intFlags &= ~2;
	f_687 = flag;
	loop = repeat;
	strcpy(filename, name);
	font = new CacheFont((FontType)14);
	if (!font)
		outOfMemory(__FILE__, 175);	// __LINE__
	flex = new FastFlex(filename, 0, ReadOnly, -1);
	if (!flex)
		outOfMemory(__FILE__, 179);	// __LINE__
	if (!flex->is_valid())
		Fatal("Error opening skipflic file.\n");
	textY = -1;
	palette = 0;
	speech = 0;
	speed = 0;
	paletteChunk = 0;
	frameData = 0;
	shown = 0;
	fading = 0;
	fadePalette = 0;
	loopCount = -1;
	frameCount = 0;
	nextEvent = 0;
	record = 1;
	numRecords = flex->totalIndexes;
	flex->getIndex(0, index);
	flex->readRecord(0, events, index.size);
	setHertz(10);
	intFlags |= 3;
	connectToRealTime(1);
	TMMousePointer::hide();
	state = 1;
}

FlicPlayer::~FlicPlayer(void)
{
	if (musicOn)
		musicSlowStop();
	if (soundOn)
		SoundManager->flush();
	if (font)
	{
		delete font;
		font = 0;
	}
	if (flex)
	{
		delete flex;
		flex = 0;
	}
	if (palette)
	{
		delete palette;
		palette = 0;
	}
	if (frameData)
	{
		farfree(frameData);
		frameData = 0;
	}
	if (fadePalette)
	{
		delete fadePalette;
		fadePalette = 0;
	}
}

void FlicPlayer::interruptHandler(unsigned long type, unsigned long)
{
	if (type == 1)
	{
		intFlags |= 2;
		return;
	}
	if (type == 2)
	{
		switch (state)
		{
		case 1:
			new FadeProcess(RGB(0, 0, 0), 0x7fff, 1);
			state = 2;
			return;
		case 2:
			if (events[nextEvent].frame == -1)
			{
				state = 3;
				return;
			}
			while (events[nextEvent].frame == frameCount)
			{
				QuickWait *wait;

				value = events[nextEvent].value;
				switch (events[nextEvent].type)
				{
				case FLIC_MUSIC:
					if (musicOn)
						musicPlay(value);
					break;
				case FLIC_STOP_MUSIC:
					if (musicOn)
						musicSlowStop();
					break;
				case FLIC_SFX:
					if (soundOn)
						playSFX(value);
					break;
				case FLIC_STOP_SFX:
					if (soundOn)
						stopSFX(value);
					break;
				case FLIC_SPEED:
					speed = value;
					setHertz(speed + 3);
					break;
				case FLIC_FADE_OUT:
					new FadeProcess(RGB(0, 0, 0), 0x7fff, 1);
					fading = 1;
					nextEvent++;
					return;
				case FLIC_FADE_IN:
					new FadeProcess(fadePalette, 0x7fff, 1);
					fading = 0;
					nextEvent++;
					return;
				case FLIC_FLASH:
					new FadeProcess(RGB(63, 63, 63), 0x7fff, 1);
					fading = 1;
					break;
				case FLIC_UNFLASH:
					new FadeProcess(fadePalette, 0x7fff, 1);
					fading = 0;
					break;
				case FLIC_LOOP:
					// The next event's frame holds the repeat count.
					if (loopCount < 0)
					{
						loopCount = events[nextEvent++].frame - 1;
						record = value;
					}
					else if (loopCount == 0)
						loopCount = -1;
					else
					{
						record = value;
						loopCount--;
					}
					break;
				case FLIC_WAIT:
					wait = new QuickWait(value, 15);
					intFlags &= ~2;
					wait->notifyOfDeath(pid);
					nextEvent++;
					return;
				case FLIC_CLEAR_SUBTITLE:
					if (!soundOn)
					{
						RestoreRect(GlobalVport::main_screen, underText, &textRect);
						delete underText;
					}
					break;
				case FLIC_SUBTITLE:
					if (soundOn)
					{
						Index speechIndex;
						flex->getIndex(value, speechIndex);
						SoundManager->discard(255);
						speech = new char[speechIndex.size - 2];
						flex->readRecord(value, speech, speechIndex.size - 2, 2);
						SoundManager->load(255, (DAD *)speech);
						SoundManager->play(255, 0, 255, 0);
					}
					else
					{
						Index textIndex;
						flex->getIndex(value - 1, textIndex);
						speech = new char[textIndex.size - 2];
						flex->readRecord(value - 1, speech, textIndex.size - 2, 2);
						if (strlen(speech + 4) == 0)
							break;
						// Text: x (0 centres), y, then the words.
						char *text = speech + 4;
						int numLines = 0;
						int width = 0;
						int i;
						char *lines[4];
						for (i = 0; i < 4; i++)
							lines[i] = 0;
						i = 0;
						lines[numLines++] = text;
						for (; text[i] != 0; i++)
						{
							width += font->width(text[i]);
							if (width >= 200)
							{
								while (text[i--] != ' ')
									;
								i++;
								width = 0;
								text[i] = 0;
								lines[numLines++] = &text[i + 1];
							}
						}
						textY = font->height(speech + 4);
						textRect.set(0, 0, 319, textY * numLines + 4);
						textY = (unsigned char)speech[2];
						textRect.moveto(0, textY - 10);
						underText = new char[textRect.height() * textRect.width()];
						if (!underText)
							outOfMemory(__FILE__, 442);	// __LINE__
						SaveRect(GlobalVport::main_screen, underText, &textRect);
						CurrentVport current(GlobalVport::main_screen);
						if (!(unsigned char)speech[0])
						{
							for (i = 0; i < 4; i++)
								if (lines[i])
									font->printf((320 - font->width(lines[i])) >> 1, textY + i * font->height('A'), lines[i]);
						}
						else
						{
							for (i = 0; i < 4; i++)
								if (lines[i])
									font->printf((unsigned char)speech[0], textY + i * font->height('A'), lines[i]);
						}
						delete speech;
					}
					break;
				}
				nextEvent++;
			}
			flex->getIndex(record, index);
			if (frameData)
			{
				delete frameData;
				frameData = 0;
			}
			if (index.size > 2)
			{
				frameData = (char *)farmalloc(index.size);
				flex->readRecord(record, frameData, index.size, 0);
				switch (*(int *)frameData)
				{
				case 1:
					paletteChunk = new char[0x304];
					paletteChunk[0] = paletteChunk[1] = paletteChunk[2] = 0;
					paletteChunk[3] = 1;
					memcpy(paletteChunk + 4, frameData + 2, 0x300);
					if (palette && memcmp(paletteChunk + 4, palette->colors, 0x300))
						GlobalVport::main_screen->clear(0);
					if (palette == 0)
						palette = (Palette *)new PaletteData(0, 256);
					if (fadePalette)
						delete fadePalette;
					fadePalette = new PaletteData;
					fadePalette->init(0, 256);
					fadePalette->init((RGB *)(paletteChunk + 4));
					if (shown && !fading)
						((Palette *)U8GamePalette)->init(MemHandle(paletteChunk));
					if (fading)
						delete paletteChunk;
					break;
				case 2:
					frameCount++;
					state = 4;
					break;
				}
			}
			record++;
			if (record >= numRecords)
				state = 3;
			return;
		case 4:
			SkipDraw(GlobalVport::main_screen, 0, 0, frameData + 2);
			if (!shown)
				shown = 1;
			state = 2;
			return;
		case 3:
			if (loop)
			{
				new FadeProcess(RGB(0, 0, 0), 0x7fff, 1);
				shown = 0;
				record = 1;
				state = 2;
				return;
			}
			killAtPriority((ProcessType)6);
			state = 6;
			return;
		case 6:
			pop(0);
			break;
		}
	}
}

int playIntro(unsigned char repeat, unsigned char menu, unsigned char flag)
{
	PauseGame();
	if (menu)
	{
		new MainMenuProcess;
		Kernel::setCurrentPriority(Kernel::currentPriority + 1);
	}
	(new FadeProcess(RGB(0, 0, 0), 0x7fff, 1))->then(new RefreshWorld);
	Kernel::setCurrentPriority(Kernel::currentPriority + 1);
	languageSensitiveFile(StaticDir, introFileName);
	FlicPlayer *player = new FlicPlayer(fileSpec(0, StaticDir, introFileName, 0), repeat, flag);
	new CoolKeyboard;
	return player->pid;
}

int playEndgame(void)
{
	BaseFile file;
	file.forceOpen(fileSpec(0, StaticDir, endGameFlagName, flagExtension), ReadWrite);
	file.write(copyrightStatement, strlen(copyrightStatement) + 1);
	file.close();
	PauseGame();
	new MainMenuProcess;
	Kernel::setCurrentPriority(Kernel::currentPriority + 1);
	(new FadeProcess(RGB(0, 0, 0), 0x7fff, 1))->then(new RefreshWorld);
	Kernel::setCurrentPriority(Kernel::currentPriority + 1);
	languageSensitiveFile(StaticDir, creditDataFile);
	new CoolKeyboard;
	new PaganProcess(fileSpec(0, StaticDir, creditDataFile, 0), 1, 1, 0x33);
	Kernel::setCurrentPriority(Kernel::currentPriority + 1);
	new FadeProcess(RGB(0, 0, 0), 0x7fff, 1);
	Kernel::setCurrentPriority(Kernel::currentPriority + 1);
	FlicPlayer *player = new FlicPlayer(fileSpec(0, StaticDir, endgameFileName, 0), 0, 1);
	return player->pid;
}

void FlicPlayer::process(void)
{
}

void QuickWait::process(void)
{
}
