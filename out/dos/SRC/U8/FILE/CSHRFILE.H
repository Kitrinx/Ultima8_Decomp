#ifndef CSHRFILE_H
#define CSHRFILE_H

#include "CFILE.H"

// A file opened for shared access, with DOS record locking.
class SharedFile : public BaseFile
{
public:
	SharedFile(void);
	SharedFile(const char *);
	void open(const char *, OpenMode);
	unsigned char lock(long, long);
	unsigned char unlock(long, long);
	virtual long read(char *, long, long = -1);
	virtual long write(char *, long, long = -1);
};

// Opens a shared file and holds its first byte locked while it is open.
class QuickExclusiveSharedFile : public SharedFile
{
public:
	QuickExclusiveSharedFile(const char *);
	virtual ~QuickExclusiveSharedFile(void);
};

// Locks a record for its lifetime, failing after a number of tries.
class RecordLock
{
public:
	SharedFile *file;
	long offset;
	long length;
	int tries;

	RecordLock(SharedFile *, long, long, int);
	~RecordLock(void);
};

#endif
