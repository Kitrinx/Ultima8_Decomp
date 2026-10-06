// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: WAIT.C

#include "KERNEL.H"
#include "WAIT.H"

RoundTimer::RoundTimer(unsigned short loops, unsigned short item)
{
	setProcessType((ProcessType)0x233);
	Kernel::setIdString(pid, "RoundTimer");
	count = loops;
	setRef(item);
}

Wait::Wait(unsigned short ticks, unsigned short item)
{
	Kernel::setIdString(pid, "Wait");
	setRef(item);
	this->ticks = ticks;
	setProcessType((ProcessType)0x226);
	count = ticks;
	setHertz(30);
	flags |= PROC_SUSPENDED;
	setSafeInterrupt();
	intFlags = 2;
}

void Wait::start(void)
{
	connectToRealTime(1);
}

void Wait::restart(void)
{
	count = ticks;
	flags |= PROC_SUSPENDED;
	intFlags = 2;
}

void Wait::process(void)
{
	timerDone();
}

void Wait::interruptHandler(unsigned long event, unsigned long)
{
	switch (event)
	{
	case 2:
		if (--count == 0)
		{
			flags &= ~PROC_SUSPENDED;
			setIntFlags(intFlags & ~2L);
			return;
		}
		break;
	case 4:
		if (!isOnRealTime())
			start();
		break;
	}
}

void FailWait::interruptHandler(unsigned long event, unsigned long)
{
	switch (event)
	{
	case 2:
		if (--count == 0)
		{
			timerDone();
			fail(0);
		}
		break;
	}
}

Timer::Timer(void)
{
	Kernel::setIdString(pid, "Timer");
	setSafeInterrupt();
	setDaemon();
	setProcessType((ProcessType)0x201);
	seconds = 0;
	tenths = 0;
	running = 0;
	setHertz(10);
	setIntFlags(intFlags | 2);
}

void Timer::stop(void)
{
	running = 0;
	disconnectFromRealTime();
}

void Timer::start(void)
{
	running = 1;
	seconds = 0;
	tenths = 0;
	connectToRealTime(1);
}

void Timer::interruptHandler(unsigned long event, unsigned long)
{
	switch (event)
	{
	case 2:
		tenths++;
		if (tenths == 10)
		{
			seconds++;
			tenths = 0;
		}
		break;
	}
}

AccurateTimer::AccurateTimer(int hertz)
{
	Kernel::setIdString(pid, "AccurateTimer");
	setSafeInterrupt();
	setDaemon();
	setProcessType((ProcessType)0x226);
	ticks = 0;
	timing = 0;
	setHertz(hertz);
	setIntFlags(intFlags | 2);
}

void AccurateTimer::stop(void)
{
	timing = 0;
	disconnectFromRealTime();
}

void AccurateTimer::start(void)
{
	timing = 1;
	ticks = 0;
	connectToRealTime(1);
}

void AccurateTimer::interruptHandler(unsigned long event, unsigned long)
{
	switch (event)
	{
	case 2:
		ticks++;
		break;
	}
}
