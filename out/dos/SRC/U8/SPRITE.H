#ifndef SPRITE_H
#define SPRITE_H

#include "..\GLIB\PROCESS.H"

// A short-lived item that plays its frames from first to last, then from last to end.
class Sprite : public Process
{
public:
	int firstFrame;
	int lastFrame;
	int endFrame;
	int frame;
	unsigned repeat;
	unsigned delay;
	unsigned char forward;
	unsigned char turned;

	Sprite(unsigned short, unsigned short, unsigned short, unsigned short, short, unsigned short, unsigned short, unsigned short, unsigned char);
	virtual void process(void);
};

int createSprite(short, short, short, short, short, short, unsigned short, unsigned short, unsigned char);
int createSprite(short, short, short, short, unsigned short, unsigned short, unsigned char);

#endif
