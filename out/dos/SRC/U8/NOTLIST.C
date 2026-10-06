// flags: -P -2 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: NOTLIST.C

#include "..\GLIB\PROCESS.H"
#include "KERNEL.H"
#include "ERROR.H"

// Tells every process linked to the dead process `pid` that it has ended.
void NotifyList::notify(unsigned short pid)
{
	unsigned i;
	PidEntry entry;
	long result;
	Process *p;
	Process *dead;
	unsigned char failed;

	if (isAllocated()) {
		dead = Kernel::getProcess(pid);
		result = dead->result;
		if (dead->flags & PROC_FAILED)
			failed = 1;
		else
			failed = 0;
		for (i = 0; (entry = getMember(i)).value != DEP_END; i++) {
			p = Kernel::getProcess(entry.getPid());
			if (entry.isPassiveInt())
				p->clrActiveInt(pid);
			if (entry.isPassiveDep())
				p->clrActiveDep(pid);
			if (entry.isActiveDep()) {
				p->setResult(result);
				if (failed) {
					p->flags |= PROC_DEPFAILED;
					if (p->flags & PROC_CONDITIONAL) {
						p->fail(result);
						continue;
					}
				} else
					p->flags &= ~PROC_DEPFAILED;
				p->checkDependencies(pid);
			}
			if (entry.isActiveInt()) {
				p->clrPassiveInt(pid);
				p->setResult(result);
				if (failed)
					p->flags |= PROC_DEPFAILED;
				else
					p->flags &= ~PROC_DEPFAILED;
				if (p->isInterrupt()) {
					ProcessInterrupt *pi = (ProcessInterrupt *)p;

					if (pi->intFlags & 1) {
						if (!pi->isTerminated() || pi->pid == pid)
							pi->interruptHandler(1, pid);
					}
				} else
					halt(__FILE__, 179);	// __LINE__
			}
		}
	}
}
