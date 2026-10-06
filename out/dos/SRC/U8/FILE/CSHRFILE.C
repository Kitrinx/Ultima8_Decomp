// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: FILE\CSHRFILE.C

#include <dos.h>
#include "CFILE.H"
#include "CSHRFILE.H"
#include "FEXIST.H"

// Inline in the shared headers when this file was built.
inline unsigned char BaseFile::is_valid(void) { return handle != -1; }
inline BaseFile::~BaseFile(void) { if (is_valid()) close(); }
inline SharedFile::SharedFile(void) {}

#define LOCK_FAILED	0x1960

SharedFile::SharedFile(const char *name) :
	BaseFile(name, RWShareReadWrite)
{
}

void SharedFile::open(const char *name, OpenMode mode)
{
	BaseFile::open(name, mode);
}

// DOS lock region; TRUE on success.
unsigned char SharedFile::lock(long start, long length)
{
	unsigned startHigh = start >> 16;
	unsigned startLow = start;
	unsigned lengthHigh = length >> 16;
	unsigned lengthLow = length;
	int file = handle;
	unsigned char locked;

	_AH = 0x5c;
	_AL = 0;
	_BX = file;
	_CX = startHigh;
	_DX = startLow;
	_SI = lengthHigh;
	_DI = lengthLow;
	geninterrupt(0x21);
	locked = !(_FLAGS & 1);
	return locked;
}

unsigned char SharedFile::unlock(long start, long length)
{
	unsigned startHigh = start >> 16;
	unsigned startLow = start;
	unsigned lengthHigh = length >> 16;
	unsigned lengthLow = length;
	int file = handle;
	unsigned char unlocked;

	_AH = 0x5c;
	_AL = 1;
	_BX = file;
	_CX = startHigh;
	_DX = startLow;
	_SI = lengthHigh;
	_DI = lengthLow;
	geninterrupt(0x21);
	unlocked = !(_FLAGS & 1);
	return unlocked;
}

long SharedFile::read(char *buffer, long count, long pos)
{
	return BaseFile::read(buffer, count, pos);
}

long SharedFile::write(char *buffer, long count, long pos)
{
	return BaseFile::write(buffer, count, pos);
}

// Waits until no one else has the file, then keeps byte 0 locked.
QuickExclusiveSharedFile::QuickExclusiveSharedFile(const char *name)
{
	if (!FileExists(name))
		forceOpen(name, RWShareReadWrite);
	else
		BaseFile::open(name, RWShareReadWrite);
	SharedFile other(name);
	while (!lock(0, 1))
		;
	while (!other.lock(0, 2))
		;
	close();
	BaseFile::open(name, RWShareReadWrite);
	lock(0, 1);
	other.unlock(0, 2);
}

QuickExclusiveSharedFile::~QuickExclusiveSharedFile(void)
{
	unlock(0, 1);
}

RecordLock::RecordLock(SharedFile *lockFile, long lockLength, long lockOffset, int lockTries)
{
	file = lockFile;
	offset = lockOffset;
	length = lockLength;
	tries = lockTries;
	int i = 0;
	while (!file->lock(offset, length))
	{
		if (tries == i)
			file->fileError(LOCK_FAILED, __FILE__, 315);	// __LINE__
		i++;
	}
}

RecordLock::~RecordLock(void)
{
	int i = 0;
	while (!file->unlock(offset, length))
	{
		if (tries == i)
			file->fileError(LOCK_FAILED, __FILE__, 330);	// __LINE__
		i++;
	}
}
