// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: FLEX\STEPFLEX.C

#include <string.h>
#include <stdio.h>
#include <dos.h>
#include <io.h>
#include "CFILE.H"
#include "FEXIST.H"
#include "STEPFLEX.H"

// Inline in the shared headers when this file was built.
inline Index::Index(void) { size = 0; offset = 0; }
inline int Index::isEmpty(void) { return size == 0; }
inline unsigned char BaseFile::is_valid(void) { return handle != -1; }
inline BaseFile::~BaseFile(void) { if (is_valid()) close(); }
inline SharedFile::SharedFile(void) {}
inline FlexFile::FlexFile(void) {}
inline FileSpec::FileSpec(char *path, int attrib) { become(path, attrib); }

// fresh: throw away changes left in an old temporary file.
StepFlexFile::StepFlexFile(char *name, short count, unsigned char fresh) :
	FlexFile(name, ReadWrite, count)
{
	tempSpec.become(name, 0);
	strcpy(tempSpec.ext, ".$$$");
	if (fresh)
		if (FileExists(fileSpec(tempSpec.drive, tempSpec.dir, tempSpec.name, tempSpec.ext)))
			unlink(fileSpec(tempSpec.drive, tempSpec.dir, tempSpec.name, tempSpec.ext));
	flexFile.init(fileSpec(tempSpec.drive, tempSpec.dir, tempSpec.name, tempSpec.ext), ReadWrite, totalIndexes);
	flexFile.flush();
}

StepFlexFile::~StepFlexFile(void)
{
	flexFile.close();
	unlink(fileSpec(tempSpec.drive, tempSpec.dir, tempSpec.name, tempSpec.ext));
}

void StepFlexFile::init(char *name, OpenMode, short count)
{
	FlexFile::init(name, ReadWrite, count);
	initTemp();
}

// TRUE if the record has been changed since the last commit.
int StepFlexFile::tempIsLater(short record)
{
	Index index;

	flexFile.getIndex(record, index);
	return index.isEmpty() ? 0 : 1;
}

// Writes the merged records to a .tmp file, which then replaces this one.
void StepFlexFile::commit(void)
{
	FileSpec spec(filename, 0);
	strcpy(spec.ext, ".tmp");
	char newName[80];
	strcpy(newName, fileSpec(spec.drive, spec.dir, spec.name, spec.ext));
	if (FileExists(newName))
		unlink(newName);
	FlexFile newFile(newName, ReadWrite, totalIndexes);
	for (int i = 0; i < totalIndexes; i++)
		if (tempIsLater(i))
			flexFile.copy(i, &newFile, -1);
		else
			copy(i, &newFile, -1);
	close();
	newFile.close();
	flexFile.close();
	strcpy(spec.ext, ".$$$");
	unlink(fileSpec(spec.drive, spec.dir, spec.name, spec.ext));
	unlink(filename);
	::rename(newName, filename);
	FlexFile::init(filename, ReadWrite, totalIndexes);
	flush();
}

void StepFlexFile::initTemp(void)
{
	FileSpec spec(filename, 0);
	strcpy(spec.ext, ".$$$");
	if (FileExists(fileSpec(spec.drive, spec.dir, spec.name, spec.ext)))
		unlink(fileSpec(spec.drive, spec.dir, spec.name, spec.ext));
	if (flexFile.handle != 6)
		flexFile.close();
	flexFile.init(fileSpec(spec.drive, spec.dir, spec.name, spec.ext), ReadWrite, totalIndexes);
	flexFile.flush();
}

void StepFlexFile::getIndex(short record, Index &index)
{
	if (tempIsLater(record))
		flexFile.getIndex(record, index);
	else
		FlexFile::getIndex(record, index);
}

void StepFlexFile::setIndex(short record, Index &index)
{
	flexFile.setIndex(record, index);
}

void StepFlexFile::readRecord(short record, void *buffer, long length, long offset)
{
	if (tempIsLater(record))
		flexFile.readRecord(record, buffer, length, offset);
	else
		FlexFile::readRecord(record, buffer, length, offset);
}

// A partial write first moves the record's index into the temporary file.
void StepFlexFile::writeRecord(short record, void *buffer, long length, long offset, unsigned char resize)
{
	if (!resize && !tempIsLater(record))
	{
		Index index;
		getIndex(record, index);
		setIndex(record, index);
		flush();
	}
	flexFile.writeRecord(record, buffer, length, offset, resize);
}

void StepFlexFile::relocate(short record, long offset)
{
	if (!tempIsLater(record))
		copy(record, &flexFile, -1);
	flexFile.relocate(record, offset);
	Index index;
	flexFile.getIndex(record, index);
	if (index.size == 0)
	{
		char zero = 0;
		flexFile.writeRecord(record, &zero, 1, 0, 1);
	}
}

void StepFlexFile::changeRecordLen(short record, Index &index, long length)
{
	if (!tempIsLater(record))
		copy(record, &flexFile, -1);
	flexFile.changeRecordLen(record, index, length);
	flexFile.flush();
}

void StepFlexFile::flush(void)
{
	BaseFile::flush();
	flexFile.flush();
}

void StepFlexFile::close(void)
{
	BaseFile::close();
	flexFile.close();
}
