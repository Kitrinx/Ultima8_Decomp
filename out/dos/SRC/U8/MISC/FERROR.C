// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: MISC\FERROR.C

#include <process.h>
#include <stdarg.h>
#include <stdio.h>
#include "CEXIT.H"
#include "INIT.H"
#include "CBIOS.H"

char *ExitMessage = "Program halted by code.\n";
char *ErrorGroupMessage = "\nError #%04x\n";
char *ErrorSubgroupMessage = "\nError #%04x, subclass %04x\n";

void Fatal(const char *format, ...)
{
	static char depth = 0;
	va_list args;
	char message[256];

	// A failing exit_code may call Fatal again; only the first call reports.
	if (depth == 0)
	{
		if (format && *format && format != message)
		{
			va_start(args, format);
			vsprintf(message, format, args);
		}
		else
			message[0] = 0;
		depth++;
		ExitSystem::exit();
		conputs(message);
		conputs(ExitMessage);
		depth--;
		SimpleExitSystem::exit();
		exit(1);
	}
}

void Fatal(short error)
{
	Fatal(ErrorGroupMessage, error);
}

void Fatal(short error, short subclass)
{
	Fatal(ErrorSubgroupMessage, error, subclass);
}
