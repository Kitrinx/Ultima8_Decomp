// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: DEBUGGER.C

#include <mem.h>
#include <string.h>
#include "ERROR.H"
#include "DEBUGGER.H"

#define MAX_BREAKS	10
#define MAX_INFOS	30

Debugger *debugger = 0;

Debugger::Debugger(void)
{
	memset(breaks, -1, sizeof breaks);
	numBreaks = 0;
	currentInfo = 0;
	steppingOver = 0;
}

// Called by the interpreter for each new source line.
void Debugger::lineChange(unsigned short line)
{
	currentLine = line;
	if (isBreakPointAt(currentFile(), currentLine - 1))
	{
		atBreakPoint();
		return;
	}
	if (steppingOver)
	{
		if (stepDepth <= 0)
			atBreakPoint();
		return;
	}
	if (tracing)
		atBreakPoint();
}

// Keeps the list sorted by file, then line.
void Debugger::addBreakPoint(char *file, int line)
{
	BreakPoint *b;
	int i;
	if (numBreaks < MAX_BREAKS)
	{
		b = breaks;
		for (i = 0; i < numBreaks; i++, b++)
		{
			int order = stricmp(file, b->file);
			if (order < 0 || b->line > line && order == 0)
			{
				movmem(b, b + 1, (numBreaks - i) * sizeof(BreakPoint));
				break;
			}
		}
		strcpy(b->file, file);
		b->line = line;
		numBreaks++;
	}
}

void Debugger::removeBreakPoint(char *file, int line)
{
	BreakPoint *b;
	int i;
	if (numBreaks <= 0)
		return;
	b = breaks;
	for (i = 0; i < numBreaks; i++, b++)
	{
		if (b->line == line && !stricmp(file, b->file))
		{
			movmem(b + 1, b, (numBreaks - i) * sizeof(BreakPoint));
			numBreaks--;
			return;
		}
	}
}

// Returns the first break point in `file` at or after `line`, or -1.
int Debugger::getNextBreak(char *file, int line)
{
	BreakPoint *b = breaks;
	int i;
	for (i = 0; i < numBreaks; i++, b++)
	{
		if (!stricmp(b->file, file))
		{
			for (; i < numBreaks; i++, b++)
			{
				if (b->line >= line)
					return i;
			}
		}
	}
	return -1;
}

unsigned char Debugger::isBreakPointAt(char *file, int line)
{
	BreakPoint *b = breaks;
	int i;
	for (i = 0; i < numBreaks; i++, b++)
	{
		if (b->line == line && !stricmp(b->file, file))
			return 1;
	}
	return 0;
}

void Debugger::pushFile(char *file, char *symbols, char *data, char *process)
{
	Info *info = &infos[currentInfo];
	strcpy(info->file, file);
	if (strlen(info->file) > 8)
		halt(__FILE__, 137);	// __LINE__
	info->symbols = symbols;
	info->data = data;
	info->process = process;
	currentInfo++;
	if (currentInfo >= MAX_INFOS)
		halt(__FILE__, 144);	// __LINE__
}

void Debugger::pushFile(Info *info)
{
	memcpy(&infos[currentInfo], info, sizeof(Info));
	currentInfo++;
	if (currentInfo >= MAX_INFOS)
		halt(__FILE__, 152);	// __LINE__
}

void Debugger::popFile(void)
{
	currentInfo--;
	if (currentInfo < 0)
		halt(__FILE__, 161);	// __LINE__
}

void Debugger::stepOver(void)
{
	stepDepth = 0;
	steppingOver = 1;
}

void Debugger::reset(void)
{
	tracing = 0;
	steppingOver = 0;
}

char *Debugger::currentFile(void)
{
	if (currentInfo > 0)
		return infos[currentInfo - 1].file;
	return 0;
}

void Debugger::atBreakPoint(void)
{
}

void Debugger::graphicVideoMode(void)
{
}
