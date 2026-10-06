// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: AUXFUNCS.C

#include "..\GLIB\PROCESS.H"
#include "KERNEL.H"
#include "NPC.H"
#include "MAIN.H"
#include "STATREST.H"
#include "STDINT.H"

HeadShaker *headShaker = 0;

HeadShaker::HeadShaker(void)
{
	Kernel::setIdString(pid, "HeadShaker");
	setProcessType((ProcessType)0x235);
	seconds = 0;
	wait = rndRange(4, 8);
	setDaemon();
	setHertz(1);
	intFlags |= 2;
	connectToRealTime(1);
	headShaker = this;
}

void HeadShaker::postLoad(void)
{
	headShaker = this;
}

void HeadShaker::interruptHandler(unsigned long event, unsigned long)
{
	if (!inGameMode)
		return;
	switch (event)
	{
	case 2:
		if (++seconds >= wait)
		{
			int last, first, second;

			wait = rndRange(4, 8);
			if (!avatar.isBusy() && !avatar.isInCombat() && !avatar.inStasis && !avatar.isDead())
			{
				if (OneIn(4))
				{
					if (OneIn(2))
					{
						first = 0x20;
						second = 0x21;
					}
					else
					{
						first = 0x21;
						second = 0x20;
					}
					avatar.doAnim((AnimSet)first, 8, 0, 0);
					avatar.doAnim((AnimSet)2, 8, 1, 0);
					avatar.doAnim((AnimSet)second, 8, 2, 0);
					avatar.doAnim((AnimSet)2, 8, 3, 0);
					return;
				}
				last = avatar.getLastAnimSet();
				if (last == 2)
					avatar.doAnim((AnimSet)(OneIn(2) ? 0x20 : 0x21), 8, 0, 0);
				else
					avatar.doAnim((AnimSet)2, 8, 0, 0);
			}
		}
		break;
	}
}

void HeadShaker::freeMemory(void)
{
	Process::freeMemory();
	headShaker = 0;
}

void HeadShaker::animationUsed(void)
{
	seconds = 0;
}
