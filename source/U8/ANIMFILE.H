#ifndef ANIMFILE_H
#define ANIMFILE_H

#include "ANIMCACH.H"
#include "CFILE.H"
#include "ITEM.H"

class AnimHeader;
class AnimStream;

// anim.dat: one offset per shape to an AnimHeader, which holds one offset per animation.
class AnimFile : public BaseFile
{
public:
	char animName[81];

	AnimFile(void) {}
	AnimFile(char *);
	void initialize(char *);
	Boolean load(unsigned short, AnimSet, AnimStream *);
	void save(unsigned short, AnimSet, AnimStream *);
	long optimize(void);
	Boolean getAnimHeader(unsigned short, AnimHeader *);
	Boolean setAnimHeader(unsigned short, AnimHeader *);
	void createHeader(unsigned short);
	void clearHeader(unsigned short);
	void setFilename(char *);
};

extern char animationFile[];

#endif
