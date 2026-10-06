#ifndef FERROR_H
#define FERROR_H

void Fatal(const char *, ...);
void Fatal(short);
void Fatal(short, short);

extern char *ExitMessage;
extern char *ErrorGroupMessage;
extern char *ErrorSubgroupMessage;

#endif
