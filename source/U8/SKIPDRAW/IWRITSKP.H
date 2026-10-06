#ifndef IWRITSKP_H
#define IWRITSKP_H

class Vport;

// Builds a skip table: a header, one offset per row (from the entry
// itself), then for each row
// alternating skip and run counts, each followed by the run's pixels.
class FastSkipMaker
{
public:
	unsigned char *start;
	short *rowOffset;
	unsigned char *out;
	unsigned x;
	unsigned y;
	unsigned width;
	unsigned height;

	FastSkipMaker(unsigned char *, short, short, short, short);
	int skip_to(short, short);
	int add_run(short, unsigned char *);
	unsigned measure_run(short);
	int add_skip(short);
	unsigned measure_skip(short);
	unsigned measure_skip_to(short, short);
};

// Walks a rectangle of a Vport, finding runs of pixels that are not the
// skip colour.
class ShapeScanner
{
public:
	int left;
	int top;
	int right;
	int bottom;
	unsigned char *pos;
	int lineSkip;
	int x;
	int y;
	int runLength;
	unsigned char skipColor;

	ShapeScanner(Vport &, short, short, short, short, short);
	unsigned char find_segment(void);
};

#endif
