#ifndef APPDEBUG_H
#define APPDEBUG_H

int howManyFreeHandles(void);
void standardDebug(char *, int);
void initDebug(void);
void applicationGarbageCollect(unsigned char);
void applicationOutOfMemory(unsigned short);

#endif
