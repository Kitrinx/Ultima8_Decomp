#ifndef KERNEL_H
#define KERNEL_H

#include <phapi.h>

class IntList;
class PriorityClass;
class Process;

enum Message
{
	Message_0
};

enum MessageAddress
{
	MessageAddress_0
};

enum MessageGroup
{
	MessageGroup_0
};

enum ProcessType
{
	ProcessType_0,
	PT_ANY = 6,	// matches every type in a search
	PT_END = 7,	// ends a list of types
	PT_FIRST = 0xf0	// the first type tracked per item
};

class Kernel
{
public:
	static void init(void);
	static void uninit(void);
	static void error(void);
	static void doProcesses(void);
	static void calculateProcessPriority(void);
	static void reconfigureCheck(void);
	static void reconfigureShrink(unsigned short);
	static unsigned char reconfigureExpand(void);
	static int getNumProcesses(unsigned short, ProcessType);
	static int getNumLinearProcesses(unsigned short, ProcessType);
	static int getOpenSlot(void);
	static unsigned spawn(Process *);
	static Process *getProcess(unsigned short);
	static void resetRef(unsigned short, ProcessType);
	static void killProcess(unsigned short, ProcessType, PriorityClass);
	static void killProcessSpecifiedPTypes(unsigned short, PriorityClass, ...);
	static void killProcessNonSpecifiedPTypes(unsigned short, PriorityClass, ...);
	static void restart(void);
	static void addDepElement(unsigned short, unsigned short);
	static Process *findValidProcess(unsigned short, ProcessType);
	static Process *findNextValidProcess(void);
	static Process *getLinearExecutor(unsigned short, ProcessType);
	static Process *getNextLinear(void);
	static void setCurrentLinearScanPid(unsigned short);
	static unsigned getCurrentProcess(void);
	static void appendLinearProcess(Process *);
	static void makeInvalid(unsigned short);
	static void remove(unsigned short);
	static void setCurrentPriority(unsigned char);
	static void checkSynchronization(Process *);
	static unsigned char shouldSynchronize(Process *);
	static void sendMessage(Message, MessageAddress, MessageGroup, unsigned short, long);
	static void setCurrentProcessSize(unsigned short);
	static void initMemory(void);
	static void freeProcesses(void);
	static void freeDaemons(void);
	static void freePhantoms(void);
	static void freeMemory(void);
	static void load(char *);
	static void save(char *);
	static void setPitDivisor(unsigned short);
	static void setPitPeriod(unsigned long);
	static void realTimeExecutor(void);
	static unsigned char isRealInterrupt(void);
	static REGS_BINT *getRegsStructure(void);
	static void addToRealTime(unsigned short, unsigned char);
	static void removeFromRealTime(unsigned short);
	static void seizeRealTimeVector(void);
	static void releaseRealTimeVector(void);
	static void enableRealTimeInterrupts(void);
	static void disableRealTimeInterrupts(void);
	static void calculateTimerRate(void);
	static void execute(unsigned long, void (*)(void));
	static void realTimeOn(void);
	static void realTimeOff(void);
	static void interrupt realTimeInterruptHandler(REGS_BINT);
	static void interruptHandlerStage2(void);
	static void checkIntegrity(void);
	static void setProcessRefActive(unsigned short);
	static void clrProcessRefActive(unsigned short);
	static void checkProcessRefActive(unsigned short);
	static unsigned char isProcessRefActive(unsigned short);
	static void setProcessTypeActive(unsigned short, ProcessType);
	static void clrProcessTypeActive(unsigned short, ProcessType);
	static void checkProcessTypeActive(unsigned short, ProcessType);
	static unsigned char isProcessTypeActive(unsigned short, ProcessType);
	static void clrAccumulator(void);
	static void checkAccumulator(void);
	static Process **processList;
	static unsigned *processSize;
	static unsigned reconfigureCounter;
	static unsigned currentScanSlot;
	static unsigned currentLinearScanPid;
	static unsigned maxProcesses;
	static unsigned totalProcesses;
	static unsigned currentProcess;
	static unsigned currentProcessSize;
	static unsigned char expandOnDemand;
	static unsigned char processing;
	static unsigned char initialized;
	static unsigned char keyboardVectorChanged[1];
	static unsigned char keyboardBufferOverflow[1];
	static unsigned char restarting;
	static unsigned numPhantoms;
	static unsigned searchingRef;
	static unsigned searchingProcessType;
	static unsigned char keyboardBufferHead[2];
	static unsigned char keyboardBufferTail[2];
	static unsigned *priorityCount;
	static unsigned long theCurrentTick;
	static unsigned char currentPriority;
	static unsigned char abortDoProcesses;
	static unsigned currentPolledIntPid;
	static unsigned char origKeyboardProt[4];
	static unsigned char origKeyboardReal[4];
	static char **idStringList;
	static void decPriorityCount(unsigned char p) { priorityCount[p]--; }
	static void incPriorityCount(unsigned char p) { priorityCount[p]++; }
	static unsigned char isProcess(Process *p) { return p != 0; }
	static void setIdString(unsigned short id, char *name) { idStringList[id] = name; }
	static void setProcessSize(unsigned short id, unsigned size) { processSize[id] = size; }
	static unsigned pitDivisor;
	static unsigned long pitPeriod;
	static IntList realTimeList;
	static unsigned char interruptDebug;
	static unsigned hertz;
	static unsigned totalSafe;
	static unsigned totalNonSafe;
	static unsigned interruptStackSegment;
	static unsigned interruptStackOffset;
	static unsigned interruptStackSize;
	static unsigned exceptionStackSegment;
	static unsigned exceptionStackOffset;
	static unsigned exceptionStackSize;
	static unsigned oldIntStackSeg;
	static unsigned oldIntStackOff;
	static unsigned oldExceptionStackSeg;
	static unsigned oldExceptionStackOff;
	static unsigned currentInterruptPid;
	static unsigned currentExceptionPid;
	static unsigned char inInterrupt;
	static unsigned char interruptPending;
	static unsigned char easyAtomicFlag;
	static unsigned char intStatusChanged;
	static unsigned char realTimeEnabled;
	static unsigned char timerVectorChanged;
	static unsigned char wantRealTime;
	static unsigned char reentrancyFlag;
	static unsigned char reentrantException;
	static unsigned char processBlank;
	static unsigned char *inDosPtr;
	static PIHANDLER origProt;
	static unsigned long origReal;
	static REGS_BINT *borlandInterruptRegsPtr;
	static REGS_BINT *borlandExceptionRegsPtr;
	static unsigned char *quickRefProcessArray;
	static unsigned char *quickProcessTypeArray;
	static unsigned long *accumulatorTable;
	static unsigned char magicNumberEnd;
	static unsigned char magicNumberTerminator;
};

void initKernel(void);
void uninitKernel(void);

#endif
