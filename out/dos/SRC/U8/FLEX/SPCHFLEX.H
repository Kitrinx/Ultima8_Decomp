#ifndef SPCHFLEX_H
#define SPCHFLEX_H

#include "..\POLIB\INC\FLEX.H"
#include "CFILE.H"

// A flex of sound samples; offsets[i] is where record i's sample data
// starts, past its 32-byte header, or 0 if it has none.
class SoundEffectFlex : public FlexFile
{
public:
	long *offsets;

	SoundEffectFlex(char *, OpenMode);
	virtual ~SoundEffectFlex(void);
};

// Speech samples; record 0 lists the phrases they say, one per record.
class SpeechFlex : public SoundEffectFlex
{
public:
	int numPhrases;
	char **phrases;

	SpeechFlex(char *, OpenMode);
	virtual ~SpeechFlex(void);
	int search(char *);
};

#endif
