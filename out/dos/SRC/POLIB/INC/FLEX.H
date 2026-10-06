#ifndef FLEX_H
#define FLEX_H

#include "CFILE.H"
#include "CSHRFILE.H"

class FlexHeader;

// Where a record lies in a flex file; an empty record has size 0.
class Index
{
public:
	long offset;
	long size;

	Index(void);
	int operator==(Index &);
	int isEmpty(void);
};

// An archive of numbered records: a FlexHeader, an Index table, then the data.
class FlexFile : public SharedFile
{
public:
	int totalIndexes;

	FlexFile(void);
	FlexFile(char *, OpenMode, short);
	virtual void init(char *, OpenMode, short);
	virtual void getIndex(short, Index &);
	virtual void setIndex(short, Index &);
	virtual void readRecord(short, void *, long = -1, long = 0);
	virtual void writeRecord(short, void *, long, long = 0, unsigned char = 1);
	virtual void setLabel(char *);
	virtual void getLabel(char *);
	virtual void setHeader(FlexHeader *);
	virtual void getHeader(FlexHeader *);
	virtual void relocate(short, long = -1);
	virtual void changeRecordLen(short, Index &, long);
	virtual long getChecksum(void);
	virtual int verifyChecksum(long * = 0);
	void create(char *, short);
	void seekRecord(short);
	void resetTotalIndexes(void);
	void copy(short, FlexFile *, short = -1);
	unsigned char verifyIndexes(void);
};

class FlexHeader
{
public:
	char label[82];
	unsigned char pad_52[2];
	long totalIndexes;
	long version;
	long fileSize;
	long checksum;
	unsigned char pad_64[28];

	FlexHeader(void);
};

// FlexFile::fileError codes
#define FLEX_MISSING	0x2001
#define FLEX_NO_COUNT	0x2002
#define FLEX_BAD_INDEX	0x2003
#define FLEX_BAD_OFFSET	0x2004

#endif
