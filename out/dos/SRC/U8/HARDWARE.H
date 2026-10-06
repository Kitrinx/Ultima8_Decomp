#ifndef HARDWARE_H
#define HARDWARE_H

#include "..\GLIB\PROCESS.H"
#include "PROCMON.H"

enum CPU
{
	CPU_0,
	CPU_8086 = 0,
	CPU_286 = 1,
	CPU_386 = 2
};

// The machine's minimum requirements, checked at startup.
class HardwareChecker
{
public:
	long minDiskSpace;
	long minMemory;
	long minLargestFragment;
	unsigned char pad_0c[2];
	CPU minCpu;

	HardwareChecker(long, long, long, CPU);
	long getHardDriveSpace(int);
	long getTotalMemory(void);
	long getLargestMemoryFragment(void);
	int getCpu(void);
	unsigned char mouseInstalled(void);
	unsigned char verifyAll(unsigned char);
};

#endif
