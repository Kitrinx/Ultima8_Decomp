#ifndef PAGAN_H
#define PAGAN_H

#include "..\GLIB\PROCESS.H"

class LinkedList;
class Msg;

// Writes the flag file that unlocks the quotes, once the credits end.
class MakeSemi : public Process
{
public:
	virtual void process(void);
};

// Escape stops the credits or quotes.
class CoolKeyboard : public Process
{
public:
	CoolKeyboard(void);
	virtual void process(void);
};

// Scrolls the credits or quotes text over a saved copy of the screen.
class PaganProcess : public ProcessRealTime
{
public:
	unsigned char pad_38[2];
	int music;
	LinkedList *messages;
	int state;		// -1 starting, 0 showing, 1 next message due, 2 ending
	unsigned offset;	// into text
	char *text;
	char *line;		// the message being shown
	char *next;		// the message after it
	Msg *current;
	unsigned char f_54;
	char *background;

	PaganProcess(char *, unsigned char, short, short);
	virtual void process(void);
	virtual ~PaganProcess(void);
	virtual void interruptHandler(unsigned long, unsigned long);
	unsigned char setupText(void);
	void kill(Msg *);
	static Msg *lnk;
};

void runCredits(unsigned char);
void runQuotes(void);

#endif
