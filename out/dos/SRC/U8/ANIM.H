#ifndef ANIM_H
#define ANIM_H

#include "..\GLIB\PROCESS.H"
#include "chargen.H"
#include "TYPE.H"

class Point;

// Keeps the Avatar from swinging again until a Wait after the swing ends.
class AvatarSwingToggler : public Process
{
public:
	AvatarSwingToggler(unsigned short);
	virtual void process(void) { pop(0); }
	virtual void pop(long);
	virtual void fail(long);
};

void waitForKey(void);
char *getAvatarStatus(char *);
unsigned char createItem(unsigned short, unsigned short, unsigned short, unsigned short, unsigned char);
void f1key(void);
void f2key(void);
void f3key(void);
void f4key(void);
void f5key(void);
void shiftF8Key(void);
void ctrlF8Key(void);
void altF8Key(void);
void jumpAvatar(char, char, Point *);
void carefulStepAvatar(char, char);
void avatarSwing(unsigned char);
void ctrlF1Key(void);
void ctrlF2Key(void);
void ctrlF3Key(void);
void ctrlF4Key(void);
void ctrlF5Key(void);
void shiftF1Key(void);
void shiftF2Key(void);
void shiftF3Key(void);
void shiftF4Key(void);
void shiftF5Key(void);
void altF1Key(void);
void altF2Key(void);
void altF3Key(void);
void altF4Key(void);
void altF5Key(void);
void handleTracker(unsigned short, unsigned short, unsigned short);
void igniteChaos(unsigned short, unsigned short, unsigned char);
unsigned char turnToFace(unsigned short, char, char, unsigned char);

extern char targetedJump;
extern WorldPoint avysDropZone;

#endif
