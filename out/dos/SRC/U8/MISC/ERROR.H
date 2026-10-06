#ifndef ERROR_H
#define ERROR_H

enum TraceLevel
{
	TraceLevel_0,
	TRACE_ALWAYS = 255
};

void halt(char *, short);
void halt(char *, short, char *, ...);
void outOfMemory(char *, short);
void outOfMemory(char *, short, char *, ...);
void unableToOpenFile(char *, short, char *);
void trace(TraceLevel, char *, ...);
void trace(TraceLevel, short, short, char *, ...);
void traceGet(TraceLevel, char *, ...);
void traceGet(TraceLevel, short, short, char *, ...);

extern int traceLevel;

#endif
