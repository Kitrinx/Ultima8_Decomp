#ifndef SNDMGR_H
#define SNDMGR_H

// A loaded sample: a 32-byte header, then the 8-bit data.
class DAD
{
public:
	long size;
	short rate;
	short f_06;
	short f_08;
	short f_0a;
	short f_0c;
	unsigned char pad_0e[18];
	char data[1];
};

struct SampleEntry
{
	short users;
	DAD *dad;
};

// Samples over STREAM_SIZE bytes play from two STREAM_CHUNK buffers that
// are refilled from the file while the other one plays.
#define STREAM_CHUNK	0x7F00
#define STREAM_SIZE		0xFE00L

#define NUM_CHANNELS	4

struct SoundChannel
{
	long handle;
	short sample;
	short priority;
	short volume;
	short rate;
	char ambient;
	char streamed;
	short chunksLeft;
	short lastChunkSize;
	char *bufferA;
	char *bufferB;
	long filePos;
	char playB;
	char needLoad;
};

class SndMgr
{
public:
	short sampleCount;
	SampleEntry *samples;
	unsigned short dmaParagraph;
	unsigned short dmaSelector;
	short maxSuspended;
	short numSuspended;
	SoundChannel *suspended;
	short activeChannels;
	long lastHandle;
	long memoryUsed;
	SoundChannel channels[NUM_CHANNELS];

	SndMgr(short, short);
	~SndMgr(void);
	unsigned char init_sound_blaster(short, short, short);
	unsigned char load(short, DAD *);
	void stop_channel(short);
	void discard(short);
	long play(short, short, short, int);
	void rewind(long);
	void stop(long);
	long play_ambient(short, short, short, int);
	void release_ambient(long);
	void stop_ambient(long);
	int get_sample_number(long);
	long get_sample_handle(short);
	void stop_all(void);
	void flush(void);
	void set_volume(long, short);
	void set_pitch(long &, short);
	void change_pitch(long &, int);
	void update(void);
	void update_ambient(void);
	void update_huge_samples(void);
	int get_channel(short, short);
	void suspend_ambient(short);
	void resume_ambient(short);
};

void SoundBlasterExitListHook(void);
extern "C" void digital_call_back(int);

// Digital sound driver.
extern "C" {
int DGTL_init(int port, int irq, int dma, unsigned selector, int, unsigned paragraph);
void DGTL_shutdown(void);
void DGTL_channel_volume(int channel, int volume);
void DGTL_channel_rate(int channel, int rate);
void DGTL_play(int channel, char *data, int length, int rate, int volume);
void DGTL_stop(int channel);
void DGTL_play_ambient(int channel, char *data, int length, int, int, int rate, int volume);
void DGTL_release(int channel);
unsigned char DAC_update(void);
extern unsigned char DACBuffer[512];
}

extern SndMgr *SoundManager;
extern unsigned char SoundSystemActive;

#endif
