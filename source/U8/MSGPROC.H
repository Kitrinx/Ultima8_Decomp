#ifndef MSGPROC_H
#define MSGPROC_H

#include "CAFONT.H"
#include "CDLIST.H"

class PaganProcess;

// One message of the credits or quotes, scrolling up the screen.
class Msg : public Link
{
public:
	int indent;
	int highlight;
	int numLines;
	int y;
	char *text;
	int offScreen;
	int allShown;
	char *lines[20];
	PaganProcess *process;
	CacheFont font;
	CacheFont titleFont;
	unsigned char scroll;

	Msg(PaganProcess *, char *);
	void run(void);
	unsigned char scrolling(void);
	static unsigned fonttype;
	static unsigned char credits;
};

#endif
