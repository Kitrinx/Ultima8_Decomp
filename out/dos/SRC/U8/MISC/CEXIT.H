#ifndef CEXIT_H
#define CEXIT_H

#include "ERROR.H"
#include "FERROR.H"

// Objects whose exit_code runs at exit, newest first.
class ExitSystem
{
public:
	ExitSystem *next;

	ExitSystem(void) { next = root; root = this; }
	virtual void exit_code(void);
	static void exit(void);
	static ExitSystem *root;
};

void Exit(int);
void Exit(const char *, ...);

extern unsigned char simpleExitSystem[4];

#endif
