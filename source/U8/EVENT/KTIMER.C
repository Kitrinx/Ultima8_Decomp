// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: EVENT\KTIMER.C

#include "KTIMER.H"

// Inline in the shared headers when this file was built.
inline void nameProcess(unsigned pid, char *name) { ((char **)Kernel::idStringList)[pid] = name; }

AilTimer *ailTimer = 0;

// 54920 microseconds: the PC's default 18.2 Hz tick.
AilTimer::AilTimer(void)
{
	setPeriod(54920L);
	intFlags = 2;
	setProcessType(0x100);
	nameProcess(pid, "AilTimer");
	setSafeInterrupt();
	flags |= 1;
	setDaemon();
}

void AilTimer::interruptHandler(unsigned long, unsigned long)
{
	API_timer();
}

void hook_timer_process(void)
{
	ailTimer->connectToRealTime(1);
}

void unhook_timer_process(void)
{
	ailTimer->disconnectFromRealTime();
}

// Called from assembly, so keeps every register.
void set_PIT_period(int period)
{
	asm pusha;
	asm push es;
	ailTimer->setPeriod((unsigned)period);
	asm pop es;
	asm popa;
}

void AilTimer::process(void)
{
}
