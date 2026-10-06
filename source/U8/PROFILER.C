// flags: -P -2 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: PROFILER.C

#include <phapi.h>
#include <stdio.h>
#include <string.h>
#include "CFILE.H"
#include "KERNEL.H"
#include "PROFILER.H"

// Inline in the shared headers when this file was built.
inline unsigned char BaseFile::is_valid(void) { return handle != -1; }
inline BaseFile::~BaseFile(void) { if (is_valid()) close(); }

// Samples the running process type 500 times a second.
Profiler::Profiler(void)
{
	Kernel::setIdString(pid, "Profiler");
	setHertz(500);
	intFlags |= 2;
	setProcessType((ProcessType)5);
	setSafeInterrupt();
	flags |= PROC_SUSPENDED;
	setDaemon();
}

void Profiler::interruptHandler(unsigned long, unsigned long)
{
	Kernel::checkAccumulator();
}

void Profiler::start(void)
{
	Kernel::clrAccumulator();
	connectToRealTime(1);
}

void Profiler::stop(void)
{
	unsigned long type;
	unsigned long total;
	unsigned long count;
	char line[81];
	BaseFile file;

	disconnectFromRealTime();
	for (type = 0, total = 0; type <= 0xfff; type++)
		total += Kernel::accumulatorTable[type];
	file.create("profiler.dat", ReadWrite);
	sprintf(line, "- Kernel Process Type Frequency Data -\n\n");
	file.write(line, strlen(line));
	for (type = 0; type <= 0xfff; type++)
		if (Kernel::accumulatorTable[type]) {
			count = Kernel::accumulatorTable[type];
			sprintf(line, "Process type:  0x%5lx - reps=%5ld - %2d%\n\r", type, count, count * 100 / total);
			file.write(line, strlen(line));
		}
}

void Profiler::process(void)
{
}
