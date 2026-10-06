// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: MISC\CEXIT.C

#include <stdarg.h>
#include <stdio.h>
#include <process.h>
#include "CEXIT.H"
#include "INIT.H"
#include "CBIOS.H"
#include "SYSTEM.H"

// Inline in the shared header when this file was built.
inline SimpleVirtualString::operator char *(void) { return string; }

ExitSystem *ExitSystem::root = 0;
unsigned char simpleExitSystem[4] = {0, 0, 0, 0};

void ExitSystem::exit_code(void)
{
}

void ExitSystem::exit(void)
{
	ExitSystem *system;

	for (system = root; system; system = system->next)
		system->exit_code();
}

void Exit(int code)
{
	ExitSystem::exit();
	SimpleExitSystem::exit();
	exit(code);
}

void Exit(const char *format, ...)
{
	va_list args;

	ExitSystem::exit();
	FORMAT_WORKSTRING(format, args);
	conputs(WorkString);
	SimpleExitSystem::exit();
	exit(0);
}
