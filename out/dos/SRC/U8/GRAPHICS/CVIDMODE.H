#ifndef CVIDMODE_H
#define CVIDMODE_H

class VideoMode
{
public:
	int mode;		// BIOS video mode

	VideoMode(void) { mode = 3; }
	void grab(void);
	void activate(void);
};

extern unsigned params[17];

#endif
