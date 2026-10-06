#ifndef ANIMCALL_H
#define ANIMCALL_H

#include "..\GLIB\PROCESS.H"
#include "MLIST.H"

// Makes an item and pops it at a spot.
class ItemCreator : public ProcessInterrupt
{
public:
	unsigned type;
	unsigned frame;
	unsigned x;
	unsigned y;
	unsigned char z;

	ItemCreator(unsigned short, unsigned short, unsigned short, unsigned short, unsigned short, unsigned char);
	virtual void process(void);
};

// Destroys its item, or the item another process made.
class ItemDestroyer : public ProcessInterrupt
{
public:

	ItemDestroyer(unsigned short);
	virtual void process(void);
	virtual void interruptHandler(unsigned long, unsigned long);
};

// Makes an item at a spot and runs its frames to the last one.
class FutureAnim : public ProcessInterrupt
{
public:
	unsigned firstFrame;
	unsigned lastFrame;
	unsigned type;
	unsigned x;
	unsigned y;
	unsigned char z;
	unsigned char started;

	FutureAnim(unsigned short, unsigned short, unsigned short, unsigned short, unsigned short, unsigned char);
	virtual void process(void);
};

int createItem(unsigned short, unsigned short, unsigned short);
void generalAnimCall(unsigned short);
void deathAnimCall(unsigned short);
void hitAnimCall(unsigned short, unsigned short);

#endif
