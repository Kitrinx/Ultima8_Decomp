#ifndef SPANKER_H
#define SPANKER_H

#include <phapi.h>

class Descriptor;

// Hooks every interrupt through a jump table so 32-bit registers survive the
// extender's handlers, and samples cs:ip for the profiler.
class Spanky
{
public:
	static void init(void);
	static void initIdtGate(void);
	static void uninit(void);
	static void dosSetPassToProtVec(short, PIHANDLER, PIHANDLER *, REALPTR *);
	static void dosSetRealProtVec(short, PIHANDLER, REALPTR, PIHANDLER *, REALPTR *);
	static void setIdtrToOriginal(void);
	static void setIdtrToIntercept(void);
	static unsigned char isIntact(void);
	static void bufferProfileData(void);
	static long getNextCsIp(void);
	static void setOriginalIdtAddress(unsigned long);
	static void checkProfiler(void);
	static void toggleProfiler(void);
	static unsigned char haveCsIps(void) { return numCsIpPtrs != 0; }
	static unsigned long *csIpPtrArray;
	static unsigned long *csIpPtrRetrievalArray;
	static unsigned short numCsIpPtrs;
	static unsigned short numCsIpRetrievalPtrs;
	static unsigned short currentCsIpRetrievalPtr;
	static unsigned char csIpPtrOverflow;
	static unsigned char csIpPtrRetrievalOverflow;
	static unsigned char profiling;
	static unsigned long *profilePtr;
	static long nextProfileSlot;
	static unsigned char profilerInitialized;
	static unsigned char intercepting;
	static unsigned char initialized;
	static Descriptor *interceptorBuffer;
	static Descriptor *origBuffer;
	static void *intJumpTable;
	static void *patchPointer;
	static unsigned short setRegsCodeSeg;
	static unsigned short setRegsOrigCodeOff;
	static unsigned short setRegsNewCodeOff;
	static unsigned long originalIdtAddress;
	static unsigned long newIdtAddress;
};

void pascal FREESPANKER(unsigned short);
void initSpanky(void);
void uninitSpanky(void);

extern "C" void getIntJumpTable(void);
extern "C" void getPatchPosition(void);
extern "C" void getSetRegsCodeInfo(void);

extern void (*setOrigIdtrCallGate)(void);
extern void (*setNewIdtrCallGate)(void);

#endif
