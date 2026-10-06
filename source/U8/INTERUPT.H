#ifndef INTERUPT_H
#define INTERUPT_H

// The 8259 PIC and DOS interrupt vector helpers.
extern "C" void setInterrupt(char, int, int);
extern "C" void *getInterrupt(char);
extern "C" void disableInterrupt(char);
extern "C" void enableInterrupt(char);
extern "C" void sendEoi(char);

#endif
