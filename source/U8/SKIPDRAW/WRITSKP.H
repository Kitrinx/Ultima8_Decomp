#ifndef WRITSKP_H
#define WRITSKP_H

#include "IWRITSKP.H"

class Vport;

// Bit 0 of the flags word in the table header: runs may be RLE coded.
#define SKIP_RLE	1

// A skip table that can share identical rows and RLE code its runs.
class SkipMaker : public FastSkipMaker
{
public:
	short *firstRow;
	unsigned char *blankRow;

	SkipMaker(unsigned char *, short, short, short, short, unsigned short);
	int skip_to(short, short);
	unsigned char segment_can_rle(short, unsigned char *);
	int add_non_rle_run(short, unsigned char *);
	unsigned measure_non_rle_run(short);
	int add_rle_run(short, unsigned char *);
	unsigned measure_rle_run(short, unsigned char *);
	void optimize(void);
};

unsigned EstimateWriteSkip(Vport &, short, short, short, short, short, unsigned short);
int WriteSkip(Vport &, short, short, short, short, short, short, unsigned char *, short, unsigned short);

#endif
