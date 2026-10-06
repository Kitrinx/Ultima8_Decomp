#ifndef FASTFLEX_H
#define FASTFLEX_H

#include "..\POLIB\INC\FLEX.H"
#include "CFILE.H"

// A FlexFile that keeps its whole Index table in memory.
class FastFlex : public FlexFile
{
public:
	Index *indexes;

	FastFlex(void);
	FastFlex(char *, Index *, OpenMode, short);
	virtual ~FastFlex(void);
	virtual void getIndex(short, Index &);
	virtual void setIndex(short, Index &);
	virtual void init(char *, Index *, OpenMode, short);
	void create(char *, short);
	void readInIndexes(void);
};

#endif
