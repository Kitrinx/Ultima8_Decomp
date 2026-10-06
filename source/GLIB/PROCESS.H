#ifndef PROCESS_H
#define PROCESS_H

#include "KERNEL.H"
#include "MLIST.H"
#include "NOTLIST.H"

class BaseFile;
class Process;
class ProcessInterrupt;

// Process::flags bits
#define PROC_SUSPENDED		0x0001
#define PROC_DAEMON		0x0002
#define PROC_INTHANDLER	0x0004
#define PROC_PHANTOM		0x0008
#define PROC_CONDITIONAL	0x0010
#define PROC_FAILED		0x0020
#define PROC_SKIP		0x0040
#define PROC_MESSAGES		0x0080
#define PROC_INTERRUPT		0x0100
#define PROC_REALTIME		0x0200
#define PROC_ONREALTIME	0x0400
#define PROC_LINEAR		0x0800
#define PROC_EXECUTOR		0x1000
#define PROC_SAFE		0x2000
#define PROC_TERMINATED	0x4000
#define PROC_DEPFAILED		0x8000	// a process this one waited on failed
#define PROC_DEPFAILED	0x8000	// a process this one was notified of failed

// Dependency list words: a pid, or one of these operators.
#define DEP_TRUE	0x401
#define DEP_AND		0x402
#define DEP_OR		0x404
#define DEP_END		0x408

class Process
{
public:
	unsigned pid;
	unsigned ref;
	ProcessType processType;
	unsigned char priority;
	unsigned char pad_09[1];
	long result;
	unsigned flags;
	unsigned f_10;
	unsigned allocations;
	unsigned messageGroups;
	List dependList;
	NotifyList notifyList;

	Process(void);
	virtual void process(void) = 0;
	virtual ~Process(void) {}
	virtual void pop(long);
	virtual void fail(long);
	virtual void receiveMessage(Message, unsigned short, long);
	virtual void freeMemory(void);
	virtual void load(BaseFile *);
	virtual void save(BaseFile *);
	virtual void postLoad(void);
	static void *operator new(unsigned);
	static void operator delete(void *);
	void setRef(unsigned short);
	void setProcessType(ProcessType);
	void setPriority(unsigned char);
	void notifyDependents(void);
	void unlinkFromKernel(void);
	void kill(void);
	void disableCriticalInterrupts(void);
	void terminate(void);
	void checkDependencies(unsigned short);
	void makeDependencyList(...);
	void clrPassiveInt(unsigned short);
	void clrActiveInt(unsigned short);
	void clrPassiveDep(unsigned short);
	void clrActiveDep(unsigned short);
	void setPassiveInt(unsigned short);
	void setActiveInt(unsigned short);
	void setPassiveDep(unsigned short);
	void setActiveDep(unsigned short);
	Process *push(Process *);
	void cpush(Process *);
	Process *then(Process *);
	void cthen(Process *);
	unsigned short push(unsigned short);
	void sendMessage(Message, MessageAddress, MessageGroup, long);
	void sendMessage(Message, unsigned short, long);
	void setLinearProcess(void);
	void clrLinearProcess(void);
	void setDaemon(void);
	void setLinearExecutor(void);
	void clrLinearExecutor(void);
	void setPhantom(void);
	int allocate(unsigned long);
	void deallocate(void *);
	void callDestructor(void);

	unsigned char getPriority(void) { return priority & 0x1f; }
	void setPid(unsigned p) { pid = p; }
	void setFlags(unsigned f) { flags = f; }
	void setResult(long r) { result = r; }
	long getResult(void) { return result; }
	unsigned char isSuspended(void) { return (flags & PROC_SUSPENDED) ? 1 : 0; }
	unsigned char isDaemon(void) { return (flags & PROC_DAEMON) ? 1 : 0; }
	unsigned char isOnIntHandler(void) { return (flags & PROC_INTHANDLER) ? 1 : 0; }
	unsigned char isPhantom(void) { return (flags & PROC_PHANTOM) ? 1 : 0; }
	unsigned char isSkipped(void) { return (flags & PROC_SKIP) ? 1 : 0; }
	unsigned char acceptsMessages(void) { return (flags & PROC_MESSAGES) ? 1 : 0; }
	unsigned char isInterrupt(void) { return (flags & PROC_INTERRUPT) ? 1 : 0; }
	unsigned char isOnRealTime(void) { return (flags & PROC_ONREALTIME) ? 1 : 0; }
	unsigned char isLinearProcess(void) { return (flags & PROC_LINEAR) ? 1 : 0; }
	unsigned char isLinearExecutor(void) { return (flags & PROC_EXECUTOR) ? 1 : 0; }
	unsigned char isSafeInterrupt(void) { return (flags & PROC_SAFE) ? 1 : 0; }
	unsigned char isTerminated(void) { return (flags & PROC_TERMINATED) ? 1 : 0; }
	unsigned char dependencyFailed(void) { return (flags & PROC_DEPFAILED) ? 1 : 0; }
	unsigned char isFailed(void) { return (flags & PROC_FAILED) ? 1 : 0; }
	unsigned char isF10_2(void) { return (f_10 & 2) ? 1 : 0; }
	unsigned char isF10_4(void) { return (f_10 & 4) ? 1 : 0; }
	unsigned char isSaved(void) { return (f_10 & 1) ? 1 : 0; }
	unsigned char isRealTime(void) { return (flags & PROC_REALTIME) ? 1 : 0; }
};

class ProcessInterrupt : public Process
{
public:
	unsigned long intFlags;

	ProcessInterrupt(void) { flags |= PROC_INTERRUPT; intFlags = 0; }
	virtual unsigned char checkInterrupt(unsigned long, unsigned long &);
	virtual void interruptHandler(unsigned long, unsigned long);
	void doNotNotifyOfDeath(unsigned short);
	void notifyOfDeath(unsigned short);
	void connectToIntHandler(void);
	void disconnectFromIntHandler(void);
};

class ProcessRealTime : public ProcessInterrupt
{
public:
	unsigned hertz;
	unsigned long period;
	unsigned long elapsed;

	ProcessRealTime(void);
	void setHertz(unsigned short);
	void setPeriod(unsigned long);
	void realTimeCycle(void);
	void connectToRealTime(unsigned char);
	void disconnectFromRealTime(void);
	void setSafeInterrupt(void);
	void clrSafeInterrupt(void);
	void setIntFlags(unsigned long f) { intFlags = f; }
};

#endif
