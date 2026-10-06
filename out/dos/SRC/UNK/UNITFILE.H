#ifndef UNITFILE_H
#define UNITFILE_H

#include "CFILE.H"
#include "INTER.H"
#include "ITEMFIND.H"

enum SearchType
{
	SearchType_0
};

class UnitFile : public BaseFile
{
public:
	char *load(char *);
	char *load(unsigned);
	int getSize(unsigned);
};

// Runs a usecode loop: each item found is written to a local
// in the running process's frame.
class Searcher
{
public:
	Data *data;
	unsigned local;
	ItemFinder *finder;

	~Searcher(void);
	int findFirst(char *, short, short, short, short, SearchType);
	int findNext(void);
};

#endif
