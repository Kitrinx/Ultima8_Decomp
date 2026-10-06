#ifndef DEBUGUMP_H
#define DEBUGUMP_H

#include "CFILE.H"
#include "DEBUGGER.H"
#include "TEXTGUMP.H"

class Event;
class NewGump;
class NewGumpId;
class Rect;
class WindowFrame;

// A usecode source file split into lines.
class ViewFile : public BaseFile
{
public:
	unsigned numLines;
	unsigned long lines[4000];
	char name[9];
	char *text;

	ViewFile(void);
	int parse(unsigned long *, char *, unsigned long, unsigned long);
	unsigned char setFile(char *);
	void loadFile(void);
	char *getLine(unsigned short);
	unsigned getNumLines(void) { return numLines; }
};

class SourceDebugger : public Debugger
{
public:
	void atBreakPoint(void);
	void takeControl(void);
};

class DebugScroller : public Scroller
{
public:
	int cursorX;
	int cursorY;
	Debugger *owner;

	DebugScroller(NewGumpId, NewGump *, Rect, Debugger *, WindowFrame *);
	virtual void commandKeyboard(Event &);
	virtual void draw(short, short);
	void cursorGotoOrigin(void);
	void scrollTo(int);
	int parseLine(char *, char *, int);
	void trackCursor(void);
	unsigned char setFile(char *);
	char *getSourceFile(void) { return viewFile->name; }
	static ViewFile *viewFile;
};

class DebugGump : public WindowGump
{
public:
	WindowFrame *sourceFrame;
	DebugScroller *scroller;
	WindowFrame *inputFrame;
	WindowGump *input;

	DebugGump(NewGumpId, NewGump *, Rect, Debugger *, unsigned char);
	virtual ~DebugGump(void);
	virtual void commandPrivate(Event &);
	virtual void commandKeyboard(Event &);
	virtual void draw(short, short);
	void getInput(char *, char *);
	void message(char *);
	void dumpSymbol(char *, char *, char *, char *, int);
	void doMenuCommand(Event &);
	void buildMenu(void);
	static unsigned char init;
	static char *watches[10];
	static unsigned char fromInterpreter;
};

void initDebugger(void);

#endif
