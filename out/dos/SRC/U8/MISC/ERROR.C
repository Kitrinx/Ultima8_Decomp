// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: MISC\ERROR.C

#include <conio.h>
#include <stdarg.h>
#include <stdio.h>
#include "ERROR.H"
#include "FERROR.H"
#include "CPROTMEM.H"
#include "APPDEBUG.H"
#include "SYSTEM.H"

// Inline in the shared header when this file was built.
inline SimpleVirtualString::operator char *(void) { return string; }

int traceLevel;

void halt(char *file, short line)
{
	if (ProtMemoryManager::failedLastRequest)
		outOfMemory(file, line);
	else
		Fatal("Halting in file %s, line %d.\n", file, line);
}

void halt(char *file, short line, char *format, ...)
{
	va_list args;

	if (format)
		FORMAT_WORKSTRING(format, args);
	else
		*(char *)WorkString = 0;
	textmode(3);
	trace(TRACE_ALWAYS, WorkString);
	trace(TRACE_ALWAYS, "\r\nHalting in file %s, line %d.\n", file, line);
	trace(TRACE_ALWAYS, "\r\n");
	Fatal(1);
}

void outOfMemory(char *file, short line)
{
	textmode(3);
	trace(TRACE_ALWAYS, "Out of memory in file %s, line %d.\n\r", file, line);
	applicationOutOfMemory(100 - FRAGMENT_0_PERCENTAGE);
	Fatal(1);
}

void outOfMemory(char *file, short line, char *format, ...)
{
	va_list args;

	if (format)
		FORMAT_WORKSTRING(format, args);
	else
		*(char *)WorkString = 0;
	textmode(3);
	trace(TRACE_ALWAYS, WorkString);
	trace(TRACE_ALWAYS, "\r\n");
	trace(TRACE_ALWAYS, "Out of memory in file %s, line %d.\n", file, line);
	applicationOutOfMemory(100 - FRAGMENT_0_PERCENTAGE);
	Fatal(1);
}

void unableToOpenFile(char *file, short line, char *name)
{
	textmode(3);
	trace(TRACE_ALWAYS, "Unable to open file %s in %s, line %d\n", name, file, line);
	Fatal(1);
}

void trace(TraceLevel level, char *format, ...)
{
	va_list args;

	FORMAT_WORKSTRING(format, args);
	if (level >= traceLevel)
		printf(WorkString);
}

void trace(TraceLevel level, short x, short y, char *format, ...)
{
	va_list args;

	FORMAT_WORKSTRING(format, args);
	if (level >= traceLevel)
	{
		gotoxy(x, y);
		printf(WorkString);
	}
}

// Like trace, then waits for a key.
void traceGet(TraceLevel level, char *format, ...)
{
	va_list args;

	FORMAT_WORKSTRING(format, args);
	if (level >= traceLevel)
	{
		printf(WorkString);
		getch();
	}
}

void traceGet(TraceLevel level, short x, short y, char *format, ...)
{
	va_list args;

	FORMAT_WORKSTRING(format, args);
	if (level >= traceLevel)
	{
		gotoxy(x, y);
		printf(WorkString);
		getch();
	}
}
