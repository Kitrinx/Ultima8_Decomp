#ifndef FLEXHAND_H
#define FLEXHAND_H

#include "..\POLIB\INC\FLEX.H"
#include "HANDLER.H"

class FlexShapeHandler : public CacheHandler
{
public:
	unsigned startHandle;
	FlexFile *flexFile;

	FlexShapeHandler(int, CacheNodeType, const char *);
	~FlexShapeHandler(void);
	char *get(unsigned);
};

#endif
