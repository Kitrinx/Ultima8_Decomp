// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: HARDWARE.C

#include "ERROR.H"
#include "CPROTMEM.H"
#include "HARDWARE.H"

static char *hardwareErrors[5] =
{
	"\n\r\n\rERROR:  You must have at least %ldK bytes of hard drive space.\n\r"
	"ERREUR: Il vous faut au moins %ld octets disponibles sur le disque dur.\n\r"
	"FEHLER: Sie ben\x94tigen mindestens %ldK Bytes Festplattenspeicher.\n\r",
	"\n\r\n\rERROR:  You must have at least 3580K bytes of free memory.\n\r"
	"ERREUR: Il vous faut au moins 3580K d'octets do m\x82moire disponbile.\n\r"
	"FEHLER: Sie ben\x94tigen mindestens 3580K Bytes Speicherkapazit\x84t.\n\r",
	"\n\r\n\rERROR:  The existing memory is too fragmented.\n\r"
	"ERREUR: La m\x82moire existante est trop fragment\x82" "e.\n\r"
	"FEHLER: Die verf\x81gbare Speicherkapazit\x84t ist zu fragmentiert.\n\r",
	"\n\r\n\rERROR:  Pagan will not run on a 8088 or 80286 processor.\n\r"
	"ERREUR: Pagan ne peut \x88tre jou\x82 sur un microprocessor 8088 ou 80286.\n\r"
	"FEHLER: Pagan l\x84\xe1t sich nicht mit 8088 oder 80286 Prozessoren betreiben.\n\r",
	"\n\r\n\rERROR:  You must have a Microsoft(tm) compatible mouse driver installed.\n\r"
	"ERREUR: Il faut vous installer un gestionnaire souris.\n\r"
	"FEHLER: Kein Microsoft(tm) kompatibler Maustreiber gefunden.\n\r"
};

HardwareChecker::HardwareChecker(long diskSpace, long memory, long largestFragment, CPU cpu)
{
	minDiskSpace = diskSpace;
	minMemory = memory;
	minLargestFragment = largestFragment;
	minCpu = cpu;
}

// Free bytes on a drive (0 = current).
long HardwareChecker::getHardDriveSpace(int drive)
{
	unsigned sectorsPerCluster, bytesPerSector, freeClusters;
	long space;

	asm {
		mov	ah, 36h
		mov	dx, drive
		int	21h
		mov	sectorsPerCluster, ax
		mov	freeClusters, bx
		mov	bytesPerSector, cx
	}
	space = (long)sectorsPerCluster * bytesPerSector * freeClusters;
	return space;
}

long HardwareChecker::getTotalMemory(void)
{
	unsigned long total, largest;
	unsigned short selectors;

	ProtMemoryManager::getMemStatus(total, largest, selectors);
	return total;
}

long HardwareChecker::getLargestMemoryFragment(void)
{
	unsigned long total, largest;
	unsigned short selectors;

	ProtMemoryManager::getMemStatus(total, largest, selectors);
	return largest;
}

// An 8086 pushes sp after decrementing it; a 286 cannot set flag bits 12-13.
int HardwareChecker::getCpu(void)
{
	asm {
		push	sp
		pop	ax
		cmp	ax, sp
		jne	is8086
		pushf
		pop	ax
		or	ax, 3000h
		push	ax
		popf
		pushf
		pop	ax
		test	ax, 3000h
		je	is286
		jmp	is386
	}
is8086:
	return CPU_8086;
is286:
	return CPU_286;
is386:
	return CPU_386;
}

unsigned char HardwareChecker::mouseInstalled(void)
{
	asm {
		xor	ax, ax
		int	33h
	}
	return _AX != 0;
}

unsigned char HardwareChecker::verifyAll(unsigned char verbose)
{
	unsigned char ok = 1;

	if (getHardDriveSpace(0) < minDiskSpace)
	{
		if (verbose)
			trace(TRACE_ALWAYS, hardwareErrors[0], minDiskSpace / 1024);
		ok = 0;
	}
	if (getTotalMemory() < minMemory)
	{
		if (verbose)
			trace(TRACE_ALWAYS, hardwareErrors[1]);
		ok = 0;
	}
	if (getLargestMemoryFragment() < minLargestFragment)
	{
		if (verbose)
			trace(TRACE_ALWAYS, hardwareErrors[2]);
		ok = 0;
	}
	if (getCpu() < minCpu)
	{
		if (verbose)
			trace(TRACE_ALWAYS, hardwareErrors[3]);
		ok = 0;
	}
	if (!mouseInstalled())
	{
		if (verbose)
			trace(TRACE_ALWAYS, hardwareErrors[4]);
		ok = 0;
	}
	return ok;
}
