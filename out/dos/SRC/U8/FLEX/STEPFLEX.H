#ifndef STEPFLEX_H
#define STEPFLEX_H

#include "..\POLIB\INC\FLEX.H"
#include "CFILE.H"
#include "FILESPEC.H"

// A FlexFile whose changes go to a temporary flex (same name, extension
// .$$$) until commit() merges them into a new copy of the file.
class StepFlexFile : public FlexFile
{
public:
	FlexFile flexFile;
	FileSpec tempSpec;

	StepFlexFile(char *, short, unsigned char);
	virtual ~StepFlexFile(void);
	virtual void init(char *, OpenMode, short);
	virtual void getIndex(short, Index &);
	virtual void setIndex(short, Index &);
	virtual void readRecord(short, void *, long = -1, long = 0);
	virtual void writeRecord(short, void *, long, long = 0, unsigned char = 1);
	virtual void relocate(short, long = -1);
	virtual void changeRecordLen(short, Index &, long);
	virtual void initTemp(void);
	int tempIsLater(short);
	void commit(void);
	void flush(void);
	void close(void);
};

#endif
