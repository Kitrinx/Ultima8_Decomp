#ifndef DSFXMAN_H
#define DSFXMAN_H

#include "..\GLIB\PROCESS.H"

class DAD;
class SoundEffectFlex;
class SpeechFlex;

// Sample number of the current speech; its data comes from speechFile.
#define SPEECH_SAMPLE	255

// A sound that follows an item: its volume falls with the item's distance.
struct Speaker
{
	long handle;
	unsigned short item;
	short maxVolume;
};

class SpeakerManager : public Process
{
public:
	Speaker *speakers;
	short count;

	SpeakerManager(short);
	virtual void process(void);
	virtual ~SpeakerManager(void);
	virtual void freeMemory(void);
	virtual void postLoad(void);
	void add(long, unsigned short, short);
	void remove(int, unsigned short);
	void remove(long);
	void flush(void);
};

// Streams the next chunk of each large sample.
class SoundTimerIITheSequel : public ProcessRealTime
{
public:
	SoundTimerIITheSequel(void);
	virtual void process(void);
	virtual ~SoundTimerIITheSequel(void);
	virtual void interruptHandler(unsigned long, unsigned long);
	void start(void);
	void stop(void);
};

// Services the sound driver and resumes suspended ambient sounds.
class SoundTimer : public ProcessRealTime
{
public:
	SoundTimer(void);
	virtual void process(void);
	virtual ~SoundTimer(void);
	virtual void interruptHandler(unsigned long, unsigned long);
	void start(void);
	void stop(void);
};

// A sound card's settings, as kept in u8.ini.
class SoundCard
{
public:
	int type;
	int port;
	int irq;
	int dma;

	SoundCard(void) { type = port = irq = 0; dma = -1; }
};

unsigned char sample_load(short, DAD *&);
void sample_load_chunk(short, char *, int, long);
void sample_report(long, short);
int GetVolumeByDistance(unsigned short, short);
void checkFlush(int);
long ActualSamplePlay(int, unsigned short, unsigned char, unsigned char, int);
long ActualSamplePlayAmbient(int, unsigned short, unsigned char, unsigned char, int);
void soundInit(int, int, int);
void soundDeInit(void);
unsigned char playSpeech(short, char *);
void playSFX(int);
void playSFX(int, unsigned char);
void singlePlay(int);
void playSFX(int, int);
void playSFX(int, int, unsigned short);
void playAmbientSFX(int);
void playAmbientSFX(int, int);
void playAmbientSFX(int, int, unsigned short);
void stopSFX(int, unsigned short);
void stopSFX(int);
void discardSFX(int);
void setVolumeSFX(int, int);
void rewindSFX(int);
unsigned char isSFXPlaying(int);
void getSoundStuff(int &, int &, int &, int &, int &, int &, int &, int &, unsigned char &, unsigned char &, char *, char *);
void playCombatMusic(char);
void playMusic(char);
void restoreMusic(void);
void changePitchSFX(int, int);

extern SoundEffectFlex *SFXfile;
extern SpeechFlex *speechFile;
extern int *voices;
extern char currentLanguage;
extern SoundTimer *TheSoundTimer;
extern SoundTimerIITheSequel *TheSoundTimerII;
extern SpeakerManager *SpkrMgr;
extern char *soundDir;
extern char *soundFileName;
extern unsigned discards[10];
extern unsigned speechEnded;
extern char *speechQueue;
extern unsigned speechIndex;
extern char *decompressionBuffer;
extern unsigned char speaking;
extern char *soundNames[4];
extern char *musicNames[7];
extern SoundCard mcard;
extern SoundCard scard;

#endif
