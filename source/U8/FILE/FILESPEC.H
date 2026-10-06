#ifndef FILESPEC_H
#define FILESPEC_H

#include <dir.h>

char *fileSpec(char *, char *, char *, char *);

// A path split into its parts, and a search through the files it matches.
class FileSpec
{
public:
	char foundName[MAXFILE];
	char foundExt[MAXEXT];
	struct ffblk found;
	int attrib;
	int done;
	char drive[MAXDRIVE];
	char dir[MAXDIR];
	char name[MAXFILE];
	char ext[MAXEXT];

	FileSpec(void);
	FileSpec(char *, char *, char *, char *);
	FileSpec(char *, int);
	void become(char *, int);
	void unlinkAll(void);
	int exists(void);
	int getNext(void);
	int isYounger(FileSpec &);
	int isYounger(unsigned long);
	long getDate(void);
	void getFirst(void);
	void clear(void);
	void reset(void);
	char *get(int);
	operator char *(void) { return fileSpec(drive, dir, name, ext); }
};

char *fileSpec(char *, char *, char *, char *);

#endif
