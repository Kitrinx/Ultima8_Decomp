#ifndef DEBUGGER_H
#define DEBUGGER_H

class BreakPoint
{
public:
	char file[9];
	int line;
};

// One entry of the usecode call stack the debugger follows.
class Info
{
public:
	char file[9];
	char *symbols;
	char *data;
	char *process;
};

class Debugger
{
public:
	int numBreaks;
	BreakPoint breaks[10];
	unsigned currentLine;
	unsigned char tracing;
	unsigned char steppingOver;
	long stepDepth;	// calls entered since the step began
	int currentInfo;
	Info infos[30];

	Debugger(void);
	virtual void atBreakPoint(void);
	virtual void graphicVideoMode(void);
	Info *getCurrentInfo(void) { return currentInfo ? &infos[currentInfo - 1] : 0; }
	void lineChange(unsigned short);
	void addBreakPoint(char *, int);
	void removeBreakPoint(char *, int);
	int getNextBreak(char *, int);
	unsigned char isBreakPointAt(char *, int);
	void pushFile(char *, char *, char *, char *);
	void pushFile(Info *);
	void popFile(void);
	void stepOver(void);
	void reset(void);
	char *currentFile(void);
};

#endif
