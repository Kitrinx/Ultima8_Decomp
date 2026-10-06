// flags: -P -2 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: STDINT.C

#include "..\GLIB\PROCESS.H"
#include "KERNEL.H"
#include "CFILE.H"
#include "STDINT.H"

StdIntHandler *stdIntHandler = 0;

StdIntHandler::StdIntHandler(void)
{
	setProcessType((ProcessType)2);
	Kernel::setIdString(pid, "StdIntHandler");
	setDaemon();
}

void StdIntHandler::process(void)
{
	ProcessInterrupt *p;
	unsigned long mask;
	unsigned long bit;
	unsigned long arg;
	int i;

	intList.last = intList.count;
	if (intList.last)
	{
		intList.last--;
		for (intList.current = 0; intList.current < intList.last; intList.current++)
		{
			Kernel::currentPolledIntPid = intList.getMember(intList.current);
			intList.changed = 0;
			p = (ProcessInterrupt *)Kernel::getProcess(Kernel::currentPolledIntPid);
			mask = p->intFlags;
			if (mask)
			{
				if (p->isSkipped())
					continue;
				for (i = 8; i <= 31; i++)
				{
					bit = 1 << i;
					if ((mask & bit) && p->checkInterrupt(bit, arg))
					{
						p->interruptHandler(bit, arg);
						if (intList.changed)
							break;
					}
				}
			}
			Kernel::currentPolledIntPid = 0;
		}
	}
}

void StdIntHandler::freeMemory(void)
{
	Process::freeMemory();
	if (intList.isAllocated())
		intList.clear();
}

void StdIntHandler::load(BaseFile *file)
{
	Process::load(file);
	intList.load(file);
}

void StdIntHandler::save(BaseFile *file)
{
	Process::save(file);
	intList.save(file);
}
