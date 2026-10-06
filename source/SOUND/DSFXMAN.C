// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: ..\SOUND\DSFXMAN.C

#include <dos.h>
#include <mem.h>
#include <stdlib.h>
#include <string.h>
#include "CFILE.H"
#include "FILESPEC.H"
#include "FEXIST.H"
#include "SPCHFLEX.H"
#include "SCRATCHM.H"
#include "CEXIT.H"
#include "KERNEL.H"
#include "chargen.H"
#include "SPANKER.H"
#include "CAMERA.H"
#include "ITEM.H"
#include "NPC.H"
#include "AMUSIC.H"
#include "SNDMGR.H"
#include "SONARC.H"
#include "DSFXMAN.H"

inline Index::Index(void) { size = 0; offset = 0; }
inline unsigned char BaseFile::is_valid(void) { return handle != -1; }
inline BaseFile::~BaseFile(void) { if (is_valid()) close(); }

extern unsigned char soundOn;
extern unsigned char musicOn;

SoundEffectFlex *SFXfile = 0;
SpeechFlex *speechFile = 0;
int *voices = 0;
char currentLanguage = 0;
SoundTimer *TheSoundTimer = 0;
SoundTimerIITheSequel *TheSoundTimerII = 0;
SpeakerManager *SpkrMgr = 0;
char *soundDir = "sound";
char *soundFileName = "sound.flx";
unsigned discards[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned speechEnded = 0;
char *speechQueue = 0;
unsigned speechIndex = 0;
char *decompressionBuffer = 0;
unsigned char speaking = 0;
char *soundNames[4] = {"No sound card", "Sound Blaster", "Sound Blaster Pro", "Sound Blaster 16"};
char *musicNames[7] = {"No music card", "Adlib", "Sound Blaster", "Sound Blaster Pro",
	"Wave Blaster", "Sound Canvas", "General MIDI"};
SoundCard mcard;
SoundCard scard;

// Large speech samples are packed in chunks; this table, read from the
// record after the sample's header, gives where each chunk starts.
struct SpeechChunk
{
	long position;
	long offset;
};

static SpeechChunk chunks[32];

#define CHUNK_TABLE	(32 + sizeof(chunks))

unsigned char sample_load(short sample, DAD *&dad)
{
	if (sample < SFXfile->totalIndexes)
	{
		Index index;
		dad = 0;
		unsigned long length;
		if (sample == SPEECH_SAMPLE)
		{
			speechFile->getIndex(speechIndex, index);
			length = speechFile->offsets[speechIndex];
		}
		else
		{
			SFXfile->getIndex(sample, index);
			length = SFXfile->offsets[sample];
		}
		if (index.size > 2)
		{
			if (length > 0xFE20)
			{
				// Only the first two chunks are loaded; the rest stream in later.
				dad = (DAD *)new char[0xFE20];
				if (dad == 0)
				{
					return 0;
				}
				speechFile->readRecord(speechIndex, dad, 32, 0);
				memset(chunks, 0, sizeof(chunks));
				speechFile->readRecord(speechIndex, chunks, sizeof(chunks), 32);
				char *buffer = theScratchMem->checkOut(0xFE00);
				if (buffer == 0)
				{
					if (dad)
					{
						delete dad;
						dad = 0;
					}
					return 0;
				}
				long packed = chunks[1].offset - chunks[0].offset;
				speechFile->readRecord(speechIndex, buffer, packed, CHUNK_TABLE);
				decompress(buffer, packed, dad->data, 1);
				theScratchMem->checkIn(buffer);
				return 1;
			}
			else
			{
				dad = (DAD *)new char[length];
				if (dad == 0)
				{
					return 0;
				}
				SoundEffectFlex *file;
				if (sample == SPEECH_SAMPLE)
				{
					file = speechFile;
					sample = speechIndex;
				}
				else
				{
					file = SFXfile;
				}
				Index index2;
				file->getIndex(sample, index2);
				char *buffer = theScratchMem->checkOut(index2.size + 32);
				if (buffer == 0)
				{
					if (dad)
					{
						delete dad;
						dad = 0;
					}
					return 0;
				}
				file->readRecord(sample, buffer, index2.size, 0);
				memcpy(dad, buffer, 32);
				decompress(buffer + 32, index2.size - 32, dad->data, 1);
				theScratchMem->checkIn(buffer);
				return 1;
			}
		}
	}
	return 0;
}

// Loads the chunk of a large sample that starts at pos.
void sample_load_chunk(short sample, char *dest, int length, long pos)
{
	SoundEffectFlex *file;
	if (sample == SPEECH_SAMPLE)
	{
		file = speechFile;
		sample = speechIndex;
	}
	else
	{
		file = SFXfile;
	}
	char *buffer = theScratchMem->checkOut(length + 32);
	if (buffer == 0)
	{
		memset(dest, 0, length);
		return;
	}
	SpeechChunk *chunk = chunks;
	for (int i = 0; i < 32; i++, chunk++)
	{
		if (chunk->position - 32 == pos)
		{
			goto found;
		}
	}
	Fatal("%s %d", __FILE__, 286);	// __LINE__
found:
	long packed = chunk[1].offset - chunk->offset;
	file->readRecord(sample, buffer, packed, chunk->offset);
	decompress(buffer, packed, dest, 1);
	theScratchMem->checkIn(buffer);
}

// Called when a sound changes state; event 0 means it has stopped. A sample
// whose last sound stopped is queued for discard.
void sample_report(long handle, short event)
{
	switch (event)
	{
	case 0:
		if (SpkrMgr)
		{
			SpkrMgr->remove(handle);
		}
		int slot = 0;
		int playing = 0;
		int sample = SoundManager->get_sample_number(handle);
		for (; discards[slot] != 0 && slot < 9; slot++)
			;
		for (int i = 0; i < NUM_CHANNELS; i++)
		{
			if (SoundManager->channels[i].handle == handle)
			{
				playing++;
			}
		}
		if (playing == 1 && sample != SPEECH_SAMPLE)
		{
			discards[slot] = sample;
		}
		if (sample == SPEECH_SAMPLE)
		{
			speechEnded = 1;
			speaking = 0;
		}
		break;
	}
}

SpeakerManager::SpeakerManager(short num)
{
	count = num;
	speakers = new Speaker[count];
	if (speakers == 0)
	{
		halt(__FILE__, 354);	// __LINE__
	}
	memset(speakers, 0, count * sizeof(Speaker));
	setDaemon();
	Kernel::setIdString(pid, "SpeakerManager");
	setProcessType((ProcessType)0x101);
}

void SpeakerManager::postLoad(void)
{
	speakers = new Speaker[count];
	if (speakers == 0)
	{
		halt(__FILE__, 375);	// __LINE__
	}
	memset(speakers, 0, count * sizeof(Speaker));
}

void SpeakerManager::freeMemory(void)
{
	Process::freeMemory();
	if (speakers)
	{
		delete speakers;
		speakers = 0;
	}
}

SpeakerManager::~SpeakerManager(void)
{
	if (speakers)
	{
		delete speakers;
		speakers = 0;
	}
}

void SpeakerManager::add(long handle, unsigned short item, short maxVolume)
{
	int i;

	for (i = 0; i < count && speakers[i].handle; i++)
		;
	if (i < count)
	{
		speakers[i].handle = handle;
		speakers[i].item = item;
		speakers[i].maxVolume = maxVolume;
	}
}

// Stops the ambient sounds of an item that are suspended.
void SpeakerManager::remove(int all, unsigned short item)
{
	int i = 0;
	int j;

	if (item == 0)
	{
		return;
	}
	if (all == 0)
	{
		return;
	}
	for (; i < count; i++)
	{
		if (speakers[i].item == item)
		{
			j = 0;
			while (j < SoundManager->maxSuspended &&
				SoundManager->suspended[j].handle != speakers[i].handle)
				j++;
			if (j < SoundManager->maxSuspended)
			{
				SoundManager->stop_ambient(speakers[i].handle);
			}
			speakers[i].handle = speakers[i].item = 0;
		}
	}
}

void SpeakerManager::remove(long handle)
{
	int i;

	for (i = 0; i < count && speakers[i].handle != handle; i++)
		;
	if (i < count)
	{
		speakers[i].handle = 0;
	}
}

void SpeakerManager::flush(void)
{
	memset(speakers, 0, count * sizeof(Speaker));
}

// Volume of a sound at an item: 0x80 with no item, else 0 to 0xFF by
// distance from the camera, at most maxVolume.
int GetVolumeByDistance(unsigned short itemRef, short maxVolume)
{
	if (itemRef == 0)
	{
		return 0x80;
	}
	Item item(itemRef);
	long dx = (int)(Camera::getX() - item.getX());
	long dy = (int)(Camera::getY() - item.getY());
	long dz = Camera::getZ() - item.getZ();
	dz *= 2;
	long dist = dx * dx + dy * dy + (dz * dz >> 2);
	int volume;
	if (dist < 0) volume = 0;
	else if (dist < 100L) volume = 16;
	else if (dist < 2500L) volume = 15;
	else if (dist < 10000L) volume = 14;
	else if (dist < 22500L) volume = 13;
	else if (dist < 40000L) volume = 12;
	else if (dist < 62500L) volume = 11;
	else if (dist < 90000L) volume = 10;
	else if (dist < 122500L) volume = 9;
	else if (dist < 160000L) volume = 8;
	else if (dist < 202500L) volume = 7;
	else if (dist < 250000L) volume = 6;
	else if (dist < 302500L) volume = 5;
	else if (dist < 360000L) volume = 4;
	else if (dist < 422500L) volume = 3;
	else if (dist < 640000L) volume = 2;
	else if (dist < 1000000L) volume = 1;
	else volume = 0;
	if (volume)
	{
		volume = volume * 16 - 1;
	}
	return volume <= maxVolume ? volume : maxVolume;
}

// Keeps each speaker's volume and priority in step with its distance.
void SpeakerManager::process(void)
{
	for (int i = 0; i < count; i++)
	{
		if (speakers[i].handle && speakers[i].item)
		{
			int volume = GetVolumeByDistance(speakers[i].item, speakers[i].maxVolume);
			SoundManager->set_volume(speakers[i].handle, volume);
			for (int j = 0; j < NUM_CHANNELS; j++)
			{
				if (SoundManager->channels[j].handle == speakers[i].handle)
				{
					SoundManager->channels[j].priority = volume;
				}
			}
		}
	}
}

// Discards the samples queued by sample_report; a negative sample flushes all.
void checkFlush(int sample)
{
	if (sample < 0 && SoundManager)
	{
		SoundManager->flush();
		return;
	}
	int i = 0;
	long size = 0;
	if (!soundOn)
	{
		return;
	}
	if (SoundManager && SoundManager->samples && SoundManager->samples[sample].dad)
	{
		size = SoundManager->samples[sample].dad->size;
	}
	for (; discards[i] != 0 && i < 10; i++)
	{
		if (sample != SPEECH_SAMPLE || size > STREAM_SIZE)
		{
			SoundManager->discard(discards[i]);
		}
		discards[i] = 0;
	}
}

long ActualSamplePlay(int sample, unsigned short item, unsigned char, unsigned char volume, int pitch)
{
	if (soundOn && sample > 0)
	{
		SoundManager->load(sample, 0);
		long handle;
		unsigned char vol = item == 0 ? volume : GetVolumeByDistance(item, volume);
		handle = SoundManager->play(sample, vol, vol, pitch);
		if (handle && item && SpkrMgr)
		{
			SpkrMgr->add(handle, item, 255);
		}
		checkFlush(sample);
		return handle;
	}
	return 0;
}

long ActualSamplePlayAmbient(int sample, unsigned short item, unsigned char, unsigned char volume, int pitch)
{
	if (soundOn && sample >= 0)
	{
		SoundManager->load(sample, 0);
		long handle;
		unsigned char vol = item == 0 ? volume : GetVolumeByDistance(item, volume);
		handle = SoundManager->play_ambient(sample, vol, vol, pitch);
		if (handle && item && SpkrMgr)
		{
			SpkrMgr->add(handle, item, 255);
		}
		checkFlush(sample);
		return handle;
	}
	return 0;
}

SoundTimerIITheSequel::SoundTimerIITheSequel(void)
{
	setHertz(1);
	setProcessType((ProcessType)0x101);
	intFlags |= 2;
	f_10 &= ~1;
	setDaemon();
	Kernel::setIdString(pid, "SoundTimer II: The Sequel");
	TheSoundTimerII = this;
}

void SoundTimerIITheSequel::start(void)
{
	connectToRealTime(1);
}

void SoundTimerIITheSequel::interruptHandler(unsigned long, unsigned long)
{
	if (SoundManager)
	{
		SoundManager->update_huge_samples();
	}
}

void SoundTimerIITheSequel::stop(void)
{
	disconnectFromRealTime();
}

SoundTimerIITheSequel::~SoundTimerIITheSequel(void)
{
	TheSoundTimerII = 0;
}

SoundTimer::SoundTimer(void)
{
	setHertz(140);
	setSafeInterrupt();
	setProcessType((ProcessType)0x101);
	setIntFlags(intFlags | 2);
	Kernel::setIdString(pid, "SoundTimer");
	f_10 &= ~1;
	setDaemon();
	TheSoundTimer = this;
}

SoundTimer::~SoundTimer(void)
{
	TheSoundTimer = 0;
}

void SoundTimer::start(void)
{
	connectToRealTime(1);
}

void SoundTimer::process(void)
{
	if (SoundManager)
	{
		SoundManager->update_ambient();
	}
}

void SoundTimer::stop(void)
{
	disconnectFromRealTime();
}

void SoundTimer::interruptHandler(unsigned long, unsigned long)
{
	if (SoundManager)
	{
		SoundManager->update();
	}
}

void soundInit(int irq, int port, int dma)
{
	memset(DACBuffer, 0, 256);
	for (int i = 0; i < 10; i++)
	{
		discards[i] = 0;
	}
	if (SoundManager == 0)
	{
		SoundManager = new SndMgr(257, 10);
	}
	SoundManager->init_sound_blaster(irq, port, dma);
	new SoundTimer;
	TheSoundTimer->start();
	new SoundTimerIITheSequel;
	TheSoundTimerII->start();
	if (SpkrMgr == 0)
	{
		SpkrMgr = new SpeakerManager(16);
	}
	if (SFXfile == 0)
	{
		SFXfile = new SoundEffectFlex(fileSpec(0, soundDir, soundFileName, 0), ReadOnly);
	}
	if (speechQueue == 0)
	{
		speechQueue = new char[512];
		memset(speechQueue, 0, 512);
	}
	if (decompressionBuffer == 0)
	{
		decompressionBuffer = new char[0x1400];
	}
}

void soundDeInit(void)
{
	int i;

	Kernel::killProcess(0, (ProcessType)0x246, PriorityClass(0x21));
	memset(DACBuffer, 0, 256);
	checkFlush(0);
	for (i = 0; i < 10; i++)
	{
		discards[i] = 0;
	}
	if (TheSoundTimer)
	{
		TheSoundTimer->stop();
		TheSoundTimer->pop(0);
		TheSoundTimer = 0;
	}
	if (TheSoundTimerII)
	{
		TheSoundTimerII->stop();
		TheSoundTimerII->pop(0);
		TheSoundTimerII = 0;
	}
	if (SoundManager)
	{
		delete SoundManager;
		SoundManager = 0;
	}
	if (SpkrMgr)
	{
		SpkrMgr->pop(0);
		SpkrMgr = 0;
	}
	if (voices)
	{
		delete voices;
		voices = 0;
	}
	if (SFXfile)
	{
		delete SFXfile;
		SFXfile = 0;
	}
	if (speechFile)
	{
		delete speechFile;
		speechFile = 0;
	}
	if (speechQueue)
	{
		delete speechQueue;
		speechQueue = 0;
	}
	if (decompressionBuffer)
	{
		delete decompressionBuffer;
		decompressionBuffer = 0;
	}
}

// Starts the speech for a line of text said by an NPC (666: no NPC). The
// speech files are sound\<language><npc type>.flx. Text after the spoken
// part waits in speechQueue.
unsigned char playSpeech(short npc, char *text)
{
	if (!soundOn || !*text)
	{
		return 0;
	}
	if (currentLanguage != avatar.getLanguageChar())
	{
		if (voices)
		{
			delete voices;
		}
		voices = 0;
	}
	if (voices == 0)
	{
		// List the NPC types that have speech in this language.
		currentLanguage = avatar.getLanguageChar();
		voices = new int[256];
		memset(voices, 0, 256);
		char number[10];
		int count = 0;
		struct find_t found;
		unsigned char done = _dos_findfirst(FileSpec(0, SoundDir, "*.FLX", 0), 0, &found);
		while (!done)
		{
			int i;
			for (i = 1; found.name[i] >= '0' && found.name[i] <= '9'; i++)
			{
				number[i - 1] = found.name[i];
			}
			number[i - 1] = 0;
			if (found.name[0] == avatar.getLanguageChar())
			{
				voices[count] = atol(number);
				count++;
			}
			done = _dos_findnext(&found);
		}
		voices[count] = 0;
	}
	int i = 0;
	Item speaker(npc);
	char number[10];
	if (npc != 666)
	{
		for (; i < 256 && voices[i] && speaker.getType() != voices[i]; i++)
			;
		if (i >= 256)
		{
			halt(__FILE__, 940);	// __LINE__
		}
		if (voices[i] == 0 && npc != 666)
		{
			return 0;
		}
	}
	checkFlush(SPEECH_SAMPLE);
	char name[20] = "sound\\";
	name[6] = avatar.getLanguageChar();
	name[7] = 0;
	if (npc != 666)
	{
		strcat(name, itoa(speaker.getType(), number, 10));
	}
	else
	{
		strcat(name, "666");
	}
	strcat(name, ".flx");
	if (speechFile == 0 || strcmp(name, speechFile->filename) != 0)
	{
		if (!FileExists(name))
		{
			return 0;
		}
		if (speechFile)
		{
			delete speechFile;
			speechFile = 0;
		}
		speechFile = new SpeechFlex(name, ReadOnly);
	}
	int skip = 0;
	while (text[skip] == ' ')
	{
		skip++;
	}
	if (!text[skip])
	{
		speechQueue[0] = speechQueue[1] = speechQueue[2] = 0;
		speechEnded = 1;
		return 1;
	}
	i = 0;
	while (text[i])
	{
		if (text[i] == '\t')
			text[i] = ' ';
		i++;
	}
	speechIndex = i = speechFile->search(text + skip);
	if (i == 0)
	{
		return 0;
	}
	Index index;
	speechFile->getIndex(i, index);
	if (index.size <= 2)
	{
		return 0;
	}
	SoundManager->discard(SPEECH_SAMPLE);
	SoundManager->load(SPEECH_SAMPLE, 0);
	SoundManager->play(SPEECH_SAMPLE, 0xF7F, 255, 0);
	speaking = 1;
	if (skip + strlen(text) > strlen(speechFile->phrases[i - 1]))
	{
		*(short *)speechQueue = npc;
		strcpy(speechQueue + 2, text + strlen(speechFile->phrases[i - 1]) + skip);
	}
	else
	{
		speechQueue[2] = 0;
	}
	return 1;
}

void playSFX(int sample)
{
	ActualSamplePlay(sample, 0, 100, 0x80, 0);
}

void playSFX(int sample, unsigned char volume)
{
	ActualSamplePlay(sample, 0, 100, volume, 0);
}

void singlePlay(int sample)
{
	if (soundOn)
	{
		SoundManager->flush();
		ActualSamplePlay(sample, 0, 100, 0x80, 0);
	}
}

void playSFX(int sample, int priority)
{
	ActualSamplePlay(sample, 0, priority, 0x80, 0);
}

void playSFX(int sample, int priority, unsigned short item)
{
	ActualSamplePlay(sample, item, priority, 0x80, 0);
}

void playAmbientSFX(int sample)
{
	ActualSamplePlayAmbient(sample, 0, 100, 0x80, 0);
}

void playAmbientSFX(int sample, int priority)
{
	ActualSamplePlayAmbient(sample, 0, priority, 0x80, 0);
}

void playAmbientSFX(int sample, int priority, unsigned short item)
{
	ActualSamplePlayAmbient(sample, item, priority, 0x80, 0);
}

void stopSFX(int all, unsigned short item)
{
	if (soundOn && SpkrMgr)
	{
		SpkrMgr->remove(all, item);
	}
}

void stopSFX(int sample)
{
	if (soundOn)
	{
		SoundManager->stop(SoundManager->get_sample_handle(sample));
	}
}

void discardSFX(int sample)
{
	if (soundOn)
	{
		SoundManager->discard(sample);
	}
}

void setVolumeSFX(int sample, int volume)
{
	if (soundOn)
	{
		long handle = SoundManager->get_sample_handle(sample);
		if (handle)
		{
			SoundManager->set_volume(handle, (unsigned char)(volume & 0xFF));
		}
	}
}

void rewindSFX(int sample)
{
	if (soundOn)
	{
		SoundManager->rewind(SoundManager->get_sample_handle(sample));
	}
}

unsigned char isSFXPlaying(int sample)
{
	unsigned char playing = 0;

	if (soundOn)
	{
		for (int i = 0; i < NUM_CHANNELS; i++)
		{
			if (SoundManager->channels[i].handle && SoundManager->channels[i].sample == sample)
			{
				playing = 1;
			}
		}
	}
	return playing;
}

// Reads the sound and music card settings from u8.ini.
void getSoundStuff(int &sport, int &sirq, int &sdma, int &sdrq, int &mport, int &mirq,
	int &mdma, int &mdrq, unsigned char &musicType, unsigned char &soundType,
	char *soundName, char *musicName)
{
	BaseFile ini("u8.ini", ReadWrite);
	ini.read((char *)&scard, 8);
	ini.read((char *)&mcard, 8);
	ini.close();
	musicType = mcard.type;
	if (musicType)
	{
		mdrq = mdma = mcard.dma;
		mport = mcard.port;
		mirq = mcard.irq;
		if (musicName)
		{
			strcpy(musicName, musicNames[musicType]);
		}
	}
	soundType = scard.type;
	if (soundType)
	{
		sdrq = sdma = scard.dma;
		sport = scard.port;
		sirq = scard.irq;
		if (soundName)
		{
			strcpy(soundName, soundNames[soundType]);
		}
	}
}

void playCombatMusic(char music)
{
	if (musicOn)
	{
		musicPlay(music);
	}
}

// 44 keeps the last music, to come back to after combat.
void playMusic(char music)
{
	if (music != 44)
	{
		avatar.music = music;
	}
	if (musicOn && !((Npc &)avatar).isInCombat() && !((Npc &)avatar).isCombatPending())
	{
		musicPlay(music);
	}
}

void restoreMusic(void)
{
	if (musicOn)
	{
		if (((Npc &)avatar).isInCombat() || ((Npc &)avatar).isCombatPending())
		{
			playCombatMusic(98);
			return;
		}
		playMusic(avatar.music);
	}
}

void changePitchSFX(int sample, int delta)
{
	if (soundOn)
	{
		long handle = SoundManager->get_sample_handle(sample);
		SoundManager->change_pitch(handle, delta);
	}
}

extern "C" void sndSETPASSTOPROTVEC(REALPTR *oldReal, PIHANDLER *oldProt, PIHANDLER handler, short vector)
{
	Spanky::dosSetPassToProtVec(vector, handler, oldProt, oldReal);
}

extern "C" void sndSETREALPROTVEC(REALPTR *oldReal, PIHANDLER *oldProt, REALPTR realHandler,
	PIHANDLER handler, short vector)
{
	Spanky::dosSetRealProtVec(vector, handler, realHandler, oldProt, oldReal);
}

inline void SoundTimerIITheSequel::process(void)
{
}
