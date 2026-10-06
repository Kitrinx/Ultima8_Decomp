// flags: -P -2 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: PROCESS.C

#include <dos.h>
#include "..\GLIB\PROCESS.H"
#include "CFILE.H"
#include "KERNEL.H"
#include "ERROR.H"
#include "CPROTMEM.H"
#include "STDINT.H"

Process::Process(void)
{
	// Processes are found through pointers kept by the kernel.
	if (_SS == FP_SEG(this))
		halt(__FILE__, 35);	// __LINE__
	ref = 0;
	flags = 0;
	f_10 = 1;
	result = 0;
	pid = 0;
	processType = (ProcessType)1;
	allocations = 0;
	messageGroups = 0;
	priority = 0;
	setPriority(Kernel::currentPriority);
	Kernel::spawn(this);
	if (Kernel::processing) {
		unsigned current = Kernel::getCurrentProcess();
		if (current) {
			Process *parent = Kernel::getProcess(current);
			if (parent->isF10_2())
				parent->f_10 |= 4;
		}
	}
	Kernel::checkSynchronization(this);
}

void *Process::operator new(unsigned size)
{
	void *p;

	p = ::operator new(size);
	if (!p)
		halt(__FILE__, 79);	// __LINE__
	Kernel::setCurrentProcessSize(size);
	return p;
}

void Process::operator delete(void *p)
{
	if (!((Process *)p)->isTerminated())
		halt(__FILE__, 100);	// __LINE__
}

void Process::setRef(unsigned short newRef)
{
	unsigned oldRef = ref;
	ProcessType type;

	ref = newRef;
	Kernel::setProcessRefActive(newRef);
	if (oldRef)
		Kernel::checkProcessRefActive(oldRef);
	type = processType;
	if (type >= 0xf0 && type <= 0xf7) {
		if (newRef)
			Kernel::setProcessTypeActive(newRef, type);
		if (oldRef)
			Kernel::checkProcessTypeActive(oldRef, type);
	}
}

void Process::setProcessType(ProcessType type)
{
	if (ref == 0)
		processType = type;
	else {
		ProcessType oldType = processType;
		processType = type;
		if (type >= 0xf0 && type <= 0xf7)
			Kernel::setProcessTypeActive(ref, type);
		if (oldType >= 0xf0 && oldType <= 0xf7)
			Kernel::checkProcessTypeActive(ref, oldType);
	}
	if (processType > 0xfff)
		halt(__FILE__, 179);	// __LINE__
}

void Process::setPriority(unsigned char newPriority)
{
	if (newPriority == 0 || newPriority > 31)
		halt(__FILE__, 198);	// __LINE__
	if (getPriority() && getPriority() == newPriority)
		return;
	if (getPriority())
		Kernel::decPriorityCount(getPriority());
	priority = newPriority;
	Kernel::incPriorityCount(getPriority());
}

void Process::notifyDependents(void)
{
	Process *next;
	PidEntry entry;	// unused, but it keeps a stack slot

	if (isLinearExecutor()) {
		Kernel::getLinearExecutor(ref, processType);
		next = Kernel::getNextLinear();
		clrLinearExecutor();
		if (Kernel::isProcess(next))
			next->setLinearExecutor();
	}
	notifyList.notify(pid);
}

void Process::unlinkFromKernel(void)
{
	if (isOnRealTime())
		((ProcessRealTime *)this)->disconnectFromRealTime();
	if (isOnIntHandler())
		((ProcessInterrupt *)this)->disconnectFromIntHandler();
	Kernel::makeInvalid(pid);
}

void Process::kill(void)
{
	notifyDependents();
	freeMemory();
}

void Process::disableCriticalInterrupts(void)
{
	if (isOnRealTime())
		((ProcessInterrupt *)this)->intFlags &= ~2L;
}

void Process::terminate(void)
{
	if (isPhantom())
		return;
	if (isTerminated())
		return;
	if (isPhantom() || isTerminated())
		halt(__FILE__, 326);	// __LINE__
	disableCriticalInterrupts();
	flags |= PROC_TERMINATED;
	kill();
	callDestructor();
	if (allocations)
		halt(__FILE__, 367);	// __LINE__
	unlinkFromKernel();
}

// The dependency list is an expression in reverse Polish order, ended by
// DEP_END: a dead pid counts as true, a live one as false.
void Process::checkDependencies(unsigned short deadPid)
{
	Process *p;
	ProcessInterrupt *pi;
	unsigned short i;
	unsigned member;
	unsigned a, b;
	PidEntry entry;
	unsigned myPid = pid;

	if (deadPid) {
		p = Kernel::getProcess(deadPid);
		p->clrActiveDep(myPid);
		clrPassiveDep(deadPid);
	}
	for (i = 0; dependList.getMember(i) != DEP_END; i++)
		if (dependList.getMember(i) == deadPid)
			dependList.setMember(i, DEP_TRUE);

	for (i = 0;; i++) {
		member = dependList.getMember(i);
		switch (member) {
		case DEP_TRUE:
			asm push 1
			break;
		case DEP_AND:
			asm pop a
			asm pop b
			if (a && b)
				asm push 1
			else
				asm push 0
			break;
		case DEP_OR:
			asm pop a
			asm pop b
			if (a || b)
				asm push 1
			else
				asm push 0
			break;
		case DEP_END:
			asm pop member
			if (member) {
				for (i = 0; (entry = dependList.getMember(i)).value != DEP_END; i++) {
					if (entry.isPid()) {
						p = Kernel::getProcess(entry.value);
						p->clrActiveDep(myPid);
						clrPassiveDep(p->pid);
					}
				}
				Kernel::checkSynchronization(this);
				dependList.clear();
				if (isInterrupt()) {
					pi = (ProcessInterrupt *)this;
					if (pi->intFlags & 4) {
						pi->interruptHandler(4, 0);
						return;
					}
				}
			}
			return;
		default:
			asm push 0
			break;
		}
	}
}

// Copies the DEP_END-terminated arguments onto the stack again and passes
// them on to constructList.
void Process::makeDependencyList(...)
{
	int size;

	asm {
		mov bx, bp
		add bx, 10
		push ss
		pop es
		mov di, bx
		mov ax, DEP_END
		mov dx, 0x400
		mov cx, dx
		cld
		repne scasw
		je found
	}
	halt(__FILE__, 542);	// __LINE__
found:
	asm {
		sub dx, cx
		mov cx, dx
		shl dx, 1
		mov size, dx
		add bx, dx
		sub bx, 2
	}
again:
	asm {
		mov ax, ss:[bx]
		push ax
		sub bx, 2
		loop again
	}
	dependList.constructList(pid);
	asm add sp, size
}

void Process::clrPassiveInt(unsigned short p)
{
	PidEntry entry;
	unsigned short index;

	entry = notifyList.isPidInList(p, index);
	if (!entry.value)
		return;
	entry.value &= ~0x800;
	notifyList.setMember(index, entry.value);
	if (!isTerminated() && !entry.isUsed())
		notifyList.removeFromList(entry.value);
}

void Process::clrActiveInt(unsigned short p)
{
	PidEntry entry;
	unsigned short index;

	entry = notifyList.isPidInList(p, index);
	if (!entry.value)
		return;
	entry.value &= ~0x1000;
	notifyList.setMember(index, entry.value);
	if (!isTerminated() && !entry.isUsed())
		notifyList.removeFromList(entry.value);
}

void Process::clrPassiveDep(unsigned short p)
{
	PidEntry entry;
	unsigned short index;

	entry = notifyList.isPidInList(p, index);
	if (!entry.value)
		return;
	entry.value &= ~0x2000;
	notifyList.setMember(index, entry.value);
	if (!isTerminated() && !entry.isUsed())
		notifyList.removeFromList(entry.value);
}

void Process::clrActiveDep(unsigned short p)
{
	PidEntry entry;
	unsigned short index;

	entry = notifyList.isPidInList(p, index);
	if (!entry.value)
		return;
	entry.value &= ~0x4000;
	notifyList.setMember(index, entry.value);
	if (!isTerminated() && !entry.isUsed())
		notifyList.removeFromList(entry.value);
}

void Process::setPassiveInt(unsigned short p)
{
	PidEntry entry;
	unsigned short index;

	entry = notifyList.isPidInList(p, index);
	if (entry.value) {
		entry.value |= 0x800;
		notifyList.setMember(index, entry.value);
		return;
	}
	entry = p;
	entry.value |= 0x800;
	notifyList.appendToList(entry.value);
}

void Process::setActiveInt(unsigned short p)
{
	PidEntry entry;
	unsigned short index;

	entry = notifyList.isPidInList(p, index);
	if (entry.value) {
		entry.value |= 0x1000;
		notifyList.setMember(index, entry.value);
		return;
	}
	entry = p;
	entry.value |= 0x1000;
	notifyList.appendToList(entry.value);
}

void Process::setPassiveDep(unsigned short p)
{
	PidEntry entry;
	unsigned short index;

	entry = notifyList.isPidInList(p, index);
	if (entry.value) {
		entry.value |= 0x2000;
		notifyList.setMember(index, entry.value);
		return;
	}
	entry = p;
	entry.value |= 0x2000;
	notifyList.appendToList(entry.value);
}

void Process::setActiveDep(unsigned short p)
{
	PidEntry entry;
	unsigned short index;

	entry = notifyList.isPidInList(p, index);
	if (entry.value) {
		entry.value |= 0x4000;
		notifyList.setMember(index, entry.value);
		return;
	}
	entry = p;
	entry.value |= 0x4000;
	notifyList.appendToList(entry.value);
}

// This process waits for child.
Process *Process::push(Process *child)
{
	char waiting;

	if (child->ref == 0)
		child->setRef(ref);
	else if (ref == 0)
		setRef(child->ref);
	child->setActiveDep(pid);
	setPassiveDep(child->pid);
	waiting = dependList.isAllocated();
	dependList.appendToList(child->pid);
	if (waiting)
		dependList.appendToList(DEP_AND);
	return child;
}

void Process::cpush(Process *child)
{
	setFlags(flags | PROC_CONDITIONAL);
	push(child);
}

// next waits for this process.
Process *Process::then(Process *next)
{
	char waiting;

	if (next->ref == 0)
		next->setRef(ref);
	else if (ref == 0)
		setRef(next->ref);
	setActiveDep(next->pid);
	next->setPassiveDep(pid);
	waiting = next->dependList.isAllocated();
	next->dependList.appendToList(pid);
	if (waiting)
		next->dependList.appendToList(DEP_AND);
	return next;
}

void Process::cthen(Process *next)
{
	next->setFlags(next->flags | PROC_CONDITIONAL);
	then(next);
}

unsigned short Process::push(unsigned short child)
{
	Process *p = Kernel::getProcess(child);
	push(p);
	return child;
}

void Process::pop(long value)
{
	flags &= ~PROC_FAILED;
	result = value;
	terminate();
}

void Process::fail(long value)
{
	flags |= PROC_FAILED;
	result = value;
	terminate();
}

void Process::sendMessage(Message message, MessageAddress address, MessageGroup group, long data)
{
	Kernel::sendMessage(message, address, group, pid, data);
}

void Process::sendMessage(Message message, unsigned short to, long data)
{
	Process *p = Kernel::getProcess(to);
	if (p->acceptsMessages())
		p->receiveMessage(message, pid, data);
}

void Process::setLinearProcess(void)
{
	if (isDaemon())
		halt(__FILE__, 942);	// __LINE__
	flags |= PROC_LINEAR;
	if (processType == 1)
		halt(__FILE__, 949);	// __LINE__
	dependList.setLinearType(processType);
	if (!isLinearExecutor())
		Kernel::appendLinearProcess(this);
}

void Process::clrLinearProcess(void)
{
	Kernel::setCurrentLinearScanPid(pid);
	if (Kernel::getNextLinear())
		halt(__FILE__, 965);	// __LINE__
	flags &= ~PROC_LINEAR;
	dependList.setLinearType(0);
}

void Process::setDaemon(void)
{
	if (isLinearProcess())
		halt(__FILE__, 976);	// __LINE__
	flags |= PROC_DAEMON;
}

void Process::setLinearExecutor(void)
{
	if (!isLinearProcess())
		halt(__FILE__, 985);	// __LINE__
	if (isDaemon())
		halt(__FILE__, 989);	// __LINE__
	flags |= PROC_EXECUTOR;
}

void Process::clrLinearExecutor(void)
{
	flags &= ~PROC_EXECUTOR;
}

void Process::setPhantom(void)
{
	if (isF10_4())
		halt(__FILE__, 1011);	// __LINE__
	if (isPhantom())
		halt(__FILE__, 1014);	// __LINE__
	flags |= PROC_PHANTOM;
	Kernel::numPhantoms++;
}

void Process::freeMemory(void)
{
	if (dependList.isAllocated())
		dependList.clear();
	if (notifyList.isAllocated())
		notifyList.clear();
}

void Process::load(BaseFile *file)
{
	dependList.load(file);
	notifyList.load(file);
}

void Process::save(BaseFile *file)
{
	dependList.save(file);
	notifyList.save(file);
}

int Process::allocate(unsigned long size)
{
	void *p;

	p = ProtMemoryManager::allocate(size, pid, 1);
	if (p) {
		allocations++;
		return (int)p;
	}
	halt(__FILE__, 1101);	// __LINE__
}

void Process::deallocate(void *p)
{
	if (allocations) {
		ProtMemoryManager::free(p, pid);
		allocations--;
		return;
	}
	halt(__FILE__, 1117);	// __LINE__
}

void ProcessInterrupt::doNotNotifyOfDeath(unsigned short other)
{
	Process *p = Kernel::getProcess(other);
	if (!p->isInterrupt())
		halt(__FILE__, 1137);	// __LINE__
	clrActiveInt(other);
	p->clrPassiveInt(pid);
}

void ProcessInterrupt::notifyOfDeath(unsigned short other)
{
	Process *p = Kernel::getProcess(other);
	if (!p->isInterrupt())
		halt(__FILE__, 1161);	// __LINE__
	setActiveInt(other);
	p->setPassiveInt(pid);
}

void ProcessInterrupt::connectToIntHandler(void)
{
	stdIntHandler->intList.addToList(pid);
	flags |= PROC_INTHANDLER;
}

void ProcessInterrupt::disconnectFromIntHandler(void)
{
	stdIntHandler->intList.removeFromList(pid);
	flags &= ~PROC_INTHANDLER;
}

ProcessRealTime::ProcessRealTime(void)
{
	setFlags(flags | PROC_REALTIME);
	hertz = 0;
	elapsed = 0;
	period = 0;
}

void ProcessRealTime::setHertz(unsigned short hz)
{
	Kernel::realTimeOff();
	hertz = hz;
	period = 1000000L / hz;
	elapsed = 0;
	Kernel::calculateTimerRate();
	Kernel::realTimeOn();
}

void ProcessRealTime::setPeriod(unsigned long microseconds)
{
	Kernel::realTimeOff();
	hertz = 1000000L / microseconds;
	period = microseconds;
	elapsed = 0;
	Kernel::calculateTimerRate();
	Kernel::realTimeOn();
}

void ProcessRealTime::realTimeCycle(void)
{
	unsigned long t = elapsed;

	t += Kernel::pitPeriod;
	if (t < period) {
		elapsed = t;
		return;
	}
	t -= period;
	elapsed = t;
	interruptHandler(2, 0);
}

void ProcessRealTime::connectToRealTime(unsigned char level)
{
	Kernel::addToRealTime(pid, level);
}

void ProcessRealTime::disconnectFromRealTime(void)
{
	asm pushf
	asm cli
	Kernel::removeFromRealTime(pid);
	flags &= ~PROC_ONREALTIME;
	asm popf
}

void ProcessRealTime::setSafeInterrupt(void)
{
	if (!isSafeInterrupt()) {
		asm pushf
		asm cli
		setFlags(flags | PROC_SAFE);
		if (isOnRealTime()) {
			Kernel::totalNonSafe--;
			Kernel::totalSafe++;
		}
		asm popf
	}
}

void ProcessRealTime::clrSafeInterrupt(void)
{
	if (isSafeInterrupt()) {
		asm pushf
		asm cli
		setFlags(flags & ~PROC_SAFE);
		if (isOnRealTime()) {
			Kernel::totalSafe--;
			Kernel::totalNonSafe++;
		}
		asm popf
	}
}
