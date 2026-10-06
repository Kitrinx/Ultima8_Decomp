// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: ..\SOUND\SNDMGR.C

#include <dos.h>
#include <mem.h>
#include <stdio.h>
#include <stdlib.h>
#include <phapi.h>
#include "CEXIT.H"
#include "SNDMGR.H"
#include "DSFXMAN.H"

unsigned char SoundSystemActive = 0;
SndMgr *SoundManager = 0;

SndMgr::SndMgr(short numSamples, short numSuspend)
{
	sampleCount = numSamples;
	samples = (SampleEntry *)malloc(sampleCount * sizeof(SampleEntry));
	setmem(samples, sampleCount * sizeof(SampleEntry), 0);
	maxSuspended = numSuspend;
	numSuspended = 0;
	suspended = (SoundChannel *)malloc(maxSuspended * sizeof(SoundChannel));
	setmem(suspended, maxSuspended * sizeof(SoundChannel), 0);
	setmem(channels, sizeof(channels), 0);
	dmaParagraph = 0;
	dmaSelector = 0;
	activeChannels = 0;
	lastHandle = 0;
	memoryUsed = 0;
}

SndMgr::~SndMgr(void)
{
	memset(MK_FP(dmaSelector, 0), 0, 256);
	if (SoundSystemActive)
	{
		DGTL_shutdown();
	}
	SoundSystemActive = 0;
	if (samples)
	{
		for (int i = 0; i < 256; i++)
		{
			if (samples[i].dad)
			{
				delete samples[i].dad;
				samples[i].dad = 0;
			}
		}
		delete samples;
		samples = 0;
	}
	if (suspended)
	{
		delete suspended;
		suspended = 0;
	}
	if (dmaSelector)
	{
		DosFreeSeg(dmaSelector);
	}
}

void SoundBlasterExitListHook(void)
{
	if (SoundSystemActive)
	{
		SoundSystemActive = 0;
		DGTL_shutdown();
	}
}

unsigned char SndMgr::init_sound_blaster(short irq, short port, short dma)
{
	if (DosAllocRealSeg(256, &dmaParagraph, &dmaSelector))
	{
		return 0;
	}
	memset(MK_FP(dmaSelector, 0), 0, 256);
	while ((dmaParagraph & 0xFFFFL) + 16 > 0xFFFFL)
	{
		printf("Boundary error in sound blaster initialization.\n");
	}
	// Mixer register 0x83
	asm {
		push ax
		push dx
		mov dx, port
		add dx, 4
		mov al, 0x83
		out dx, al
		inc dx
		mov al, 0x0B
		out dx, al
		pop dx
		pop ax
	}
	if (DGTL_init(port, irq, dma, dmaSelector, 0, dmaParagraph) == -1)
	{
		return 0;
	}
	SoundSystemActive = 1;
	return 1;
}

unsigned char SndMgr::load(short sample, DAD *dad)
{
	if (samples[sample].users > 0)
	{
		samples[sample].users++;
	}
	else
	{
		if (dad)
		{
			samples[sample].dad = dad;
		}
		else if (!sample_load(sample, samples[sample].dad))
		{
			return 0;
		}
		if (samples[sample].dad)
		{
			samples[sample].users++;
			memoryUsed += samples[sample].dad->size + 32;
		}
	}
	return 1;
}

void SndMgr::stop_channel(short channel)
{
	if (channels[channel].handle == 0)
	{
		return;
	}
	activeChannels--;
	channels[channel].handle = 0;
	DGTL_stop(channel);
}

void SndMgr::discard(short sample)
{
	int i;

	if (samples == 0)
	{
		return;
	}
	if (samples[sample].users <= 0)
	{
		return;
	}
	samples[sample].users--;
	if (samples[sample].users != 0)
	{
		return;
	}
	for (i = 0; i < NUM_CHANNELS; i++)
	{
		if (channels[i].sample == sample)
		{
			stop_channel(i);
		}
	}
	if (numSuspended)
	{
		for (i = 0; i < maxSuspended; i++)
		{
			if (suspended[i].sample == sample)
			{
				suspended[i].handle = 0;
				numSuspended--;
			}
		}
	}
	if (samples[sample].dad)
	{
		memoryUsed -= samples[sample].dad->size + 32;
		delete samples[sample].dad;
		samples[sample].dad = 0;
	}
}

long SndMgr::play(short sample, short priority, short volume, int pitch)
{
	if (samples[sample].users == 0)
	{
		return 0;
	}
	if (samples[sample].dad == 0)
	{
		samples[sample].users = 0;
		return 0;
	}
	asm pushf
	asm cli
	int channel = get_channel(sample, priority);
	if (channel == -1)
	{
		asm popf
		return 0;
	}
	activeChannels++;
	channels[channel].handle = ++lastHandle;
	channels[channel].sample = sample;
	channels[channel].priority = priority;
	channels[channel].volume = volume;
	channels[channel].rate = samples[sample].dad->rate - pitch;
	channels[channel].ambient = 0;
	channels[channel].streamed = 0;
	channels[channel].needLoad = 0;
	sample_report(channels[channel].handle, 1);
	if (samples[sample].dad->size > STREAM_SIZE)
	{
		channels[channel].streamed = 1;
		channels[channel].filePos = STREAM_SIZE;
		channels[channel].chunksLeft = samples[sample].dad->size / STREAM_CHUNK - 1;
		channels[channel].lastChunkSize = samples[sample].dad->size % STREAM_CHUNK;
		if (channels[channel].lastChunkSize)
		{
			channels[channel].chunksLeft++;
		}
		channels[channel].playB = 1;
		channels[channel].bufferA = samples[sample].dad->data;
		channels[channel].bufferB = samples[sample].dad->data + STREAM_CHUNK;
		DGTL_play(channel, channels[channel].bufferA, STREAM_CHUNK,
			channels[channel].rate, channels[channel].volume);
	}
	else
	{
		channels[channel].chunksLeft = 0;
		DGTL_play(channel, samples[sample].dad->data, samples[sample].dad->size,
			channels[channel].rate, channels[channel].volume);
	}
	asm popf
	return channels[channel].handle;
}

void SndMgr::rewind(long handle)
{
	int i;

	for (i = 0; i < NUM_CHANNELS && channels[i].handle != handle; i++)
		;
	if (i < NUM_CHANNELS)
	{
		sample_report(channels[i].handle, 0);
		sample_report(channels[i].handle, 1);
		DGTL_play(i, samples[channels[i].sample].dad->data, samples[channels[i].sample].dad->size,
			samples[channels[i].sample].dad->rate, channels[i].volume);
	}
}

void SndMgr::stop(long handle)
{
	int i;

	for (i = 0; i < NUM_CHANNELS && channels[i].handle != handle; i++)
		;
	if (i < NUM_CHANNELS)
	{
		sample_report(channels[i].handle, 0);
		stop_channel(i);
	}
}

long SndMgr::play_ambient(short sample, short priority, short volume, int pitch)
{
	if (samples[sample].users == 0)
	{
		return 0;
	}
	asm pushf
	asm cli
	int channel = get_channel(sample, priority);
	if (channel == -1)
	{
		asm popf
		return 0;
	}
	activeChannels++;
	channels[channel].handle = ++lastHandle;
	channels[channel].sample = sample;
	channels[channel].priority = priority;
	channels[channel].volume = volume;
	channels[channel].rate = samples[sample].dad->rate - pitch;
	channels[channel].ambient = 1;
	channels[channel].streamed = 0;
	channels[channel].needLoad = 0;
	sample_report(channels[channel].handle, 1);
	DGTL_play_ambient(channel, samples[sample].dad->data, samples[sample].dad->size,
		samples[sample].dad->f_08, samples[sample].dad->f_0c,
		channels[channel].rate, channels[channel].volume);
	asm popf
	return channels[channel].handle;
}

void SndMgr::release_ambient(long handle)
{
	int i;

	for (i = 0; i < NUM_CHANNELS && channels[i].handle != handle; i++)
		;
	if (i < NUM_CHANNELS)
	{
		if (channels[i].ambient == 0)
		{
			return;
		}
		sample_report(channels[i].handle, 2);
		DGTL_release(i);
		return;
	}
	else if (numSuspended)
	{
		for (i = 0; i < maxSuspended && suspended[i].handle != handle; i++)
			;
		if (i < maxSuspended)
		{
			sample_report(suspended[i].handle, 0);
			numSuspended--;
			suspended[i].handle = 0;
		}
	}
}

void SndMgr::stop_ambient(long handle)
{
	int i;

	for (i = 0; i < NUM_CHANNELS && channels[i].handle != handle; i++)
		;
	if (i < NUM_CHANNELS)
	{
		sample_report(channels[i].handle, 0);
		stop_channel(i);
		return;
	}
	if (numSuspended)
	{
		for (i = 0; i < maxSuspended && suspended[i].handle != handle; i++)
			;
		if (i < maxSuspended)
		{
			sample_report(suspended[i].handle, 0);
			numSuspended--;
			suspended[i].handle = 0;
		}
	}
}

int SndMgr::get_sample_number(long handle)
{
	for (int i = 0; i < NUM_CHANNELS; i++)
	{
		if (channels[i].handle == handle)
		{
			return channels[i].sample;
		}
	}
	return -1;
}

long SndMgr::get_sample_handle(short sample)
{
	for (int i = 0; i < NUM_CHANNELS; i++)
	{
		if (channels[i].sample == sample)
		{
			return channels[i].handle;
		}
	}
	return 0xFFFF;
}

void SndMgr::stop_all(void)
{
	int i;

	asm pushf
	asm cli
	for (i = 0; i < NUM_CHANNELS; i++)
	{
		if (channels[i].handle)
		{
			sample_report(channels[i].handle, 0);
			stop_channel(i);
		}
	}
	for (i = 0; i < maxSuspended; i++)
	{
		if (suspended[i].handle)
		{
			sample_report(suspended[i].handle, 0);
			numSuspended--;
			suspended[i].handle = 0;
		}
	}
	asm popf
}

void SndMgr::flush(void)
{
	stop_all();
	setmem(suspended, maxSuspended * sizeof(SoundChannel), 0);
	setmem(channels, sizeof(channels), 0);
	for (int i = 0; i < sampleCount; i++)
	{
		if (samples[i].users)
		{
			memoryUsed -= samples[i].dad->size + 32;
			if (samples[i].dad)
			{
				delete samples[i].dad;
				samples[i].dad = 0;
			}
			samples[i].users = 0;
		}
	}
}

void SndMgr::set_volume(long handle, short volume)
{
	int i;

	for (i = 0; i < NUM_CHANNELS && channels[i].handle != handle; i++)
		;
	if (i < NUM_CHANNELS)
	{
		DGTL_channel_volume(i, volume);
		channels[i].volume = volume;
		return;
	}
	if (numSuspended)
	{
		for (i = 0; i < maxSuspended && suspended[i].handle != handle; i++)
			;
		if (i < maxSuspended)
		{
			suspended[i].volume = volume;
		}
	}
}

void SndMgr::set_pitch(long &handle, short rate)
{
	int i;

	for (i = 0; i < NUM_CHANNELS && channels[i].handle != handle; i++)
		;
	if (i < NUM_CHANNELS)
	{
		channels[i].rate = rate;
		DGTL_channel_rate(i, channels[i].rate);
		return;
	}
	if (numSuspended)
	{
		for (i = 0; i < maxSuspended && suspended[i].handle != handle; i++)
			;
		if (i < maxSuspended)
		{
			suspended[i].rate = rate;
		}
	}
}

void SndMgr::change_pitch(long &handle, int delta)
{
	int i;

	for (i = 0; i < NUM_CHANNELS && channels[i].handle != handle; i++)
		;
	if (i < NUM_CHANNELS)
	{
		channels[i].rate += delta;
		DGTL_channel_rate(i, channels[i].rate);
		return;
	}
	if (numSuspended)
	{
		for (i = 0; i < maxSuspended && suspended[i].handle != handle; i++)
			;
		if (i < maxSuspended)
		{
			suspended[i].rate += delta;
		}
	}
}

void SndMgr::update(void)
{
	DAC_update();
}

// Gives free channels back to suspended ambient sounds.
void SndMgr::update_ambient(void)
{
	int i = 0;

	while (activeChannels < NUM_CHANNELS && numSuspended > 0)
	{
		if (channels[i].handle == 0)
		{
			resume_ambient(i);
		}
		i++;
		if (i == NUM_CHANNELS)
		{
			break;
		}
	}
}

// Loads the next chunk of each streamed sample into the buffer not playing.
void SndMgr::update_huge_samples(void)
{
	int i;
	char *buffer;

	for (i = 0; i < NUM_CHANNELS; i++)
	{
		if (channels[i].needLoad)
		{
			if (channels[i].playB)
			{
				buffer = channels[i].bufferA;
			}
			else
			{
				buffer = channels[i].bufferB;
			}
			if (channels[i].chunksLeft == 2)
			{
				sample_load_chunk(channels[i].sample, buffer, channels[i].lastChunkSize, channels[i].filePos);
			}
			else
			{
				sample_load_chunk(channels[i].sample, buffer, STREAM_CHUNK, channels[i].filePos);
			}
			channels[i].playB = !channels[i].playB;
			channels[i].filePos += STREAM_CHUNK;
			channels[i].chunksLeft--;
			channels[i].needLoad = 0;
		}
	}
}

// Finds a free channel, or takes the one with the lowest priority if it is
// not above the new sound's.
int SndMgr::get_channel(short sample, short priority)
{
	int i;

	if (activeChannels < NUM_CHANNELS)
	{
		if (samples[sample].users == 0 && load(sample, 0) == 0)
		{
			return -1;
		}
		for (i = 0; i < NUM_CHANNELS && channels[i].handle; i++)
			;
		if (i >= NUM_CHANNELS)
		{
			Fatal("Play count mismatch, pc=%u", activeChannels);
		}
	}
	else
	{
		i = 0;
		for (int j = 1; j < NUM_CHANNELS; j++)
		{
			if (channels[j].priority < channels[i].priority)
			{
				i = j;
			}
		}
		if (channels[i].priority <= priority)
		{
			if (samples[sample].users == 0 && load(sample, 0) == 0)
			{
				return -1;
			}
			if (channels[i].ambient == 1)
			{
				suspend_ambient(i);
			}
			else
			{
				sample_report(channels[i].handle, 0);
			}
			stop_channel(i);
		}
		else
		{
			return -1;
		}
	}
	return i;
}

void SndMgr::suspend_ambient(short channel)
{
	int i;

	if (numSuspended == maxSuspended)
	{
		// The original reads channels[i] before i is set.
		sample_report(channels[i].handle, 0);
		return;
	}
	for (i = 0; i < maxSuspended && suspended[i].handle; i++)
		;
	if (i < maxSuspended)
	{
		numSuspended++;
		suspended[i] = channels[channel];
		sample_report(suspended[i].handle, 3);
	}
}

void SndMgr::resume_ambient(short channel)
{
	int i;

	for (i = 0; i < maxSuspended && !suspended[i].handle; i++)
		;
	if (i < maxSuspended)
	{
		numSuspended--;
		channels[channel] = suspended[i];
		suspended[i].handle = 0;
		activeChannels++;
		sample_report(channels[channel].handle, 4);
		DGTL_play_ambient(channel, samples[channels[channel].sample].dad->data,
			samples[channels[channel].sample].dad->size,
			samples[channels[channel].sample].dad->f_08,
			samples[channels[channel].sample].dad->f_0c,
			channels[channel].rate, channels[channel].volume);
	}
}

// Called by the driver when a channel finishes its buffer.
extern "C" void digital_call_back(int channel)
{
	if (SoundManager->channels[channel].streamed)
	{
		if (SoundManager->channels[channel].chunksLeft)
		{
			char *buffer;
			if (SoundManager->channels[channel].playB)
			{
				buffer = SoundManager->channels[channel].bufferB;
			}
			else
			{
				buffer = SoundManager->channels[channel].bufferA;
			}
			if (SoundManager->channels[channel].chunksLeft > 1)
			{
				SoundManager->channels[channel].needLoad = 1;
				DGTL_play(channel, buffer, STREAM_CHUNK,
					SoundManager->channels[channel].rate, SoundManager->channels[channel].volume);
				return;
			}
			DGTL_play(channel, buffer, SoundManager->channels[channel].lastChunkSize,
				SoundManager->channels[channel].rate, SoundManager->channels[channel].volume);
			SoundManager->channels[channel].chunksLeft = 0;
			return;
		}
		sample_report(SoundManager->channels[channel].handle, 0);
		SoundManager->activeChannels--;
		SoundManager->channels[channel].handle = 0;
		return;
	}
	sample_report(SoundManager->channels[channel].handle, 0);
	SoundManager->activeChannels--;
	SoundManager->channels[channel].handle = 0;
}
